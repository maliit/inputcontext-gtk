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


#include <X11/keysym.h>
#include <gdk/gdkx.h> // For retrieving XID
#include <maliit-glib/maliitbus.h>

#include "client-imcontext-gtk.h"
#include "qt-gtk-translate.h"
#include "debug.h"

static GType _meego_imcontext_type = 0;
static GtkIMContextClass *parent_class = NULL;

static MeegoIMContext *focused_imcontext = NULL;
static GtkWidget *focused_widget = NULL;

gboolean redirect_keys = FALSE;

static void meego_imcontext_finalize(GObject *object);

static void meego_imcontext_class_init(MeegoIMContextClass *klass);
static void meego_imcontext_init(MeegoIMContext *meego_imcontext);

static void meego_imcontext_focus_in(GtkIMContext *context);
static void meego_imcontext_focus_out(GtkIMContext *context);
static gboolean meego_imcontext_filter_key_event(GtkIMContext *context, GdkEventKey *event);
static void meego_imcontext_reset(GtkIMContext *context);
static void meego_imcontext_get_preedit_string(GtkIMContext *context, gchar **str, PangoAttrList **attrs, gint *cursor_pos);
static void meego_imcontext_set_preedit_enabled(GtkIMContext *context, gboolean enabled);
static void meego_imcontext_set_client_window(GtkIMContext *context, GdkWindow *window);
static void meego_imcontext_set_cursor_location(GtkIMContext *context, GdkRectangle *area);
static void meego_imcontext_update_widget_info(MeegoIMContext *imcontext);

static gboolean meego_imcontext_im_initiated_hide(MaliitContext *obj, GDBusMethodInvocation *invocation, gpointer user_data);
static gboolean meego_imcontext_commit_string(MaliitContext *obj, GDBusMethodInvocation *invocation, const gchar *string,
                                              gint replacement_start, gint replacement_length, gint cursor_pos,
                                              gpointer user_data);
static gboolean meego_imcontext_update_preedit(MaliitContext *obj, GDBusMethodInvocation *invocation, const gchar *string,
                                               GVariant *formatListData, gint replaceStart, gint replaceLength, gint cursorPos,
                                               gpointer user_data);
static gboolean meego_imcontext_key_event(MaliitContext *obj, GDBusMethodInvocation *invocation, gint type, gint key,
                                          gint modifiers, const gchar *text, gboolean auto_repeat, gint count,
                                          guchar request_type, gpointer user_data);
static gboolean meego_imcontext_set_redirect_keys(MaliitContext *obj, GDBusMethodInvocation *invocation, gboolean enabled,
                                                  gpointer user_data);
static gboolean meego_imcontext_notify_extended_attribute_changed (MaliitContext *obj, GDBusMethodInvocation *invocation,
                                                                   gint id, const gchar *target, const gchar *target_item,
                                                                   const gchar *attribute, GVariant *variant_value,
                                                                   gpointer user_data);
static gboolean meego_imcontext_update_input_method_area (MaliitContext *obj, GDBusMethodInvocation *invocation,
                                                          gint x, gint y, gint width, gint height, gpointer user_data);
static void meego_imcontext_invoke_action(MaliitServer *obj, const char *action, const char* sequence, gpointer user_data);

static GtkIMContext *meego_imcontext_get_slave_imcontext(void);

static const gchar *const WIDGET_INFO_WIN_ID = "winId";
static const gchar *const WIDGET_INFO_FOCUS_STATE = "focusState";
static const gchar *const WIDGET_INFO_ATTRIBUTE_EXTENSION_ID = "toolbarId";
static const gchar *const WIDGET_INFO_ATTRIBUTE_EXTENSION_FILENAME = "toolbar";
static const gchar *const WIDGET_INFO_SURROUNDING_TEXT = "surroundingText";
static const gchar *const WIDGET_INFO_CURSOR_POSITION = "cursorPosition";


GType meego_imcontext_get_type()
{
    return _meego_imcontext_type;
}


