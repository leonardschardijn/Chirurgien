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

#include <amtk/amtk.h>
#include <glib/gi18n.h>

#include "chirurgien-analyzer-view.h"
#include "chirurgien-actions.h"


G_DEFINE_TYPE (ChirurgienWindow, chirurgien_window, GTK_TYPE_APPLICATION_WINDOW)

static GActionEntry win_entries[] =
{
    { "open", chirurgien_actions_open, NULL, NULL, NULL },
    { "close", chirurgien_actions_close, NULL, NULL, NULL },
    { "hex-view", chirurgien_actions_hex_view, NULL, NULL, NULL },
    { "text-view", chirurgien_actions_text_view, NULL, NULL, NULL },
    { "next-tab", chirurgien_actions_next_tab, NULL, NULL, NULL },
    { "previous-tab", chirurgien_actions_previous_tab, NULL, NULL, NULL },

    /* Only used when using Window Manager decorations */
    { "recent", chirurgien_actions_recent_open_wmd, "s", NULL, NULL }
};

static void
restore_window_state (ChirurgienWindow *window)
{
    gint width, height;
    GdkWindowState state;

    g_settings_get (window->window_settings, "size", "(ii)", &width, &height);

    gtk_window_set_default_size (GTK_WINDOW (window), width, height);

    state = g_settings_get_int (window->window_settings, "state");
    if ((state & GDK_WINDOW_STATE_MAXIMIZED) != 0)
        gtk_window_maximize (GTK_WINDOW (window));
    else
        gtk_window_unmaximize (GTK_WINDOW (window));

    if ((state & GDK_WINDOW_STATE_STICKY ) != 0)
        gtk_window_stick (GTK_WINDOW (window));
    else
        gtk_window_unstick (GTK_WINDOW (window));
}

/*
 * Handle the configure-event signal
 */
static gboolean
chirurgien_window_configure_event (GtkWidget *widget,
                                   GdkEventConfigure *event)
{
    ChirurgienWindow *window;
    gint width, height;

    window = CHIRURGIEN_WINDOW (widget);

    if (gtk_widget_get_realized (widget) && (window->window_state & (GDK_WINDOW_STATE_MAXIMIZED | GDK_WINDOW_STATE_FULLSCREEN)) == 0)
    {
        gtk_window_get_size (GTK_WINDOW (window), &width, &height);
        g_settings_set (window->window_settings, "size", "(ii)", width, height);
    }

    return GTK_WIDGET_CLASS (chirurgien_window_parent_class)->configure_event (widget, event);
}

/*
 * Handle the window-state-event signal
 */
static gboolean
chirurgien_window_window_state_event (GtkWidget *widget,
                                      GdkEventWindowState *event)
{
    ChirurgienWindow *window;

    window = CHIRURGIEN_WINDOW (widget);
    window->window_state = event->new_window_state;

    g_settings_set_int (window->window_settings, "state", window->window_state);

    return GTK_WIDGET_CLASS (chirurgien_window_parent_class)->window_state_event (widget, event);
}

/*
 * Escape underscores for the WMD recent files list
 * (Shamelessly copied from Xfce's Mousepad (Thanks!))
 */
static gchar *
escape_underscores (const gchar *label)
{
    GString *result;
    const gchar *character;

    result = g_string_sized_new (strlen (label));

    for (character = label; *character != '\0'; character++)
    {
        if (G_UNLIKELY (*character == '_'))
            g_string_append (result, "__");
        else
            g_string_append_c (result, *character);
    }

    return g_string_free (result, FALSE);
}

/*
 * (Re)builds the recent files list when using Window Manager decorations
 */
