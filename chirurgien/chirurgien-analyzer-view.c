/* chirurgien-analyzer-view.c
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

#include "chirurgien-analyzer-view.h"

#include <chirurgien-analyzer.h>

#include "chirurgien-navigation-button.h"


struct _ChirurgienAnalyzerView
{
    GtkPaned       parent_instance;

    GtkNotebook    *hex_text_notebook;
    GtkNotebook    *description_notebook;

    GtkTextView    *hex_view;
    GtkTextView    *text_view;
    GtkButtonBox   *hex_navigation;
    GtkButtonBox   *text_navigation;
    GtkGrid        *file_description;

    gchar          *file_path;
    guchar         *file_contents;
    gsize          file_size;

    gint           bytes_per_line;

    GSettings      *preferences_settings;
    GSettings      *window_settings;
};

G_DEFINE_TYPE (ChirurgienAnalyzerView, chirurgien_analyzer_view, GTK_TYPE_PANED)

static void
chirurgien_analyzer_view_dispose (GObject *object)
{
    ChirurgienAnalyzerView *view;

    view = CHIRURGIEN_ANALYZER_VIEW (object);
    g_free (view->file_contents);
    g_free (view->file_path);
    view->file_contents = NULL;
    view->file_path = NULL;

    G_OBJECT_CLASS (chirurgien_analyzer_view_parent_class)->dispose (object);
}

static void
chirurgien_analyzer_view_class_init (ChirurgienAnalyzerViewClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    GtkWidgetClass *gtkwidget_class = GTK_WIDGET_CLASS (class);

    gobject_class->dispose = chirurgien_analyzer_view_dispose;

    gtk_widget_class_set_template_from_resource (gtkwidget_class,
                                         "/io/github/leonardschardijn/chirurgien/ui/chirurgien-analyzer-view.ui");
    gtk_widget_class_bind_template_child (gtkwidget_class, ChirurgienAnalyzerView, hex_text_notebook);
    gtk_widget_class_bind_template_child (gtkwidget_class, ChirurgienAnalyzerView, description_notebook);
    gtk_widget_class_bind_template_child (gtkwidget_class, ChirurgienAnalyzerView, hex_view);
    gtk_widget_class_bind_template_child (gtkwidget_class, ChirurgienAnalyzerView, text_view);
    gtk_widget_class_bind_template_child (gtkwidget_class, ChirurgienAnalyzerView, hex_navigation);
    gtk_widget_class_bind_template_child (gtkwidget_class, ChirurgienAnalyzerView, text_navigation);
    gtk_widget_class_bind_template_child (gtkwidget_class, ChirurgienAnalyzerView, file_description);
}

static void
chirurgien_analyzer_view_init (ChirurgienAnalyzerView *view)
{
    GtkAdjustment *scroll1, *scroll2;

    gtk_widget_init_template (GTK_WIDGET (view));

    scroll1 = gtk_scrollable_get_hadjustment (GTK_SCROLLABLE (view->hex_view));
    scroll2 = gtk_scrollable_get_hadjustment (GTK_SCROLLABLE (view->text_view));
    g_object_bind_property (scroll1, "value", scroll2, "value", G_BINDING_BIDIRECTIONAL);

    scroll1 = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (view->hex_view));
    scroll2 = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (view->text_view));
    g_object_bind_property (scroll1, "value", scroll2, "value", G_BINDING_BIDIRECTIONAL);

    scroll1 = gtk_scrollable_get_vadjustment
            (GTK_SCROLLABLE (gtk_widget_get_parent
            (gtk_widget_get_parent (GTK_WIDGET (view->hex_navigation)))));
    scroll2 = gtk_scrollable_get_vadjustment
            (GTK_SCROLLABLE (gtk_widget_get_parent
            (gtk_widget_get_parent (GTK_WIDGET (view->text_navigation)))));
    g_object_bind_property (scroll1, "value", scroll2, "value", G_BINDING_BIDIRECTIONAL);
}

ChirurgienAnalyzerView *
chirurgien_analyzer_view_new (ChirurgienWindow *window)
{
    ChirurgienAnalyzerView *view = CHIRURGIEN_ANALYZER_VIEW (g_object_new (CHIRURGIEN_ANALYZER_VIEW_TYPE, NULL));

    view->preferences_settings = window->preferences_settings;
    view->window_settings = window->window_settings;

    return view;
}

static void
file_print (ChirurgienAnalyzerView *view,
            gchar *hex_contents,
            gchar *text_contents,
            gsize hex_text_size,
            guchar *file_contents,
            gsize file_size)
{
    const gchar *hex_chars = "0123456789ABCDEF";
    gchar *hex_pointer, *text_pointer;
    gsize i;

    hex_pointer = hex_contents;
    text_pointer = text_contents;
    for (i = 0; i < file_size; ++i)
    {
        *hex_pointer++ = hex_chars[ (file_contents[i] & 0xF0) >> 4 ];
        *hex_pointer++ = hex_chars[ (file_contents[i] & 0x0F) ];
        *hex_pointer++ = ' ';

        if (g_ascii_isgraph (file_contents[i]))
            *text_pointer++ = file_contents[i];
        else
            *text_pointer++ = '.';
        *text_pointer++ = ' ';
        *text_pointer++ = ' ';
    }

    view->bytes_per_line = g_settings_get_int (view->preferences_settings, "bytes-per-line");
    view->bytes_per_line *= 3;

    hex_pointer = hex_contents;
    text_pointer = text_contents;
    for (i = view->bytes_per_line - 1; i < hex_text_size; i += view->bytes_per_line)
    {
        *(hex_pointer + i) = '\n';
        *(text_pointer + i) = '\n';
    }
}

static gboolean
scroll_to_mark (__attribute__((unused)) GtkWidget *widget,
                __attribute__((unused)) GdkEvent *event,
                gpointer   user_data)
{
    ChirurgienAnalyzerView *view;
    ChirurgienNavigationButton *button;

    view = CHIRURGIEN_ANALYZER_VIEW (user_data);
    button = CHIRURGIEN_NAVIGATION_BUTTON (widget);

    if (!gtk_notebook_get_current_page (view->hex_text_notebook))
    {
        gtk_text_view_scroll_to_mark (view->hex_view, chirurgien_navigation_button_get_mark (button),
                                      0.005, TRUE, 0.0, 0.0);
    }
    else
    {
        gtk_text_view_scroll_to_mark (view->text_view, chirurgien_navigation_button_get_mark (button),
                                      0.005, TRUE, 0.0, 0.0);
    }

    return FALSE;
}

static void
build_navigation_buttons (ChirurgienAnalyzerView *view, GSList *hex_marks, GSList *text_marks)
{
    GSList *index;
    GtkWidget *button;
    GtkStyleContext *context;

    for (index = hex_marks;
    index != NULL;
    index = index->next)
    {
        button = chirurgien_navigation_button_new (GTK_TEXT_MARK (index->data));
        index = index->next;
        gtk_button_set_label (GTK_BUTTON (button), index->data);
        g_signal_connect (button, "button-release-event", G_CALLBACK (scroll_to_mark), view);
        context = gtk_widget_get_style_context (button);
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_FLAT);
        gtk_box_pack_start (GTK_BOX (view->hex_navigation), button, FALSE, FALSE, 0);
    }
    gtk_widget_show_all (GTK_WIDGET (view->hex_navigation));

    for (index = text_marks;
    index != NULL;
    index = index->next)
    {
        button = chirurgien_navigation_button_new (GTK_TEXT_MARK (index->data));
        index = index->next;
        gtk_button_set_label (GTK_BUTTON (button), index->data);
        g_signal_connect (button, "button-release-event", G_CALLBACK (scroll_to_mark), view);
        context = gtk_widget_get_style_context (button);
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_FLAT);
        gtk_box_pack_start (GTK_BOX (view->text_navigation), button, FALSE, FALSE, 0);
    }
    gtk_widget_show_all (GTK_WIDGET (view->text_navigation));

    if (!g_settings_get_boolean (view->preferences_settings, "show-navigation"))
    {
        gtk_widget_hide (gtk_widget_get_parent (gtk_widget_get_parent
                        (gtk_widget_get_parent (GTK_WIDGET (view->hex_navigation)))));
        gtk_widget_hide (gtk_widget_get_parent (gtk_widget_get_parent
                        (gtk_widget_get_parent (GTK_WIDGET (view->text_navigation)))));
    }
}

static void
destroy_navigation_buttons (GtkWidget *widget,
                            __attribute__((unused)) gpointer data)
{
    gtk_widget_destroy (widget);
}

void
chirurgien_analyzer_view_prepare_analysis (ChirurgienAnalyzerView *view,
                                           GFile *file)
{
    g_autoptr (GFileInputStream) file_input;
    g_autoptr (GFileInfo) file_info;

    g_autofree gchar *hex_contents = NULL;
    g_autofree gchar *text_contents = NULL;
    guchar *file_contents;

    gsize file_size, hex_text_size;

    GtkTextBuffer *hex_buffer, *text_buffer;

    view->file_path = g_file_get_path (file);
    file_input = g_file_read (file, NULL, NULL);
    file_info = g_file_input_stream_query_info (file_input, G_FILE_ATTRIBUTE_STANDARD_SIZE, NULL, NULL);
    file_size = g_file_info_get_size (file_info);

    /* Limit file size to 10 MiB */
    if (file_size > 10485760)
        file_size = 10485760;

    hex_text_size = file_size * 3;
    file_contents = g_malloc (file_size);
    hex_contents = g_malloc (hex_text_size);
    text_contents = g_malloc (hex_text_size);

    g_input_stream_read_all (G_INPUT_STREAM (file_input), file_contents, file_size, NULL, NULL, NULL);

    hex_buffer = gtk_text_view_get_buffer (view->hex_view);
    text_buffer = gtk_text_view_get_buffer (view->text_view);

    file_print (view, hex_contents, text_contents, hex_text_size, file_contents, file_size);
    gtk_text_buffer_set_text (hex_buffer, hex_contents, hex_text_size - 1);
    gtk_text_buffer_set_text (text_buffer, text_contents, hex_text_size - 1);

    view->file_contents = file_contents;
    view->file_size = file_size;

    g_settings_bind (view->window_settings, "position", view, "position", G_SETTINGS_BIND_DEFAULT);

    g_settings_bind (view->preferences_settings, "file-justification",
                     view->hex_view, "justification", G_SETTINGS_BIND_GET);
    g_settings_bind (view->preferences_settings, "file-justification",
                     view->text_view, "justification", G_SETTINGS_BIND_GET);

    /* Bind the visible property of the GtkScrolledWindow */
    /* GtkScrolledWindow <- GtkViewport <- GtkFrame <- GtkButtonBox (hex/text_navigation) */
    g_settings_bind (view->preferences_settings, "show-navigation",
                     gtk_widget_get_parent (gtk_widget_get_parent
                     (gtk_widget_get_parent (GTK_WIDGET (view->hex_navigation)))),
                     "visible", G_SETTINGS_BIND_GET);
    g_settings_bind (view->preferences_settings, "show-navigation",
                     gtk_widget_get_parent (gtk_widget_get_parent
                     (gtk_widget_get_parent (GTK_WIDGET (view->text_navigation)))),
                     "visible", G_SETTINGS_BIND_GET);
}

