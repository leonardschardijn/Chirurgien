/* tiff-ascii-tag.c
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

#include <glib/gi18n.h>

#include "tiff-analyzer.h"


void
analyze_ascii_tag (AnalyzerFile *file,
                   guint16 field_type,
                   guint32 count,
                   guint32 value_offset,
                   gboolean is_little_endian,
                   GSList **tagged_bytes,
                   gint ascii_tag,
                   AnalyzerTab *tab)
{
    gchar *ascii_tag_types[] = {
        /* TIFF ASCII Tags */
        _("Tag: DocumentName"),
        _("Tag: ImageDescription"),
        _("Tag: Make"),
        _("Tag: Model"),
        _("Tag: PageName"),
        _("Tag: Software"),
        _("Tag: DateTime"),
        _("Tag: Artist"),
        _("Tag: HostComputer"),
        _("Tag: Copyright"),
        /* Exif ASCII Tags */
        _("Tag: SpectralSensitivity"),
        _("Tag: DateTimeOriginal"),
        _("Tag: DateTimeDigitized"),
        _("Tag: SubSecTime"),
        _("Tag: SubSecTimeOriginal"),
        _("Tag: SubSecTimeDigitized"),
        _("Tag: RelatedSoundFile"),
        _("Tag: ImageUniqueID"),
        _("Tag: CameraOwnerName"),
        _("Tag: BodySerialNumber"),
        _("Tag: LensMake"),
        _("Tag: LensModel"),
        _("Tag: LensSerialNumber"),
        /* Exif ASCII Tags*/
        _("Tag: GPSLatitudeRef")
    };

    gchar *ascii_tags[] = {
        /* TIFF ASCII Tags */
        "DocumentName",
        "ImageDescription",
        "Make",
        "Model",
        "PageName",
        "Software",
        "DateTime",
        "Artist",
        "HostComputer",
        "Copyright",
        /* Exif ASCII Tags */
        "SpectralSensitivity",
        "DateTimeOriginal",
        "DateTimeDigitized",
        "SubSecTime",
        "SubSecTimeOriginal",
        "SubSecTimeDigitized",
        "RelatedSoundFile",
        "ImageUniqueID",
        "CameraOwnerName",
        "BodySerialNumber",
        "LensMake",
        "LensModel",
        "LensSerialNumber",
        /* Exif GPSInfo ASCII Tags */
        "GPSLatitudeRef"
    };

    gboolean read_success;
    gsize save_pointer;
    g_autofree gchar *ascii_text;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, ascii_tag_types[ascii_tag]);

    if (field_type == 2) // ASCII
        analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: ASCII"));
    else
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Invalid field type"));

    if (ascii_tag == ASCII_DateTime && count != 20)
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Invalid count"));
        goto END_COUNT;
    }
    analyzer_utils_tag (file, COUNT_COLOR, 4, _("Count"));

    END_COUNT:
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag offset"));

    ascii_text = g_malloc (count);

    if (!is_little_endian)
        value_offset = g_ntohl (value_offset);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    read_success = analyzer_utils_read (ascii_text, file, count);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, count, ascii_tags[ascii_tag]);

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, ascii_tags[ascii_tag]);

    SET_POINTER (file, save_pointer);

    if (read_success)
    {
        if (ascii_text[count - 1] == '\0')
            count--;
    }
    else
    {
        count = 0;
    }

    analyzer_utils_add_text_tab (tab, ascii_tags[ascii_tag], ascii_text, count);
}
