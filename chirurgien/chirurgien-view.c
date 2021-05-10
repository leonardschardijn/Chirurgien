/* chirurgien-view.c
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

#include "chirurgien-view.h"

#include <glib/gi18n.h>

#include <chirurgien-formats.h>
#include "chirurgien-colors.h"

#include "chirurgien-view-tab.h"
#include "chirurgien-editor.h"
#include "chirurgien-actions.h"

#define UNUSED_DATA_COLOR 8


typedef struct {
    /* File offset of the modification */
    gsize modification_offset;

    /* Length of the modified data */
    gsize modification_length;

    /* The modified data */
    guchar *modification;

} FileModification;


struct _ChirurgienView
{
    GtkWidget             parent_instance;

    /* The main widget, container for everything else */
    GtkPaned             *main;

    /* Container for the description panel */
    GtkNotebook          *description;

    /* Current line length, in characters */
    gint                  line_length;
    /* If the line length was manually set */
    gboolean              manual_line_length;

    /* Character width, in Pango units
     * Used to calculate line_length */
    gint                  char_width;
    /* Line height, in Pango units
     * Used to calculate buffer_size */
    gint                  line_height;

    /* The file view */
    GtkWidget            *file_view;
    PangoLayout          *view_layout;
    gchar                *view_buffer;

    /* Active view type */
    ChirurgienViewType    active_view;

    /* View selection buttons */
    GtkToggleButton      *hex_view;
    GtkToggleButton      *text_view;

    /* Current view buffer size, in characters */
    gsize                 buffer_size;

    /* Adjustment for the view scrollbar */
    GtkAdjustment        *adjustment;

    /* The file fields, a list of FileField structs */
    GSList               *file_fields;

    /* The modifications stack */
    GQueue                modifications;
    /* Current modification index */
    guint                 modification_index;

    /* The 'Overview' page in the description panel */
    GtkBox               *overview;

    /* The index of the byte pointed to by the mouse */
    gint                  current_mouse_index;
    /* List of fields at the byte pointed to by the mouse */
    GSList               *fields_at_mouse_index;
    /* Number of fields at the mouse index */
    gint                  n_fields_at_mouse_index;

    /* File offset of the scroll, in bytes */
    gsize                 scroll_offset;

    /* The popover selected field */
    FileField            *selected_field;

    /* The view's tab on the main window notebook */
    ChirurgienViewTab    *view_tab;

    /* The file being viewed */
    gchar                *file_path;
    guchar               *file_contents;
    gsize                 file_size;
    gsize                 formatted_file_size;

    /* The navigation icon above the navigation buttons */
    GtkWidget            *navigation_icon;
    /* The navigation buttons */
    GtkBox               *navigation;
    /* Selected navigation target */
    FileField            *navigation_target;

    /* View state */
    gboolean              has_file;
    gboolean              modified;
    guint                 modification_save_point;

    /* Button to reanalyze after editing without automatic reanalysis */
    GtkRevealer          *reanalyze_notice;

    GSettings            *preferences_settings;
};

G_DEFINE_TYPE (ChirurgienView, chirurgien_view, GTK_TYPE_WIDGET)

static void
switch_view (GtkToggleButton *togglebutton,
             gpointer         user_data)
{
    ChirurgienView *view;

    view = CHIRURGIEN_VIEW (gtk_widget_get_ancestor (GTK_WIDGET (togglebutton),
                                                     CHIRURGIEN_TYPE_VIEW));

    if (gtk_toggle_button_get_active (togglebutton))
    {
        view->active_view = GPOINTER_TO_INT (user_data);
        gtk_widget_queue_draw (view->file_view);
    }
}

static void
resize_view (G_GNUC_UNUSED GtkDrawingArea *area,
             gint     width,
             gint     height,
             gpointer user_data)
{
    ChirurgienView *view;
    gint line_length, visible_lines, total_lines;
    gsize buffer_size;

    view = user_data;

    if (!width || !height)
    {
        view->line_length = view->buffer_size = 0;

        if (view->view_buffer)
            g_free (g_steal_pointer (&view->view_buffer));

        return;
    }

    width *= PANGO_SCALE;
    height *= PANGO_SCALE;

    visible_lines = height / view->line_height;

    /* Use the user-defined line length */
    if (view->manual_line_length)
    {
        line_length = view->line_length;

        buffer_size = line_length * visible_lines;
    }
    /* Calculate the line length */
    else
    {
        line_length = width / view->char_width;

        switch (line_length % 3)
        {
            case 2:
            view->line_length = ++line_length;

            break;
            case 1:
            view->line_length = --line_length;

            break;
            default:
            view->line_length = line_length;
        }

        buffer_size = line_length * visible_lines;
    }

    if (view->buffer_size != buffer_size)
    {
        view->buffer_size = buffer_size;

        total_lines = view->formatted_file_size / line_length;

        if(view->formatted_file_size % line_length)
            total_lines++;

        gtk_adjustment_configure (view->adjustment,
                                  gtk_adjustment_get_value (view->adjustment),
                                  0,
                                  total_lines,
                                  1,
                                  visible_lines - 1,
                                  visible_lines);

        if (view->view_buffer)
            g_free (view->view_buffer);

        view->view_buffer = g_malloc (view->buffer_size);
    }
}

