/* chirurgien-editor.c
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

#include "chirurgien-editor.h"

#include "chirurgien-utils.h"


struct _ChirurgienEditor
{
    GtkWidget             parent_instance;

    gint                  line_length;

    gint                  char_width;
    gint                  line_height;

    GtkWidget            *hex_view;
    PangoLayout          *hex_layout;
    gchar                *hex_buffer;

    GtkWidget            *text_view;
    PangoLayout          *text_layout;
    gchar                *text_buffer;

    gsize                 buffer_size;

    gsize                 hex_cursor;
    gsize                 text_cursor;

    ChirurgienViewType    selected_view;

    gsize                 raw_scroll_offset;
    gsize                 formatted_scroll_offset;

    guchar               *contents;
    gsize                 contents_size;
    gsize                 formatted_contents_size;

    GtkAdjustment        *adjustment;
};

G_DEFINE_TYPE (ChirurgienEditor, chirurgien_editor, GTK_TYPE_WIDGET)

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

static void
handle_click (GtkGestureClick *gesture,
              gint             n_press,
              gdouble          x,
              gdouble          y,
              gpointer         user_data)
{
    ChirurgienEditor *editor;
    GtkWidget *clicked_view;
    gint byte_index;

    editor = user_data;

    if (n_press == 1)
    {
        clicked_view = gtk_event_controller_get_widget
                              (GTK_EVENT_CONTROLLER (gesture));

        if (pango_layout_xy_to_index (editor->hex_layout,
                                      x * PANGO_SCALE,
                                      y * PANGO_SCALE,
                                      &byte_index,
                                      NULL))
        {
            if (editor->hex_view == clicked_view)
            {
                editor->selected_view = CHIRURGIEN_HEX_VIEW;
                editor->hex_cursor = editor->formatted_scroll_offset;

                if (byte_index % 3 == 2)
                    editor->hex_cursor += byte_index - 1;
                else
                    editor->hex_cursor += byte_index;
                editor->text_cursor = (byte_index - (byte_index % 3));
            }
            else if (editor->text_view == clicked_view)
            {
                editor->selected_view = CHIRURGIEN_TEXT_VIEW;
                editor->text_cursor = editor->formatted_scroll_offset;

                editor->text_cursor += (byte_index - (byte_index % 3));
                editor->hex_cursor = editor->text_cursor;
            }
        }

        gtk_widget_grab_focus (clicked_view);
        gtk_widget_queue_draw (editor->hex_view);
        gtk_widget_queue_draw (editor->text_view);
    }
}

static void
cursor_forward (ChirurgienEditor *editor,
                guint             count,
                gboolean          auto_scroll)
{
    gsize last_hex_index, last_text_index, last_buffer_index;

    last_hex_index = editor->formatted_contents_size - 2;
    last_text_index = editor->formatted_contents_size - 3;
    last_buffer_index = editor->formatted_scroll_offset + editor->buffer_size;

    if (editor->selected_view == CHIRURGIEN_HEX_VIEW)
    {
        editor->hex_cursor += count;

        if (editor->hex_cursor % 3 == 2)
            editor->hex_cursor++;

        if (editor->hex_cursor >= last_hex_index)
            editor->hex_cursor = last_hex_index;

        editor->text_cursor = editor->hex_cursor - (editor->hex_cursor % 3);

    }
    else if (editor->selected_view == CHIRURGIEN_TEXT_VIEW)
    {
        editor->text_cursor += count;
        if (editor->text_cursor % 3)
            editor->text_cursor += 3 - (editor->text_cursor % 3);

        if (editor->text_cursor >= last_text_index)
            editor->text_cursor = last_text_index;

        editor->hex_cursor = editor->text_cursor;
    }

    if (auto_scroll && editor->hex_cursor > last_buffer_index)
        gtk_adjustment_set_value (editor->adjustment,
              gtk_adjustment_get_value (editor->adjustment) + 1);
}

static void
cursor_backward (ChirurgienEditor *editor,
                 guint             count,
                 gboolean          auto_scroll)
{
    if (editor->selected_view == CHIRURGIEN_HEX_VIEW)
    {
        if (editor->hex_cursor < count)
            editor->hex_cursor = 0;
        else
            editor->hex_cursor -= count;

        if (editor->hex_cursor % 3 == 2)
            editor->hex_cursor--;

        editor->text_cursor = editor->hex_cursor - (editor->hex_cursor % 3);
    }
    else if (editor->selected_view == CHIRURGIEN_TEXT_VIEW)
    {
        if (editor->text_cursor < count)
            editor->text_cursor = 0;
        else
            editor->text_cursor -= count;

        editor->text_cursor -= editor->text_cursor % 3;
        editor->hex_cursor = editor->text_cursor;
    }

    if (auto_scroll && editor->hex_cursor < editor->formatted_scroll_offset)
        gtk_adjustment_set_value (editor->adjustment,
              gtk_adjustment_get_value (editor->adjustment) - 1);
}

static gboolean
handle_key_event (G_GNUC_UNUSED GtkEventControllerKey *controller,
                  guint           keyval,
                  G_GNUC_UNUSED guint                  keycode,
                  GdkModifierType state,
                  gpointer        user_data)
{
    ChirurgienEditor *editor;
    gboolean key_handled = TRUE;

    gsize byte_index;

    editor = user_data;

    switch (keyval)
    {
        case GDK_KEY_KP_Left:
        case GDK_KEY_Left:
        cursor_backward (editor, 1, TRUE);

        break;
        case GDK_KEY_KP_Right:
        case GDK_KEY_Right:
        cursor_forward (editor, 1, TRUE);

        break;
        case GDK_KEY_KP_Up:
        case GDK_KEY_Up:
        cursor_backward (editor, editor->line_length, TRUE);

        break;
        case GDK_KEY_KP_Down:
        case GDK_KEY_Down:
        cursor_forward (editor, editor->line_length, TRUE);

        break;
        case GDK_KEY_KP_Page_Up:
        case GDK_KEY_Page_Up:
        cursor_backward (editor, gtk_adjustment_get_page_increment (editor->adjustment) * editor->line_length, FALSE);
        gtk_adjustment_set_value (editor->adjustment,
              gtk_adjustment_get_value (editor->adjustment) - gtk_adjustment_get_page_increment (editor->adjustment));

        break;
        case GDK_KEY_KP_Page_Down:
        case GDK_KEY_Page_Down:
        cursor_forward (editor, gtk_adjustment_get_page_increment (editor->adjustment) * editor->line_length, FALSE);
        gtk_adjustment_set_value (editor->adjustment,
              gtk_adjustment_get_value (editor->adjustment) + gtk_adjustment_get_page_increment (editor->adjustment));

        break;
        case GDK_KEY_KP_Home:
        case GDK_KEY_Home:
        if (state & GDK_CONTROL_MASK)
        {
            cursor_backward (editor, editor->hex_cursor, FALSE);
            gtk_adjustment_set_value (editor->adjustment, gtk_adjustment_get_lower (editor->adjustment));
        }
        else
        {
            cursor_backward (editor, editor->hex_cursor % editor->line_length, FALSE);
        }

        break;
        case GDK_KEY_KP_End:
        case GDK_KEY_End:
        if (state & GDK_CONTROL_MASK)
        {
            cursor_forward (editor, editor->formatted_contents_size - editor->text_cursor, FALSE);
            gtk_adjustment_set_value (editor->adjustment, gtk_adjustment_get_upper (editor->adjustment));
        }
        else
        {
            byte_index = editor->line_length - (editor->text_cursor % editor->line_length);
            if (byte_index > 4)
                cursor_forward (editor, byte_index - 4, FALSE);
        }

        break;
        default:
        if (editor->selected_view == CHIRURGIEN_HEX_VIEW)
        {
            byte_index = editor->hex_cursor / 3;
            keyval = gdk_keyval_to_upper (keyval);

            if ((keyval >= GDK_KEY_0 && keyval <= GDK_KEY_9) ||
               (keyval >= GDK_KEY_A && keyval <= GDK_KEY_F))
            {
                if (!(editor->hex_cursor % 3))
                    editor->contents[ byte_index ] =
                                 (g_ascii_xdigit_value (keyval) << 4) +
                                 (0x0F & editor->contents[ byte_index ]);
                else
                    editor->contents[ byte_index ] =
                                 (g_ascii_xdigit_value (keyval)) +
                                 (0xF0 & editor->contents[ byte_index ]);
                cursor_forward (editor, 1, TRUE);
            }
            else if (keyval >= GDK_KEY_KP_0 && keyval <= GDK_KEY_KP_9)
            {
                if (!(editor->hex_cursor % 3))
                    editor->contents[ byte_index ] =
                                 ((keyval - GDK_KEY_KP_0) << 4) +
                                 (0x0F & editor->contents[ byte_index ]);
                else
                    editor->contents[ byte_index ] =
                                 (keyval - GDK_KEY_KP_0) +
                                 (0xF0 & editor->contents[ byte_index ]);
                cursor_forward (editor, 1, TRUE);
            }
            else if (keyval == GDK_KEY_BackSpace)
            {
                if (!(editor->hex_cursor % 3))
                    editor->contents[ byte_index ] = 0x0F & editor->contents[ byte_index ];
                else
                    editor->contents[ byte_index ] = 0xF0 & editor->contents[ byte_index ];
                cursor_backward (editor, 1, TRUE);
            }
            else
            {
                key_handled = FALSE;
            }
        }
        else if (editor->selected_view == CHIRURGIEN_TEXT_VIEW)
        {
            byte_index = editor->text_cursor / 3;
            if (keyval >= GDK_KEY_space && keyval <= GDK_KEY_asciitilde)
            {
                editor->contents[ byte_index ] = keyval;
                cursor_forward (editor, 1, TRUE);
            }
            else if (keyval >= GDK_KEY_KP_0 && keyval <= GDK_KEY_KP_9)
            {
                editor->contents[ byte_index ] = (keyval - GDK_KEY_KP_0) + '0';
                cursor_forward (editor, 1, TRUE);
            }
            else if (keyval == GDK_KEY_BackSpace)
            {
                editor->contents[ byte_index ] = '\0';
                cursor_backward (editor, 1, TRUE);
            }
            else
            {
                key_handled = FALSE;
            }
        }
    }

    if (key_handled)
    {
        gtk_widget_queue_draw (editor->hex_view);
        gtk_widget_queue_draw (editor->text_view);
    }

    return key_handled;
}

static void
scroll_editor (GtkAdjustment *adjustment,
               gpointer       user_data)
{
    ChirurgienEditor *editor;

    editor = user_data;

    editor->raw_scroll_offset = editor->formatted_scroll_offset = gtk_adjustment_get_value (adjustment);
    editor->raw_scroll_offset *= editor->line_length / 3;
    editor->formatted_scroll_offset *= editor->line_length;

    gtk_widget_queue_draw (GTK_WIDGET (editor->hex_view));
    gtk_widget_queue_draw (GTK_WIDGET (editor->text_view));
}

static void
resize_editor (G_GNUC_UNUSED GtkDrawingArea *area,
               gint     width,
               gint     height,
               gpointer user_data)
{
    ChirurgienEditor *editor;
    gint line_length, visible_lines, total_lines;
    gsize buffer_size;

    editor = user_data;

    if (!width || !height)
    {
        editor->line_length = editor->buffer_size = 0;

        if (editor->hex_buffer)
            g_free (g_steal_pointer (&editor->hex_buffer));
        if (editor->text_buffer)
            g_free (g_steal_pointer (&editor->text_buffer));

        return;
    }

    width *= PANGO_SCALE;
    height *= PANGO_SCALE;

    editor->line_length = line_length = width / editor->char_width;
    switch (line_length % 3)
    {
        case 2:
            editor->line_length = ++line_length;
            break;
        case 1:
            editor->line_length = --line_length;
            break;
        default:
            editor->line_length = line_length;
    }

    visible_lines = height / editor->line_height;

    buffer_size = line_length * visible_lines;

    if (editor->buffer_size != buffer_size)
    {
        editor->buffer_size = buffer_size;

        total_lines = editor->formatted_contents_size / line_length;

        if(editor->formatted_contents_size % line_length)
            total_lines++;

        gtk_adjustment_configure (editor->adjustment,
                                  gtk_adjustment_get_value (editor->adjustment),
                                  0,
                                  total_lines,
                                  1,
                                  visible_lines - 1,
                                  visible_lines);

        if (editor->hex_buffer)
            g_free (g_steal_pointer (&editor->hex_buffer));
        if (editor->text_buffer)
            g_free (g_steal_pointer (&editor->text_buffer));

        editor->hex_buffer = g_malloc (editor->buffer_size);
        editor->text_buffer = g_malloc (editor->buffer_size);
    }
}

static void
draw_hex_cursor (ChirurgienEditor *editor)
{
    PangoAttrList *attribute_list;
    PangoAttribute *attribute;

    attribute_list = pango_attr_list_new ();

    if (editor->selected_view == CHIRURGIEN_HEX_VIEW)
    {
        attribute = pango_attr_background_new (12079,
                                               46260,
                                               61166);
        attribute->start_index = editor->hex_cursor - editor->formatted_scroll_offset;
        attribute->end_index = attribute->start_index + 1;
    }
    else
    {
        attribute = pango_attr_foreground_new (12079,
                                               46260,
                                               61166);
        attribute->start_index = editor->hex_cursor - editor->formatted_scroll_offset;
        attribute->end_index = attribute->start_index + 2;
    }

    pango_attr_list_insert (attribute_list, attribute);
    pango_layout_set_attributes (editor->hex_layout, attribute_list);

    pango_attr_list_unref (attribute_list);
}

static void
draw_text_cursor (ChirurgienEditor *editor)
{
    PangoAttrList *attribute_list;
    PangoAttribute *attribute;

    attribute_list = pango_attr_list_new ();

    if (editor->selected_view == CHIRURGIEN_TEXT_VIEW)
    {
        attribute = pango_attr_background_new (12079,
                                               46260,
                                               61166);
        attribute->start_index = editor->text_cursor - editor->formatted_scroll_offset;
        attribute->end_index = attribute->start_index + 1;
    }
    else
    {
        attribute = pango_attr_foreground_new (12079,
                                               46260,
                                               61166);
        attribute->start_index = editor->text_cursor - editor->formatted_scroll_offset;
        attribute->end_index = attribute->start_index + 1;
    }

    pango_attr_list_insert (attribute_list, attribute);
    pango_layout_set_attributes (editor->text_layout, attribute_list);

    pango_attr_list_unref (attribute_list);
}

static void
draw_hex_view (GtkDrawingArea *drawing_area,
               cairo_t        *cr,
               G_GNUC_UNUSED gint width,
               G_GNUC_UNUSED gint height,
               gpointer        user_data)
{
    ChirurgienEditor *editor;
    GdkRGBA color;
    GtkStyleContext *context;

    editor = user_data;

    context = gtk_widget_get_style_context (GTK_WIDGET (drawing_area));

    gtk_style_context_get_color (context, &color);
    gdk_cairo_set_source_rgba (cr, &color);

    pango_layout_set_attributes (editor->hex_layout, NULL);

    chirurgien_utils_hex_print (editor->hex_buffer,
                                editor->contents,
                                editor->raw_scroll_offset,
                                editor->buffer_size,
                                editor->contents_size,
                                editor->line_length);
    draw_hex_cursor (editor);

    pango_layout_set_text (editor->hex_layout, editor->hex_buffer, editor->buffer_size);
    pango_cairo_show_layout (cr, editor->hex_layout);
}

static void
draw_text_view (GtkDrawingArea *drawing_area,
                cairo_t        *cr,
                G_GNUC_UNUSED gint width,
                G_GNUC_UNUSED gint height,
                gpointer        user_data)
{
    ChirurgienEditor *editor;
    GdkRGBA color;
    GtkStyleContext *context;

    editor = user_data;

    context = gtk_widget_get_style_context (GTK_WIDGET (drawing_area));

    gtk_style_context_get_color (context, &color);
    gdk_cairo_set_source_rgba (cr, &color);

    pango_layout_set_attributes (editor->text_layout, NULL);

    chirurgien_utils_text_print (editor->text_buffer,
                                 editor->contents,
                                 editor->raw_scroll_offset,
                                 editor->buffer_size,
                                 editor->contents_size,
                                 editor->line_length);
    draw_text_cursor (editor);

    pango_layout_set_text (editor->text_layout, editor->text_buffer, editor->buffer_size);
    pango_cairo_show_layout (cr, editor->text_layout);
}

static void
chirurgien_editor_dispose (GObject *object)
{
    ChirurgienEditor *editor;

    editor = CHIRURGIEN_EDITOR (object);

    gtk_widget_unparent (gtk_widget_get_first_child (GTK_WIDGET (editor)));
    gtk_widget_unparent (gtk_widget_get_first_child (GTK_WIDGET (editor)));
    gtk_widget_unparent (gtk_widget_get_first_child (GTK_WIDGET (editor)));

    g_object_unref (g_steal_pointer (&editor->hex_layout));
    g_object_unref (g_steal_pointer (&editor->text_layout));

    g_free (g_steal_pointer (&editor->contents));
    g_free (g_steal_pointer (&editor->hex_buffer));
    g_free (g_steal_pointer (&editor->text_buffer));

    G_OBJECT_CLASS (chirurgien_editor_parent_class)->dispose (object);
}

static void
chirurgien_editor_class_init (ChirurgienEditorClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    G_OBJECT_CLASS (klass)->dispose = chirurgien_editor_dispose;

    gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);

    gtk_widget_class_set_template_from_resource (widget_class,
                                 "/io/github/leonardschardijn/chirurgien/ui/chirurgien-editor.ui");
    gtk_widget_class_bind_template_child (widget_class, ChirurgienEditor, hex_view);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienEditor, text_view);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienEditor, adjustment);
}

static void
chirurgien_editor_init (ChirurgienEditor *editor)
{
    PangoContext *context;
    PangoFontMetrics *font_metrics;
    GtkEventController *controller;

    gtk_widget_init_template (GTK_WIDGET (editor));

    context = gtk_widget_get_pango_context (GTK_WIDGET (editor->hex_view));

    font_metrics = pango_context_get_metrics (context, pango_context_get_font_description (context),
                                              pango_context_get_language (context));

    editor->char_width = pango_font_metrics_get_approximate_digit_width (font_metrics);
    editor->line_height = pango_font_metrics_get_ascent (font_metrics) +
                          pango_font_metrics_get_descent (font_metrics);

    pango_font_metrics_unref (font_metrics);

    editor->hex_layout = gtk_widget_create_pango_layout (GTK_WIDGET (editor->hex_view), NULL);
    editor->text_layout = gtk_widget_create_pango_layout (GTK_WIDGET (editor->text_view), NULL);

    gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (editor->hex_view), draw_hex_view, editor, NULL);
    gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (editor->text_view), draw_text_view, editor, NULL);
    g_signal_connect (editor->hex_view, "resize", G_CALLBACK (resize_editor), editor);

    g_signal_connect (editor->adjustment, "value-changed", G_CALLBACK (scroll_editor), editor);

    controller = GTK_EVENT_CONTROLLER (gtk_event_controller_scroll_new
                                      (GTK_EVENT_CONTROLLER_SCROLL_VERTICAL |
                                       GTK_EVENT_CONTROLLER_SCROLL_DISCRETE));
    g_signal_connect (controller, "scroll", G_CALLBACK (handle_scroll_event), editor->adjustment);
    gtk_widget_add_controller (editor->hex_view, controller);
    controller = GTK_EVENT_CONTROLLER (gtk_event_controller_scroll_new
                                      (GTK_EVENT_CONTROLLER_SCROLL_VERTICAL |
                                       GTK_EVENT_CONTROLLER_SCROLL_DISCRETE));
    g_signal_connect (controller, "scroll", G_CALLBACK (handle_scroll_event), editor->adjustment);
    gtk_widget_add_controller (editor->text_view, controller);

    controller = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (controller), 0);
    g_signal_connect (controller, "pressed", G_CALLBACK (handle_click), editor);
    gtk_widget_add_controller (editor->hex_view, controller);
    controller = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (controller), 0);
    g_signal_connect (controller, "pressed", G_CALLBACK (handle_click), editor);
    gtk_widget_add_controller (editor->text_view, controller);

    controller = gtk_event_controller_key_new ();
    g_signal_connect (controller, "key-pressed", G_CALLBACK (handle_key_event), editor);
    gtk_widget_add_controller (GTK_WIDGET (editor->hex_view), controller);
    controller = gtk_event_controller_key_new ();
    g_signal_connect (controller, "key-pressed", G_CALLBACK (handle_key_event), editor);
    gtk_widget_add_controller (GTK_WIDGET (editor->text_view), controller);

    editor->selected_view = CHIRURGIEN_HEX_VIEW;

    editor->hex_buffer = NULL;
    editor->text_buffer = NULL;
    editor->buffer_size = 0;

    editor->hex_cursor = 0;
    editor->text_cursor = 0;

    editor->raw_scroll_offset = 0;
    editor->formatted_scroll_offset = 0;

    editor->contents_size = 0;
}

/*** Public API ***/

GtkWidget *
chirurgien_editor_new ()
{
    return GTK_WIDGET (g_object_new (CHIRURGIEN_TYPE_EDITOR,
                       NULL));
}

void
chirurgien_editor_set_contents (ChirurgienEditor *editor,
                                const guchar     *contents,
                                gsize             contents_size)
{
    editor->contents = g_malloc (contents_size);
    memcpy (editor->contents, contents, contents_size);

    editor->contents_size = contents_size;
    editor->formatted_contents_size = contents_size * 3;
}

const guchar *
chirurgien_editor_get_contents (ChirurgienEditor *editor)
{
    return editor->contents;
}

gsize
chirurgien_editor_get_contents_size (ChirurgienEditor *editor)
{
    return editor->contents_size;
}
