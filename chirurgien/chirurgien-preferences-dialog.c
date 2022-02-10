/* chirurgien-preferences-dialog.c
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

#include "chirurgien-preferences-dialog.h"

#include "chirurgien-globals.h"


struct _ChirurgienPreferencesDialog
{
    GtkDialog          parent_instance;

    GtkCheckButton    *navigation;
    GtkCheckButton    *description;
    GtkCheckButton    *auto_analysis;
    GtkCheckButton    *extra_buttons;

    GtkColorButton    *color0;
    GtkColorButton    *color1;
    GtkColorButton    *color2;
    GtkColorButton    *color3;
    GtkColorButton    *color4;
    GtkColorButton    *color5;
    GtkColorButton    *color6;
    GtkColorButton    *color7;
    GtkColorButton    *color8;

    GtkButton         *default_colors;

    GtkFontButton     *font_button;

    GtkCheckButton    *manual_lines;
    GtkSpinButton     *bytes;

    GSettings         *preferences_settings;
};

G_DEFINE_TYPE (ChirurgienPreferencesDialog, chirurgien_preferences_dialog, GTK_TYPE_DIALOG)

static void
set_colors (ChirurgienPreferencesDialog *dialog)
{
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog->color0), &chirurgien_colors[0]);
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog->color1), &chirurgien_colors[1]);
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog->color2), &chirurgien_colors[2]);
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog->color3), &chirurgien_colors[3]);
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog->color4), &chirurgien_colors[4]);
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog->color5), &chirurgien_colors[5]);
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog->color6), &chirurgien_colors[6]);
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog->color7), &chirurgien_colors[7]);
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog->color8), &chirurgien_colors[8]);
}

static void
set_preferences (ChirurgienPreferencesDialog *dialog)
{
    gint bytes_per_line;

    gtk_check_button_set_active (dialog->navigation,
                                 g_settings_get_boolean (dialog->preferences_settings,
                                                         "hide-navigation"));
    gtk_check_button_set_active (dialog->description,
                                 g_settings_get_boolean (dialog->preferences_settings,
                                                         "hide-description"));
    gtk_check_button_set_active (dialog->auto_analysis,
                                 g_settings_get_boolean (dialog->preferences_settings,
                                                         "auto-analysis"));
    gtk_check_button_set_active (dialog->extra_buttons,
                                 g_settings_get_boolean (dialog->preferences_settings,
                                                         "show-extra-buttons"));

    gtk_font_chooser_set_font (GTK_FONT_CHOOSER (dialog->font_button),
                               g_settings_get_string (dialog->preferences_settings,
                                                      "font"));

    g_settings_bind (dialog->preferences_settings, "hide-navigation",
                     dialog->navigation, "active",
                     G_SETTINGS_BIND_SET);
    g_settings_bind (dialog->preferences_settings, "hide-description",
                     dialog->description, "active",
                     G_SETTINGS_BIND_SET);
    g_settings_bind (dialog->preferences_settings, "auto-analysis",
                     dialog->auto_analysis, "active",
                     G_SETTINGS_BIND_SET);
    g_settings_bind (dialog->preferences_settings, "show-extra-buttons",
                     dialog->extra_buttons, "active",
                     G_SETTINGS_BIND_SET);

    g_settings_bind (dialog->preferences_settings, "font",
                     dialog->font_button, "font",
                     G_SETTINGS_BIND_SET);

    bytes_per_line = g_settings_get_int (dialog->preferences_settings, "bytes-per-line");
    if (bytes_per_line)
    {
        gtk_check_button_set_active (dialog->manual_lines, TRUE);
        gtk_spin_button_set_value (dialog->bytes, bytes_per_line);
    }

    set_colors (dialog);
}

static void
reset_default_colors (G_GNUC_UNUSED GtkButton *button,
                      gpointer user_data)
{
    ChirurgienPreferencesDialog *dialog;
    gchar *color_string;

    dialog = user_data;

    for (gint i = 0; i < CHIRURGIEN_TOTAL_COLORS; i++)
    {
        g_settings_reset (dialog->preferences_settings,
                          get_color_name (i));

        color_string = g_settings_get_string (dialog->preferences_settings,
                                              get_color_name (i));

        gdk_rgba_parse (&chirurgien_colors[i], color_string);

        pango_colors[i].red = chirurgien_colors[i].red * 65535;
        pango_colors[i].green = chirurgien_colors[i].green * 65535;
        pango_colors[i].blue = chirurgien_colors[i].blue * 65535;

        pango_alphas[i] = chirurgien_colors[i].alpha * 65535;

        g_free (color_string);
    }

    set_colors (dialog);
}

static void
color_selected (GtkColorButton *button,
                gpointer        user_data)
{
    ChirurgienPreferencesDialog *dialog;
    gint color_index;

    GdkRGBA color;
    g_autofree gchar *color_string;

    dialog = CHIRURGIEN_PREFERENCES_DIALOG (gtk_widget_get_ancestor (GTK_WIDGET (button),
                                            CHIRURGIEN_TYPE_PREFERENCES_DIALOG));
    color_index = GPOINTER_TO_INT (user_data);

    gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (button), &color);

    color_string = gdk_rgba_to_string (&color);
    gdk_rgba_parse (&color, color_string);

    chirurgien_colors[ color_index ] = color;

    pango_colors[ color_index ].red = color.red * 65535;
    pango_colors[ color_index ].green = color.green * 65535;
    pango_colors[ color_index ].blue = color.blue * 65535;
    pango_alphas[ color_index ] = color.alpha * 65535;

    g_settings_set_string (dialog->preferences_settings,
                           get_color_name (color_index),
                           color_string);
}

static void
close_preferences (ChirurgienPreferencesDialog *dialog,
                   G_GNUC_UNUSED gint response_id)
{
    gint bytes_per_line = 0;

    if (gtk_check_button_get_active (dialog->manual_lines))
        bytes_per_line = (gint) gtk_spin_button_get_value (dialog->bytes);

    g_settings_set_int (dialog->preferences_settings,
                        "bytes-per-line",
                        bytes_per_line);
}

static void
chirurgien_preferences_dialog_class_init(ChirurgienPreferencesDialogClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class,
                             "/io/github/leonardschardijn/chirurgien/ui/chirurgien-preferences-dialog.ui");
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, navigation);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, description);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, auto_analysis);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, extra_buttons);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, color0);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, color1);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, color2);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, color3);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, color4);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, color5);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, color6);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, color7);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, color8);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, default_colors);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, font_button);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, manual_lines);
    gtk_widget_class_bind_template_child (widget_class, ChirurgienPreferencesDialog, bytes);
}

static void
chirurgien_preferences_dialog_init (ChirurgienPreferencesDialog *dialog)
{
    gtk_widget_init_template (GTK_WIDGET (dialog));

    g_signal_connect (dialog->color0, "color-set", G_CALLBACK (color_selected), GINT_TO_POINTER (0));
    g_signal_connect (dialog->color1, "color-set", G_CALLBACK (color_selected), GINT_TO_POINTER (1));
    g_signal_connect (dialog->color2, "color-set", G_CALLBACK (color_selected), GINT_TO_POINTER (2));
    g_signal_connect (dialog->color3, "color-set", G_CALLBACK (color_selected), GINT_TO_POINTER (3));
    g_signal_connect (dialog->color4, "color-set", G_CALLBACK (color_selected), GINT_TO_POINTER (4));
    g_signal_connect (dialog->color5, "color-set", G_CALLBACK (color_selected), GINT_TO_POINTER (5));
    g_signal_connect (dialog->color6, "color-set", G_CALLBACK (color_selected), GINT_TO_POINTER (6));
    g_signal_connect (dialog->color7, "color-set", G_CALLBACK (color_selected), GINT_TO_POINTER (7));
    g_signal_connect (dialog->color8, "color-set", G_CALLBACK (color_selected), GINT_TO_POINTER (8));

    g_signal_connect (dialog->default_colors, "clicked", G_CALLBACK (reset_default_colors), dialog);

    g_signal_connect (dialog, "response", G_CALLBACK (close_preferences), NULL);

    g_object_bind_property (dialog->manual_lines, "active",
                            gtk_widget_get_parent (GTK_WIDGET (dialog->bytes)), "visible",
                            G_BINDING_DEFAULT);
}

/*** Public API ***/

GtkWidget *
chirurgien_preferences_dialog_new (ChirurgienWindow *window)
{
    ChirurgienPreferencesDialog *dialog;

    dialog = g_object_new (CHIRURGIEN_TYPE_PREFERENCES_DIALOG,
                           "transient-for", window,
                           NULL);

    dialog->preferences_settings = chirurgien_window_get_preferences (window);
    set_preferences (dialog);

    return GTK_WIDGET (dialog);
}