static void
scroll_view (G_GNUC_UNUSED GtkAdjustment *adjustment,
             gpointer user_data)
{
    ChirurgienView *view;

    view = user_data;

    view->scroll_offset = gtk_adjustment_get_value (view->adjustment);
    view->scroll_offset *= (view->line_length / 3);

    gtk_widget_queue_draw (view->file_view);
}

static void
highlight_fields (ChirurgienView *view,
                  PangoLayout    *layout)
{
    PangoAttrList *attribute_list;
    PangoAttribute *attribute, *alpha_attribute, *navigation_attribute,
                   *additional_attribute = NULL, *additional_alpha_attribute;

    FileField *file_field;

    guint real_field_start;
    gsize field_end, scroll_end;

    pango_layout_set_attributes (layout, NULL);

    if (!view->file_fields)
        return;

    /* Last byte of the buffer/frame */
    scroll_end = view->scroll_offset + view->buffer_size / 3;

    attribute_list = pango_attr_list_new ();

    for (GSList *i = view->file_fields; i != NULL; i = i->next)
    {
        file_field = i->data;

        if (file_field->field_offset > scroll_end)
            break;

        field_end = file_field->field_offset + file_field->field_length;
        if (field_end <= view->scroll_offset)
            continue;

        if (file_field->background)
        {
            attribute = pango_attr_background_new (pango_colors[ file_field->color_index ].red,
                                                   pango_colors[ file_field->color_index ].green,
                                                   pango_colors[ file_field->color_index ].blue);
            alpha_attribute = pango_attr_background_alpha_new (pango_alphas [ file_field->color_index ]);
        }
        else
        {
            attribute = pango_attr_foreground_new (pango_colors[ file_field->color_index ].red,
                                                   pango_colors[ file_field->color_index ].green,
                                                   pango_colors[ file_field->color_index ].blue);
            alpha_attribute = pango_attr_foreground_alpha_new (pango_alphas [ file_field->color_index ]);
        }

        real_field_start = (file_field->field_offset - view->scroll_offset) * 3;

        if (file_field->field_offset < view->scroll_offset)
            attribute->start_index = PANGO_ATTR_INDEX_FROM_TEXT_BEGINNING;
        else
            attribute->start_index = real_field_start;

        attribute->end_index = ((field_end - view->scroll_offset) * 3) - 1;

        /* Additional color */
        if (file_field->additional_color_index != -1 &&
            attribute->start_index == real_field_start)
        {
            if (file_field->background)
            {
                additional_attribute = pango_attr_background_new (pango_colors[ file_field->additional_color_index ].red,
                                                                  pango_colors[ file_field->additional_color_index ].green,
                                                                  pango_colors[ file_field->additional_color_index ].blue);
                additional_alpha_attribute = pango_attr_background_alpha_new (pango_alphas [ file_field->additional_color_index ]);
            }
            else
            {
                additional_attribute = pango_attr_foreground_new (pango_colors[ file_field->additional_color_index ].red,
                                                                  pango_colors[ file_field->additional_color_index ].green,
                                                                  pango_colors[ file_field->additional_color_index ].blue);
                additional_alpha_attribute = pango_attr_foreground_alpha_new (pango_alphas [ file_field->additional_color_index ]);
            }

            additional_attribute->start_index = additional_attribute->end_index = attribute->start_index;
            additional_attribute->end_index += 2;
            attribute->start_index += 2;

            additional_alpha_attribute->start_index = additional_attribute->start_index;
            additional_alpha_attribute->end_index = additional_attribute->end_index;

            pango_attr_list_insert (attribute_list, additional_attribute);
            pango_attr_list_insert (attribute_list, additional_alpha_attribute);
        }

        alpha_attribute->start_index = attribute->start_index;
        alpha_attribute->end_index = attribute->end_index;

        pango_attr_list_insert (attribute_list, attribute);
        pango_attr_list_insert (attribute_list, alpha_attribute);

        /* Nagivation target */
        if (view->navigation_target == file_field)
        {
            navigation_attribute = pango_attr_weight_new (PANGO_WEIGHT_ULTRABOLD);
            if (additional_attribute)
            {
                navigation_attribute->start_index = additional_attribute->start_index;
                navigation_attribute->end_index = additional_attribute->end_index;
            }
            else
            {
                navigation_attribute->start_index = attribute->start_index;
                navigation_attribute->end_index = attribute->end_index;
            }
            pango_attr_list_insert (attribute_list, navigation_attribute);
        }

        additional_attribute = NULL;
    }

    pango_layout_set_attributes (layout, attribute_list);

    pango_attr_list_unref (attribute_list);

    view->navigation_target = NULL;
}

static void
draw_view (GtkDrawingArea *drawing_area,
           cairo_t        *cr,
           G_GNUC_UNUSED gint width,
           G_GNUC_UNUSED gint height,
           gpointer        user_data)
{
    ChirurgienView *view;
    GdkRGBA color;
    GtkStyleContext *context;

    view = user_data;

    context = gtk_widget_get_style_context (GTK_WIDGET (drawing_area));

    gtk_style_context_get_color (context, &color);
    gdk_cairo_set_source_rgba (cr, &color);

    if (view->active_view == CHIRURGIEN_HEX_VIEW)
    {
        chirurgien_utils_hex_print (view->view_buffer,
                                    view->file_contents,
                                    view->scroll_offset,
                                    view->buffer_size,
                                    view->file_size,
                                    view->line_length);
    }
    else if (view->active_view == CHIRURGIEN_TEXT_VIEW)
    {
        chirurgien_utils_text_print (view->view_buffer,
                                     view->file_contents,
                                     view->scroll_offset,
                                     view->buffer_size,
                                     view->file_size,
                                     view->line_length);
    }

    highlight_fields (view, view->view_layout);

    pango_layout_set_text (view->view_layout, view->view_buffer, view->buffer_size);
    pango_cairo_show_layout (cr, view->view_layout);
}

