/* chirurgien-view-tab.c
 *
 * Copyright (C) 2021 - Daniel Léonard Schardijn
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "chirurgien-view-tab.h"

#include <glib/gi18n.h>


struct _ChirurgienViewTab
{
    GtkWidget      parent_instance;

    GtkImage      *locked;
    GtkLabel      *unsaved;
    GtkLabel      *label;
    GtkButton     *close_button;

    PangoAttrList *attribute_list;
};

G_DEFINE_TYPE (ChirurgienViewTab, chirurgien_view_tab, GTK_TYPE_WIDGET)

static void
chirurgien_view_tab_dispose (GObject *object)
{
    ChirurgienViewTab *view_tab;

    view_tab = CHIRURGIEN_VIEW_TAB (object);

    pango_attr_list_unref (g_steal_pointer (&view_tab->attribute_list));

    G_OBJECT_CLASS (chirurgien_view_tab_parent_class)->dispose (object);
}

static void
chirurgien_view_tab_class_init (ChirurgienViewTabClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    G_OBJECT_CLASS (klass)->dispose = chirurgien_view_tab_dispose;

    gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);

    gtk_widget_class_set_template_from_resource (widget_class,
                                         "/io/github/leonardschardijn/chirurgien/ui/chirurgien-view-tab.ui");
    gtk_widget_class_bind_template_child (widget_class, ChirurgienViewTab, locked);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienViewTab, unsaved);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienViewTab, label);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienViewTab, close_button);
}

static void
chirurgien_view_tab_init (ChirurgienViewTab *view_tab)
{
    PangoAttribute *modified_attribute;

    gtk_widget_init_template (GTK_WIDGET (view_tab));

    view_tab->attribute_list = pango_attr_list_new ();

    modified_attribute = pango_attr_foreground_new (65535, 0, 0);
    modified_attribute->start_index = PANGO_ATTR_INDEX_FROM_TEXT_BEGINNING;
    modified_attribute->end_index = PANGO_ATTR_INDEX_TO_TEXT_END;

    pango_attr_list_insert (view_tab->attribute_list, modified_attribute);
}

static void
close_response (GtkDialog *self,
                gint       response_id,
                gpointer   user_data)
{
    GtkNotebook *files_notebook;

    gtk_window_destroy (GTK_WINDOW (self));

    if (response_id == GTK_RESPONSE_YES)
    {
        files_notebook = GTK_NOTEBOOK (gtk_widget_get_ancestor (user_data, GTK_TYPE_NOTEBOOK));
        gtk_notebook_remove_page (files_notebook, gtk_notebook_page_num (files_notebook, user_data));
    }
}

static void
close_tab (G_GNUC_UNUSED GtkButton *button,
           gpointer user_data)
{
    GtkWidget *dialog, *close_button;
    GtkNotebook *files_notebook;

    if (chirurgien_view_unsaved (user_data))
    {
        dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_ancestor (user_data, GTK_TYPE_WINDOW)),
                                         GTK_DIALOG_MODAL,
                                         GTK_MESSAGE_WARNING,
                                         GTK_BUTTONS_NONE,
                                         _("Unsaved modifications"));

        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                          _("Applied modifications will be lost"));

        close_button = gtk_dialog_add_button  (GTK_DIALOG (dialog),
                                             _("Close without saving"),
                                               GTK_RESPONSE_YES);

        gtk_widget_add_css_class (close_button, "destructive-action");

        gtk_dialog_add_button  (GTK_DIALOG (dialog),
                              _("Cancel"),
                                GTK_RESPONSE_CANCEL);

        g_signal_connect (dialog, "response", G_CALLBACK (close_response), user_data);
        gtk_window_present (GTK_WINDOW (dialog));
    }
    else
    {
        files_notebook = GTK_NOTEBOOK (gtk_widget_get_ancestor (user_data, GTK_TYPE_NOTEBOOK));
        gtk_notebook_remove_page (files_notebook, gtk_notebook_page_num (files_notebook, user_data));
    }
}

/*** Public API ***/

GtkWidget *
chirurgien_view_tab_new ()
{
    return g_object_new (CHIRURGIEN_TYPE_VIEW_TAB, NULL);
}

void
chirurgien_view_tab_set_view (ChirurgienViewTab *view_tab,
                              ChirurgienView    *view)
{
    g_signal_connect (view_tab->close_button, "clicked", G_CALLBACK (close_tab), view);
}

void
chirurgien_view_tab_set_label (ChirurgienViewTab *view_tab,
                               const gchar       *label,
                               const gchar       *tooltip)
{
    if (!g_utf8_validate (label, -1, NULL))
        label = _("[INVALID ENCODING]");
    if (!g_utf8_validate (tooltip, -1, NULL))
        label = _("[INVALID ENCODING]");

    gtk_label_set_text (view_tab->label, label);

    gtk_widget_set_tooltip_text (GTK_WIDGET (view_tab->label), tooltip);
}

void
chirurgien_view_tab_set_locked (ChirurgienViewTab *view_tab,
                                gboolean           locked)
{
    if (locked)
        gtk_widget_show (GTK_WIDGET (view_tab->locked));
    else
        gtk_widget_hide (GTK_WIDGET (view_tab->locked));
}

void
chirurgien_view_tab_set_modified (ChirurgienViewTab *view_tab,
                                  gboolean           modified)
{
    if (modified)
    {
        gtk_label_set_attributes (view_tab->unsaved, view_tab->attribute_list);
        gtk_label_set_attributes (view_tab->label, view_tab->attribute_list);
    }
    else
    {
        gtk_label_set_attributes (view_tab->unsaved, NULL);
        gtk_label_set_attributes (view_tab->label, NULL);
    }
}

void
chirurgien_view_tab_set_unsaved (ChirurgienViewTab *view_tab,
                                 gboolean           unsaved)
{
    if (unsaved)
        gtk_widget_show (GTK_WIDGET (view_tab->unsaved));
    else
        gtk_widget_hide (GTK_WIDGET (view_tab->unsaved));
}

void
chirurgien_view_tab_close (ChirurgienViewTab *view_tab)
{
    g_signal_emit_by_name (view_tab->close_button, "clicked", NULL);
}
