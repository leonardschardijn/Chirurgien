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


struct _ChirurgienPreferencesDialog
{
    GtkDialog parent_instance;

    GtkWidget *bytes;
    GtkWidget *justification;
    GtkWidget *navigation;

    GSettings *preferences_settings;
};

G_DEFINE_TYPE (ChirurgienPreferencesDialog, chirurgien_preferences_dialog, GTK_TYPE_DIALOG)

static void
chirurgien_preferences_dialog_class_init(ChirurgienPreferencesDialogClass *class)
{
    GtkWidgetClass *gtkwidget_class = GTK_WIDGET_CLASS (class);

    gtk_widget_class_set_template_from_resource (gtkwidget_class,
                             "/io/github/leonardschardijn/chirurgien/ui/chirurgien-preferences-dialog.ui");
    gtk_widget_class_bind_template_child (gtkwidget_class, ChirurgienPreferencesDialog, bytes);
    gtk_widget_class_bind_template_child (gtkwidget_class, ChirurgienPreferencesDialog, justification);
    gtk_widget_class_bind_template_child (gtkwidget_class, ChirurgienPreferencesDialog, navigation);
}

static void
chirurgien_preferences_dialog_init (ChirurgienPreferencesDialog *dialog)
{
    gtk_widget_init_template (GTK_WIDGET (dialog));
}

GtkWidget *
chirurgien_preferences_dialog_new (ChirurgienWindow *parent)
{
    ChirurgienPreferencesDialog *dialog;
    GtkJustification justification;

    dialog = g_object_new (CHIRURGIEN_PREFERENCES_DIALOG_TYPE,
                           "transient-for", parent,
                           NULL);

    dialog->preferences_settings = parent->preferences_settings;

    gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->bytes),
                               g_settings_get_int (dialog->preferences_settings, "bytes-per-line"));

    justification = g_settings_get_enum (dialog->preferences_settings, "file-justification");
    if (justification == GTK_JUSTIFY_LEFT)
        gtk_combo_box_set_active ((GTK_COMBO_BOX (dialog->justification)), 0);
    else if (justification == GTK_JUSTIFY_CENTER)
        gtk_combo_box_set_active ((GTK_COMBO_BOX (dialog->justification)), 1);
    else if (justification == GTK_JUSTIFY_RIGHT)
        gtk_combo_box_set_active ((GTK_COMBO_BOX (dialog->justification)), 2);
    
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->navigation), 
                                  g_settings_get_boolean (dialog->preferences_settings, "show-navigation"));

    return GTK_WIDGET (dialog);
}

void
chirurgien_preferences_dialog_update_settings (ChirurgienPreferencesDialog *dialog)
{
    gint active;

    g_settings_set_int (dialog->preferences_settings, "bytes-per-line",
                        gtk_spin_button_get_value (GTK_SPIN_BUTTON (dialog->bytes)));

    active = gtk_combo_box_get_active (GTK_COMBO_BOX (dialog->justification));
    g_settings_set_enum (dialog->preferences_settings, "file-justification",
                         active == 0 ? GTK_JUSTIFY_LEFT :
                         active == 1 ? GTK_JUSTIFY_CENTER :
                         GTK_JUSTIFY_RIGHT);

    g_settings_set_boolean (dialog->preferences_settings, "show-navigation",
                            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->navigation)));
}