static gboolean
handle_scroll_event (G_GNUC_UNUSED GtkEventControllerScroll *controller,
                     G_GNUC_UNUSED gdouble                   dx,
                     gdouble  dy,
                     gpointer user_data)
{
    gtk_adjustment_set_value (user_data,
                              gtk_adjustment_get_value (user_data) + dy);

    return TRUE;
}

static gboolean
handle_key_event (G_GNUC_UNUSED GtkEventControllerKey *controller,
                  guint    keyval,
                  G_GNUC_UNUSED guint                  keycode,
                  G_GNUC_UNUSED GdkModifierType        state,
                  gpointer user_data)
{
    switch (keyval)
    {
        case GDK_KEY_Up:
        gtk_adjustment_set_value (user_data,
              gtk_adjustment_get_value (user_data) - 1);

        break;
        case GDK_KEY_Down:
        gtk_adjustment_set_value (user_data,
              gtk_adjustment_get_value (user_data) + 1);

        break;
        case GDK_KEY_Page_Up:
        gtk_adjustment_set_value (user_data,
              gtk_adjustment_get_value (user_data) - (gtk_adjustment_get_page_size (user_data) - 1));

        break;
        case GDK_KEY_Page_Down:
        gtk_adjustment_set_value (user_data,
              gtk_adjustment_get_value (user_data) + (gtk_adjustment_get_page_size (user_data) - 1));

        break;
        case GDK_KEY_Home:
            gtk_adjustment_set_value (user_data, gtk_adjustment_get_lower (user_data));

        break;
        case GDK_KEY_End:
            gtk_adjustment_set_value (user_data, gtk_adjustment_get_upper (user_data));
    }

    return TRUE;
}

static void
handle_motion_event (G_GNUC_UNUSED GtkEventControllerMotion *controller,
                     gdouble  x,
                     gdouble  y,
                     gpointer user_data)
{
    ChirurgienView *view;
    FileField *file_field;
    gint byte_index;

    GString *field_tooltip;

    view = user_data;

    if (!pango_layout_xy_to_index (view->view_layout,
                                   x * PANGO_SCALE,
                                   y * PANGO_SCALE,
                                   &byte_index,
                                   NULL))
    {
        view->current_mouse_index = -1;
        view->n_fields_at_mouse_index = 0;

        gtk_widget_set_tooltip_text (view->file_view, NULL);

        return;
    }

    byte_index = view->scroll_offset + (byte_index - (byte_index % 3)) / 3;

    /* Same byte index, nothing to do */
    if (view->current_mouse_index == byte_index)
        return;

    view->current_mouse_index = byte_index;
    g_slist_free (g_steal_pointer (&view->fields_at_mouse_index));
    view->n_fields_at_mouse_index = 0;

    field_tooltip = g_string_new (NULL);

    /* Build the list of fields at the new mouse index */
    for (GSList *i = view->file_fields; i != NULL; i = i->next)
    {
        file_field = i->data;

        if (file_field->field_offset > (gsize) byte_index)
            break;

        if (file_field->field_offset + file_field->field_length > (gsize) byte_index)
        {
            view->fields_at_mouse_index = g_slist_prepend (view->fields_at_mouse_index, file_field);

            if (field_tooltip->len)
                field_tooltip = g_string_append_c (field_tooltip, '\n');

            field_tooltip = g_string_append (field_tooltip, file_field->field_name);

            view->n_fields_at_mouse_index++;
        }
    }

    gtk_widget_set_tooltip_text (view->file_view, field_tooltip->str);

    g_string_free (field_tooltip, TRUE);
}

static void
file_modified (ChirurgienView *view)
{
    view->modified = TRUE;

    if (view->modification_save_point == view->modification_index)
        chirurgien_view_tab_set_unsaved (view->view_tab, FALSE);
    else
        chirurgien_view_tab_set_unsaved (view->view_tab, TRUE);

    if (g_settings_get_boolean (view->preferences_settings, "auto-analysis"))
    {
        chirurgien_view_redo_analysis (view);
    }
    else
    {
        gtk_revealer_set_reveal_child (view->reanalyze_notice, TRUE);

        chirurgien_view_tab_set_modified (view->view_tab, TRUE);

        gtk_widget_queue_draw (view->file_view);
    }
}