void
chirurgien_window_build_recent (ChirurgienWindow* window)
{
    GList *recent_files, *index;
    GtkRecentInfo *recent_entry;
    GMenuItem *recent_item;
    gchar *escaped_label;
    gint position = 0;

    g_menu_remove_all (window->recent_menu);
    recent_files = gtk_recent_chooser_get_items (GTK_RECENT_CHOOSER (window->recent_files));
    for (index = recent_files;
         index != NULL;
         index = index->next)
    {
        recent_entry = index->data;
        escaped_label = escape_underscores (gtk_recent_info_get_display_name (recent_entry));
        recent_item = g_menu_item_new (escaped_label, NULL);
        g_menu_item_set_action_and_target (recent_item, "win.recent",
                                           "s", gtk_recent_info_get_uri (recent_entry));

        g_menu_insert_item (window->recent_menu, position++, recent_item);

        g_object_unref (recent_item);
        g_free (escaped_label);
        gtk_recent_info_unref (recent_entry);
    }
    g_list_free (recent_files);
}

/*
 * Window Manager decorations 
 * 
 * Create the File submenu:
 *  File
 *    Open
 *    Recent files
 *    Close tab
 *    Quit
 */
static void
create_open_recent_wmd (ChirurgienWindow *window,
                        GMenu *menubar)
{
    GMenu *file_menu, *recent_submenu, *quit_section;
    GMenuItem *recent_item;

    recent_submenu = g_menu_new ();
    window->recent_menu = recent_submenu;

    chirurgien_window_build_recent (window);

    file_menu = g_menu_new ();

    g_menu_insert (file_menu, 0, _("_Open"), "win.open");

    recent_item = g_menu_item_new (_("Recent files"), NULL);
    g_menu_item_set_link (recent_item, G_MENU_LINK_SUBMENU, G_MENU_MODEL (recent_submenu));
    g_menu_append_item (file_menu, recent_item);

    quit_section = g_menu_new ();
    g_menu_insert (quit_section, 0, _("_Close tab"), "win.close");
    g_menu_insert (quit_section, 1, _("_Quit"), "app.quit");
    g_menu_append_section(file_menu, NULL, G_MENU_MODEL (quit_section));

    g_menu_prepend_submenu (menubar, _("_File"), G_MENU_MODEL (file_menu));

    g_object_unref (recent_item);
    g_object_unref (quit_section);
    g_object_unref (file_menu);
}

/*
 * Client-side decorations
 * 
 * Create the Open button and recent files list at the top left corner
 */
static void
create_open_recent_csd (ChirurgienWindow *window)
{
    GtkWidget *box;
    GtkStyleContext *context;
    GtkWidget *open_button;
    GtkWidget *open_recent_button;

    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    context = gtk_widget_get_style_context (box);
    gtk_style_context_add_class (context, GTK_STYLE_CLASS_LINKED);

    open_button = gtk_button_new_with_label (_("Open"));
    gtk_widget_set_tooltip_text (open_button, _("Open a file"));
    gtk_actionable_set_action_name (GTK_ACTIONABLE (open_button), "win.open");

    open_recent_button = gtk_menu_button_new ();
    gtk_widget_set_tooltip_text (open_recent_button, _("Open a recently used file"));

    gtk_menu_button_set_popup (GTK_MENU_BUTTON (open_recent_button), window->recent_files);

    gtk_container_add (GTK_CONTAINER (box), open_button);
    gtk_container_add (GTK_CONTAINER (box), open_recent_button);

    gtk_container_add_with_properties (GTK_CONTAINER (window->headerbar), box, "position", 0, NULL);
}

/*
 * Create the window using Client-side decorations or Window Manager decorations
 */
