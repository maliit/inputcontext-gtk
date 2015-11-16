/*
 * Copyright (C) 2010, Intel Corporation.
 * Copyright (C) 2012 One Laptop per Child Association
 * Copyright (C) 2012 Canonical Ltd
 *
 * Author: Raymond Liu <raymond.liu@intel.com>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */


#include <gdk/gdk.h>
#include <maliit-glib/maliitbus.h>

#ifdef HAVE_X11
#include <gdk/gdkx.h> // For retrieving XID
#endif /* HAVE_X11 */

#include "client-imcontext-gtk.h"
#include "qt-gtk-translate.h"
#include "debug.h"

static GType _maliit_im_context_type = 0;
static GtkIMContextClass *parent_class = NULL;

static MaliitIMContext *focused_im_context = NULL;
static GtkWidget *focused_widget = NULL;

gboolean redirect_keys = FALSE;

static void maliit_im_context_finalize(GObject *object);

static void maliit_im_context_class_init(MaliitIMContextClass *klass);
static void maliit_im_context_init(MaliitIMContext *maliit_im_context);

static void maliit_im_context_focus_in(GtkIMContext *context);
static void maliit_im_context_focus_out(GtkIMContext *context);
static gboolean maliit_im_context_filter_key_event(GtkIMContext *context, GdkEventKey *event);
static void maliit_im_context_reset(GtkIMContext *context);
static void maliit_im_context_get_preedit_string(GtkIMContext *context, gchar **str, PangoAttrList **attrs, gint *cursor_pos);
static void maliit_im_context_set_preedit_enabled(GtkIMContext *context, gboolean enabled);
static void maliit_im_context_set_client_window(GtkIMContext *context, GdkWindow *window);
static void maliit_im_context_set_cursor_location(GtkIMContext *context, GdkRectangle *area);
static void maliit_im_context_update_widget_info(MaliitIMContext *im_context);

static gboolean maliit_im_context_im_initiated_hide(MaliitContext *obj, GDBusMethodInvocation *invocation, gpointer user_data);
static gboolean maliit_im_context_commit_string(MaliitContext *obj, GDBusMethodInvocation *invocation, const gchar *string,
                                              gint replacement_start, gint replacement_length, gint cursor_pos,
                                              gpointer user_data);
static gboolean maliit_im_context_update_preedit(MaliitContext *obj, GDBusMethodInvocation *invocation, const gchar *string,
                                               GVariant *formatListData, gint replaceStart, gint replaceLength, gint cursorPos,
                                               gpointer user_data);
static gboolean maliit_im_context_key_event(MaliitContext *obj, GDBusMethodInvocation *invocation, gint type, gint key,
                                          gint modifiers, const gchar *text, gboolean auto_repeat, gint count,
                                          guchar request_type, gpointer user_data);
static gboolean maliit_im_context_set_redirect_keys(MaliitContext *obj, GDBusMethodInvocation *invocation, gboolean enabled,
                                                  gpointer user_data);
static gboolean maliit_im_context_notify_extended_attribute_changed (MaliitContext *obj, GDBusMethodInvocation *invocation,
                                                                   gint id, const gchar *target, const gchar *target_item,
                                                                   const gchar *attribute, GVariant *variant_value,
                                                                   gpointer user_data);
static gboolean maliit_im_context_update_input_method_area (MaliitContext *obj, GDBusMethodInvocation *invocation,
                                                          gint x, gint y, gint width, gint height, gpointer user_data);
static void maliit_im_context_invoke_action(MaliitServer *obj, const char *action, const char* sequence, gpointer user_data);

static GtkIMContext *maliit_im_context_get_slave_imcontext(void);

#ifdef HAVE_X11
static const gchar *const WIDGET_INFO_WIN_ID = "winId";
#endif /* HAVE_X11 */
static const gchar *const WIDGET_INFO_FOCUS_STATE = "focusState";
static const gchar *const WIDGET_INFO_ATTRIBUTE_EXTENSION_ID = "toolbarId";
static const gchar *const WIDGET_INFO_ATTRIBUTE_EXTENSION_FILENAME = "toolbar";
static const gchar *const WIDGET_INFO_SURROUNDING_TEXT = "surroundingText";
static const gchar *const WIDGET_INFO_CURSOR_POSITION = "cursorPosition";


