/* tiff-undefined-tag.c
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

#include "exif-analyzer.h"


guint32
common_undefined_tag_structure (AnalyzerFile *file,
                                guint16 field_type,
                                guint32 count,
                                guint32 expected_count,
                                guint32 value_offset,
                                gboolean is_little_endian)
{
    if (field_type == 7) // UNDEFINED
        analyzer_utils_tag (file, FIELD_TYPE_COLOR, 2, _("Field type: UNDEFINED"));
    else
        analyzer_utils_tag_error (file, ERROR_COLOR_1, 2, _("Invalid field type"));

    if (expected_count)
    {
        if (count != expected_count)
            analyzer_utils_tag_error (file, ERROR_COLOR_1, 4, _("Invalid count"));
        else
            analyzer_utils_tag (file, COUNT_COLOR, 4, _("Count"));
    }
    else
    {
        analyzer_utils_tag (file, COUNT_COLOR, 4, _("Count"));
    }

    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, 4, _("Tag value"));

    if (!is_little_endian)
        value_offset = g_ntohl (value_offset);

    return value_offset;
}

/*
 * Exif UNDEFINED tags
 */

void
analyze_exifversion_tag (AnalyzerFile *file,
                         AnalyzerTab *exif_tab,
                         guint16 field_type,
                         guint32 count,
                         guint32 value_offset,
                         gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ExifVersion"));

    common_undefined_tag_structure (file, field_type, count, 4,
                                    0, is_little_endian);

    if (!strncmp ("0220", (gchar *) &value_offset, 4))
        description_message = "Exif v2.2";
    else if (!strncmp ("0221", (gchar *) &value_offset, 4))
        description_message = "Exif v2.21";
    else if (!strncmp ("0230", (gchar *) &value_offset, 4))
        description_message = "Exif v2.3";
    else
        description_message = _("<span foreground=\"red\">INVALID</span>");

    analyzer_utils_describe_tooltip_tab (exif_tab, "ExifVersion", description_message,
                                         "ExifVersion\n"
                                         "<tt>30 32 32 30<sub>16</sub></tt>\tExif v2.2\n"
                                         "<tt>30 32 32 31<sub>16</sub></tt>\tExif v2.21\n"
                                         "<tt>30 32 33 30<sub>16</sub></tt>\tExif v2.3");
}

void
analyze_componentsconfiguration_tag (AnalyzerFile *file,
                                     AnalyzerTab *exif_tab,
                                     guint16 field_type,
                                     guint32 count,
                                     guint32 value_offset,
                                     gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: ComponentsConfiguration"));

    value_offset = common_undefined_tag_structure (file, field_type, count, 4,
                                                   value_offset, is_little_endian);

    if (value_offset == 0x00060504)
        description_message = _("Uncompressed RGB");
    if (value_offset == 0x00030201)
        description_message = _("Other cases");
    else
        description_message = _("<span foreground=\"red\">INVALID</span>");

    analyzer_utils_describe_tooltip_tab (exif_tab, "ComponentsConfiguration", description_message,
                                         _("ComponentsConfiguration\n"
                                         "<tt>04 05 06 00<sub>16</sub></tt>\tUncompressed RGB\n"
                                         "<tt>01 02 03 00<sub>16</sub></tt>\tOther cases"));
}

void
analyze_makernote_tag (AnalyzerFile *file,
                       AnalyzerTab *exif_tab,
                       guint16 field_type,
                       guint32 count,
                       guint32 value_offset,
                       gboolean is_little_endian,
                       GSList **tagged_bytes)
{
    gboolean read_success;
    gsize save_pointer;
    gchar *makernote;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: MakerNote"));

    value_offset = common_undefined_tag_structure (file, field_type, count, 0,
                                                   value_offset, is_little_endian);

    if (count > 4)
    {
        makernote = g_malloc (count);

        save_pointer = GET_POINTER (file);

        SET_POINTER (file, value_offset);
        *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

        read_success = analyzer_utils_read (makernote, file, count);
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, count, "MakerNote");

        *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

        SET_POINTER (file, value_offset);
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "MakerNote");

        SET_POINTER (file, save_pointer);

        if (read_success && g_utf8_validate (makernote, count, NULL))
            analyzer_utils_add_text_tab (exif_tab, "MakerNote", makernote, count);
        else
            analyzer_utils_add_text_tab (exif_tab, "MakerNote", "", 0);

        g_free (makernote);
    }
    else
    {
        makernote = (gchar *) &value_offset;
        if (g_utf8_validate (makernote, count, NULL))
            analyzer_utils_add_text_tab (exif_tab, "MakerNote", makernote, count);
        else
            analyzer_utils_add_text_tab (exif_tab, "MakerNote", "", 0);
    }
}

