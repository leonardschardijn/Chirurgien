/* chirurgien-analyzer-info-dialog.c
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

#include "chirurgien-analyzer-info-dialog.h"

#include <glib/gi18n.h>


struct _ChirurgienAnalyzerInfoDialog
{
    GtkDialog parent_instance;

    GtkStack *stack;
    GtkRevealer *revealer;
    GtkButton *button;
};

G_DEFINE_TYPE (ChirurgienAnalyzerInfoDialog, chirurgien_analyzer_info_dialog, GTK_TYPE_DIALOG)

static void
chirurgien_analyzer_info_dialog_class_init(ChirurgienAnalyzerInfoDialogClass *class)
{
    GtkWidgetClass *gtkwidget_class = GTK_WIDGET_CLASS (class);

    gtk_widget_class_set_template_from_resource (gtkwidget_class,
                                                 "/io/github/leonardschardijn/chirurgien/ui/chirurgien-analyzer-info-dialog.ui");
    gtk_widget_class_bind_template_child (gtkwidget_class, ChirurgienAnalyzerInfoDialog, stack);
}

static void
show_note (__attribute__((unused)) GtkButton *button,
           gpointer user_data)
{
    ChirurgienAnalyzerInfoDialog *dialog = user_data;

    gtk_revealer_set_reveal_child (dialog->revealer, !gtk_revealer_get_child_revealed (dialog->revealer));
}

static void
chirurgien_analyzer_info_dialog_init (ChirurgienAnalyzerInfoDialog *dialog)
{
    GtkBuilder *builder;
    GtkWidget *widget, *icon;

    gtk_widget_init_template (GTK_WIDGET (dialog));

    builder = gtk_builder_new_from_resource ("/io/github/leonardschardijn/chirurgien/analyzer-info/intro-tab.ui");
    widget = GTK_WIDGET (gtk_builder_get_object (builder, "intro-tab"));
    gtk_stack_add_titled (dialog->stack, widget, "Introduction", _("Introduction"));
    dialog->revealer = GTK_REVEALER (gtk_builder_get_object (builder, "revealer"));;
    dialog->button = GTK_BUTTON (gtk_builder_get_object (builder, "button"));;
    icon = gtk_image_new_from_icon_name ("dialog-question-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image (dialog->button, icon);
    g_signal_connect (dialog->button, "clicked", G_CALLBACK (show_note), dialog);
    g_object_unref (builder);

    builder = gtk_builder_new_from_resource ("/io/github/leonardschardijn/chirurgien/analyzer-info/elf-tab.ui");
    widget = GTK_WIDGET (gtk_builder_get_object (builder, "elf-tab"));
    gtk_stack_add_titled (dialog->stack, widget, "ELF", "ELF");
    g_object_unref (builder);

    builder = gtk_builder_new_from_resource ("/io/github/leonardschardijn/chirurgien/analyzer-info/jpeg-tab.ui");
    widget = GTK_WIDGET (gtk_builder_get_object (builder, "jpeg-tab"));
    gtk_stack_add_titled (dialog->stack, widget, "JPEG", "JPEG");
    g_object_unref (builder);

    builder = gtk_builder_new_from_resource ("/io/github/leonardschardijn/chirurgien/analyzer-info/png-tab.ui");
    widget = GTK_WIDGET (gtk_builder_get_object (builder, "png-tab"));
    gtk_stack_add_titled (dialog->stack, widget, "PNG", "PNG");
    g_object_unref (builder);

    builder = gtk_builder_new_from_resource ("/io/github/leonardschardijn/chirurgien/analyzer-info/tiff-tab.ui");
    widget = GTK_WIDGET (gtk_builder_get_object (builder, "tiff-tab"));
    gtk_stack_add_titled (dialog->stack, widget, "TIFF", "TIFF");
    g_object_unref (builder);
}

GtkWidget *
chirurgien_analyzer_info_dialog_new (GtkWindow *parent)
{
    return GTK_WIDGET (g_object_new (CHIRURGIEN_ANALYZER_INFO_DIALOG_TYPE,
                       "transient-for", parent,
                       NULL));
}