GType maliit_im_context_get_type()
{
    return _maliit_im_context_type;
}


void
maliit_im_context_register_type(GTypeModule *type_module)
{
    static const GTypeInfo maliit_im_context_info = {
        sizeof(MaliitIMContextClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) maliit_im_context_class_init,
        NULL,
        NULL,
        sizeof(MaliitIMContext),
        0,
        (GInstanceInitFunc) maliit_im_context_init,
        NULL
    };

    if (_maliit_im_context_type)
        return;

    if (type_module) {
        _maliit_im_context_type =
            g_type_module_register_type(
                type_module,
                GTK_TYPE_IM_CONTEXT,
                "MaliitIMContext",
                &maliit_im_context_info,
                (GTypeFlags)0);
    } else {
        _maliit_im_context_type =
            g_type_register_static(
                GTK_TYPE_IM_CONTEXT,
                "MaliitIMContext",
                &maliit_im_context_info,
                (GTypeFlags)0);
    }
}



// staff for fallback slave GTK simple im_context
static void
slave_commit(GtkIMContext *slave, const char *text, gpointer data)
{
    UNUSED(slave);
    UNUSED(data);
    DBG("text = %s", text);
    if (focused_im_context && text) {
        g_signal_emit_by_name(focused_im_context, "commit", text);
    }
}


static void
slave_preedit_changed(GtkIMContext *slave, gpointer data)
{
    UNUSED(data);
    gchar *str = NULL;
    gint cursor_pos = 0;
    PangoAttrList *attrs = NULL;

    STEP();
    if (!focused_im_context || !slave)
        return;

    gtk_im_context_get_preedit_string(slave, &str, &attrs, &cursor_pos);

    if (str != NULL) {
        g_free(focused_im_context->preedit_str);
        focused_im_context->preedit_str = str;
    }

    focused_im_context->preedit_cursor_pos = cursor_pos;

    if (focused_im_context->preedit_attrs != NULL)
        pango_attr_list_unref(focused_im_context->preedit_attrs);

    focused_im_context->preedit_attrs = attrs;

    g_signal_emit_by_name(focused_im_context, "preedit-changed");
}


static GtkIMContext *
maliit_im_context_get_slave_imcontext(void)
{
    static GtkIMContext *slave_ic = NULL;

    if (!slave_ic) {
        slave_ic = gtk_im_context_simple_new();
        //g_signal_connect(G_OBJECT(slave_ic), "preedit-start", G_CALLBACK(slave_preedit_start), NULL);
        //g_signal_connect(G_OBJECT(slave_ic), "preedit-end", G_CALLBACK(slave_preedit_end), NULL);
        g_signal_connect(G_OBJECT(slave_ic), "preedit-changed", G_CALLBACK(slave_preedit_changed), NULL);
        g_signal_connect(G_OBJECT(slave_ic), "commit", G_CALLBACK(slave_commit), NULL);
    }

    return slave_ic;
}


GtkIMContext *
maliit_im_context_new(void)
{
    MaliitIMContext *ic = MALIIT_IM_CONTEXT(g_object_new(MALIIT_TYPE_IM_CONTEXT, NULL));
    return GTK_IM_CONTEXT(ic);
}

static void
maliit_im_context_dispose(GObject *object)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(object);

    if (im_context->context)
        g_signal_handlers_disconnect_by_data (im_context->context, object);
    if (im_context->server)
        g_signal_handlers_disconnect_by_data (im_context->server, object);

    g_clear_object(&im_context->context);
    g_clear_object(&im_context->server);

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void
maliit_im_context_finalize(GObject *object)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(object);

    if (im_context->widget_state)
        g_variant_unref(im_context->widget_state);

    if (im_context->client_window)
        g_object_unref(im_context->client_window);

    if (im_context->registry)
        g_object_unref(im_context->registry);

    G_OBJECT_CLASS(parent_class)->finalize(object);
}


