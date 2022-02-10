/* chirurgien-formats-dialog.c
 *
 * Copyright (C) 2020 - Daniel Léonard Schardijn
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

#include "chirurgien-formats-dialog.h"

#include <glib/gi18n.h>

#include "chirurgien-actions.h"

#include <chirurgien-types.h>
#include "chirurgien-globals.h"
#include "formats/validator/validator-utils.h"


typedef struct
{
    /* The description's name */
    gchar                *name;
    /* The whole format description panel (GtkGrid) */
    GtkWidget            *description;
    /* Quick access to the colors */
    GtkColorChooser      *format_colors[CHIRURGIEN_TOTAL_COLORS];

} FormatDescription;


struct _ChirurgienFormatsDialog
{
    GtkDialog             parent_instance;

    GtkStack             *formats;

    GtkButton            *new_format;
    GtkScrolledWindow    *message;

    GtkListBox           *system_formats;
    GtkBox               *user_formats_page;
    GtkListBox           *user_formats;
};

G_DEFINE_TYPE (ChirurgienFormatsDialog, chirurgien_formats_dialog, GTK_TYPE_DIALOG)

static gint
sort_colors (gconstpointer a,
             gconstpointer b)
{
    const FormatColor *color_a, *color_b;

    color_a = a;
    color_b = b;

    return color_a->color_index - color_b->color_index;
}

static FormatDescription *
build_format_description (const FormatDefinition *format_definition)
{
    const FormatColor *color;
    FormatDescription *format_description;

    GtkWidget *grid, *widget1, *widget2;
    GtkTextBuffer *buffer;
    GtkTextIter start;

    GHashTable *color_map;

    PangoAttrList *attribute_list;
    PangoAttribute *size, *weight;

    GList *color_iter, *color_list;

    gint grid_row, grid_column, color_types;

    grid_row = grid_column = 0;

    /* Format description grid */
    grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 3);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 30);
    gtk_widget_set_can_focus (grid, FALSE);

    /* Format name */
    widget1 = gtk_label_new (format_definition->format_name);
    gtk_widget_set_margin_bottom (widget1, 5);

    attribute_list = pango_attr_list_new ();

    weight = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
    size = pango_attr_scale_new (PANGO_SCALE_LARGE);

    weight->start_index = size->start_index = PANGO_ATTR_INDEX_FROM_TEXT_BEGINNING;
    weight->end_index = size->end_index = PANGO_ATTR_INDEX_TO_TEXT_END;

    pango_attr_list_insert (attribute_list, weight);
    pango_attr_list_insert (attribute_list, size);
    gtk_label_set_attributes (GTK_LABEL (widget1), attribute_list);

    pango_attr_list_unref (attribute_list);

    gtk_grid_attach (GTK_GRID (grid), widget1, grid_column, grid_row++, 6, 1);

    /* Format color */
    color_list = g_hash_table_get_values (format_definition->colors);
    color_list = g_list_sort (color_list, sort_colors);

    color_map = g_hash_table_new (g_str_hash, g_str_equal);

    color_types = 2;

    format_description = g_slice_new0 (FormatDescription);

    while (color_types--)
    {
        /* Background colors */
        if (color_types)
        {
            widget1 = gtk_label_new (_("Background colors"));
        }
        /* Foreground colors */
        else
        {
            widget1 = gtk_label_new (_("Foreground colors"));
            gtk_widget_set_margin_top (widget1, 5);
        }
        gtk_widget_set_margin_bottom (widget1, 5);
        gtk_grid_attach (GTK_GRID (grid), widget1, grid_column, grid_row++, 6, 1);

        for (color_iter = color_list;
             color_iter;
             color_iter = color_iter->next)
        {
            color = color_iter->data;

            if (color->color_index > 8)
                continue;

            /* Background colors */
            if (color_types)
            {
                if (!color->background)
                    continue;
            }
            /* Foreground colors */
            else
            {
                if (color->background)
                    continue;
            }

            widget2 = gtk_color_button_new ();
            gtk_widget_set_can_target (widget2, FALSE);
            format_description->format_colors[color->color_index] = GTK_COLOR_CHOOSER (widget2);

            widget1 = g_hash_table_lookup (color_map, color->color_name);
            if (!widget1)
            {
                widget1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
                gtk_widget_set_halign (widget1, GTK_ALIGN_END);
                gtk_widget_set_can_target (widget1, FALSE);
                gtk_widget_add_css_class (widget1, "linked");
                g_hash_table_insert (color_map, color->color_name, widget1);

                gtk_box_append (GTK_BOX (widget1), widget2);

                widget2 = gtk_label_new (color->color_name);

                gtk_grid_attach (GTK_GRID (grid), widget1, grid_column, grid_row, 1, 1);
                gtk_grid_attach_next_to (GTK_GRID (grid), widget2, widget1, GTK_POS_RIGHT, 1, 1);
                grid_column += 2;

                if (grid_column == 6)
                {
                    grid_row++;
                    grid_column = 0;
                }
            }
            else
            {
                 gtk_box_append (GTK_BOX (widget1), widget2);
            }
        }

        if (grid_column)
        {
            grid_row++;
            grid_column = 0;
        }
    }

    /* Format details */
    widget1 = gtk_label_new (_("Details"));
    gtk_widget_set_margin_bottom (widget1, 5);

    gtk_grid_attach (GTK_GRID (grid), widget1, grid_column, grid_row++, 6, 1);

    widget1 = gtk_scrolled_window_new ();
    gtk_widget_set_hexpand (widget1, TRUE);
    gtk_widget_set_vexpand (widget1, TRUE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget1),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_AUTOMATIC);
    widget2 = gtk_text_view_new ();
    gtk_widget_set_can_target (widget2, FALSE);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (widget2), 10);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (widget2), 10);
    gtk_text_view_set_top_margin (GTK_TEXT_VIEW (widget2), 7);
    gtk_text_view_set_bottom_margin (GTK_TEXT_VIEW (widget2), 7);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (widget2), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (widget2), FALSE);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (widget2), GTK_WRAP_WORD);

    if (format_definition->details)
    {
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget2));
        gtk_text_buffer_get_start_iter (buffer, &start);
        gtk_text_buffer_insert_markup (buffer, &start,
                                       format_definition->details, -1);
    }

    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (widget1), widget2);
    gtk_grid_attach (GTK_GRID (grid), widget1, grid_column, grid_row, 6, 1);

    format_description->description = g_object_ref_sink (grid);

    g_hash_table_destroy (color_map);
    g_list_free (color_list);

    return format_description;
}