void
analyze_usercomment_tag (AnalyzerFile *file,
                         AnalyzerTab *exif_tab,
                         guint16 field_type,
                         guint32 count,
                         guint32 value_offset,
                         gboolean is_little_endian,
                         GSList **tagged_bytes)
{
    gboolean read_success;
    gsize save_pointer;
    gchar *usercomment;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: UserComment"));

    value_offset = common_undefined_tag_structure (file, field_type, count, 0,
                                                   value_offset, is_little_endian);

    usercomment = g_malloc (count);

    save_pointer = GET_POINTER (file);

    SET_POINTER (file, value_offset);
    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    read_success = analyzer_utils_read (usercomment, file, count);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, count, "UserComment");

    *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

    SET_POINTER (file, value_offset);
    analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "UserComment");

    SET_POINTER (file, save_pointer);

    if (read_success && count > 8 && g_utf8_validate (usercomment + 8, count - 8, NULL))
        analyzer_utils_add_text_tab (exif_tab, "UserComment", usercomment + 8, count - 8);
    else
        analyzer_utils_add_text_tab (exif_tab, "UserComment", "", 0);

    g_free (usercomment);
}

void
analyze_flashpixversion_tag (AnalyzerFile *file,
                             AnalyzerTab *exif_tab,
                             guint16 field_type,
                             guint32 count,
                             guint32 value_offset,
                             gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: FlashpixVersion"));

    value_offset = common_undefined_tag_structure (file, field_type, count, 4,
                                                   value_offset, is_little_endian);

    if (value_offset == 0x30303130)
        description_message = "Flashpix Format v1.0";
    else
        description_message = _("<span foreground=\"red\">INVALID</span>");

    analyzer_utils_describe_tooltip_tab (exif_tab, "FlashpixVersion", description_message,
                                         "FlashpixVersion\n"
                                         "<tt>30 30 31 30<sub>16</sub></tt>\tFlashpix Format v1.0");
}

void
analyze_filesource_tag (AnalyzerFile *file,
                        AnalyzerTab *exif_tab,
                        guint16 field_type,
                        guint32 count,
                        guint32 value_offset,
                        gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: FileSource"));

    value_offset = common_undefined_tag_structure (file, field_type, count, 1,
                                                   value_offset, is_little_endian);

    switch (value_offset)
    {
        case 0:
            description_message = _("Others");
        break;
        case 1:
            description_message = _("Scanner of transparent type");
        break;
        case 2:
            description_message = _("Scanner of reflex type");
        break;
        case 3:
            description_message = _("DSC");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "FileSource", description_message,
                                         _("FileSource\n"
                                         "<tt>00<sub>16</sub></tt>\tOthers\n"
                                         "<tt>01<sub>16</sub></tt>\tScanner of transparent type\n"
                                         "<tt>02<sub>16</sub></tt>\tScanner of reflex type\n"
                                         "<tt>03<sub>16</sub></tt>\tDSC"));
}

void
analyze_scenetype_tag (AnalyzerFile *file,
                       AnalyzerTab *exif_tab,
                       guint16 field_type,
                       guint32 count,
                       guint32 value_offset,
                       gboolean is_little_endian)
{
    gchar *description_message;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: SceneType"));

    value_offset = common_undefined_tag_structure (file, field_type, count, 1,
                                                   value_offset, is_little_endian);

    switch (value_offset)
    {
        case 1:
            description_message = _("A directly photographed image");
        break;
        default:
            description_message = _("<span foreground=\"red\">INVALID</span>");
    }

    analyzer_utils_describe_tooltip_tab (exif_tab, "SceneType", description_message,
                                         _("SceneType\n"
                                         "<tt>01<sub>16</sub></tt>\tA directly photographed image"));
}

void
analyze_cfapattern_tag (AnalyzerFile *file,
                        __attribute__((unused)) AnalyzerTab *exif_tab,
                        guint16 field_type,
                        guint32 count,
                        guint32 value_offset,
                        gboolean is_little_endian,
                        GSList **tagged_bytes)
{
    gsize save_pointer;

    analyzer_utils_tag (file, TIFF_TAG_COLOR, 2, _("Tag: CFAPattern"));

    value_offset = common_undefined_tag_structure (file, field_type, count, 0,
                                                   value_offset, is_little_endian);

    if (count > 4)
    {
        save_pointer = GET_POINTER (file);

        SET_POINTER (file, value_offset);
        *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

        ADVANCE_POINTER (file, count);
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_1, count, "CFAPattern");

        *tagged_bytes = g_slist_append (*tagged_bytes, GUINT_TO_POINTER (GET_POINTER (file)));

        SET_POINTER (file, value_offset);
        analyzer_utils_tag (file, VALUE_OFFSET_COLOR_2, 1, "CFAPattern");

        SET_POINTER (file, save_pointer);
    }
}
