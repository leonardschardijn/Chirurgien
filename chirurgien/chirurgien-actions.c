/* chirurgien-actions.c
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

#include "chirurgien-actions.h"

#include <glib/gi18n.h>

#include <chirurgien-formats.h>

#include "chirurgien-view-tab.h"
#include "chirurgien-formats-dialog.h"
#include "chirurgien-preferences-dialog.h"


static gboolean
in_flatpak_sandbox (void)
{
    static const char *flatpak_sandbox = NULL;

    if (G_UNLIKELY (flatpak_sandbox == NULL))
    {
        if (g_file_test ("/.flatpak-info", G_FILE_TEST_EXISTS))
            flatpak_sandbox = "1";
        else
            flatpak_sandbox = "0";
    }

    return flatpak_sandbox[0] == '1';
}

void
chirurgien_actions_disable_csd (G_GNUC_UNUSED GSimpleAction *action,
                                GVariant *parameter,
                                gpointer  user_data)
{
    ChirurgienWindow *window;
    GtkWidget *dialog;

    window = CHIRURGIEN_WINDOW (gtk_application_get_active_window (user_data));

    if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (gtk_window_get_child (GTK_WINDOW (window)))))
    {
        dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                         GTK_DIALOG_MODAL,
                                         GTK_MESSAGE_INFO,
                                         GTK_BUTTONS_CANCEL,
                                         _("Open files need to be closed"));

        g_signal_connect (dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
        gtk_window_present (GTK_WINDOW (dialog));

        return;
    }

    g_settings_set_boolean(chirurgien_window_get_preferences (window), "disable-csd",
                           g_variant_get_boolean (parameter));

    g_application_hold (user_data);

    gtk_window_close (GTK_WINDOW (window));
    g_application_activate (user_data);

    g_application_release (user_data);
}

static void
close_preferences (G_GNUC_UNUSED GtkDialog *dialog,
                   G_GNUC_UNUSED gint       response_id,
                   gpointer user_data)
{
    GtkNotebook *notebook;
    ChirurgienView *view;
    gint pages;

    notebook = GTK_NOTEBOOK (gtk_window_get_child (user_data));

    chirurgien_window_load_view_font (user_data);

    pages = gtk_notebook_get_n_pages (notebook);

    for (gint i = 0; i < pages; i++)
    {
        view = CHIRURGIEN_VIEW (gtk_notebook_get_nth_page (notebook, i));
        chirurgien_view_refresh (view);
    }
}

void
chirurgien_actions_preferences (G_GNUC_UNUSED GSimpleAction *action,
                                G_GNUC_UNUSED GVariant      *parameter,
                                gpointer user_data)
{
    ChirurgienWindow *window;
    GtkWidget *dialog;

    window = CHIRURGIEN_WINDOW (gtk_application_get_active_window (user_data));

    dialog = chirurgien_preferences_dialog_new (window);
    g_signal_connect (dialog, "response", G_CALLBACK (close_preferences), window);

    gtk_window_present (GTK_WINDOW (dialog));
}

void
chirurgien_actions_formats (G_GNUC_UNUSED GSimpleAction *action,
                            G_GNUC_UNUSED GVariant      *parameter,
                            gpointer user_data)
{
    GtkWindow *window;
    GtkWidget *dialog;

    window = gtk_application_get_active_window (user_data);

    dialog = chirurgien_formats_dialog_new (window);
    gtk_window_present (GTK_WINDOW (dialog));
}

void
chirurgien_actions_shortcuts (G_GNUC_UNUSED GSimpleAction *action,
                              G_GNUC_UNUSED GVariant      *parameter,
                              gpointer user_data)
{
    GtkWindow *window;
    GtkWidget *shortcuts_window;
    GtkBuilder *builder;

    window = gtk_application_get_active_window (user_data);

    builder = gtk_builder_new_from_resource ("/io/github/leonardschardijn/chirurgien/ui/chirurgien-shortcuts-dialog.ui");
    shortcuts_window = GTK_WIDGET (gtk_builder_get_object (builder, "shortcuts"));

    gtk_window_set_transient_for (GTK_WINDOW (shortcuts_window), window);
    gtk_window_present (GTK_WINDOW (shortcuts_window));

    g_object_unref (builder);
}

void
chirurgien_actions_about (G_GNUC_UNUSED GSimpleAction *action,
                          G_GNUC_UNUSED GVariant      *parameter,
                          gpointer user_data)
{
    GtkWindow *window;
    GString *system_information;

    static const gchar * const authors[] = {
        "Daniel Léonard Schardijn",
        NULL
    };

    window = gtk_application_get_active_window (user_data);

    system_information = g_string_new (_("Runtime libraries"));

    g_string_append_printf (system_information,
                            "\n\tGTK\t%d.%d.%d\n",
                            gtk_get_major_version (),
                            gtk_get_minor_version (),
                            gtk_get_micro_version ());
    g_string_append_printf (system_information,
                            "\tGLib\t%d.%d.%d\n",
                            glib_major_version,
                            glib_minor_version,
                            glib_micro_version);
    g_string_append_printf (system_information,
                            "\tPango\t%s\n",
                            pango_version_string ());
    g_string_append_printf (system_information,
                            "\tCairo\t%s",
                            cairo_version_string ());

    gtk_show_about_dialog (window,
                    "program-name", "Chirurgien",
                    "authors", authors,
                    "comments", _("Understand and manipulate file formats"),
                    "copyright", "Copyright 2020-2022 – Daniel Léonard Schardijn",
                    "license-type", GTK_LICENSE_GPL_3_0,
                    "logo-icon-name", "io.github.leonardschardijn.Chirurgien",
                    "version", VERSION,
                    "website", "https://github.com/leonardschardijn/Chirurgien",
                    "website-label", _("Website"),
                    "system-information", system_information->str,
                    NULL);

    g_string_free (system_information, TRUE);
}

void
chirurgien_actions_quit (G_GNUC_UNUSED GSimpleAction *action,
                         G_GNUC_UNUSED GVariant      *parameter,
                         gpointer user_data)
{
    gtk_window_close (GTK_WINDOW (gtk_application_get_active_window (user_data)));
}

static void
open_response (GtkNativeDialog *self,
               gint             response_id)
{
    ChirurgienWindow *window;
    g_autoptr (GFile) file = NULL;

    if (response_id == GTK_RESPONSE_ACCEPT)
    {
        window = CHIRURGIEN_WINDOW (gtk_native_dialog_get_transient_for (self));

        file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (self));

        chirurgien_actions_new_view (window, file);

        /* FIXME:
         * When running under Flatpak, the portal used to open/save files seems to use the
         * host's ~/.local/share/recently-used.xbel recent list and NOT the application's
         * Flatpak-specific ~/.var/app/<app-id>/data/recently-used.xbel recent list.
         * Thus, the entry needs to be added manually
         */
        if (in_flatpak_sandbox ())
        {
            g_autofree gchar *uri = g_file_get_uri (file);
            gtk_recent_manager_add_item (gtk_recent_manager_get_default (), uri);
        }

        chirurgien_window_update_recent (window, file);
    }

    gtk_native_dialog_destroy (self);
    g_object_unref (self);
}