static void
edit_field_response (GtkDialog *dialog,
                     int        response_id,
                     gpointer   user_data)
{
    ChirurgienView *view;
    ChirurgienEditor *editor;
    FileModification *modification, *old_modification;

    const guchar *file_contents;
    const guchar *new_contents;

    view = user_data;
    editor = CHIRURGIEN_EDITOR (gtk_widget_get_first_child
                               (gtk_dialog_get_content_area (dialog)));

    if (response_id == GTK_RESPONSE_ACCEPT)
    {
        modification = g_slice_new (FileModification);

        modification->modification_offset = view->selected_field->field_offset;
        modification->modification_length = view->selected_field->field_length;
        modification->modification = g_malloc (modification->modification_length);

        file_contents = view->file_contents + modification->modification_offset;
        new_contents = chirurgien_editor_get_contents (editor);

        for (gsize i = 0; i < modification->modification_length; i++)
            modification->modification[i] = new_contents[i] ^ file_contents[i];

        /* Delete now outdated modifications */
        while ((old_modification = g_queue_pop_nth (&view->modifications,
                                                    view->modification_index + 1)))
        {
            g_free (old_modification->modification);
            g_slice_free (FileModification, old_modification);
        }

        /* Add the modification */
        g_queue_push_tail (&view->modifications, modification);

        /* Let the redo operation apply the change */
        chirurgien_actions_redo (NULL, NULL, gtk_widget_get_ancestor (GTK_WIDGET (view),
                                                                      CHIRURGIEN_TYPE_WINDOW));
    }

    gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
edit_field (GtkButton *button)
{
    ChirurgienView *view;
    GtkWidget *popover;

    GtkWidget *editor, *edition_dialog, *content_area;

    view = CHIRURGIEN_VIEW (gtk_widget_get_ancestor (GTK_WIDGET (button),
                                                     CHIRURGIEN_TYPE_VIEW));
    popover = gtk_widget_get_ancestor (GTK_WIDGET (button),
                                       GTK_TYPE_POPOVER);

    gtk_popover_popdown (GTK_POPOVER (popover));

    editor = chirurgien_editor_new ();
    chirurgien_editor_set_contents (CHIRURGIEN_EDITOR (editor),
                                    view->file_contents + view->selected_field->field_offset,
                                    view->selected_field->field_length);

    edition_dialog = gtk_dialog_new_with_buttons (view->selected_field->field_name,
                         GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (view), CHIRURGIEN_TYPE_WINDOW)),
                         GTK_DIALOG_MODAL | GTK_DIALOG_USE_HEADER_BAR,
                         _("Cancel"),
                         GTK_RESPONSE_CANCEL,
                         _("Accept"),
                         GTK_RESPONSE_ACCEPT,
                         NULL);
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (edition_dialog));
    gtk_box_append (GTK_BOX (content_area), editor);

    gtk_window_set_default_size (GTK_WINDOW (edition_dialog), 600, 240);

    g_signal_connect (edition_dialog, "response", G_CALLBACK (edit_field_response), view);

    gtk_window_present (GTK_WINDOW (edition_dialog));
}

static void
extract_field (GtkButton *button)
{
    ChirurgienWindow *window;
    ChirurgienView *view, *extract_view;

    g_autofree gchar *basename;

    view = CHIRURGIEN_VIEW (gtk_widget_get_ancestor (GTK_WIDGET (button),
                                                     CHIRURGIEN_TYPE_VIEW));
    window = CHIRURGIEN_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (view),
                                                         CHIRURGIEN_TYPE_WINDOW));

    extract_view = chirurgien_view_new (window);
    extract_view->file_size = view->selected_field->field_length;
    extract_view->formatted_file_size = extract_view->file_size * 3;
    extract_view->file_contents = g_malloc (extract_view->file_size);

    memcpy (extract_view->file_contents,
            view->file_contents + view->selected_field->field_offset,
            extract_view->file_size);

    basename = g_path_get_basename (view->file_path);

    extract_view->file_path = g_strdup_printf ("%s [%s]", basename,
                                               view->selected_field->field_name);
    chirurgien_view_tab_set_label (extract_view->view_tab,
                                   extract_view->file_path,
                                   extract_view->file_path);
    extract_view->has_file = FALSE;

    chirurgien_actions_show_view (window, extract_view);
}

