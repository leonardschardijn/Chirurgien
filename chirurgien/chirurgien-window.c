/* chirurgien-window.c
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

#include "chirurgien-window.h"

#include <glib/gi18n.h>

#include "chirurgien-colors.h"

#include "chirurgien-view.h"
#include "chirurgien-actions.h"


/* Available colors */
GdkRGBA colors[TOTAL_COLORS];
PangoColor pango_colors[TOTAL_COLORS];
guint16 pango_alphas[TOTAL_COLORS];

const gchar *color_names[TOTAL_COLORS] = {
    "color0", "color1", "color2",
    "color3", "color4", "color5",
    "color6", "color7", "color8",
};

struct _ChirurgienWindow
{
    GtkApplicationWindow   parent_instance;

    GQueue                 recent_files;
    GMenu                 *recent_menu;
    gboolean               recent_rebuild_needed;

    GSettings             *preferences_settings;
    GSettings             *state_settings;
};

G_DEFINE_TYPE (ChirurgienWindow, chirurgien_window, GTK_TYPE_APPLICATION_WINDOW)

static GActionEntry win_entries[] =
{
    { "open", chirurgien_actions_open, NULL, NULL, NULL },
    { "save", chirurgien_actions_save, NULL, NULL, NULL },
    { "save-as", chirurgien_actions_save, NULL, NULL, NULL },
    { "close-tab", chirurgien_actions_close, NULL, NULL, NULL },
    { "reanalyze", chirurgien_actions_reanalyze, NULL, NULL, NULL },
    { "hex-view", chirurgien_actions_hex_view, NULL, NULL, NULL },
    { "text-view", chirurgien_actions_text_view, NULL, NULL, NULL },
    { "undo", chirurgien_actions_undo, NULL, NULL, NULL },
    { "redo", chirurgien_actions_redo, NULL, NULL, NULL },
    { "next-tab", chirurgien_actions_next_tab, NULL, NULL, NULL },
    { "previous-tab", chirurgien_actions_previous_tab, NULL, NULL, NULL },

    { "recent", chirurgien_actions_recent_open, "s", NULL, NULL }
};

static void
restore_window_size (ChirurgienWindow *window)
{
    gint width, height;

    g_settings_get (window->state_settings, "size", "(ii)", &width, &height);
    gtk_window_set_default_size (GTK_WINDOW (window), width, height);
}

static void
notify_size_change (GObject *gobject,
                    G_GNUC_UNUSED GParamSpec *pspec)
{
    ChirurgienWindow *window;
    gint width, height;

    window = CHIRURGIEN_WINDOW (gobject);

    if (gtk_widget_get_realized (GTK_WIDGET (window)) && !gtk_window_is_maximized (GTK_WINDOW (window)))
    {
        gtk_window_get_default_size (GTK_WINDOW (window), &width, &height);
        g_settings_set (window->state_settings, "size", "(ii)", width, height);
    }
}

static gboolean
handle_drop (G_GNUC_UNUSED GtkDropTarget *drop_target,
             const GValue* value,
             G_GNUC_UNUSED gdouble        x,
             G_GNUC_UNUSED gdouble        y,
             gpointer      user_data)
{
    if (!G_VALUE_HOLDS (value, G_TYPE_FILE))
        return FALSE;

    chirurgien_actions_new_view (user_data, g_value_get_object (value));

    return TRUE;
}

/*
 * Escape underscores for the recent file list
 * (Shamelessly copied from Xfce's Mousepad (Thanks!))
 */
static gchar *
escape_underscores (const gchar *label)
{
    GString *result;
    const gchar *character;

    result = g_string_new (NULL);

    for (character = label; *character != '\0'; character++)
    {
        if (G_UNLIKELY (*character == '_'))
            g_string_append (result, "__");
        else
            g_string_append_c (result, *character);
    }

    return g_string_free (result, FALSE);
}

static void
build_recent_menu (ChirurgienWindow *window)
{
    GList *list_item;
    GMenuItem *recent_item;
    gchar *item_name;

    gint menu_entries;

    g_menu_remove_all (window->recent_menu);

    for (list_item = window->recent_files.head, menu_entries = 0;
         list_item != NULL;
         list_item = list_item->next, menu_entries++)
    {
        item_name = escape_underscores (gtk_recent_info_get_display_name (list_item->data));
        recent_item = g_menu_item_new (item_name, NULL);
        g_menu_item_set_action_and_target (recent_item, "win.recent",
                                           "s", gtk_recent_info_get_uri (list_item->data));

        g_menu_append_item (window->recent_menu, recent_item);

        g_free (item_name);
        g_object_unref (recent_item);
    }

    if (!menu_entries)
    {
        recent_item = g_menu_item_new (_("No recent entries"), NULL);
        g_menu_append_item (window->recent_menu, recent_item);
        g_object_unref (recent_item);
    }
}