static void
maliit_im_context_class_init(MaliitIMContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    parent_class = (GtkIMContextClass *)g_type_class_peek_parent(klass);
    GtkIMContextClass *imclass = GTK_IM_CONTEXT_CLASS(klass);

    gobject_class->dispose = maliit_im_context_dispose;
    gobject_class->finalize = maliit_im_context_finalize;

    imclass->focus_in = maliit_im_context_focus_in;
    imclass->focus_out = maliit_im_context_focus_out;
    imclass->filter_keypress = maliit_im_context_filter_key_event;
    imclass->reset = maliit_im_context_reset;
    imclass->set_client_window = maliit_im_context_set_client_window;
    imclass->get_preedit_string = maliit_im_context_get_preedit_string;
    imclass->set_cursor_location = maliit_im_context_set_cursor_location;
    imclass->set_use_preedit = maliit_im_context_set_preedit_enabled;
}


static MaliitContext *
get_context(MaliitIMContext *context)
{
    GError *error = NULL;

    if (!context->context && maliit_is_running()) {
        context->context = maliit_get_context_sync(NULL, &error);

        if (context->context) {
            g_object_ref(context->context);
            g_signal_connect(context->context, "handle-im-initiated-hide",
                             G_CALLBACK(maliit_im_context_im_initiated_hide), context);
            g_signal_connect(context->context, "handle-commit-string",
                             G_CALLBACK(maliit_im_context_commit_string), context);
            g_signal_connect(context->context, "handle-update-preedit",
                             G_CALLBACK(maliit_im_context_update_preedit), context);
            g_signal_connect(context->context, "handle-key-event",
                             G_CALLBACK(maliit_im_context_key_event), context);
            g_signal_connect(context->context, "handle-set-redirect-keys",
                             G_CALLBACK(maliit_im_context_set_redirect_keys), context);
            g_signal_connect(context->context, "handle-notify-extended-attribute-changed",
                             G_CALLBACK(maliit_im_context_notify_extended_attribute_changed), context);
            g_signal_connect(context->context, "handle-update-input-method-area",
                             G_CALLBACK(maliit_im_context_update_input_method_area), context);
        } else {
            g_warning("Unable to connect to context: %s", error->message);
            g_clear_error(&error);
        }
    }

    return context->context;
}


static MaliitServer *
get_server(MaliitIMContext *context)
{
    GError *error = NULL;

    if (!context->server && maliit_is_running()) {
        get_context(context);

        context->server = maliit_get_server_sync(NULL, &error);

        if (context->server) {
            g_object_ref(context->server);
            g_signal_connect(context->server, "invoke-action", G_CALLBACK(maliit_im_context_invoke_action), context);
        } else {
            g_warning("Unable to connect to server: %s", error->message);
            g_clear_error(&error);
        }
    }

    return context->server;
}


static void
maliit_im_context_init(MaliitIMContext *self)
{
    self->client_window = NULL;

    self->cursor_location.x = -1;
    self->cursor_location.y = -1;
    self->cursor_location.width = 0;
    self->cursor_location.height = 0;

    self->preedit_str = NULL;
    self->preedit_attrs = NULL;
    self->preedit_cursor_pos = 0;

    self->focus_state = FALSE;

    if (maliit_is_running()) {
        get_context(self);

        self->registry = maliit_attribute_extension_registry_get_instance();
    }
}


static void
maliit_im_context_focus_in(GtkIMContext *context)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(context);
    gboolean focus_changed = TRUE;
    GError *error = NULL;

    if (!maliit_is_running())
        return;

    DBG("im_context = %p", im_context);

    if (focused_im_context && focused_im_context != im_context)
        maliit_im_context_focus_out(GTK_IM_CONTEXT(focused_im_context));
    focused_im_context = im_context;

    im_context->focus_state = TRUE;
    maliit_im_context_update_widget_info(im_context);

    if (maliit_server_call_activate_context_sync(get_server(im_context),
                                                 NULL,
                                                 &error)) {
        if (maliit_server_call_update_widget_information_sync(get_server(im_context),
                                                              im_context->widget_state,
                                                              focus_changed,
                                                              NULL,
                                                              &error)) {
            if (!maliit_server_call_show_input_method_sync(get_server(im_context),
                                                           NULL,
                                                           &error)) {
                g_warning("Unable to show input method: %s", error->message);
                g_clear_error(&error);
            }
        } else {
            g_warning("Unable to update widget information: %s", error->message);
            g_clear_error(&error);
        }
    } else {
        g_warning("Unable to activate context: %s", error->message);
        g_clear_error(&error);
    }

    // TODO: anything else than call "activateContext" and "showInputMethod" ?
}