static void
create_field_popover (GtkPopover *popover,
                      FileField  *file_field)
{
    ChirurgienView *view;
    GtkWidget *grid, *widget, *button;

    view = CHIRURGIEN_VIEW (gtk_widget_get_ancestor (GTK_WIDGET (popover),
                                                     CHIRURGIEN_TYPE_VIEW));

    grid = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
    gtk_grid_set_row_spacing (GTK_GRID (grid), 15);
    gtk_widget_set_halign (grid, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_start (grid, 5);
    gtk_widget_set_margin_end (grid, 5);
    gtk_widget_set_margin_bottom (grid, 5);
    gtk_widget_set_margin_top (grid, 5);

    widget = gtk_label_new (file_field->field_name);

    gtk_grid_attach (GTK_GRID (grid), widget, 0, 0, 2, 1);

    widget = gtk_button_new_from_icon_name ("system-search-symbolic");
    gtk_widget_set_tooltip_text (widget, _("Extract field to separate tab"));
    gtk_widget_add_css_class (widget, "circular");

    g_signal_connect (widget, "clicked", G_CALLBACK (extract_field), NULL);

    button = gtk_button_new_from_icon_name ("document-edit-symbolic");
    gtk_widget_set_tooltip_text (button, _("Edit field"));
    gtk_widget_add_css_class (button, "circular");
    gtk_widget_set_halign (button, GTK_ALIGN_END);

    g_signal_connect (button, "clicked", G_CALLBACK (edit_field), NULL);

    gtk_widget_set_halign (widget, GTK_ALIGN_START);

    gtk_grid_attach (GTK_GRID (grid), button, 0, 1, 1, 1);
    gtk_grid_attach_next_to (GTK_GRID (grid), widget, button, GTK_POS_RIGHT, 1, 1);

    view->selected_field = file_field;

    gtk_popover_set_child (popover, grid);
}

static void
field_selected (GtkButton *button,
                gpointer   user_data)
{
    GtkPopover *popover;

    popover = GTK_POPOVER (gtk_widget_get_ancestor (GTK_WIDGET (button),
                           GTK_TYPE_POPOVER));

    gtk_widget_unparent (gtk_popover_get_child (popover));
    create_field_popover (popover, user_data);
}

static void
handle_click (GtkGestureClick *gesture,
              gint             n_press,
              gdouble          x,
              gdouble          y,
              gpointer         user_data)
{
    ChirurgienView *view;
    GtkWidget *popover, *grid, *widget, *button;
    GdkRectangle position;

    FileField *file_field;
    gint grid_row;

    view = user_data;

    if (n_press == 1)
    {
        gtk_widget_grab_focus (gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (gesture)));
    }
    else if (n_press == 2)
    {
        if (!view->n_fields_at_mouse_index)
            return;

        popover = gtk_popover_new ();
        gtk_popover_set_has_arrow (GTK_POPOVER (popover), FALSE);

        gtk_widget_set_parent (popover, gtk_widget_get_parent
                              (gtk_event_controller_get_widget
                              (GTK_EVENT_CONTROLLER (gesture))));

        position.x = x + gtk_widget_get_margin_start (gtk_event_controller_get_widget
                                                     (GTK_EVENT_CONTROLLER (gesture)));
        position.y = y + gtk_widget_get_margin_top (gtk_event_controller_get_widget
                                                   (GTK_EVENT_CONTROLLER (gesture))) + 10;
        position.height = 0;
        position.width = 0;
        gtk_popover_set_pointing_to (GTK_POPOVER (popover), &position);

        if (view->n_fields_at_mouse_index == 1)
        {
            create_field_popover (GTK_POPOVER (popover), view->fields_at_mouse_index->data);
        }
        else if (view->n_fields_at_mouse_index > 1)
        {
            grid = gtk_grid_new ();
            gtk_grid_set_column_spacing (GTK_GRID (grid), 10);
            gtk_grid_set_row_spacing (GTK_GRID (grid), 15);
            gtk_widget_set_halign (grid, GTK_ALIGN_CENTER);
            gtk_widget_set_margin_start (grid, 5);
            gtk_widget_set_margin_end (grid, 5);
            gtk_widget_set_margin_bottom (grid, 5);
            gtk_widget_set_margin_top (grid, 5);

            widget = gtk_label_new (_("Multiple fields overlap at this position"));

            gtk_grid_attach (GTK_GRID (grid), widget, 0, 0, 2, 1);

            grid_row = 1;
            for (GSList *i = view->fields_at_mouse_index; i != NULL; i = i->next)
            {
                file_field = i->data;

                widget = gtk_label_new (file_field->field_name);
                gtk_widget_set_halign (widget, GTK_ALIGN_END);

                button = gtk_button_new_from_icon_name ("go-next-symbolic");
                gtk_widget_set_halign (button, GTK_ALIGN_START);
                gtk_widget_set_tooltip_text (button, _("Select field"));
                gtk_widget_add_css_class (button, "circular");

                g_signal_connect (button, "clicked", G_CALLBACK (field_selected), file_field);

                gtk_grid_attach (GTK_GRID (grid), widget, 0, grid_row++, 1, 1);
                gtk_grid_attach_next_to (GTK_GRID (grid), button, widget, GTK_POS_RIGHT, 1, 1);
            }

            gtk_popover_set_child (GTK_POPOVER (popover), grid);
        }

        g_signal_connect (popover, "closed", G_CALLBACK (gtk_widget_unparent), NULL);
        gtk_popover_popup (GTK_POPOVER (popover));
    }
}

static void
navigate_view (GtkButton *button,
               gpointer   user_data)
{
    ChirurgienView *view;
    FileField *file_field;

    gsize field_line;

    view = CHIRURGIEN_VIEW (gtk_widget_get_ancestor (GTK_WIDGET (button), CHIRURGIEN_TYPE_VIEW));
    file_field = user_data;

    field_line = (file_field->field_offset * 3) / view->line_length;

    if (field_line <= 3)
        field_line = 0;
    else
        field_line -= 3;

    view->navigation_target = file_field;

    gtk_adjustment_set_value (view->adjustment, field_line);

    /* Drawing may have not been scheduled */
    gtk_widget_queue_draw (view->file_view);
}