void
meego_imcontext_register_type(GTypeModule *type_module)
{
    static const GTypeInfo meego_imcontext_info = {
        sizeof(MeegoIMContextClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) meego_imcontext_class_init,
        NULL,
        NULL,
        sizeof(MeegoIMContext),
        0,
        (GInstanceInitFunc) meego_imcontext_init,
        NULL
    };

    if (_meego_imcontext_type)
        return;

    if (type_module) {
        _meego_imcontext_type =
            g_type_module_register_type(
                type_module,
                GTK_TYPE_IM_CONTEXT,
                "MeegoIMContext",
                &meego_imcontext_info,
                (GTypeFlags)0);
    } else {
        _meego_imcontext_type =
            g_type_register_static(
                GTK_TYPE_IM_CONTEXT,
                "MeegoIMContext",
                &meego_imcontext_info,
                (GTypeFlags)0);
    }
}



// staff for fallback slave GTK simple imcontext
static void
slave_commit(GtkIMContext *slave, const char *text, gpointer data)
{
    UNUSED(slave);
    UNUSED(data);
    DBG("text = %s", text);
    if (focused_imcontext && text) {
        g_signal_emit_by_name(focused_imcontext, "commit", text);
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
    if (!focused_imcontext || !slave)
        return;

    gtk_im_context_get_preedit_string(slave, &str, &attrs, &cursor_pos);

    if (str != NULL) {
        g_free(focused_imcontext->preedit_str);
        focused_imcontext->preedit_str = str;
    }

    focused_imcontext->preedit_cursor_pos = cursor_pos;

    if (focused_imcontext->preedit_attrs != NULL)
        pango_attr_list_unref(focused_imcontext->preedit_attrs);

    focused_imcontext->preedit_attrs = attrs;

    g_signal_emit_by_name(focused_imcontext, "preedit-changed");
}


static GtkIMContext *
meego_imcontext_get_slave_imcontext(void)
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
meego_imcontext_new(void)
{
    MeegoIMContext *ic = MEEGO_IMCONTEXT(g_object_new(MEEGO_TYPE_IMCONTEXT, NULL));
    return GTK_IM_CONTEXT(ic);
}

static void
meego_imcontext_dispose(GObject *object)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(object);

    g_signal_handlers_disconnect_by_data (imcontext->context, object);
    g_signal_handlers_disconnect_by_data (imcontext->server, object);

    g_clear_object(&imcontext->context);
    g_clear_object(&imcontext->server);

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void
meego_imcontext_finalize(GObject *object)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(object);

    if (imcontext->widget_state)
        g_variant_unref(imcontext->widget_state);

    if (imcontext->client_window)
        g_object_unref(imcontext->client_window);

    if (imcontext->registry)
        g_object_unref(imcontext->registry);

    G_OBJECT_CLASS(parent_class)->finalize(object);
}


static void
meego_imcontext_class_init(MeegoIMContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    parent_class = (GtkIMContextClass *)g_type_class_peek_parent(klass);
    GtkIMContextClass *imclass = GTK_IM_CONTEXT_CLASS(klass);

    gobject_class->dispose = meego_imcontext_dispose;
    gobject_class->finalize = meego_imcontext_finalize;

    imclass->focus_in = meego_imcontext_focus_in;
    imclass->focus_out = meego_imcontext_focus_out;
    imclass->filter_keypress = meego_imcontext_filter_key_event;
    imclass->reset = meego_imcontext_reset;
    imclass->set_client_window = meego_imcontext_set_client_window;
    imclass->get_preedit_string = meego_imcontext_get_preedit_string;
    imclass->set_cursor_location = meego_imcontext_set_cursor_location;
    imclass->set_use_preedit = meego_imcontext_set_preedit_enabled;
}


static void
meego_imcontext_init(MeegoIMContext *self)
{
    GError *error = NULL;

    self->client_window = NULL;

    self->cursor_location.x = -1;
    self->cursor_location.y = -1;
    self->cursor_location.width = 0;
    self->cursor_location.height = 0;

    self->preedit_str = NULL;
    self->preedit_attrs = NULL;
    self->preedit_cursor_pos = 0;

    self->focus_state = FALSE;

    self->server = maliit_get_server_sync(NULL, &error);

    if (!self->server) {
        g_warning("Unable to connect to server: %s", error->message);
        g_clear_error(&error);
    }

    self->context = maliit_get_context_sync(NULL, &error);

    if (!self->context) {
        g_warning("Unable to connect to context: %s", error->message);
        g_clear_error(&error);
    }

    self->registry = maliit_attribute_extension_registry_get_instance();

    g_signal_connect(self->context, "handle-im-initiated-hide",
                     G_CALLBACK(meego_imcontext_im_initiated_hide), self);
    g_signal_connect(self->context, "handle-commit-string",
                     G_CALLBACK(meego_imcontext_commit_string), self);
    g_signal_connect(self->context, "handle-update-preedit",
                     G_CALLBACK(meego_imcontext_update_preedit), self);
    g_signal_connect(self->context, "handle-key-event",
                     G_CALLBACK(meego_imcontext_key_event), self);
    g_signal_connect(self->context, "handle-set-redirect-keys",
                     G_CALLBACK(meego_imcontext_set_redirect_keys), self);
    g_signal_connect(self->context, "handle-notify-extended-attribute-changed",
                     G_CALLBACK(meego_imcontext_notify_extended_attribute_changed), self);
    g_signal_connect(self->context, "handle-update-input-method-area",
                     G_CALLBACK(meego_imcontext_update_input_method_area), self);
    g_signal_connect(self->server, "invoke-action",
                     G_CALLBACK(meego_imcontext_invoke_action), self);
}