static void
maliit_im_context_focus_out(GtkIMContext *context)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(context);
    GError *error = NULL;

    if (!maliit_is_running())
        return;

    DBG("im_context = %p", im_context);

    maliit_im_context_reset(context);

    im_context->focus_state = FALSE;
    focused_im_context = NULL;
    focused_widget = NULL;

    maliit_im_context_update_widget_info(im_context);

    if (maliit_server_call_update_widget_information_sync(get_server(im_context),
                                                          im_context->widget_state,
                                                          TRUE,
                                                          NULL,
                                                          &error)) {
        if (!maliit_server_call_hide_input_method_sync(get_server(im_context),
                                                       NULL,
                                                       &error)) {
            g_warning("Unable to hide input method: %s", error->message);
            g_clear_error(&error);
        }
    } else {
        g_warning("Unable to update widget information: %s", error->message);
        g_clear_error(&error);
    }

    // TODO: anything else than call "hideInputMethod" ?
}


static gboolean
maliit_im_context_filter_key_event(GtkIMContext *context, GdkEventKey *event)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(context);
    int qevent_type = 0, qt_keycode = 0, qt_modifier = 0;
    gchar *text = "";
    GError *error = NULL;

    if (!maliit_is_running()) {
        gchar string[10];
        gunichar c = gdk_keyval_to_unicode(event->keyval);

#if GTK_MAJOR_VERSION == 2
        GdkModifierType no_text_input_mask = GDK_MOD1_MASK | GDK_CONTROL_MASK;
#elif GTK_MAJOR_VERSION == 3
        GdkDisplay *display = gdk_window_get_display(event->window);
        GdkKeymap *keymap = gdk_keymap_get_for_display(display);
        GdkModifierIntent intent = GDK_MODIFIER_INTENT_NO_TEXT_INPUT;
        GdkModifierType no_text_input_mask = gdk_keymap_get_modifier_mask(keymap, intent);
#endif /* GTK_MAJOR_VERSION */

        if (c && !g_unichar_iscntrl(c) && event->type == GDK_KEY_PRESS && !(event->state & no_text_input_mask)) {
            string[g_unichar_to_utf8(c, string)] = 0;

            g_signal_emit_by_name(im_context, "commit", string);

            return TRUE;
        }

        return FALSE;
    }

    focused_widget = gtk_get_event_widget((GdkEvent *)event);

    DBG("event type=0x%x, state=0x%x, keyval=0x%x, keycode=0x%x, group=%d",
        event->type, event->state, event->keyval, event->hardware_keycode, event->group);

    if (focused_im_context != im_context)
        maliit_im_context_focus_in(context);

    if ((event->state & IM_FORWARD_MASK) || !redirect_keys) {
        GtkIMContext *slave = maliit_im_context_get_slave_imcontext();
        return gtk_im_context_filter_keypress(slave, event);
    }

    if (!gdk_key_event_to_qt(event, &qevent_type, &qt_keycode, &qt_modifier))
        return FALSE;

    if (!maliit_server_call_process_key_event_sync(get_server(im_context),
                                                   qevent_type,
                                                   qt_keycode,
                                                   qt_modifier,
                                                   text,
                                                   0,
                                                   1,
                                                   event->hardware_keycode,
                                                   event->state,
                                                   event->time,
                                                   NULL,
                                                   &error)) {
        g_warning("Unable to process key event: %s", error->message);
        g_clear_error(&error);
    }

    return TRUE;
}


