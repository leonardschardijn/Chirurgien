/* chirurgien-analyzer.c
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

#include "chirurgien-analyzer.h"

#include "analyzer-utils.h"
#include "png/chirurgien-analyze-png.h"


void
chirurgien_analyzer_analyze (AnalyzerFile *file)
{
    const guchar png_magic_number[] = { 0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A };

    if (!memcmp (file->file_contents, png_magic_number, 8))
    {
        chirurgien_analyze_png (file);
    }
    else
    {
        analyzer_utils_add_description (file,
                "<span weight=\"bold\" size=\"larger\">Unrecognized file format</span>",
                NULL, NULL, 0, 20);
    }
}
