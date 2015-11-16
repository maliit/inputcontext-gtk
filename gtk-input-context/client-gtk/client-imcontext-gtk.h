/*
 * Copyright (C) 2010, Intel Corporation.
 *
 * Author: Raymond Liu <raymond.liu@intel.com>
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

#ifndef _CLIENT_IMCONTEXT_GTK_H
#define _CLIENT_IMCONTEXT_GTK_H

#include <gtk/gtk.h>
#include <maliit-glib/maliitserver.h>
#include <maliit-glib/maliitcontext.h>
#include <maliit-glib/maliitattributeextensionregistry.h>

G_BEGIN_DECLS

// Be careful not to override the existing flag of GDK
// Currently bit 15-25 unused, so we pick a middle one.
typedef enum {
    IM_FORWARD_MASK = 1 << 20
} IMModifierType;

typedef struct _MaliitIMContext MaliitIMContext;
typedef struct _MaliitIMContextClass MaliitIMContextClass;

#define MALIIT_TYPE_IM_CONTEXT            (maliit_im_context_get_type())
#define MALIIT_IM_CONTEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), MALIIT_TYPE_IM_CONTEXT, MaliitIMContext))
#define MALIIT_IM_CONTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), MALIIT_TYPE_IM_CONTEXT, MaliitIMContextClass))
#define MALIIT_IS_IM_CONTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), MALIIT_TYPE_IM_CONTEXT))
#define MALIIT_IS_IM_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MALIIT_TYPE_IM_CONTEXT))
#define MALIIT_IM_CONTEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), MALIIT_TYPE_IM_CONTEXT, MaliitIMContextClass))


struct _MaliitIMContext {
    GtkIMContext parent;

    MaliitServer *server;
    MaliitContext *context;
    MaliitAttributeExtensionRegistry *registry;

    GdkWindow *client_window;
    GdkRectangle cursor_location;

    gchar *preedit_str;
    PangoAttrList *preedit_attrs;
    gint preedit_cursor_pos;
    GVariant *widget_state; /* Mapping between string and GVariants with properties of the focused widget */
    gboolean focus_state; /* TRUE means a widget is focused, FALSE means no widget is focused */

    GdkRectangle keyboard_area;
};

struct _MaliitIMContextClass {
    GtkIMContextClass parent;
};

GType maliit_im_context_get_type(void);

void maliit_im_context_register_type(GTypeModule *type_module);
GtkIMContext *maliit_im_context_new(void);

G_END_DECLS

#endif //_CLIENT_IMCONTEXT_GTK_H
