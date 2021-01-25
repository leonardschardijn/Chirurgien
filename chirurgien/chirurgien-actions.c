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
#include "chirurgien-actions-analyzer.h"

#include <amtk/amtk.h>
#include <glib/gi18n.h>

#include "chirurgien-analyzer-view.h"
#include "chirurgien-analyzer-info-dialog.h"
#include "chirurgien-preferences-dialog.h"


/*
 * Private functions
 */
static gboolean     close_tab                (GtkWidget *,
                                              GdkEvent *,
                                              gpointer);
static void         long_operation_window    (ChirurgienWindow *,
                                              gboolean);
static void         update_bytes_per_line    (ChirurgienWindow *);


void
chirurgien_actions_disable_csd (__attribute__((unused)) GSimpleAction *action,
                                GVariant *parameter,
                                gpointer user_data)
{
    GtkApplication *app;
    ChirurgienWindow *window;
    GtkWidget *dialog;
    gint result;

    app = GTK_APPLICATION (user_data);
    window = CHIRURGIEN_WINDOW (gtk_application_get_active_window (app));

    if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook)))
    {
        dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_USE_HEADER_BAR,
                                         GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
                                         _("Open tabs will be closed!"));
        result = gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);
    }
    else
    {
        result = GTK_RESPONSE_OK;
    }

    if (result == GTK_RESPONSE_OK)
    {
        g_settings_set_boolean(window->preferences_settings, "disable-csd",
                               g_variant_get_boolean (parameter));

        g_application_hold (G_APPLICATION (app));

        gtk_widget_destroy (GTK_WIDGET (window));
        g_application_activate (G_APPLICATION (app));

        g_application_release (G_APPLICATION (app));
    }
}

void
chirurgien_actions_preferences (__attribute__((unused)) GSimpleAction *action,
                                __attribute__((unused)) GVariant *parameter,
                                gpointer user_data)
{
    ChirurgienWindow *window;

    GtkWidget *dialog;
    gint bytes_prev, bytes_new;

    window = CHIRURGIEN_WINDOW (gtk_application_get_active_window (GTK_APPLICATION (user_data)));

    bytes_prev = bytes_new = g_settings_get_int (window->preferences_settings, "bytes-per-line");

    dialog = chirurgien_preferences_dialog_new (window);
    gtk_dialog_run (GTK_DIALOG (dialog));

    chirurgien_preferences_dialog_update_settings (CHIRURGIEN_PREFERENCES_DIALOG (dialog));

    bytes_new = g_settings_get_int (window->preferences_settings, "bytes-per-line");

    gtk_widget_destroy (GTK_WIDGET (dialog));

    if (bytes_prev != bytes_new)
        update_bytes_per_line (window);
}