static void
build_navigation_buttons (ChirurgienView *view)
{
    FileField *file_field;
    GtkWidget *button;

    for (GSList *i = view->file_fields; i != NULL; i = i->next)
    {
        file_field = i->data;

        if (file_field->navigation_label)
        {
            button = gtk_button_new_with_label (file_field->navigation_label);
            g_signal_connect (button, "clicked", G_CALLBACK (navigate_view), file_field);
            gtk_widget_add_css_class (button, "flat");

            gtk_box_append (view->navigation, button);
        }
    }

    /* There are no navigation labels */
    if (!gtk_widget_get_first_child (GTK_WIDGET (view->navigation)))
    {
        gtk_widget_hide (view->navigation_icon);
        gtk_widget_hide (gtk_widget_get_ancestor (GTK_WIDGET (view->navigation), GTK_TYPE_SCROLLED_WINDOW));
    }
    else
    {
        g_settings_bind (view->preferences_settings, "hide-navigation",
                         view->navigation_icon, "visible",
                         G_SETTINGS_BIND_GET | G_SETTINGS_BIND_INVERT_BOOLEAN);
        g_settings_bind (view->preferences_settings, "hide-navigation",
                         gtk_widget_get_ancestor (GTK_WIDGET (view->navigation), GTK_TYPE_SCROLLED_WINDOW), "visible",
                         G_SETTINGS_BIND_GET | G_SETTINGS_BIND_INVERT_BOOLEAN);
    }
}

static gint
sort_file_fields (gconstpointer a,
                  gconstpointer b)
{
    const FileField *field_a, *field_b;

    field_a = a;
    field_b = b;

    return field_a->field_offset - field_b->field_offset;
}

static void
find_unused_bytes (ChirurgienView *view)
{
    FileField *file_field, *unused_data;
    GSList *new_fields;
    gsize tagged_up_to, field_end;

    new_fields = NULL;
    tagged_up_to = 0;

    for (GSList *i = view->file_fields; i != NULL; i = i->next)
    {
        file_field = i->data;
        if (tagged_up_to < file_field->field_offset)
        {
            unused_data = g_slice_new (FileField);

            unused_data->field_name = _("Unused data");
            unused_data->field_offset = tagged_up_to;
            unused_data->field_length = file_field->field_offset - tagged_up_to;
            unused_data->color_index = UNUSED_DATA_COLOR;
            unused_data->background = FALSE;
            unused_data->navigation_label = NULL;
            unused_data->additional_color_index = -1;

            new_fields = g_slist_prepend (new_fields, unused_data);
        }

        field_end = file_field->field_offset + file_field->field_length;

        if (tagged_up_to < field_end)
            tagged_up_to = field_end;
    }

    if (tagged_up_to < view->file_size)
    {
        unused_data = g_slice_new (FileField);

        unused_data->field_name = _("Unused data");
        unused_data->field_offset = tagged_up_to;
        unused_data->field_length = view->file_size - tagged_up_to;
        unused_data->color_index = UNUSED_DATA_COLOR;
        unused_data->background = FALSE;
        unused_data->navigation_label = NULL;
        unused_data->additional_color_index = -1;

        new_fields = g_slist_prepend (new_fields, unused_data);
    }

    if (new_fields)
        view->file_fields = g_slist_sort (g_slist_concat (view->file_fields, new_fields), sort_file_fields);
}

static void
get_view_measures (ChirurgienView *view)
{
    PangoContext *context;
    PangoFontMetrics *font_metrics;

    gint bytes_per_line;

    context = gtk_widget_get_pango_context (view->file_view);
    font_metrics = pango_context_get_metrics (context, pango_context_get_font_description (context),
                                              pango_context_get_language (context));

    view->char_width =  pango_font_metrics_get_approximate_digit_width (font_metrics);

    view->line_height = pango_font_metrics_get_ascent (font_metrics) +
                        pango_font_metrics_get_descent (font_metrics);

    bytes_per_line = g_settings_get_int (view->preferences_settings, "bytes-per-line");
    if (bytes_per_line)
    {
        view->manual_line_length = TRUE;
        view->line_length = bytes_per_line * 3;
    }
    else
    {
        view->manual_line_length = FALSE;
    }

    pango_font_metrics_unref (font_metrics);
}

static void
chirurgien_view_dispose (GObject *object)
{
    ChirurgienView *view;
    FileModification *modification;

    view = CHIRURGIEN_VIEW (object);

    gtk_widget_unparent (GTK_WIDGET (g_steal_pointer (&view->main)));

    for (GSList *i = view->file_fields; i != NULL; i = i->next)
        g_slice_free (FileField, i->data);
    g_slist_free (g_steal_pointer (&view->file_fields));

    for (GList *i = view->modifications.head; i != NULL; i = i->next)
    {
        modification = i->data;
        g_free (modification->modification);
        g_slice_free (FileModification, modification);
    }
    g_queue_clear (&view->modifications);
    g_queue_init (&view->modifications);

    g_slist_free (g_steal_pointer (&view->fields_at_mouse_index));

    g_object_unref (g_steal_pointer (&view->view_layout));

    g_free (g_steal_pointer (&view->view_buffer));

    g_free (g_steal_pointer (&view->file_contents));
    g_free (g_steal_pointer (&view->file_path));

    G_OBJECT_CLASS (chirurgien_view_parent_class)->dispose (object);
}

static void
chirurgien_view_class_init (ChirurgienViewClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    G_OBJECT_CLASS (klass)->dispose = chirurgien_view_dispose;

    gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);

    gtk_widget_class_set_template_from_resource (widget_class,
                                         "/io/github/leonardschardijn/chirurgien/ui/chirurgien-view.ui");
    gtk_widget_class_bind_template_child (widget_class, ChirurgienView, main);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienView, description);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienView, file_view);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienView, hex_view);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienView, text_view);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienView, navigation_icon);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienView, navigation);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienView, overview);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienView, adjustment);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienView, view_tab);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienView, reanalyze_notice);
}

