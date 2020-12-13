/* chirurgien-navigation-button.c
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

#include "chirurgien-navigation-button.h"


struct _ChirurgienNavigationButton
{
    GtkButton parent_instance;

    GtkTextMark *mark;
};

G_DEFINE_TYPE (ChirurgienNavigationButton, chirurgien_navigation_button, GTK_TYPE_BUTTON)

static void
chirurgien_navigation_button_init (__attribute__((unused)) ChirurgienNavigationButton *button)
{

}

static void
chirurgien_navigation_button_class_init (__attribute__((unused)) ChirurgienNavigationButtonClass *class)
{

}

GtkWidget *
chirurgien_navigation_button_new (GtkTextMark *mark)
{
    ChirurgienNavigationButton *button = CHIRURGIEN_NAVIGATION_BUTTON (g_object_new (CHIRURGIEN_NAVIGATION_BUTTON_TYPE, NULL));

    button->mark = mark;

    return GTK_WIDGET (button);
}

GtkTextMark *
chirurgien_navigation_button_get_mark (ChirurgienNavigationButton *button)
{
    return button->mark;
}
