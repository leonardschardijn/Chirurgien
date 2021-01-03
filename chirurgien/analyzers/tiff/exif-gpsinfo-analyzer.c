/* exif-gpsinfo-analyzer.c
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

#include "exif-gpsinfo-analyzer.h"

#include <glib/gi18n.h>

#include "chirurgien-analyze-exif.h"


void
chirurgien_analyze_gpsinfo (AnalyzerFile *file,
                            gboolean is_little_endian,
                            GSList **tagged_bytes,
                            AnalyzerTab *gpsinfo_tab,
                            AnalyzerTab *gpsinfo_ascii_tab)
{
    const guint16 tiff_tags[EXIF_GPSINFO_TAGS] =
    {
        0x0000, // GPSVersionID
        0x0001  // GPSLatitudeRef
    };

    guint16 ifd_entries;

    guint16 tiff_tag, field_type;
    guint32 count, value_offset;

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    if (!analyzer_utils_read (&ifd_entries, file , 2))
    {
        analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
        return;
    }
    analyzer_utils_tag_navigation (file, IFD_COLOR, 2, _("Number of directory entries (GPSInfo)"), "IFD");

    if (!is_little_endian)
        ifd_entries = g_ntohs (ifd_entries);
    ifd_entries++;

    /* Tag loop */
    while (ifd_entries)
    {
        if (ifd_entries == 1)
        {
            if (!analyzer_utils_read (&count, file, 4))
            {
                analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
                *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
                return;
            }

            analyzer_utils_tag (file, IFD_COLOR, 4, _("IFD end (GPSInfo)"));
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
            break;
        }

        /* TIFF tag */
        if (!analyzer_utils_read (&tiff_tag, file , 2))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
            return;
        }
        if (!is_little_endian)
            tiff_tag = g_ntohs (tiff_tag);

        /* Field type */
        if (!analyzer_utils_read (&field_type, file , 2))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
            return;
        }
        if (!is_little_endian)
            field_type = g_ntohs (field_type);

        /* Count */
        if (!analyzer_utils_read (&count, file , 4))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
            return;
        }
        if (!is_little_endian)
            count = g_ntohl (count);

        /* Tag value or offset */
        if (!analyzer_utils_read (&value_offset, file , 4))
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
            *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));
            return;
        }

        /* Analyze tag data */
        if (tiff_tag == tiff_tags[GPSVersionID])
        {
            analyze_gpsversionid_tag (file, gpsinfo_tab, field_type, count, value_offset);
        }
        else if (tiff_tag == tiff_tags[GPSLatitudeRef])
        {
            analyze_ascii_tag (file, field_type, count, value_offset, is_little_endian,
                               tagged_bytes, ASCII_GPSLatitudeRef, gpsinfo_ascii_tab);
        }
        else
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Tag: Unknown"));
            analyzer_utils_tag_error (file, ERROR_COLOR_2, 2, _("Field type"));
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Count"));
            analyzer_utils_tag_error (file, ERROR_COLOR_2, 4, _("Tag value or offset"));
        }

        ifd_entries--;
    }

}