void
chirurgien_actions_analyzer_info (__attribute__((unused)) GSimpleAction *action,
                                  __attribute__((unused)) GVariant *parameter,
                                  gpointer user_data)
{
    GtkWindow *window;
    GtkWidget *dialog;

    window = gtk_application_get_active_window (GTK_APPLICATION (user_data));

    dialog = chirurgien_analyzer_info_dialog_new (window);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
chirurgien_actions_shortcuts (__attribute__((unused)) GSimpleAction *action,
                              __attribute__((unused)) GVariant *parameter,
                              gpointer user_data)
{
    GtkWindow *window;
    GtkWidget *shortcuts_window;
    GtkBuilder *builder;

    window = gtk_application_get_active_window (GTK_APPLICATION (user_data));

    builder = gtk_builder_new_from_resource ("/io/github/leonardschardijn/chirurgien/ui/chirurgien-shortcuts-dialog.ui");
    shortcuts_window = GTK_WIDGET (gtk_builder_get_object (builder, "shortcuts"));
    g_object_unref (builder);

    gtk_window_set_transient_for (GTK_WINDOW (shortcuts_window), window);

    gtk_widget_show_all (shortcuts_window);
    gtk_window_present (GTK_WINDOW (shortcuts_window));
}

void
chirurgien_actions_about (__attribute__((unused)) GSimpleAction *action,
                          __attribute__((unused)) GVariant *parameter,
                          gpointer user_data)
{
    GtkWindow *window;

    window = gtk_application_get_active_window (GTK_APPLICATION (user_data));

    static const gchar * const authors[] = {
        "Daniel Léonard Schardijn",
        NULL
    };

    gtk_show_about_dialog (window,
                    "program-name", "Chirurgien",
                    "authors", authors,
                    "comments", _("Chirurgien is a simple tool that helps to understand file formats"),
                    "copyright", "Copyright 2020-2021 – Daniel Léonard Schardijn",
                    "license-type", GTK_LICENSE_GPL_3_0,
                    "logo-icon-name", "io.github.leonardschardijn.Chirurgien",
                    "version", VERSION,
                    "website", "https://github.com/leonardschardijn/Chirurgien",
                    "website-label", _("Website"),
                    NULL);
}

void
chirurgien_actions_quit (__attribute__((unused)) GSimpleAction *action,
                         __attribute__((unused)) GVariant *parameter,
                         gpointer user_data)
{
    g_application_quit (G_APPLICATION (user_data));
}

void
chirurgien_actions_open (__attribute__((unused)) GSimpleAction *action,
                         __attribute__((unused)) GVariant *parameter,
                         gpointer user_data)
{
    ChirurgienWindow *window;

    g_autoptr (GFile) file = NULL;
    g_autofree gchar *uri = NULL;

    GtkFileChooserNative *dialog;
    gint result;

    window = CHIRURGIEN_WINDOW (user_data);

    dialog = gtk_file_chooser_native_new (_("Analyze file (max. file size: 10MiB)"), GTK_WINDOW (window),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          NULL, NULL);

    result = gtk_native_dialog_run (GTK_NATIVE_DIALOG (dialog));

    uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));

    g_object_unref (dialog);

    if (result == GTK_RESPONSE_ACCEPT)
    {
        file = g_file_new_for_uri (uri);
        chirurgien_actions_analyze_file (window, file);

        /* This is required if the application is running under flatpak, the recent files
         * are not updated otherwise */
        gtk_recent_manager_add_item (gtk_recent_manager_get_default (), uri);
    }

    /* Window Manager decorations: Update the recent files list, as it may have changed */
    if (g_settings_get_boolean (window->preferences_settings, "disable-csd"))
        g_signal_emit_by_name (window,"update-recent");
}

void
chirurgien_actions_close (__attribute__((unused)) GSimpleAction *action,
                          __attribute__((unused)) GVariant *parameter,
                          gpointer user_data)
{
    ChirurgienWindow *window;
    gint page;

    window = CHIRURGIEN_WINDOW (user_data);

    if ((page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook))) != -1)
    {
        gtk_notebook_remove_page (GTK_NOTEBOOK (window->notebook), page);
    }
}



void
chirurgien_actions_hex_view (__attribute__((unused)) GSimpleAction *action,
                             __attribute__((unused)) GVariant *parameter,
                             gpointer user_data)
{
    ChirurgienWindow *window;
    ChirurgienAnalyzerView *view;

    window = CHIRURGIEN_WINDOW (user_data);

    if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook)))
    {
        view = CHIRURGIEN_ANALYZER_VIEW (gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->notebook), 
                                         gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook))));
        gtk_notebook_set_current_page (chirurgien_analyzer_view_get_hex_text_notebook (view), 0);
    }
}

void
chirurgien_actions_text_view (__attribute__((unused)) GSimpleAction *action,
                              __attribute__((unused)) GVariant *parameter,
                              gpointer user_data)
{
    ChirurgienWindow *window;
    ChirurgienAnalyzerView *view;

    window = CHIRURGIEN_WINDOW (user_data);

    if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook)))
    {
        view = CHIRURGIEN_ANALYZER_VIEW (gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->notebook), 
                                         gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook))));
        gtk_notebook_set_current_page (chirurgien_analyzer_view_get_hex_text_notebook (view), 1);
    }
}

