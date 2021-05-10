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

#include "chirurgien-colors.h"


struct _ChirurgienFormatsDialog
{
    GtkDialog      parent_instance;

    GtkStack      *formats;
    GtkRevealer   *revealer;
    GtkButton     *button;
};

G_DEFINE_TYPE (ChirurgienFormatsDialog, chirurgien_formats_dialog, GTK_TYPE_DIALOG)

static void
show_note (G_GNUC_UNUSED GtkButton *button,
           gpointer user_data)
{
    ChirurgienFormatsDialog *dialog = user_data;

    gtk_revealer_set_reveal_child (dialog->revealer, !gtk_revealer_get_child_revealed (dialog->revealer));
}

static void
apply_markup (GtkTextBuffer *buffer)
{
    GtkTextIter start, end;
    g_autofree gchar *markup;

    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);

    markup = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
    gtk_text_buffer_delete (buffer, &start, &end);

    gtk_text_buffer_insert_markup (buffer, &start, markup, -1);
}

static void
add_format (ChirurgienFormatsDialog *dialog,
            const gchar             *resource_path,
            const gchar             *format_name)
{
    GtkBuilder *builder;
    GtkWidget *format_info;
    GtkColorChooser *color;

    GtkTextBuffer *buffer;

    builder = gtk_builder_new_from_resource (resource_path);

    format_info = GTK_WIDGET (gtk_builder_get_object (builder, "format-info"));
    buffer = GTK_TEXT_BUFFER (gtk_builder_get_object (builder, "details"));

    apply_markup (buffer);

    for (gint i = 0; i < TOTAL_COLORS; i++)
    {
        color = GTK_COLOR_CHOOSER (gtk_builder_get_object (builder, color_names[i]));

        if (color)
            gtk_color_chooser_set_rgba (color, &colors[i]);
    }

    gtk_stack_add_titled (dialog->formats, format_info, format_name, format_name);
    g_object_unref (builder);
}

static void
chirurgien_formats_dialog_class_init (ChirurgienFormatsDialogClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                         "/io/github/leonardschardijn/chirurgien/ui/chirurgien-formats-dialog.ui");
    gtk_widget_class_bind_template_child (widget_class, ChirurgienFormatsDialog, formats);
}

static void
chirurgien_formats_dialog_init (ChirurgienFormatsDialog *dialog)
{
    GtkBuilder *builder;
    GtkWidget *intro;
    GtkTextBuffer *buffer;

    gtk_widget_init_template (GTK_WIDGET (dialog));

    builder = gtk_builder_new_from_resource ("/io/github/leonardschardijn/chirurgien/formats/intro-tab.ui");

    intro = GTK_WIDGET (gtk_builder_get_object (builder, "intro"));
    buffer = GTK_TEXT_BUFFER (gtk_builder_get_object (builder, "notes"));

    apply_markup (buffer);
    gtk_stack_add_titled (dialog->formats, intro, "Introduction", _("Introduction"));

    dialog->revealer = GTK_REVEALER (gtk_builder_get_object (builder, "revealer"));;
    dialog->button = GTK_BUTTON (gtk_builder_get_object (builder, "button"));;

    g_signal_connect (dialog->button, "clicked", G_CALLBACK (show_note), dialog);

    g_object_unref (builder);

    /* Formats */
    add_format (dialog, "/io/github/leonardschardijn/chirurgien/formats/cpio-tab.ui", "cpio");
    add_format (dialog, "/io/github/leonardschardijn/chirurgien/formats/elf-tab.ui", "ELF");
    add_format (dialog, "/io/github/leonardschardijn/chirurgien/formats/gif-tab.ui", "GIF");
    add_format (dialog, "/io/github/leonardschardijn/chirurgien/formats/jpeg-tab.ui", "JPEG");
    add_format (dialog, "/io/github/leonardschardijn/chirurgien/formats/png-tab.ui", "PNG");
    add_format (dialog, "/io/github/leonardschardijn/chirurgien/formats/tar-tab.ui", "tar");
    add_format (dialog, "/io/github/leonardschardijn/chirurgien/formats/tiff-tab.ui", "TIFF");
}

/*** Public API ***/

GtkWidget *
chirurgien_formats_dialog_new (GtkWindow *window)
{
    return GTK_WIDGET (g_object_new (CHIRURGIEN_TYPE_FORMATS_DIALOG,
                       "transient-for", window,
                       NULL));
}