static void
maliit_im_context_reset(GtkIMContext *context)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(context);
    GError *error = NULL;

    if (!maliit_is_running())
        return;

    DBG("im_context = %p", im_context);

    if (im_context != focused_im_context) {
        return;
    }

    /* Commit preedit if it is not empty */
    if (focused_im_context && focused_im_context->preedit_str && focused_im_context->preedit_str[0]) {
        char *commit_string = focused_im_context->preedit_str;
        focused_im_context->preedit_str = g_strdup("");
        focused_im_context->preedit_cursor_pos = 0;
        g_signal_emit_by_name(focused_im_context, "preedit-changed");
        g_signal_emit_by_name(focused_im_context, "commit", commit_string);
        g_free(commit_string);
    }

    if (!maliit_server_call_reset_sync(get_server(im_context), NULL, &error)) {
        g_warning("Unable to reset: %s", error->message);
        g_clear_error(&error);
    }
}


static void
maliit_im_context_get_preedit_string(GtkIMContext *context, gchar **str, PangoAttrList **attrs, gint *cursor_pos)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(context);

    if (!maliit_is_running()) {
        if (str)
            *str = g_strdup("");

        if (attrs)
            *attrs = pango_attr_list_new();

        if (cursor_pos)
            *cursor_pos = 0;

        return;
    }

    DBG("im_context = %p", im_context);

    if (str) {
        if (im_context->preedit_str)
            *str = g_strdup(im_context->preedit_str);
        else
            *str = g_strdup("");
    }

    if (attrs) {
        if (im_context->preedit_attrs) {
            *attrs = im_context->preedit_attrs;
            pango_attr_list_ref(im_context->preedit_attrs);
        } else {
            *attrs = pango_attr_list_new();
        }
    }

    if (cursor_pos)
        *cursor_pos = im_context->preedit_cursor_pos;
}


static void
maliit_im_context_set_preedit_enabled(GtkIMContext *context, gboolean enabled)
{
    UNUSED(context);
    UNUSED(enabled);

    if (!maliit_is_running())
        return;

    // TODO: Seems QT/MEEGO don't need it, it will always showing preedit.
    return;
}


static void
maliit_im_context_set_client_window(GtkIMContext *context, GdkWindow *window)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(context);

    if (!maliit_is_running())
        return;

    STEP();

    if (im_context->client_window)
        g_object_unref(im_context->client_window);

    if (window)
        g_object_ref(window);

    im_context->client_window = window;

    // TODO: might need to update cursor position or other staff later using this info?
}


static void
maliit_im_context_set_cursor_location(GtkIMContext *context, GdkRectangle *area)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(context);
    //DBG("im_context = %p, x=%d, y=%d, w=%d, h=%d", im_context,
    //  area->x, area->y, area->width, area->height);

    if (!maliit_is_running())
        return;

    im_context->cursor_location = *area;

    // TODO: call updateWidgetInformation?
    //The cursor location from GTK widget is simillar to ImMicroFocus info of a QWidget
    //Thus we might need to update Qt::ImMicroFocus info according to this.
    //But MEEGO IM seems not using this info at all
}