void
chirurgien_actions_next_tab (__attribute__((unused)) GSimpleAction *action,
                             __attribute__((unused)) GVariant *parameter,
                             gpointer user_data)
{
    ChirurgienWindow *window;
    gint page, last_page;

    window = CHIRURGIEN_WINDOW (user_data);

    last_page = gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook)) - 1;
    if ((page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook))) != -1)
    {
        if (page == last_page)
            gtk_notebook_set_current_page (GTK_NOTEBOOK (window->notebook), 0);
        else
            gtk_notebook_next_page (GTK_NOTEBOOK (window->notebook));
    }
}

void
chirurgien_actions_previous_tab (__attribute__((unused)) GSimpleAction *action,
                                 __attribute__((unused)) GVariant *parameter,
                                 gpointer user_data)
{
    ChirurgienWindow *window;
    gint page;

    window = CHIRURGIEN_WINDOW (user_data);

    if ((page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook))) != -1)
    {
        if (page == 0)
            gtk_notebook_set_current_page (GTK_NOTEBOOK (window->notebook), -1);
        else
            gtk_notebook_prev_page (GTK_NOTEBOOK (window->notebook));
    }
}

void
chirurgien_actions_recent_open_wmd (__attribute__((unused)) GSimpleAction *action,
                                    GVariant *parameter,
                                    gpointer user_data)
{
    ChirurgienWindow *window;
    g_autoptr (GFile) file = NULL;

    window = CHIRURGIEN_WINDOW (user_data);
    file = g_file_new_for_uri (g_variant_get_string (parameter, NULL));

    chirurgien_actions_analyze_file (window, file);
}

void
chirurgien_actions_analyze_file (ChirurgienWindow *window,
                                 GFile *file)
{
    g_autoptr (GFileInfo) file_info;
    g_autofree gchar *file_path = NULL;

    ChirurgienAnalyzerView *view;
    GtkWidget *label, *button, *widget;
    GtkStyleContext *context;

    gint notebook_index;

    GError *error = NULL;

    file_info = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME","G_FILE_ATTRIBUTE_STANDARD_SIZE
                                   ","G_FILE_ATTRIBUTE_ACCESS_CAN_READ, G_FILE_QUERY_INFO_NONE, NULL, &error);
    /* The file was probably deleted */
    if (file_info == NULL)
    {
        widget = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                                         GTK_BUTTONS_CLOSE, error->message);
        gtk_dialog_run (GTK_DIALOG (widget));
        gtk_widget_destroy (widget);
        return;
    }
    /* The file is empty */
    if (!g_file_info_get_size (file_info))
    {
        widget = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                                         GTK_BUTTONS_CLOSE, _("The selected file is empty."));
        gtk_dialog_run (GTK_DIALOG (widget));
        gtk_widget_destroy (widget);
        return;
    }
    /* The file cannot be read */
    if (!g_file_info_get_attribute_boolean (file_info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ))
    {
        widget = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                                         GTK_BUTTONS_CLOSE, _("The selected file cannot be read."));
        gtk_dialog_run (GTK_DIALOG (widget));
        gtk_widget_destroy (widget);
        return;
    }

    file_path = g_file_get_path (file);

    label = gtk_label_new (g_file_info_get_display_name (file_info));
    gtk_widget_set_tooltip_text (label, file_path);

    view = chirurgien_analyzer_view_new (window);
    chirurgien_analyzer_view_prepare_analysis (view, file);

    button = gtk_button_new_from_icon_name ("window-close-symbolic", GTK_ICON_SIZE_BUTTON);
    g_signal_connect (button, "button-release-event", G_CALLBACK (close_tab), view);

    context = gtk_widget_get_style_context (button);
    gtk_style_context_add_class (context, GTK_STYLE_CLASS_FLAT);

    widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start (GTK_BOX (widget), label, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (widget), button, FALSE, FALSE, 0);

    gtk_widget_show_all (widget);
    gtk_widget_show_all (GTK_WIDGET (view));

    long_operation_window (window, TRUE);
    chirurgien_analyzer_view_execute_analysis (view);

    notebook_index = gtk_notebook_append_page (GTK_NOTEBOOK (window->notebook), GTK_WIDGET (view), widget);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (window->notebook), notebook_index);
}