static gboolean
enable_disable_format (G_GNUC_UNUSED GtkSwitch* self,
                       G_GNUC_UNUSED gboolean   state,
                       gpointer user_data)
{
    FormatDefinition *format_definition;

    format_definition = user_data;
    format_definition->disabled = !format_definition->disabled;

    return FALSE;
}

static void
add_system_format_description (ChirurgienFormatsDialog *dialog,
                               FormatDefinition        *format_definition,
                               const FormatDescription *format_description)
{
    GtkWidget *box, *widget;

    for (gint i = 0; i < CHIRURGIEN_TOTAL_COLORS; i++)
    {
        if (format_description->format_colors[i])
            gtk_color_chooser_set_rgba (format_description->format_colors[i],
                                        &chirurgien_colors[i]);
    }

    gtk_stack_add_titled (dialog->formats,
                          format_description->description,
                          NULL,
                          format_definition->short_format_name);

    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 150);
    gtk_widget_set_tooltip_text (box, _("Enable/disable format"));

    widget = gtk_label_new (format_definition->short_format_name);
    gtk_widget_set_halign (widget, GTK_ALIGN_START);
    gtk_widget_set_hexpand (widget, TRUE);
    gtk_box_append (GTK_BOX (box), widget);

    widget = gtk_switch_new ();
    gtk_widget_set_halign (widget, GTK_ALIGN_END);
    gtk_widget_set_valign (widget, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand (widget, TRUE);
    gtk_switch_set_state (GTK_SWITCH (widget), !format_definition->disabled);
    g_signal_connect (widget, "state-set", G_CALLBACK (enable_disable_format), format_definition);

    gtk_box_append (GTK_BOX (box), widget);

    gtk_list_box_append (dialog->system_formats, box);
}

static void
remove_user_format (GtkButton *self,
                    gpointer   user_data)
{
    ChirurgienFormatsDialog *dialog;

    FormatDescription *format_description;
    gint format_index;

    dialog = CHIRURGIEN_FORMATS_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (self),
                                                                 CHIRURGIEN_TYPE_FORMATS_DIALOG));

    format_index = g_list_index (chirurgien_user_format_definitions, user_data);

    format_description = g_list_nth (chirurgien_user_format_descriptions, format_index)->data;

    chirurgien_user_format_definitions = g_list_remove (chirurgien_user_format_definitions,
                                                        user_data);
    chirurgien_user_format_descriptions = g_list_remove (chirurgien_user_format_descriptions,
                                                        format_description);
    gtk_stack_remove (dialog->formats,
                      gtk_stack_get_child_by_name (dialog->formats, format_description->name));
    gtk_list_box_remove (dialog->user_formats, gtk_widget_get_ancestor (GTK_WIDGET (self),
                                                                        GTK_TYPE_LIST_BOX_ROW));

    format_definition_destroy (user_data);
    g_free (format_description->name);
    g_object_unref (format_description->description);
    g_slice_free (FormatDescription, format_description);
}