static gint
compare_recent_entries (gconstpointer a,
                        gconstpointer b,
                        G_GNUC_UNUSED gpointer user_data)
{
    GtkRecentInfo *recent_a, *recent_b;
    GDateTime *date_a, *date_b;

    recent_a = (gpointer) a;
    recent_b = (gpointer) b;

    gtk_recent_info_get_application_info (recent_a,
                                          g_get_prgname (),
                                          NULL,
                                          NULL,
                                          &date_a);
    gtk_recent_info_get_application_info (recent_b,
                                          g_get_prgname (),
                                          NULL,
                                          NULL,
                                          &date_b);

    return g_date_time_compare (date_a, date_b);
}

static void
build_recent_files (ChirurgienWindow *window)
{
    GList *recent_files, *list_item;

    recent_files = gtk_recent_manager_get_items (gtk_recent_manager_get_default ());

    g_queue_init (&window->recent_files);

    /* Get all recent files used by the application */
    for (list_item = recent_files;
         list_item != NULL;
         list_item = list_item->next)
    {
        if (gtk_recent_info_has_application (list_item->data, g_get_prgname ()))
            g_queue_push_head (&window->recent_files, list_item->data);
        else
            gtk_recent_info_unref (list_item->data);
    }

    g_list_free (recent_files);

    g_queue_sort (&window->recent_files, compare_recent_entries, NULL);
    g_queue_reverse (&window->recent_files);

    /* Keep only the 10 most recent files */
    for (list_item = g_queue_peek_nth_link (&window->recent_files, 10);
         list_item != NULL;
         list_item = list_item->next)
    {
        gtk_recent_info_unref (list_item->data);
        list_item = list_item->prev;
        g_queue_delete_link (&window->recent_files, list_item->next);
    }

    build_recent_menu (window);
}

static void
recent_changed (G_GNUC_UNUSED GtkRecentManager *recent_manager,
                gpointer user_data)
{
    ChirurgienWindow *window;
    GList *list_item;

    window = user_data;

    if (!window->recent_rebuild_needed)
        return;

    window->recent_rebuild_needed = FALSE;

    for (list_item = window->recent_files.head;
         list_item != NULL;
         list_item = list_item->next)
    {
        gtk_recent_info_unref (list_item->data);
    }

    g_queue_clear (&window->recent_files);

    build_recent_files (window);
}

static void
create_window (ChirurgienWindow *window)
{
    GtkBuilder *builder;
    GMenuModel *menu;

    GtkWidget *headerbar, *box, *widget;

    headerbar = gtk_header_bar_new ();

    /* Hamburger menu */
    widget = gtk_menu_button_new ();
    gtk_menu_button_set_icon_name (GTK_MENU_BUTTON (widget), "open-menu-symbolic");
    builder = gtk_builder_new_from_resource ("/io/github/leonardschardijn/chirurgien/menus/hamburger-menu.ui");
    menu = G_MENU_MODEL (gtk_builder_get_object (builder, "hamburger-menu"));
    gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (widget), menu);

    gtk_header_bar_pack_end (GTK_HEADER_BAR (headerbar), widget);

    g_object_unref (builder);

    /* Open/Recent files buttons */
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class (box, "linked");

    widget = gtk_button_new_with_label (_("Open"));
    gtk_widget_set_tooltip_text (widget, _("Open a file"));
    gtk_actionable_set_action_name (GTK_ACTIONABLE (widget), "win.open");
    gtk_box_append (GTK_BOX (box), widget);

    widget = gtk_menu_button_new ();
    gtk_widget_set_tooltip_text (widget, _("Open a recently used file"));
    build_recent_files (window);
    gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (widget), G_MENU_MODEL (window->recent_menu));
    gtk_box_append (GTK_BOX (box), widget);

    gtk_header_bar_pack_start (GTK_HEADER_BAR (headerbar), box);

    /* Save/Save as buttons */
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class (box, "linked");

    widget = gtk_button_new_with_label (_("Save"));
    gtk_widget_set_tooltip_text (widget, _("Save the current file"));
    gtk_actionable_set_action_name (GTK_ACTIONABLE (widget), "win.save");
    gtk_box_append (GTK_BOX (box), widget);

    widget = gtk_menu_button_new ();
    menu = G_MENU_MODEL (g_menu_new ());
    g_menu_insert (G_MENU (menu), 0, _("Save as..."), "win.save-as");
    gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (widget), menu);
    gtk_box_append (GTK_BOX (box), widget);

    g_object_unref (menu);

    gtk_header_bar_pack_end (GTK_HEADER_BAR (headerbar), box);

    /* Undo/Redo buttons */
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class (box, "linked");

    widget = gtk_button_new_from_icon_name ("edit-undo-symbolic");
    gtk_widget_set_tooltip_text (widget, _("Undo previous modification"));
    gtk_actionable_set_action_name (GTK_ACTIONABLE (widget), "win.undo");
    gtk_box_append (GTK_BOX (box), widget);

    widget = gtk_button_new_from_icon_name ("edit-redo-symbolic");
    gtk_widget_set_tooltip_text (widget, _("Redo previous modification"));
    gtk_actionable_set_action_name (GTK_ACTIONABLE (widget), "win.redo");
    gtk_box_append (GTK_BOX (box), widget);

    gtk_header_bar_pack_end (GTK_HEADER_BAR (headerbar), box);

    gtk_window_set_titlebar (GTK_WINDOW (window), headerbar);
}

