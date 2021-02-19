/* tar-analyzer.h
 *
 * Copyright (C) 2021 - Daniel Léonard Schardijn
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

#pragma once

#include <gtk/gtk.h>

#include <chirurgien-analyzer-file.h>
#include <analyzer-utils.h>

G_BEGIN_DECLS

#define EMPTY_BLOCK_COLOR &colors[1]
#define HEADER_DATA_COLOR_1 &colors[2]
#define HEADER_DATA_COLOR_2 &colors[3]
#define FILE_CONTENTS_COLOR &colors[5]
#define ERROR_COLOR &colors[6]
#define PADDING_COLOR &colors[8]

G_END_DECLS