static void
create_window (ChirurgienWindow *window)
{
    GtkBuilder *builder;
    GMenuModel *menu;
    GtkWidget *main, *widget;
    GtkStyleContext* context;
    GAction *action;

    AmtkApplicationWindow *amtk_window;
    GtkWidget *recent_files;

    amtk_window = amtk_application_window_get_from_gtk_application_window (GTK_APPLICATION_WINDOW (window));
    recent_files = amtk_application_window_create_open_recent_menu (amtk_window);
    window->recent_files = g_object_ref_sink (recent_files);

    /* Window Manager decorations */
    if (g_settings_get_boolean (window->preferences_settings, "disable-csd"))
    {
        gtk_window_set_title (GTK_WINDOW (window), "Chirurgien");
        builder = gtk_builder_new_from_resource ("/io/github/leonardschardijn/chirurgien/menus/menu-wmd.ui");
        menu = G_MENU_MODEL (gtk_builder_get_object (builder, "menu-wmd"));
        gtk_application_set_menubar (GTK_APPLICATION (g_application_get_default ()), menu);

        action = g_action_map_lookup_action (G_ACTION_MAP (g_application_get_default ()), "disable-csd");
        g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (TRUE));

        /* Connect to the signal to update the recent files list */
        g_signal_connect (window, "update-recent", G_CALLBACK (chirurgien_window_build_recent), NULL);
        
        create_open_recent_wmd (window, G_MENU (menu));
    }
    /* Client-side decorations */
    else
    {
        widget = gtk_header_bar_new ();
        gtk_header_bar_set_title (GTK_HEADER_BAR (widget), "Chirurgien");
        gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (widget), TRUE);
        gtk_window_set_titlebar (GTK_WINDOW (window), widget);
        window->headerbar = widget;

        widget = gtk_menu_button_new ();
        gtk_button_set_image (GTK_BUTTON (widget),
                              gtk_image_new_from_icon_name ("open-menu-symbolic",
                                                            GTK_ICON_SIZE_MENU));
        builder = gtk_builder_new_from_resource ("/io/github/leonardschardijn/chirurgien/menus/menu-csd.ui");
        menu = G_MENU_MODEL (gtk_builder_get_object (builder, "menu-csd"));
        gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (widget), menu);

        gtk_container_add_with_properties (GTK_CONTAINER (window->headerbar), widget,
                                           "position", 0,
                                           "pack-type", GTK_PACK_END,
                                           NULL);

        create_open_recent_csd (window);
        gtk_widget_show_all (GTK_WIDGET (window->headerbar));

        gtk_application_set_menubar (GTK_APPLICATION (g_application_get_default ()), NULL);

        action = g_action_map_lookup_action (G_ACTION_MAP (g_application_get_default ()), "disable-csd");
        g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (FALSE));
    }
    g_object_unref (builder);

    main = gtk_overlay_new ();

    widget = gtk_notebook_new ();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (widget), TRUE);
    gtk_container_add (GTK_CONTAINER (main), widget);
    window->notebook = widget;

    widget = gtk_label_new (_("Analyzing..."));
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_add_class (context, "app-notification");
    gtk_overlay_add_overlay (GTK_OVERLAY (main), widget);
    window->analyzing_message = widget;

    widget = gtk_label_new (_("Processing..."));
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_add_class (context, "app-notification");
    gtk_overlay_add_overlay (GTK_OVERLAY (main), widget);
    window->processing_message = widget;

    gtk_widget_show_all (main);
    gtk_widget_hide (window->analyzing_message);
    gtk_widget_hide (window->processing_message);

    gtk_container_add (GTK_CONTAINER (window), main);
}

static void
toggle_tab_actions (ChirurgienWindow *window, gboolean enable)
{
    GAction *close_action, *hex_view_action, *text_view_action,
            *next_tab_action, *prev_tab_action;

    close_action = g_action_map_lookup_action (G_ACTION_MAP (window), "close");
    hex_view_action = g_action_map_lookup_action (G_ACTION_MAP (window), "hex-view");
    text_view_action = g_action_map_lookup_action (G_ACTION_MAP (window), "text-view");
    next_tab_action = g_action_map_lookup_action (G_ACTION_MAP (window), "next-tab");
    prev_tab_action = g_action_map_lookup_action (G_ACTION_MAP (window), "previous-tab");

    if (enable)
    {
        g_simple_action_set_enabled (G_SIMPLE_ACTION (close_action), TRUE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (hex_view_action), TRUE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (text_view_action), TRUE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (next_tab_action), TRUE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (prev_tab_action), TRUE);
    }
    else
    {
        g_simple_action_set_enabled (G_SIMPLE_ACTION (close_action), FALSE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (hex_view_action), FALSE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (text_view_action), FALSE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (next_tab_action), FALSE);
        g_simple_action_set_enabled (G_SIMPLE_ACTION (prev_tab_action), FALSE);
    }
}