static void
chirurgien_view_init (ChirurgienView *view)
{
    GtkEventController *controller;

    g_type_ensure (CHIRURGIEN_TYPE_VIEW_TAB);

    gtk_widget_init_template (GTK_WIDGET (view));

    gtk_paned_set_shrink_start_child (view->main, FALSE);
    gtk_paned_set_shrink_end_child (view->main, FALSE);

    view->view_layout = gtk_widget_create_pango_layout (view->file_view, NULL);

    g_signal_connect (view->file_view, "resize", G_CALLBACK (resize_view), view);
    g_signal_connect (view->adjustment, "value-changed", G_CALLBACK (scroll_view), view);

    gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (view->file_view), draw_view, view, NULL);

    controller = GTK_EVENT_CONTROLLER (gtk_event_controller_scroll_new
                                      (GTK_EVENT_CONTROLLER_SCROLL_VERTICAL |
                                       GTK_EVENT_CONTROLLER_SCROLL_DISCRETE));
    g_signal_connect (controller, "scroll", G_CALLBACK (handle_scroll_event), view->adjustment);
    gtk_widget_add_controller (view->file_view, controller);

    controller = gtk_event_controller_key_new ();
    g_signal_connect (controller, "key-pressed", G_CALLBACK (handle_key_event), view->adjustment);
    gtk_widget_add_controller (view->file_view, controller);

    controller = gtk_event_controller_motion_new ();
    g_signal_connect (controller, "motion", G_CALLBACK (handle_motion_event), view);
    gtk_widget_add_controller (view->file_view, controller);

    controller = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (controller), 0);
    g_signal_connect (controller, "pressed", G_CALLBACK (handle_click), view);
    gtk_widget_add_controller (view->file_view, controller);

    g_signal_connect (view->hex_view, "toggled", G_CALLBACK (switch_view), GINT_TO_POINTER (CHIRURGIEN_HEX_VIEW));
    g_signal_connect (view->text_view, "toggled", G_CALLBACK (switch_view), GINT_TO_POINTER (CHIRURGIEN_TEXT_VIEW));

    chirurgien_view_tab_set_view (view->view_tab, view);

    g_queue_init (&view->modifications);
    view->modification_index = G_MAXUINT;

    view->active_view = CHIRURGIEN_HEX_VIEW;

    view->view_buffer = NULL;
    view->buffer_size = 0;

    view->file_fields = NULL;

    view->current_mouse_index = -1;
    view->fields_at_mouse_index = NULL;
    view->n_fields_at_mouse_index = 0;

    view->navigation_target = NULL;

    view->has_file = FALSE;
    view->modified = FALSE;
    view->modification_save_point = G_MAXUINT;
}

/*** Public API ***/

ChirurgienView *
chirurgien_view_new (ChirurgienWindow *window)
{
    ChirurgienView *view;

    view = CHIRURGIEN_VIEW (g_object_new (CHIRURGIEN_TYPE_VIEW, NULL));

    view->preferences_settings = chirurgien_window_get_preferences (window);

    get_view_measures (view);

    g_settings_bind (chirurgien_window_get_state (window), "position",
                     view->main, "position",
                     G_SETTINGS_BIND_DEFAULT);
    g_settings_bind (view->preferences_settings, "hide-description",
                     view->description, "visible",
                     G_SETTINGS_BIND_GET | G_SETTINGS_BIND_INVERT_BOOLEAN);

    return view;
}