static void
toggle_view_actions (ChirurgienWindow *window,
                     gboolean          enable)
{
    GAction *save_action, *save_as_action, *close_tab_action, *reanalyze_action,
            *hex_view_action, *text_view_action, *next_tab_action, *prev_tab_action;

    save_action = g_action_map_lookup_action (G_ACTION_MAP (window), "save");
    save_as_action = g_action_map_lookup_action (G_ACTION_MAP (window), "save-as");
    close_tab_action = g_action_map_lookup_action (G_ACTION_MAP (window), "close-tab");
    reanalyze_action = g_action_map_lookup_action (G_ACTION_MAP (window), "reanalyze");
    hex_view_action = g_action_map_lookup_action (G_ACTION_MAP (window), "hex-view");
    text_view_action = g_action_map_lookup_action (G_ACTION_MAP (window), "text-view");
    next_tab_action = g_action_map_lookup_action (G_ACTION_MAP (window), "next-tab");
    prev_tab_action = g_action_map_lookup_action (G_ACTION_MAP (window), "previous-tab");

    if (enable)
    {
        g_simple_action_set_enabled (G_SIMPLE_ACTION (save_action), TRUE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (save_as_action), TRUE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (close_tab_action), TRUE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (reanalyze_action), TRUE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (hex_view_action), TRUE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (text_view_action), TRUE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (next_tab_action), TRUE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (prev_tab_action), TRUE);
    }
    else
    {
        g_simple_action_set_enabled (G_SIMPLE_ACTION (save_action), FALSE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (save_as_action), FALSE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (close_tab_action), FALSE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (reanalyze_action), FALSE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (hex_view_action), FALSE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (text_view_action), FALSE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (next_tab_action), FALSE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (prev_tab_action), FALSE);
    }
}

static void
close_response (GtkDialog *self,
                gint       response_id,
                gpointer   user_data)
{
    gtk_window_destroy (GTK_WINDOW (self));

    if (response_id == GTK_RESPONSE_YES)
        gtk_window_destroy (user_data);
}

static gboolean
close_window (GtkWindow *window)
{
    ChirurgienView *view;
    GtkNotebook *files_notebook;
    gint pages, unsaved_files;

    GtkWidget *dialog, *close_button;

    unsaved_files = 0;
    files_notebook = GTK_NOTEBOOK (gtk_window_get_child (window));
    pages = gtk_notebook_get_n_pages (files_notebook);

    for (gint i = 0; i < pages; i++)
    {
        view = CHIRURGIEN_VIEW (gtk_notebook_get_nth_page (files_notebook, i));

        if (chirurgien_view_unsaved (view))
            unsaved_files++;
    }

    if (unsaved_files)
    {
        dialog = gtk_message_dialog_new (window,
                                         GTK_DIALOG_MODAL,
                                         GTK_MESSAGE_WARNING,
                                         GTK_BUTTONS_NONE,
                                         _("Unsaved files"));

        if (unsaved_files == 1)
            gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                          _("There is one unsaved modified file"));
        else
            gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                          _("There are %d unsaved modified files"),
                                            unsaved_files);

        close_button = gtk_dialog_add_button  (GTK_DIALOG (dialog),
                                             _("Close without saving"),
                                               GTK_RESPONSE_YES);

        gtk_widget_add_css_class (close_button, "destructive-action");

        gtk_dialog_add_button  (GTK_DIALOG (dialog),
                              _("Cancel"),
                                GTK_RESPONSE_CANCEL);

        g_signal_connect (dialog, "response", G_CALLBACK (close_response), window);
        gtk_window_present (GTK_WINDOW (dialog));

        return TRUE;
    }

    return FALSE;
}