void
chirurgien_analyzer_view_execute_analysis (ChirurgienAnalyzerView *view,
                                           gboolean initial_analysis)
{
    AnalyzerFile file;

    file.hex_buffer = gtk_text_view_get_buffer (view->hex_view);
    file.text_buffer = gtk_text_view_get_buffer (view->text_view);
    file.hex_navigation_marks = NULL;
    file.text_navigation_marks = NULL;
    file.file_description = initial_analysis ? view->file_description : NULL;
    file.description_notebook = initial_analysis ? view->description_notebook : NULL;
    file.file_contents = view->file_contents;
    file.file_size = view->file_size;
    file.file_contents_index = 0;
    file.hex_buffer_index = 0;
    file.description_lines_count = 0;

    chirurgien_analyzer_analyze (&file);

    build_navigation_buttons (view, file.hex_navigation_marks, file.text_navigation_marks);
    g_slist_free (file.hex_navigation_marks);
    g_slist_free (file.text_navigation_marks);
}

void chirurgien_analyzer_view_update_lines (ChirurgienAnalyzerView *view)
{
    gint bytes_prev;
    gsize i;
    gchar *hex_pointer, *text_pointer;

    GtkTextBuffer *hex_buffer, *text_buffer;
    GtkTextIter start, end;
    g_autofree gchar *hex_contents = NULL;
    g_autofree gchar *text_contents = NULL;
    gsize hex_text_size;

    bytes_prev = view->bytes_per_line;
    view->bytes_per_line = g_settings_get_int (view->preferences_settings, "bytes-per-line");
    view->bytes_per_line *= 3;

    hex_buffer = gtk_text_view_get_buffer (view->hex_view);
    text_buffer = gtk_text_view_get_buffer (view->text_view);

    gtk_text_buffer_get_iter_at_offset (hex_buffer, &start, 0);
    gtk_text_buffer_get_iter_at_offset (hex_buffer, &end, -1);
    hex_contents = gtk_text_buffer_get_text (hex_buffer, &start, &end, FALSE);

    gtk_text_buffer_get_iter_at_offset (text_buffer, &start, 0);
    gtk_text_buffer_get_iter_at_offset (text_buffer, &end, -1);
    text_contents = gtk_text_buffer_get_text (text_buffer, &start, &end, FALSE);

    hex_text_size = (view->file_size * 3) - 1;

    hex_pointer = hex_contents;
    text_pointer = text_contents;
    for (i = bytes_prev - 1; i < hex_text_size; i += bytes_prev)
    {
        *(hex_pointer + i) = ' ';
        *(text_pointer + i) = ' ';
    }

    hex_pointer = hex_contents;
    text_pointer = text_contents;
    for (i = view->bytes_per_line - 1; i < hex_text_size; i += view->bytes_per_line)
    {
        *(hex_pointer + i) = '\n';
        *(text_pointer + i) = '\n';
    }

    hex_buffer = gtk_text_buffer_new (NULL);
    gtk_text_buffer_set_text (hex_buffer, hex_contents, hex_text_size);
    gtk_text_view_set_buffer (view->hex_view, hex_buffer);

    text_buffer = gtk_text_buffer_new (NULL);
    gtk_text_buffer_set_text (text_buffer, text_contents, hex_text_size);
    gtk_text_view_set_buffer (view->text_view, text_buffer);

    gtk_container_foreach (GTK_CONTAINER (view->hex_navigation), destroy_navigation_buttons, NULL);
    gtk_container_foreach (GTK_CONTAINER (view->text_navigation), destroy_navigation_buttons, NULL);

    chirurgien_analyzer_view_execute_analysis (view, FALSE);
}

const gchar *
chirurgien_analyzer_view_get_file_path (ChirurgienAnalyzerView *view)
{
    return view->file_path;
}

GtkNotebook *
chirurgien_analyzer_view_get_hex_text_notebook (ChirurgienAnalyzerView *view)
{
    return view->hex_text_notebook;
}