void
chirurgien_actions_open (G_GNUC_UNUSED GSimpleAction *action,
                         G_GNUC_UNUSED GVariant      *parameter,
                         gpointer user_data)
{
    GtkFileChooserNative *dialog;

    dialog = gtk_file_chooser_native_new (_("Open file (max. file size: 50MiB)"), user_data,
                                            GTK_FILE_CHOOSER_ACTION_OPEN, NULL, NULL);

    gtk_native_dialog_set_modal (GTK_NATIVE_DIALOG (dialog), TRUE);

    g_signal_connect (dialog, "response", G_CALLBACK (open_response), NULL);
    gtk_native_dialog_show (GTK_NATIVE_DIALOG (dialog));
}

static void
save_response (GtkNativeDialog *self,
               gint             response_id,
               gpointer         user_data)
{
    ChirurgienWindow *window;
    g_autoptr (GFile) file = NULL;

    GtkWidget *error_dialog;

    if (response_id == GTK_RESPONSE_ACCEPT)
    {
        window = CHIRURGIEN_WINDOW (gtk_native_dialog_get_transient_for (self));

        file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (self));

        if (chirurgien_view_save (user_data, file))
        {
            /* FIXME:
             * When running under Flatpak, the portal used to open/save files seems to use the
             * host's ~/.local/share/recently-used.xbel recent list and NOT the application's
             * Flatpak-specific ~/.var/app/<app-id>/data/recently-used.xbel recent list.
             * Thus, the entry needs to be added manually
             */
            if (in_flatpak_sandbox ())
            {
                g_autofree gchar *uri = g_file_get_uri (file);
                gtk_recent_manager_add_item (gtk_recent_manager_get_default (), uri);
            }

            chirurgien_window_update_recent (window, file);
        }
        else
        {
            error_dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                                 GTK_BUTTONS_CLOSE, _("Failed to save file."));

            g_signal_connect (error_dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
            gtk_window_present (GTK_WINDOW (error_dialog));
        }
    }

    gtk_native_dialog_destroy (self);
    g_object_unref (self);
}