static void
switch_page (G_GNUC_UNUSED GtkNotebook *notebook,
             GtkWidget *page,
             G_GNUC_UNUSED guint        page_num,
             gpointer   user_data)
{
    ChirurgienView *view;
    g_autofree char *filename;

    gboolean undo_available, redo_available;

    view = CHIRURGIEN_VIEW (page);

    filename = g_path_get_basename (chirurgien_view_get_file_path (view));
    gtk_window_set_title (user_data, filename);

    chirurgien_view_query_modifications (view,
                                         &undo_available,
                                         &redo_available);

    chirurgien_window_set_undo (user_data, undo_available);
    chirurgien_window_set_redo (user_data, redo_available);
}

static void
page_added (GtkNotebook *notebook,
            G_GNUC_UNUSED GtkWidget *page,
            G_GNUC_UNUSED guint      page_num,
            gpointer     user_data)
{
    ChirurgienWindow *window;

    window = user_data;

    if (gtk_notebook_get_n_pages (notebook) == 1)
    {
        gtk_widget_show (GTK_WIDGET (notebook));
        toggle_view_actions (window, TRUE);
    }

}

static void
page_removed (GtkNotebook *notebook,
              G_GNUC_UNUSED GtkWidget *page,
              G_GNUC_UNUSED guint      page_num,
              gpointer     user_data)
{
    ChirurgienWindow *window;

    window = user_data;

    if (!gtk_notebook_get_n_pages (notebook))
    {
        gtk_widget_hide (GTK_WIDGET (notebook));
        toggle_view_actions (window, FALSE);
        gtk_window_set_title (GTK_WINDOW (window), "Chirurgien");

        chirurgien_window_set_undo (user_data, FALSE);
        chirurgien_window_set_redo (user_data, FALSE);
    }
}

static void
chirurgien_window_dispose (GObject *object)
{
    ChirurgienWindow *window;
    GList *list_item;

    window = CHIRURGIEN_WINDOW (object);

    if (window->state_settings)
        g_settings_apply (window->state_settings);

    for (list_item = window->recent_files.head;
         list_item != NULL;
         list_item = list_item->next)
    {
        gtk_recent_info_unref (list_item->data);
    }

    g_queue_clear (&window->recent_files);
    g_queue_init (&window->recent_files);

    g_clear_object (&window->preferences_settings);
    g_clear_object (&window->state_settings);
    g_clear_object (&window->recent_menu);

    G_OBJECT_CLASS (chirurgien_window_parent_class)->dispose (object);
}

static void
chirurgien_window_class_init (ChirurgienWindowClass *klass)
{
    G_OBJECT_CLASS (klass)->dispose = chirurgien_window_dispose;
}

static void
chirurgien_window_init (ChirurgienWindow *window)
{
    GtkWidget *widget;
    GtkDropTarget *drop_target;
    gchar *color_string;

    g_action_map_add_action_entries (G_ACTION_MAP (window),
                                     win_entries, G_N_ELEMENTS (win_entries),
                                     window);

    window->preferences_settings = g_settings_new ("io.github.leonardschardijn.chirurgien.preferences");
    window->state_settings = g_settings_new ("io.github.leonardschardijn.chirurgien.state");
    g_settings_delay (window->state_settings);

    window->recent_menu = g_menu_new ();

    create_window (window);
    toggle_view_actions (window, FALSE);

    chirurgien_window_set_undo (window, FALSE);
    chirurgien_window_set_redo (window, FALSE);

    g_signal_connect (gtk_recent_manager_get_default (), "changed",
                      G_CALLBACK (recent_changed), window);

    g_signal_connect (window, "close-request",
                      G_CALLBACK (close_window), NULL);

    g_signal_connect (window, "notify::default-width",
                      G_CALLBACK (notify_size_change), NULL);
    g_signal_connect (window, "notify::default-height",
                      G_CALLBACK (notify_size_change), NULL);

    widget = gtk_notebook_new ();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (widget), TRUE);
    g_signal_connect (widget, "switch-page", G_CALLBACK (switch_page), window);
    g_signal_connect (widget, "page-added", G_CALLBACK (page_added), window);
    g_signal_connect (widget, "page-removed", G_CALLBACK (page_removed), window);

    gtk_window_set_child (GTK_WINDOW (window), widget);
    gtk_widget_hide (widget);

    window->recent_rebuild_needed = FALSE;

    restore_window_size (window);

    gtk_window_set_title (GTK_WINDOW (window), "Chirurgien");

    drop_target = gtk_drop_target_new (G_TYPE_FILE, GDK_ACTION_COPY);
    g_signal_connect (drop_target, "drop", G_CALLBACK (handle_drop), window);
    gtk_widget_add_controller (GTK_WIDGET (window), GTK_EVENT_CONTROLLER (drop_target));

    chirurgien_window_load_view_font (window);

    /* Load colors */
    for (gint i = 0; i < TOTAL_COLORS; i++)
    {
        color_string = g_settings_get_string (window->preferences_settings, color_names[i]);

        gdk_rgba_parse (&colors[i], color_string);

        pango_colors[i].red = colors[i].red * 65535;
        pango_colors[i].green = colors[i].green * 65535;
        pango_colors[i].blue = colors[i].blue * 65535;

        pango_alphas[i] = colors[i].alpha * 65535;

        g_free (color_string);
    }
}