void
chirurgien_view_set_file (ChirurgienView *view,
                          GFile          *file)
{
    g_autoptr (GFileInputStream) file_input;
    g_autoptr (GFileInfo) file_info;

    g_autofree gchar *basename;

    file_input = g_file_read (file, NULL, NULL);
    file_info = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_SIZE","G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
                                   G_FILE_QUERY_INFO_NONE, NULL, NULL);

    view->file_size = g_file_info_get_size (file_info);

    /* Limit file size to 50 MiB */
    if (view->file_size > 52428800)
        view->file_size = 52428800;

    view->formatted_file_size = view->file_size * 3;

    view->file_contents = g_malloc (view->file_size);

    g_input_stream_read_all (G_INPUT_STREAM (file_input), view->file_contents, view->file_size,
                             NULL, NULL, NULL);

    if (g_file_info_get_attribute_boolean (file_info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
    {
        view->file_path = g_file_get_path (file);
        view->has_file = TRUE;
    }
    else
    {
        chirurgien_view_tab_set_locked (view->view_tab, TRUE);
        view->file_path = g_file_get_basename (file);
        view->has_file = FALSE;
    }

    basename = g_file_get_basename (file);
    chirurgien_view_tab_set_label (view->view_tab,
                                   basename,
                                   view->file_path);
}

void
chirurgien_view_do_analysis (ChirurgienView *view)
{
    FormatsFile file;
    gboolean unused_bytes;

    file.file_contents = view->file_contents;
    file.file_size = view->file_size;
    file.file_contents_index = 0;
    file.file_fields = NULL;
    file.description = view->description;
    file.overview = view->overview;

    unused_bytes = chirurgien_formats_analyze (&file);

    view->file_fields = g_slist_sort (file.file_fields, sort_file_fields);

    if (unused_bytes)
        find_unused_bytes (view);

    build_navigation_buttons (view);
}

void
chirurgien_view_redo_analysis (ChirurgienView *view)
{
    GtkWidget *child_widget;
    gint description_pages;

    if (!view->modified)
        return;

    for (GSList *i = view->file_fields; i != NULL; i = i->next)
        g_slice_free (FileField, i->data);
    g_slist_free (g_steal_pointer (&view->file_fields));

    g_slist_free (g_steal_pointer (&view->fields_at_mouse_index));

    view->current_mouse_index = -1;
    view->n_fields_at_mouse_index = 0;

    view->selected_field = NULL;

    for (child_widget = gtk_widget_get_first_child (GTK_WIDGET (view->navigation));
         child_widget != NULL;
         child_widget = gtk_widget_get_first_child (GTK_WIDGET (view->navigation)))
        gtk_widget_unparent (child_widget);

    for (child_widget = gtk_widget_get_first_child (GTK_WIDGET (view->overview));
         child_widget != NULL;
         child_widget = gtk_widget_get_first_child (GTK_WIDGET (view->overview)))
        gtk_widget_unparent (child_widget);

    description_pages = gtk_notebook_get_n_pages (view->description);

    while (--description_pages)
        gtk_notebook_remove_page (view->description, -1);

    chirurgien_view_do_analysis (view);

    view->modified = FALSE;
    chirurgien_view_tab_set_modified (view->view_tab, FALSE);
    gtk_revealer_set_reveal_child (view->reanalyze_notice, FALSE);

    gtk_widget_queue_draw (view->file_view);
}

gboolean
chirurgien_view_save (ChirurgienView *view,
                      GFile          *file)
{
    if (g_file_replace_contents (file, (const gchar *) view->file_contents, view->file_size, NULL,
                                 FALSE, G_FILE_CREATE_NONE, NULL, NULL, NULL))
    {
        g_free (view->file_path);
        view->file_path = g_file_get_path (file);
        view->has_file = TRUE;
        view->modification_save_point = view->modification_index;
        chirurgien_view_tab_set_unsaved (view->view_tab, FALSE);
        chirurgien_view_tab_set_locked (view->view_tab, FALSE);
        chirurgien_view_tab_set_label (view->view_tab,
                                       g_file_get_basename (file),
                                       g_file_get_path (file));

        /* Changes window title if the file name changed */
        g_signal_emit_by_name (gtk_widget_get_ancestor (GTK_WIDGET (view), GTK_TYPE_NOTEBOOK),
                               "switch-page", view, NULL);

        return TRUE;
    }

    return FALSE;
}

void
chirurgien_view_undo (ChirurgienView *view)
{
    FileModification *modification;
    guchar *contents;

    if (view->modification_index == G_MAXUINT)
        return;

    modification = g_queue_peek_nth (&view->modifications, view->modification_index);

    contents = view->file_contents + modification->modification_offset;

    for (gsize i = 0; i < modification->modification_length; i++)
        contents[i] ^= modification->modification[i];

    view->modification_index--;

    file_modified (view);
}

void
chirurgien_view_redo (ChirurgienView *view)
{
    FileModification *modification;
    guchar *contents;

    if (view->modification_index == view->modifications.length - 1)
        return;

    modification = g_queue_peek_nth (&view->modifications, ++view->modification_index);

    contents = view->file_contents + modification->modification_offset;

    for (gsize i = 0; i < modification->modification_length; i++)
        contents[i] ^= modification->modification[i];

    file_modified (view);
}

void
chirurgien_view_query_modifications (ChirurgienView *view,
                                     gboolean       *undo_available,
                                     gboolean       *redo_available)
{
    if (!view->modifications.length)
    {
        *undo_available = FALSE;
        *redo_available = FALSE;
    }
    else if (view->modification_index == G_MAXUINT)
    {
        *undo_available = FALSE;
        *redo_available = TRUE;
    }
    else if (view->modification_index == view->modifications.length - 1)
    {
        *undo_available = TRUE;
        *redo_available = FALSE;
    }
    else
    {
        *undo_available = TRUE;
        *redo_available = TRUE;
    }
}

void
chirurgien_view_select_view (ChirurgienView    *view,
                             ChirurgienViewType selection)
{
    if (selection == CHIRURGIEN_HEX_VIEW)
        gtk_toggle_button_set_active (view->hex_view, TRUE);
    else if (selection == CHIRURGIEN_TEXT_VIEW)
        gtk_toggle_button_set_active (view->text_view, TRUE);
}

void
chirurgien_view_refresh (ChirurgienView *view)
{
    get_view_measures (view);

    gtk_widget_queue_resize (view->file_view);
    gtk_widget_queue_draw (view->file_view);
}

GtkWidget *
chirurgien_view_get_view_tab (ChirurgienView *view)
{
    return GTK_WIDGET (view->view_tab);
}

const gchar *
chirurgien_view_get_file_path (ChirurgienView *view)
{
    return view->file_path;
}

gboolean
chirurgien_view_has_file (ChirurgienView *view)
{
    return view->has_file;
}

gboolean
chirurgien_view_unsaved (ChirurgienView *view)
{
    return view->modification_index != view->modification_save_point;
}
