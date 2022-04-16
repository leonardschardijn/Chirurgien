/* chirurgien-application.c
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

#include "chirurgien-application.h"

#include "chirurgien-window.h"
#include "chirurgien-actions.h"

#include <chirurgien-formats.h>


struct _ChirurgienApplication
{
    GtkApplication parent_instance;
};

G_DEFINE_TYPE (ChirurgienApplication, chirurgien_application, GTK_TYPE_APPLICATION)

static GActionEntry app_entries[] =
{
    { "disable-csd", NULL, NULL, "false", chirurgien_actions_disable_csd },
    { "preferences", chirurgien_actions_preferences, NULL, NULL, NULL },
    { "formats", chirurgien_actions_formats, NULL, NULL, NULL },
    { "shortcuts", chirurgien_actions_shortcuts, NULL, NULL, NULL },
    { "about", chirurgien_actions_about, NULL, NULL, NULL },
    { "quit", chirurgien_actions_quit, NULL, NULL, NULL }
};

static void
chirurgien_application_startup (GApplication *app)
{
    G_APPLICATION_CLASS (chirurgien_application_parent_class)->startup (app);

    g_action_map_add_action_entries (G_ACTION_MAP (app),
                                     app_entries, G_N_ELEMENTS (app_entries),
                                     app);

    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.preferences", (const gchar *[]) {"<Primary>P", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.about", (const gchar *[]) {"F1", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.formats", (const gchar *[]) {"F2", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.quit", (const gchar *[]) {"<Primary>Q", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.open", (const gchar *[]) {"<Primary>O", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.save", (const gchar *[]) {"<Primary>S", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.close-tab", (const gchar *[]) {"<Primary>W", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.reanalyze", (const gchar *[]) {"<Primary>R", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.hex-view", (const gchar *[]) {"<Primary>H", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.text-view", (const gchar *[]) {"<Primary>T", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.undo", (const gchar *[]) {"<Primary>Z", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.redo", (const gchar *[]) {"<Primary><Shift>Z", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.next-tab", (const gchar *[]) {"<Primary><Alt>Page_Down", NULL});
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), "win.previous-tab", (const gchar *[]) {"<Primary><Alt>Page_Up", NULL});

    /* Initialize supported formats */
    chirurgien_formats_initialize ("/io/github/leonardschardijn/chirurgien/format-definitions/cpio-format.xml");
    chirurgien_formats_initialize ("/io/github/leonardschardijn/chirurgien/format-definitions/elf-format.xml");
    chirurgien_formats_initialize ("/io/github/leonardschardijn/chirurgien/format-definitions/gif-format.xml");
    chirurgien_formats_initialize ("/io/github/leonardschardijn/chirurgien/format-definitions/jpeg-format.xml");
    chirurgien_formats_initialize ("/io/github/leonardschardijn/chirurgien/format-definitions/pe-format.xml");
    chirurgien_formats_initialize ("/io/github/leonardschardijn/chirurgien/format-definitions/png-format.xml");
    chirurgien_formats_initialize ("/io/github/leonardschardijn/chirurgien/format-definitions/tar-format.xml");
    chirurgien_formats_initialize ("/io/github/leonardschardijn/chirurgien/format-definitions/tiff-format.xml");
    chirurgien_formats_initialize ("/io/github/leonardschardijn/chirurgien/format-definitions/webp-format.xml");
}

static void
chirurgien_application_activate (GApplication *app)
{
    ChirurgienWindow *window;

    window = chirurgien_window_new (GTK_APPLICATION (app));
    gtk_window_present (GTK_WINDOW (window));
}

static void
chirurgien_application_open (GApplication  *app,
                             GFile        **files,
                             gint           n_files,
                             G_GNUC_UNUSED const gchar *hint)
{
    ChirurgienWindow *window;

    window = chirurgien_window_new (GTK_APPLICATION (app));

    for (gint i = 0; i < n_files; i++)
        if (!chirurgien_actions_new_view (window, files[i]))
            break;

    gtk_window_present (GTK_WINDOW (window));
}

static void
chirurgien_application_init (G_GNUC_UNUSED ChirurgienApplication *app)
{
    gtk_window_set_default_icon_name ("io.github.leonardschardijn.Chirurgien");
}

static void
chirurgien_application_class_init (ChirurgienApplicationClass *klass)
{
    G_APPLICATION_CLASS (klass)->startup = chirurgien_application_startup;
    G_APPLICATION_CLASS (klass)->activate = chirurgien_application_activate;
    G_APPLICATION_CLASS (klass)->open = chirurgien_application_open;
}

/*** Public API ***/

ChirurgienApplication *
chirurgien_application_new (void)
{
    return g_object_new (CHIRURGIEN_TYPE_APPLICATION,
                         "application-id", "io.github.leonardschardijn.Chirurgien",
                         "flags", G_APPLICATION_HANDLES_OPEN,
                         NULL);
}