static void
add_user_format_description (ChirurgienFormatsDialog *dialog,
                             FormatDefinition        *format_definition,
                             FormatDescription       *format_description)
{
    GtkWidget *box, *widget;

    for (gint i = 0; i < CHIRURGIEN_TOTAL_COLORS; i++)
    {
        if (format_description->format_colors[i])
            gtk_color_chooser_set_rgba (format_description->format_colors[i],
                                        &chirurgien_colors[i]);
    }

    format_description->name = g_strdup_printf ("%d",
                               g_list_index (chirurgien_user_format_definitions, format_definition));

    gtk_stack_add_titled (dialog->formats,
                          format_description->description,
                          format_description->name,
                          format_definition->short_format_name);

    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 150);
    gtk_widget_set_tooltip_text (box, _("Remove format"));

    widget = gtk_label_new (format_definition->short_format_name);
    gtk_widget_set_halign (widget, GTK_ALIGN_START);
    gtk_widget_set_hexpand (widget, TRUE);
    gtk_box_append (GTK_BOX (box), widget);

    widget = gtk_button_new_from_icon_name ("window-close-symbolic");
    gtk_widget_set_halign (widget, GTK_ALIGN_END);
    gtk_widget_set_valign (widget, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand (widget, TRUE);
    gtk_widget_add_css_class (widget, "circular");
    gtk_widget_add_css_class (widget, "flat");
    g_signal_connect (widget, "clicked", G_CALLBACK (remove_user_format), format_definition);

    gtk_box_append (GTK_BOX (box), widget);

    gtk_list_box_append (dialog->user_formats, box);
}

static void
chirurgien_formats_dialog_class_init (ChirurgienFormatsDialogClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                         "/io/github/leonardschardijn/chirurgien/ui/chirurgien-formats-dialog.ui");
    gtk_widget_class_bind_template_child (widget_class, ChirurgienFormatsDialog, formats);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienFormatsDialog, new_format);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienFormatsDialog, message);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienFormatsDialog, system_formats);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienFormatsDialog, user_formats_page);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienFormatsDialog, user_formats);
}

static void
chirurgien_formats_dialog_init (ChirurgienFormatsDialog *dialog)
{
    const FormatDefinition *format_definition;

    GSList *sys_format_iter, *sys_desc_iter;
    GList *user_format_iter, *user_desc_iter;

    gtk_widget_init_template (GTK_WIDGET (dialog));

    g_signal_connect (dialog->new_format, "clicked", G_CALLBACK (chirurgien_actions_load_format), dialog);

    if (!chirurgien_system_format_descriptions)
    {
        for (sys_format_iter = chirurgien_system_format_definitions;
             sys_format_iter;
             sys_format_iter = sys_format_iter->next)
        {
            format_definition = sys_format_iter->data;

            chirurgien_system_format_descriptions = g_slist_append (chirurgien_system_format_descriptions,
                                                    build_format_description (format_definition));
        }
    }

    for (sys_format_iter = chirurgien_system_format_definitions,
         sys_desc_iter = chirurgien_system_format_descriptions;
         sys_format_iter && sys_desc_iter;
         sys_format_iter = sys_format_iter->next, sys_desc_iter = sys_desc_iter->next)
    {
        add_system_format_description (dialog,
                                       sys_format_iter->data,
                                       sys_desc_iter->data);
    }

    gtk_stack_add_titled (dialog->formats,
                          GTK_WIDGET (dialog->user_formats_page),
                          NULL,
                          _("User-defined formats"));

    for (user_format_iter = chirurgien_user_format_definitions,
         user_desc_iter = chirurgien_user_format_descriptions;
         user_format_iter && user_desc_iter;
         user_format_iter = user_format_iter->next, user_desc_iter = user_desc_iter->next)
    {
        add_user_format_description (dialog,
                                     user_format_iter->data,
                                     user_desc_iter->data);
    }
}

/*** Public API ***/

GtkWidget *
chirurgien_formats_dialog_new (GtkWindow *window)
{
    return GTK_WIDGET (g_object_new (CHIRURGIEN_TYPE_FORMATS_DIALOG,
                       "transient-for", window,
                       NULL));
}

void
chirurgien_formats_dialog_set_message (ChirurgienFormatsDialog *dialog,
                                       const gchar             *message)
{
    GtkWidget *text_view;
    GtkTextBuffer *buffer;

    GtkTextIter start, end;

    text_view = gtk_scrolled_window_get_child (dialog->message);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);
    gtk_text_buffer_delete (buffer, &start, &end);

    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_insert_markup (buffer, &start,
                                   message, -1);

    gtk_widget_show (GTK_WIDGET (dialog->message));
}

void
chirurgien_formats_dialog_add_format (ChirurgienFormatsDialog *dialog)
{
    GList *new_user_format;
    FormatDescription *new_user_desc;

    new_user_format = g_list_last (chirurgien_user_format_definitions);

    new_user_desc = build_format_description (new_user_format->data);
    chirurgien_user_format_descriptions = g_list_append (chirurgien_user_format_descriptions,
                                          new_user_desc);

    add_user_format_description (dialog,
                                 new_user_format->data,
                                 new_user_desc);
}