void
chirurgien_actions_embedded_file (__attribute__((unused)) GtkButton *analyze_button,
                                  gpointer user_data)
{
    ChirurgienWindow *window;
    ChirurgienAnalyzerView *view, *embedded_view;

    GtkWidget *label, *button, *widget;
    GtkStyleContext *context;

    gint notebook_index;

    guchar *embedded_file_contents;
    gsize embedded_file_size;
    gchar *embedded_file_name;

    g_autofree gchar *original_file_name;

    window = CHIRURGIEN_WINDOW (gtk_application_get_active_window
                               (GTK_APPLICATION (g_application_get_default ())));

    view = CHIRURGIEN_ANALYZER_VIEW (gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->notebook),
                                     gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook))));

    embedded_file_size = chirurgien_analyzer_view_get_embedded_file (view, GPOINTER_TO_UINT (user_data), &embedded_file_contents);

    original_file_name = g_path_get_basename (chirurgien_analyzer_view_get_file_path (view));

    embedded_view = chirurgien_analyzer_view_new (window);
    embedded_file_name = chirurgien_analyzer_view_prepare_analysis_embedded (embedded_view, embedded_file_contents,
                                                                             embedded_file_size, original_file_name);

    label = gtk_label_new (embedded_file_name);
    gtk_widget_set_tooltip_text (label, embedded_file_name);

    button = gtk_button_new_from_icon_name ("window-close-symbolic", GTK_ICON_SIZE_BUTTON);
    g_signal_connect (button, "button-release-event", G_CALLBACK (close_tab), embedded_view);

    context = gtk_widget_get_style_context (button);
    gtk_style_context_add_class (context, GTK_STYLE_CLASS_FLAT);

    widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start (GTK_BOX (widget), label, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (widget), button, FALSE, FALSE, 0);

    gtk_widget_show_all (widget);
    gtk_widget_show_all (GTK_WIDGET (embedded_view));

    long_operation_window (window, TRUE);
    chirurgien_analyzer_view_execute_analysis (embedded_view);

    notebook_index = gtk_notebook_append_page (GTK_NOTEBOOK (window->notebook), GTK_WIDGET (embedded_view), widget);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (window->notebook), notebook_index);
}

static gboolean
close_tab (__attribute__((unused)) GtkWidget *widget,
           __attribute__((unused)) GdkEvent *event,
           gpointer   user_data)
{
    gtk_widget_destroy(GTK_WIDGET (user_data));

    return TRUE;
}

static void
update_bytes_per_line (ChirurgienWindow *window)
{
    ChirurgienAnalyzerView *view;
    gint i, notebook_pages;

    long_operation_window (window, FALSE);

    notebook_pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook));

    for (i = 0; i < notebook_pages; i++)
    {
        view = CHIRURGIEN_ANALYZER_VIEW (gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->notebook), i));
        chirurgien_analyzer_view_update_lines (view);
    }
}

static void
long_operation_window (ChirurgienWindow *window,
                       gboolean analyzing)
{
    if (analyzing)
    {
        gtk_widget_show (window->analyzing_message);
        while (g_main_context_iteration (NULL, FALSE));
        gtk_widget_hide (window->analyzing_message);
    }
    else
    {
        gtk_widget_show (window->processing_message);
        while (g_main_context_iteration (NULL, FALSE));
        gtk_widget_hide (window->processing_message);
    }
}