void
chirurgien_actions_save (GSimpleAction *action,
                         G_GNUC_UNUSED GVariant *parameter,
                         gpointer       user_data)
{
    GtkFileChooserNative *dialog;
    g_autoptr (GFile) file = NULL;
    g_autofree gchar *basename = NULL;

    GtkNotebook *notebook;
    ChirurgienView *view;

    notebook = GTK_NOTEBOOK (gtk_window_get_child (user_data));
    view = CHIRURGIEN_VIEW (gtk_notebook_get_nth_page (notebook,
                            gtk_notebook_get_current_page (notebook)));

    /* Action is save and a file already exists: just save */
    if (!g_strcmp0 (g_action_get_name (G_ACTION (action)), "save") &&
        chirurgien_view_has_file (view))
    {
        file = g_file_new_for_path (chirurgien_view_get_file_path (view));
        chirurgien_view_save (view, file);
    }
    /* Actions is save-as or no file exists: open save dialog */
    else
    {
        dialog = gtk_file_chooser_native_new (_("Save file"), user_data,
                                                GTK_FILE_CHOOSER_ACTION_SAVE, NULL, NULL);

        gtk_native_dialog_set_modal (GTK_NATIVE_DIALOG (dialog), TRUE);

        basename = g_path_get_basename (chirurgien_view_get_file_path (view));
        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), basename);

        g_signal_connect (dialog, "response", G_CALLBACK (save_response), view);
        gtk_native_dialog_show (GTK_NATIVE_DIALOG (dialog));
    }
}

void
chirurgien_actions_close (G_GNUC_UNUSED GSimpleAction *action,
                          G_GNUC_UNUSED GVariant      *parameter,
                          gpointer user_data)
{
    GtkNotebook *notebook;
    ChirurgienView *view;

    notebook = GTK_NOTEBOOK (gtk_window_get_child (user_data));
    view = CHIRURGIEN_VIEW (gtk_notebook_get_nth_page (notebook,
                            gtk_notebook_get_current_page (notebook)));

    chirurgien_view_tab_close (CHIRURGIEN_VIEW_TAB (chirurgien_view_get_view_tab (view)));
}

void
chirurgien_actions_reanalyze (G_GNUC_UNUSED GSimpleAction *action,
                              G_GNUC_UNUSED GVariant      *parameter,
                              gpointer user_data)
{
    GtkNotebook *notebook;
    ChirurgienView *view;

    notebook = GTK_NOTEBOOK (gtk_window_get_child (user_data));
    view = CHIRURGIEN_VIEW (gtk_notebook_get_nth_page (notebook,
                            gtk_notebook_get_current_page (notebook)));

    chirurgien_view_redo_analysis (view);
}

void
chirurgien_actions_hex_view (G_GNUC_UNUSED GSimpleAction *action,
                             G_GNUC_UNUSED GVariant      *parameter,
                             gpointer user_data)
{
    GtkNotebook *notebook;
    ChirurgienView *view;

    notebook = GTK_NOTEBOOK (gtk_window_get_child (user_data));
    view = CHIRURGIEN_VIEW (gtk_notebook_get_nth_page (notebook,
                            gtk_notebook_get_current_page (notebook)));

    chirurgien_view_select_view (view, CHIRURGIEN_HEX_VIEW);
}