static void
switch_page (__attribute__((unused)) GtkNotebook *notebook,
             GtkWidget *page,
             __attribute__((unused)) guint page_num,
             gpointer user_data)
{
    ChirurgienWindow *window;
    ChirurgienAnalyzerView *view;

    window = CHIRURGIEN_WINDOW (user_data);
    view = CHIRURGIEN_ANALYZER_VIEW (page);

    if (window->headerbar != NULL)
        gtk_header_bar_set_subtitle (GTK_HEADER_BAR (window->headerbar),
                                     chirurgien_analyzer_view_get_file_path (view));
}

static void
page_added (GtkNotebook *notebook,
            __attribute__((unused)) GtkWidget *page,
            __attribute__((unused)) guint page_num,
            gpointer user_data)
{
    ChirurgienWindow *window;

    window = CHIRURGIEN_WINDOW (user_data);

    if (gtk_notebook_get_n_pages (notebook) == 1)
        toggle_tab_actions (window, TRUE);

}

static void
page_removed (GtkNotebook *notebook,
              __attribute__((unused)) GtkWidget *page,
              __attribute__((unused)) guint page_num,
              gpointer user_data)
{
    ChirurgienWindow *window;

    window = CHIRURGIEN_WINDOW (user_data);

    if (!gtk_notebook_get_n_pages (notebook))
    {
        toggle_tab_actions (window, FALSE);

        if (window->headerbar != NULL)
            gtk_header_bar_set_subtitle (GTK_HEADER_BAR (window->headerbar), NULL);
    }
}

static void
chirurgien_window_init (ChirurgienWindow *window)
{
    g_action_map_add_action_entries (G_ACTION_MAP (window),
                                     win_entries, G_N_ELEMENTS (win_entries),
                                     window);

    window->preferences_settings = g_settings_new ("io.github.leonardschardijn.chirurgien.preferences");
    window->window_settings = g_settings_new ("io.github.leonardschardijn.chirurgien.state");
    g_settings_delay (window->window_settings);

    create_window (window);
    toggle_tab_actions (window, FALSE);

    g_signal_connect (window->notebook, "switch-page", G_CALLBACK (switch_page), window);
    g_signal_connect (window->notebook, "page-added", G_CALLBACK (page_added), window);
    g_signal_connect (window->notebook, "page-removed", G_CALLBACK (page_removed), window);

    restore_window_state (window);
}

static void
chirurgien_window_dispose (GObject *object)
{
    ChirurgienWindow *window;

    window = CHIRURGIEN_WINDOW (object);

    if (window->window_settings != NULL)
        g_settings_apply (window->window_settings);

    g_clear_object (&window->window_settings);
    g_clear_object (&window->preferences_settings);
    g_clear_object (&window->recent_files);
    g_clear_object (&window->recent_menu);

    G_OBJECT_CLASS (chirurgien_window_parent_class)->dispose (object);
}

static void
chirurgien_window_class_init (ChirurgienWindowClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    GtkWidgetClass *gtkwidget_class = GTK_WIDGET_CLASS (class);

    gobject_class->dispose = chirurgien_window_dispose;

    gtkwidget_class->window_state_event = chirurgien_window_window_state_event;
    gtkwidget_class->configure_event = chirurgien_window_configure_event;

    /* Only used when using Window Manager decorations */
    /* Used to update the recent files list */
    g_signal_new ("update-recent",
                  G_OBJECT_CLASS_TYPE (gobject_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

ChirurgienWindow *
chirurgien_window_new (ChirurgienApplication *app)
{
    return g_object_new (CHIRURGIEN_WINDOW_TYPE, "application", app, NULL);
}