/* Update the widget_state map with current information about the widget. */
void
maliit_im_context_update_widget_info(MaliitIMContext *im_context)
{
    GVariantDict dict;

    /* Clear table */
    g_variant_dict_init(&dict, NULL);

    /* Focus state */
    g_variant_dict_insert(&dict, WIDGET_INFO_FOCUS_STATE, "b", im_context->focus_state);

    if (im_context->focus_state) {
        /* Window ID */
#ifdef HAVE_X11
#if GTK_MAJOR_VERSION == 2
        if (im_context->client_window) {
            guint64 xid = GDK_WINDOW_XID(im_context->client_window);
            g_variant_dict_insert(&dict, WIDGET_INFO_WIN_ID, "t", xid);
        }
#elif GTK_MAJOR_VERSION == 3
        if (GDK_IS_X11_WINDOW(im_context->client_window)) {
            guint64 xid = GDK_WINDOW_XID(im_context->client_window);
            g_variant_dict_insert(&dict, WIDGET_INFO_WIN_ID, "t", xid);
        }
#endif /* GTK_MAJOR_VERSION */
#endif /* HAVE_X11 */

        /* Attribute extensions */
        if (im_context->client_window) {
            gpointer user_data = NULL;
            GtkWidget* widget = NULL;
            MaliitAttributeExtension *extension;

            gdk_window_get_user_data (im_context->client_window, &user_data);

            widget = GTK_WIDGET (user_data);

            user_data = g_object_get_qdata (G_OBJECT (widget),
                                            MALIIT_ATTRIBUTE_EXTENSION_DATA_QUARK);

            if (user_data) {
                extension = MALIIT_ATTRIBUTE_EXTENSION (user_data);

                g_variant_dict_insert (&dict, WIDGET_INFO_ATTRIBUTE_EXTENSION_ID,
                                       "i", maliit_attribute_extension_get_id (extension));
                g_variant_dict_insert (&dict, WIDGET_INFO_ATTRIBUTE_EXTENSION_FILENAME,
                                       "s", maliit_attribute_extension_get_filename (extension));
            }
        }

        /* Surrounding text */
        GtkIMContext *context = GTK_IM_CONTEXT(im_context);
        gchar *surrounding_text;
        gint cursor_index;
        if (gtk_im_context_get_surrounding(context, &surrounding_text, &cursor_index))
        {
            g_variant_dict_insert(&dict, WIDGET_INFO_SURROUNDING_TEXT, "s", surrounding_text);
            g_variant_dict_insert(&dict, WIDGET_INFO_CURSOR_POSITION, "i", cursor_index);
        }
    }

    im_context->widget_state = g_variant_ref_sink(g_variant_dict_end(&dict));
}

// Call back functions for dbus obj
gboolean
maliit_im_context_im_initiated_hide(MaliitContext *obj,
                                  GDBusMethodInvocation *invocation,
                                  gpointer user_data)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(user_data);
    if (im_context != focused_im_context)
        return FALSE;

    if (focused_im_context && focused_im_context->client_window) {
        gpointer user_data = NULL;
        GtkWidget* parent_widget = NULL;

        gdk_window_get_user_data (focused_im_context->client_window, &user_data);

        parent_widget = GTK_WIDGET (user_data);

        while (parent_widget && !GTK_IS_WINDOW (parent_widget)) {
            parent_widget = gtk_widget_get_parent (parent_widget);
        }
        if (parent_widget) {
            gtk_window_set_focus (GTK_WINDOW (parent_widget), NULL);
            maliit_context_complete_im_initiated_hide (obj, invocation);
            return TRUE;
        }
    }

    return FALSE;
}

gboolean
maliit_im_context_commit_string(MaliitContext *obj,
                              GDBusMethodInvocation *invocation,
                              const gchar *string,
                              int replacement_start G_GNUC_UNUSED,
                              int replacement_length G_GNUC_UNUSED,
                              int cursor_pos G_GNUC_UNUSED,
                              gpointer user_data)
{
    DBG("string is:%s", string);

    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(user_data);
    if (im_context != focused_im_context)
        return FALSE;

    if (focused_im_context) {
        g_free(focused_im_context->preedit_str);
        focused_im_context->preedit_str = g_strdup("");
        focused_im_context->preedit_cursor_pos = 0;
        g_signal_emit_by_name(focused_im_context, "preedit-changed");
        g_signal_emit_by_name(focused_im_context, "commit", string);
        maliit_context_complete_commit_string(obj, invocation);
        return TRUE;
    }

    return FALSE;
}

typedef enum
{
    MaliitPreeditDefault,
    MaliitPreeditNoCandidates,
    MaliitPreeditKeyPress,
    MaliitPreeditUnconvertible,
    MaliitPreeditActive
} MaliitPreeditFace;

static void
get_byte_range_from_unicode_offsets (const gchar *string,
                                     gint         utf8_start,
                                     gint         utf8_length,
                                     gint        *byte_start,
                                     gint        *byte_end)
{
    gint start;
    gint end;

    /* we provide start index and length in utf8 characters, but pango
     * expects start and end indices in bytes.
     */
    if (g_utf8_validate (string, -1, NULL)) {
        const gchar * const start_pointer = g_utf8_offset_to_pointer (string, utf8_start);
        const gchar * const end_pointer = g_utf8_offset_to_pointer (string, utf8_start + utf8_length);

        /* pointer arithmetics, there you have it. */
        start = start_pointer - string;
        end = end_pointer - string;
    } else {
        start = utf8_start;
        end = utf8_start + utf8_length;
    }

    if (byte_start) {
        *byte_start = start;
    }
    if (byte_end) {
        *byte_end = end;
    }
}