void
chirurgien_actions_text_view (G_GNUC_UNUSED GSimpleAction *action,
                              G_GNUC_UNUSED GVariant      *parameter,
                              gpointer user_data)
{
    GtkNotebook *notebook;
    ChirurgienView *view;

    notebook = GTK_NOTEBOOK (gtk_window_get_child (user_data));
    view = CHIRURGIEN_VIEW (gtk_notebook_get_nth_page (notebook,
                            gtk_notebook_get_current_page (notebook)));

    chirurgien_view_select_view (view, CHIRURGIEN_TEXT_VIEW);
}

void
chirurgien_actions_undo (G_GNUC_UNUSED GSimpleAction *action,
                         G_GNUC_UNUSED GVariant      *parameter,
                         gpointer user_data)
{
    GtkNotebook *notebook;
    ChirurgienView *view;

    gboolean undo_available, redo_available;

    notebook = GTK_NOTEBOOK (gtk_window_get_child (user_data));
    view = CHIRURGIEN_VIEW (gtk_notebook_get_nth_page (notebook,
                            gtk_notebook_get_current_page (notebook)));

    chirurgien_view_undo (view);

    chirurgien_view_query_modifications (view,
                                         &undo_available,
                                         &redo_available);

    chirurgien_window_set_undo (user_data, undo_available);
    chirurgien_window_set_redo (user_data, redo_available);
}

void
chirurgien_actions_redo (G_GNUC_UNUSED GSimpleAction *action,
                         G_GNUC_UNUSED GVariant      *parameter,
                         gpointer user_data)
{
    GtkNotebook *notebook;
    ChirurgienView *view;

    gboolean undo_available, redo_available;

    notebook = GTK_NOTEBOOK (gtk_window_get_child (user_data));
    view = CHIRURGIEN_VIEW (gtk_notebook_get_nth_page (notebook,
                            gtk_notebook_get_current_page (notebook)));

    chirurgien_view_redo (view);

    chirurgien_view_query_modifications (view,
                                         &undo_available,
                                         &redo_available);

    chirurgien_window_set_undo (user_data, undo_available);
    chirurgien_window_set_redo (user_data, redo_available);
}

void
chirurgien_actions_next_tab (G_GNUC_UNUSED GSimpleAction *action,
                             G_GNUC_UNUSED GVariant      *parameter,
                             gpointer user_data)
{
    GtkNotebook *notebook;
    gint page, last_page;

    notebook = GTK_NOTEBOOK (gtk_window_get_child (user_data));

    last_page = gtk_notebook_get_n_pages (notebook) - 1;
    page = gtk_notebook_get_current_page (notebook);

    if (page == last_page)
        gtk_notebook_set_current_page (notebook, 0);
    else
        gtk_notebook_next_page (notebook);
}

void
chirurgien_actions_previous_tab (G_GNUC_UNUSED GSimpleAction *action,
                                 G_GNUC_UNUSED GVariant      *parameter,
                                 gpointer user_data)
{
    GtkNotebook *notebook;
    gint page;

    notebook = GTK_NOTEBOOK (gtk_window_get_child (user_data));

    page = gtk_notebook_get_current_page (notebook);

    if (page == 0)
        gtk_notebook_set_current_page (notebook, -1);
    else
        gtk_notebook_prev_page (notebook);
}

void
chirurgien_actions_recent_open (G_GNUC_UNUSED GSimpleAction *action,
                                GVariant *parameter,
                                gpointer  user_data)
{
    ChirurgienWindow *window;
    g_autoptr (GFile) file = NULL;

    window = user_data;
    file = g_file_new_for_uri (g_variant_get_string (parameter, NULL));

    chirurgien_actions_new_view (window, file);
}