/*** Public API ***/

ChirurgienWindow *
chirurgien_window_new (GtkApplication *app)
{
    ChirurgienWindow *window;

    window =  g_object_new (CHIRURGIEN_TYPE_WINDOW,
                            "application", app,
                            NULL);

    g_settings_bind (window->state_settings, "maximized", window, "maximized", G_SETTINGS_BIND_DEFAULT);

    return window;
}

void
chirurgien_window_update_recent (ChirurgienWindow *window,
                                 GFile            *file)
{
    GList *list_item;
    g_autoptr (GtkRecentInfo) recent_info;
    g_autofree gchar *uri;

    gboolean found = FALSE;

    uri = g_file_get_uri (file);
    recent_info = gtk_recent_manager_lookup_item (gtk_recent_manager_get_default (), uri, NULL);

    /* The file is not present in the recent files, a rebuild is needed */
    if (!recent_info)
    {
        window->recent_rebuild_needed = TRUE;
        return;
    }

    for (list_item = window->recent_files.head;
         list_item != NULL && !found;
         list_item = list_item->next)
    {
        if (gtk_recent_info_match (list_item->data, recent_info))
        {
            g_queue_unlink (&window->recent_files, list_item);
            g_queue_push_head_link (&window->recent_files, list_item);

            found = TRUE;
        }
    }

    if (!found)
    {
        gtk_recent_info_unref (g_queue_pop_tail (&window->recent_files));
        g_queue_push_head (&window->recent_files, g_steal_pointer (&recent_info));
    }

    build_recent_menu (window);
}

void
chirurgien_window_load_view_font (ChirurgienWindow *window)
{
    GdkDisplay *display;
    GtkCssProvider *provider;
    PangoFontDescription *font_description;

    g_autofree gchar *css_font;

    font_description = pango_font_description_from_string (g_settings_get_string
                                                          (window->preferences_settings,
                                                           "font"));

    css_font = g_strdup_printf (".chirurgien-font { font-family: %s; font-size: %dpt; }",
                                pango_font_description_get_family (font_description),
                                pango_font_description_get_size (font_description) / PANGO_SCALE);

    display = gdk_display_get_default ();

    provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_data (provider, css_font, -1);

    gtk_style_context_add_provider_for_display (display, GTK_STYLE_PROVIDER (provider),
                                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    pango_font_description_free (font_description);
}

void
chirurgien_window_set_undo (ChirurgienWindow *window,
                            gboolean          enabled)
{
    GAction *action;

    action = g_action_map_lookup_action (G_ACTION_MAP (window), "undo");

    g_simple_action_set_enabled (G_SIMPLE_ACTION (action), enabled);
}

void
chirurgien_window_set_redo (ChirurgienWindow *window,
                            gboolean          enabled)
{
    GAction *action;

    action = g_action_map_lookup_action (G_ACTION_MAP (window), "redo");

    g_simple_action_set_enabled (G_SIMPLE_ACTION (action), enabled);
}

GSettings *
chirurgien_window_get_preferences (ChirurgienWindow *window)
{
    return window->preferences_settings;
}

GSettings *
chirurgien_window_get_state (ChirurgienWindow *window)
{
    return window->state_settings;
}