gboolean
maliit_im_context_update_preedit(MaliitContext *obj,
                               GDBusMethodInvocation *invocation,
                               const gchar *string,
                               GVariant *formatListData,
                               gint replaceStart G_GNUC_UNUSED,
                               gint replaceLength G_GNUC_UNUSED,
                               gint cursorPos,
                               gpointer user_data)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(user_data);
    if (im_context != focused_im_context)
        return FALSE;

    DBG("im_context = %p string = %s cursorPos = %d", im_context, string, cursorPos);

    if (focused_im_context) {
        guint iter;
        PangoAttrList* attrs;

        g_free(focused_im_context->preedit_str);
        focused_im_context->preedit_str = g_strdup(string);
        /* If cursorPos is -1 explicitly set it to the end of the preedit */
        if (cursorPos == -1) {
            cursorPos = g_utf8_strlen(string, -1);
        }
        focused_im_context->preedit_cursor_pos = cursorPos;

        /* attributes */
        attrs = pango_attr_list_new();

        for (iter = 0; iter < g_variant_n_children(formatListData); ++iter) {
            gint start;
            gint length;
            MaliitPreeditFace preedit_face;
            gint byte_start;
            gint byte_end;
            PangoAttribute* new_attrs[2] = { NULL, NULL };
            gint attr_iter;

            g_variant_get_child(formatListData, iter, "(iii)", &start, &length, &preedit_face);

            get_byte_range_from_unicode_offsets(string, start, length, &byte_start, &byte_end);

            switch (preedit_face) {
            case MaliitPreeditNoCandidates:
                new_attrs[0] = pango_attr_underline_new (PANGO_UNDERLINE_ERROR);
                new_attrs[1] = pango_attr_underline_color_new (65535, 0, 0);
                break;

            case MaliitPreeditUnconvertible: {
                const gint gray = (2 << 15) - 1; /* halfway from 0 to 65535 */

                new_attrs[0] = pango_attr_foreground_new (gray, gray, gray);
            } break;

            case MaliitPreeditActive:
                new_attrs[0] = pango_attr_foreground_new(39168, 12800, 52224);
                new_attrs[1] = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
                break;

            case MaliitPreeditKeyPress:
            case MaliitPreeditDefault:
                new_attrs[0] = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
                new_attrs[1] = pango_attr_underline_color_new(0, 0, 0);
                break;
            }

            for (attr_iter = 0; attr_iter < 2; ++attr_iter) {
                if (new_attrs[attr_iter]) {
                    new_attrs[attr_iter]->start_index = byte_start;
                    new_attrs[attr_iter]->end_index = byte_end;

                    pango_attr_list_insert(attrs, new_attrs[attr_iter]);
                }
            }
        }

        if (focused_im_context->preedit_attrs) {
            pango_attr_list_unref (focused_im_context->preedit_attrs);
        }
        focused_im_context->preedit_attrs = attrs;

        g_signal_emit_by_name(focused_im_context, "preedit-changed");

        maliit_context_complete_update_preedit(obj, invocation);
        return TRUE;
    }

    return FALSE;
}

gboolean
maliit_im_context_key_event(MaliitContext *obj,
                          GDBusMethodInvocation *invocation,
                          gint type,
                          gint key,
                          gint modifiers,
                          const gchar *text,
                          gboolean auto_repeat G_GNUC_UNUSED,
                          int count G_GNUC_UNUSED,
                          guchar request_type G_GNUC_UNUSED,
                          gpointer user_data)
{
    GdkEventKey *event = NULL;
    GdkWindow *window = NULL;

    STEP();
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(user_data);
    if (im_context != focused_im_context)
        return FALSE;

    if (focused_im_context)
        window = focused_im_context->client_window;

    event = qt_key_event_to_gdk(type, key, modifiers, text, window);
    if (!event)
        return FALSE;

    event->send_event = TRUE;
    event->state |= IM_FORWARD_MASK;

    gdk_event_put((GdkEvent *)event);
    gdk_event_free((GdkEvent *)event);

    maliit_context_complete_key_event(obj, invocation);
    return TRUE;
}