static gchar *
validate_file (GFile *file)
{
    g_autoptr (GFileInfo) file_info;

    GError *error = NULL;
    gchar *error_message = NULL;

    file_info = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_SIZE","G_FILE_ATTRIBUTE_ACCESS_CAN_READ,
                                   G_FILE_QUERY_INFO_NONE, NULL, &error);
    /* The file was probably deleted */
    if (error)
    {
        error_message = g_strdup_printf (_("Error: %s"), error->message);
        g_error_free (error);
    }
    /* The file is empty */
    else if (!g_file_info_get_size (file_info))
    {
        error_message = g_strdup (_("The selected file is empty."));
    }
    /* The file cannot be read */
    else if (!g_file_info_get_attribute_boolean (file_info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ))
    {
        error_message = g_strdup (_("The selected file cannot be read."));
    }

    return error_message;
}

static void
load_format_response (GtkNativeDialog *self,
                      gint             response_id)
{
    ChirurgienFormatsDialog *dialog;
    g_autoptr (GFile) file = NULL;

    GString *escaped_error_message;
    g_autofree gchar *dialog_message = NULL;
    g_autofree gchar *error_message = NULL;

    if (response_id == GTK_RESPONSE_ACCEPT)
    {
        dialog = CHIRURGIEN_FORMATS_DIALOG (gtk_native_dialog_get_transient_for (self));

        file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (self));

        if (!(error_message = validate_file (file)))
            error_message = chirurgien_formats_load (file);

        if (error_message)
        {
            escaped_error_message = g_string_new (error_message);
            g_string_replace (escaped_error_message, "<", "&lt;", 0);
            g_string_replace (escaped_error_message, ">", "&gt;", 0);

            dialog_message = g_strdup_printf ("<span weight=\"bold\" foreground=\"red\">%s</span>\n\n%s",
                                              _("Failed to create format definition!"),
                                              escaped_error_message->str);
            chirurgien_formats_dialog_set_message (dialog,
                                                   dialog_message);
            g_string_free (escaped_error_message, TRUE);
        }
        else
        {
            dialog_message = g_strdup_printf ("<span weight=\"bold\" foreground=\"green\">%s</span>",
                                              _("Format definition successfully created!"));
            chirurgien_formats_dialog_set_message (dialog,
                                                   dialog_message);
            chirurgien_formats_dialog_add_format (dialog);
        }
    }

    gtk_native_dialog_destroy (self);
    g_object_unref (self);
}

void
chirurgien_actions_load_format (G_GNUC_UNUSED GtkButton *self,
                                gpointer user_data)
{
    GtkFileChooserNative *dialog;

    dialog = gtk_file_chooser_native_new (_("Load new format"), user_data,
                                            GTK_FILE_CHOOSER_ACTION_OPEN, NULL, NULL);

    gtk_native_dialog_set_modal (GTK_NATIVE_DIALOG (dialog), TRUE);

    g_signal_connect (dialog, "response", G_CALLBACK (load_format_response), NULL);
    gtk_native_dialog_show (GTK_NATIVE_DIALOG (dialog));
}

void
chirurgien_actions_new_view (ChirurgienWindow *window,
                             GFile            *file)
{
    ChirurgienView *view;

    GtkWidget *error_dialog = NULL;
    g_autofree gchar *error_message = NULL;

    error_message = validate_file (file);

    if (error_message)
    {
        error_dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_CLOSE, "%s", error_message);
        g_signal_connect (error_dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
        gtk_window_present (GTK_WINDOW (error_dialog));

        return;
    }

    view = chirurgien_view_new (window);
    chirurgien_view_set_file (view, file);

    chirurgien_actions_show_view (window, view);
}

void
chirurgien_actions_show_view (ChirurgienWindow *window,
                              ChirurgienView   *view)
{
    gint notebook_index;

    chirurgien_view_do_analysis (view);

    notebook_index = gtk_notebook_append_page (GTK_NOTEBOOK (gtk_window_get_child (GTK_WINDOW (window))),
                                               GTK_WIDGET (view),
                                               chirurgien_view_get_view_tab (view));
    gtk_notebook_set_current_page (GTK_NOTEBOOK (gtk_window_get_child (GTK_WINDOW (window))),
                                   notebook_index);
}