static void
meego_imcontext_focus_in(GtkIMContext *context)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(context);
    gboolean focus_changed = TRUE;
    GError *error = NULL;

    DBG("imcontext = %p", imcontext);

    if (focused_imcontext && focused_imcontext != imcontext)
        meego_imcontext_focus_out(GTK_IM_CONTEXT(focused_imcontext));
    focused_imcontext = imcontext;

    imcontext->focus_state = TRUE;
    meego_imcontext_update_widget_info(imcontext);

    if (maliit_server_call_activate_context_sync(imcontext->server,
                                                 NULL,
                                                 &error)) {
        if (maliit_server_call_update_widget_information_sync(imcontext->server,
                                                              imcontext->widget_state,
                                                              focus_changed,
                                                              NULL,
                                                              &error)) {
            if (!maliit_server_call_show_input_method_sync(imcontext->server,
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
meego_imcontext_focus_out(GtkIMContext *context)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(context);
    GError *error = NULL;

    DBG("imcontext = %p", imcontext);

    meego_imcontext_reset(context);

    imcontext->focus_state = FALSE;
    focused_imcontext = NULL;
    focused_widget = NULL;

    meego_imcontext_update_widget_info(imcontext);

    if (maliit_server_call_update_widget_information_sync(imcontext->server,
                                                          imcontext->widget_state,
                                                          TRUE,
                                                          NULL,
                                                          &error)) {
        if (!maliit_server_call_hide_input_method_sync(imcontext->server,
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
meego_imcontext_filter_key_event(GtkIMContext *context, GdkEventKey *event)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(context);
    int qevent_type = 0, qt_keycode = 0, qt_modifier = 0;
    gchar *text = "";
    GError *error = NULL;

    focused_widget = gtk_get_event_widget((GdkEvent *)event);

    DBG("event type=0x%x, state=0x%x, keyval=0x%x, keycode=0x%x, group=%d",
        event->type, event->state, event->keyval, event->hardware_keycode, event->group);

    if (focused_imcontext != imcontext)
        meego_imcontext_focus_in(context);

    if ((event->state & IM_FORWARD_MASK) || !redirect_keys) {
        GtkIMContext *slave = meego_imcontext_get_slave_imcontext();
        return gtk_im_context_filter_keypress(slave, event);
    }

    if (!gdk_key_event_to_qt(event, &qevent_type, &qt_keycode, &qt_modifier))
        return FALSE;

    if (!maliit_server_call_process_key_event_sync(imcontext->server,
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
meego_imcontext_reset(GtkIMContext *context)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(context);
    GError *error = NULL;

    DBG("imcontext = %p", imcontext);

    if (imcontext != focused_imcontext) {
        return;
    }

    /* Commit preedit if it is not empty */
    if (focused_imcontext && focused_imcontext->preedit_str && focused_imcontext->preedit_str[0]) {
        char *commit_string = focused_imcontext->preedit_str;
        focused_imcontext->preedit_str = g_strdup("");
        focused_imcontext->preedit_cursor_pos = 0;
        g_signal_emit_by_name(focused_imcontext, "preedit-changed");
        g_signal_emit_by_name(focused_imcontext, "commit", commit_string);
        g_free(commit_string);
    }

    if (!maliit_server_call_reset_sync(imcontext->server, NULL, &error)) {
        g_warning("Unable to reset: %s", error->message);
        g_clear_error(&error);
    }
}


static void
meego_imcontext_get_preedit_string(GtkIMContext *context, gchar **str, PangoAttrList **attrs, gint *cursor_pos)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(context);

    DBG("imcontext = %p", imcontext);

    if (str) {
        if (imcontext->preedit_str)
            *str = g_strdup(imcontext->preedit_str);
        else
            *str = g_strdup("");
    }

    if (attrs) {
        if (imcontext->preedit_attrs) {
            *attrs = imcontext->preedit_attrs;
            pango_attr_list_ref(imcontext->preedit_attrs);
        } else {
            *attrs = pango_attr_list_new();
        }
    }

    if (cursor_pos)
        *cursor_pos = imcontext->preedit_cursor_pos;
}


static void
meego_imcontext_set_preedit_enabled(GtkIMContext *context, gboolean enabled)
{
    UNUSED(context);
    UNUSED(enabled);
    // TODO: Seems QT/MEEGO don't need it, it will always showing preedit.
    return;
}


static void
meego_imcontext_set_client_window(GtkIMContext *context, GdkWindow *window)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(context);
    STEP();

    if (imcontext->client_window)
        g_object_unref(imcontext->client_window);

    if (window)
        g_object_ref(window);

    imcontext->client_window = window;

    // TODO: might need to update cursor position or other staff later using this info?
}


static void
meego_imcontext_set_cursor_location(GtkIMContext *context, GdkRectangle *area)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(context);
    //DBG("imcontext = %p, x=%d, y=%d, w=%d, h=%d", imcontext,
    //  area->x, area->y, area->width, area->height);

    imcontext->cursor_location = *area;

    // TODO: call updateWidgetInformation?
    //The cursor location from GTK widget is simillar to ImMicroFocus info of a QWidget
    //Thus we might need to update Qt::ImMicroFocus info according to this.
    //But MEEGO IM seems not using this info at all
}

/* Update the widget_state map with current information about the widget. */
void
meego_imcontext_update_widget_info(MeegoIMContext *imcontext)
{
    GVariantDict dict;

    /* Clear table */
    g_variant_dict_init(&dict, NULL);

    /* Focus state */
    g_variant_dict_insert(&dict, WIDGET_INFO_FOCUS_STATE, "b", imcontext->focus_state);

    if (imcontext->focus_state) {
        /* Window ID */
        if (imcontext->client_window) {
            guint64 xid = GDK_WINDOW_XID(imcontext->client_window);
            g_variant_dict_insert(&dict, WIDGET_INFO_WIN_ID, "t", xid);
        }

        /* Attribute extensions */
        if (imcontext->client_window) {
            gpointer user_data = NULL;
            GtkWidget* widget = NULL;
            MaliitAttributeExtension *extension;

            gdk_window_get_user_data (imcontext->client_window, &user_data);

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
        GtkIMContext *context = GTK_IM_CONTEXT(imcontext);
        gchar *surrounding_text;
        gint cursor_index;
        if (gtk_im_context_get_surrounding(context, &surrounding_text, &cursor_index))
        {
            g_variant_dict_insert(&dict, WIDGET_INFO_SURROUNDING_TEXT, "s", surrounding_text);
            g_variant_dict_insert(&dict, WIDGET_INFO_CURSOR_POSITION, "i", cursor_index);
        }
    }

    imcontext->widget_state = g_variant_ref_sink(g_variant_dict_end(&dict));
}

// Call back functions for dbus obj
gboolean
meego_imcontext_im_initiated_hide(MaliitContext *obj,
                                  GDBusMethodInvocation *invocation,
                                  gpointer user_data)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(user_data);
    if (imcontext != focused_imcontext)
        return FALSE;

    if (focused_imcontext && focused_imcontext->client_window) {
        gpointer user_data = NULL;
        GtkWidget* parent_widget = NULL;

        gdk_window_get_user_data (focused_imcontext->client_window, &user_data);

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
meego_imcontext_commit_string(MaliitContext *obj,
                              GDBusMethodInvocation *invocation,
                              const gchar *string,
                              int replacement_start G_GNUC_UNUSED,
                              int replacement_length G_GNUC_UNUSED,
                              int cursor_pos G_GNUC_UNUSED,
                              gpointer user_data)
{
    DBG("string is:%s", string);

    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(user_data);
    if (imcontext != focused_imcontext)
        return FALSE;

    if (focused_imcontext) {
        g_free(focused_imcontext->preedit_str);
        focused_imcontext->preedit_str = g_strdup("");
        focused_imcontext->preedit_cursor_pos = 0;
        g_signal_emit_by_name(focused_imcontext, "preedit-changed");
        g_signal_emit_by_name(focused_imcontext, "commit", string);
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
meego_imcontext_update_preedit(MaliitContext *obj,
                               GDBusMethodInvocation *invocation,
                               const gchar *string,
                               GVariant *formatListData,
                               gint replaceStart G_GNUC_UNUSED,
                               gint replaceLength G_GNUC_UNUSED,
                               gint cursorPos,
                               gpointer user_data)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(user_data);
    if (imcontext != focused_imcontext)
        return FALSE;

    DBG("imcontext = %p string = %s cursorPos = %d", imcontext, string, cursorPos);

    if (focused_imcontext) {
        guint iter;
        PangoAttrList* attrs;

        g_free(focused_imcontext->preedit_str);
        focused_imcontext->preedit_str = g_strdup(string);
        /* If cursorPos is -1 explicitly set it to the end of the preedit */
        if (cursorPos == -1) {
            cursorPos = g_utf8_strlen(string, -1);
        }
        focused_imcontext->preedit_cursor_pos = cursorPos;

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

        if (focused_imcontext->preedit_attrs) {
            pango_attr_list_unref (focused_imcontext->preedit_attrs);
        }
        focused_imcontext->preedit_attrs = attrs;

        g_signal_emit_by_name(focused_imcontext, "preedit-changed");

        maliit_context_complete_update_preedit(obj, invocation);
        return TRUE;
    }

    return FALSE;
}

gboolean
meego_imcontext_key_event(MaliitContext *obj,
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
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(user_data);
    if (imcontext != focused_imcontext)
        return FALSE;

    if (focused_imcontext)
        window = focused_imcontext->client_window;

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
meego_imcontext_invoke_action(MaliitServer *obj G_GNUC_UNUSED,
                              const char *action,
                              const char *sequence G_GNUC_UNUSED,
                              gpointer user_data)
{
    GtkWidget* widget = NULL;
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(user_data);

    if (imcontext != focused_imcontext)
        return;

    gdk_window_get_user_data (imcontext->client_window, &user_data);
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
meego_imcontext_set_redirect_keys(MaliitContext *obj,
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
meego_imcontext_notify_extended_attribute_changed (MaliitContext *obj,
                                                   GDBusMethodInvocation *invocation,
                                                   gint id,
                                                   const gchar *target,
                                                   const gchar *target_item,
                                                   const gchar *attribute,
                                                   GVariant *variant_value,
                                                   gpointer user_data)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(user_data);
    if (imcontext != focused_imcontext)
        return FALSE;

    maliit_attribute_extension_registry_update_attribute (focused_imcontext->registry,
                                                          id,
                                                          target,
                                                          target_item,
                                                          attribute,
                                                          variant_value);

    maliit_context_complete_notify_extended_attribute_changed(obj, invocation);
    return TRUE;
}

gboolean
meego_imcontext_update_input_method_area (MaliitContext *obj,
                                          GDBusMethodInvocation *invocation,
                                          gint x,
                                          gint y,
                                          gint width,
                                          gint height,
                                          gpointer user_data)
{
    MeegoIMContext *imcontext = MEEGO_IMCONTEXT(user_data);
    GdkRectangle cursor_rect, osk_rect = { x, y, width, height };
    guint clear_area_id;

    if (!imcontext->client_window)
      return FALSE;

    if (imcontext->keyboard_area.x == x &&
        imcontext->keyboard_area.y == y &&
        imcontext->keyboard_area.width == width &&
        imcontext->keyboard_area.height == height)
      return FALSE;

    clear_area_id = g_signal_lookup ("clear-area", GTK_TYPE_IM_CONTEXT);

    if (clear_area_id == 0)
      return FALSE;

    imcontext->keyboard_area = osk_rect;

    gdk_window_get_root_coords (imcontext->client_window,
                                imcontext->cursor_location.x,
                                imcontext->cursor_location.y,
                                &cursor_rect.x, &cursor_rect.y);
    cursor_rect.width = imcontext->cursor_location.width;
    cursor_rect.height = imcontext->cursor_location.height;

    g_signal_emit (imcontext, clear_area_id, 0, &osk_rect, &cursor_rect);

    maliit_context_complete_update_input_method_area(obj, invocation);
    return TRUE;
}