static unsigned int
find_signal(const char *action, const char *alternative, GtkWidget *widget)
{
    unsigned int signal = g_signal_lookup(action, G_OBJECT_TYPE(widget));

    if (signal || alternative == NULL) {
        return signal;
    }

    return g_signal_lookup(alternative, G_OBJECT_TYPE(widget));
}

void
maliit_im_context_invoke_action(MaliitServer *obj G_GNUC_UNUSED,
                              const char *action,
                              const char *sequence G_GNUC_UNUSED,
                              gpointer user_data)
{
    GtkWidget* widget = NULL;
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(user_data);

    if (im_context != focused_im_context)
        return;

    gdk_window_get_user_data (im_context->client_window, &user_data);
    widget = GTK_WIDGET (user_data);

    if (widget) {
        char *alternative = NULL;
        unsigned int signal;

        if (g_strcmp0(action, "copy") == 0 ||
            g_strcmp0(action, "cut") == 0 ||
            g_strcmp0(action, "paste") == 0)
        {
            alternative = g_strdup_printf("%s-clipboard", action);
        }

        signal = find_signal(action, alternative, widget);
        g_free(alternative);

        if (signal) {
            g_signal_emit(widget, signal, 0);
            return;
        }
    }
}

gboolean
maliit_im_context_set_redirect_keys(MaliitContext *obj,
                                  GDBusMethodInvocation *invocation,
                                  gboolean enabled,
                                  gpointer user_data G_GNUC_UNUSED)
{
    DBG("enabled = %d", enabled);
    redirect_keys = enabled;
    maliit_context_complete_set_redirect_keys(obj, invocation);
    return TRUE;
}

gboolean
maliit_im_context_notify_extended_attribute_changed (MaliitContext *obj,
                                                   GDBusMethodInvocation *invocation,
                                                   gint id,
                                                   const gchar *target,
                                                   const gchar *target_item,
                                                   const gchar *attribute,
                                                   GVariant *variant_value,
                                                   gpointer user_data)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(user_data);
    if (im_context != focused_im_context)
        return FALSE;

    maliit_attribute_extension_registry_update_attribute (focused_im_context->registry,
							  id,
							  target,
							  target_item,
							  attribute,
							  variant_value);

    maliit_context_complete_notify_extended_attribute_changed(obj, invocation);
    return TRUE;
}

gboolean
maliit_im_context_update_input_method_area (MaliitContext *obj,
                                          GDBusMethodInvocation *invocation,
                                          gint x,
                                          gint y,
                                          gint width,
                                          gint height,
                                          gpointer user_data)
{
    MaliitIMContext *im_context = MALIIT_IM_CONTEXT(user_data);
    GdkRectangle cursor_rect, osk_rect = { x, y, width, height };
    guint clear_area_id;

    if (!im_context->client_window)
      return FALSE;

    if (im_context->keyboard_area.x == x &&
        im_context->keyboard_area.y == y &&
        im_context->keyboard_area.width == width &&
        im_context->keyboard_area.height == height)
      return FALSE;

    clear_area_id = g_signal_lookup ("clear-area", GTK_TYPE_IM_CONTEXT);

    if (clear_area_id == 0)
      return FALSE;

    im_context->keyboard_area = osk_rect;

    gdk_window_get_root_coords (im_context->client_window,
                                im_context->cursor_location.x,
                                im_context->cursor_location.y,
                                &cursor_rect.x, &cursor_rect.y);
    cursor_rect.width = im_context->cursor_location.width;
    cursor_rect.height = im_context->cursor_location.height;

    g_signal_emit (im_context, clear_area_id, 0, &osk_rect, &cursor_rect);

    maliit_context_complete_update_input_method_area(obj, invocation);
    return TRUE;
}
