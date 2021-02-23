/* gif-analyzer.c
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

#include <config.h>

#include "gif-analyzer.h"

#include <glib/gi18n.h>

#include "chirurgien-analyze-gif.h"


void
chirurgien_analyze_gif (AnalyzerFile *file)
{
    guint block_counts[BLOCKS];
    gchar *blocks[BLOCKS] = {
        _("Header"),
        _("Logical Screen Descriptor"),
        _("Image Descriptor"),
        _("Graphic Control Extension"),
        _("Plain Text Extension"),
        _("Application Extension"),
        _("Comment Extension"),
        _("Trailer")
    };

    AnalyzerTab graphics_tab;
    guint8 block_label;
    gchar *description_message;

    memset (block_counts, 0, sizeof (guint) * BLOCKS);

    analyzer_utils_set_title (file, "<span weight=\"bold\" size=\"larger\">"
                                    "Graphics Interchange Format"
                                    "</span>");

    analyzer_utils_tag_navigation (file, HEADER_BLOCK_COLOR, 6, _("GIF header block"), _("Header"));
    ADVANCE_POINTER (file, 6);
    block_counts[Header]++;

    if (!analyze_logical_screen_descriptor_block (file))
        return;
    block_counts[LogicalScreenDescriptor]++;

    analyzer_utils_init_tab (&graphics_tab);
    analyzer_utils_describe_tab (&graphics_tab, _("<b>Graphic rendering blocks</b>"), NULL);

    /* Block loop */
    while (FILE_HAS_DATA (file))
    {
        /* Loop should have ended at the Trailer */
        if (block_counts[Trailer])
        {
            analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data, file ends at Trailer block"));
            break;
        }

        analyzer_utils_read (&block_label, file, 1);

        if (block_label == 0x2C) // Image descriptor
        {
            analyzer_utils_tag_navigation (file, BLOCK_LABEL_COLOR, 1, _("Image Descriptor block label"),
                                           _("ID"));
            block_counts[ImageDescriptor]++;

            if (!analyze_image_descriptor_block (file, &graphics_tab))
                break;
        }
        else if (block_label == 0x3B) // Trailer
        {
            analyzer_utils_tag_navigation (file, BLOCK_LABEL_COLOR, 1, _("Trailer block label"),
                                           _("Trailer"));
            block_counts[Trailer]++;
        }
        else if (block_label == 0x21) // Extension block
        {
            if (!analyzer_utils_read (&block_label, file, 1))
            {
                analyzer_utils_tag_error (file, ERROR_COLOR_1, 1, _("Unrecognized block label"));
                break;
            }

            if (block_label == 0xFF) // Application Extension
            {
                analyzer_utils_tag_navigation (file, BLOCK_LABEL_COLOR, 2, _("Application Extension block label"),
                                               _("AE"));
                block_counts[ApplicationExtension]++;

                if (!analyze_application_ext_block (file))
                    break;
            }
            else if (block_label == 0xFE) // Comment Extension
            {
                analyzer_utils_tag_navigation (file, BLOCK_LABEL_COLOR, 2, _("Comment Extension block label"),
                                               _("CE"));
                block_counts[CommentExtension]++;

                if (!analyze_comment_ext_block (file))
                    break;
            }
            else if (block_label == 0xF9) // Graphic Control Extension
            {
                analyzer_utils_tag_navigation (file, BLOCK_LABEL_COLOR, 2, _("Graphic Control Extension block label"),
                                               _("GCE"));
                block_counts[GraphicalControlExtension]++;

                if (!analyze_graphic_control_ext_block (file, &graphics_tab))
                    break;
            }
            else if (block_label == 0x01) // Plain Text Extension
            {
                analyzer_utils_tag_navigation (file, BLOCK_LABEL_COLOR, 2, _("Plain Text Extension block label"),
                                               _("PTE"));
                block_counts[PlainTextExtension]++;

                if (!analyze_plain_text_ext_block (file, &graphics_tab))
                    break;
            }
            else
            {
                analyzer_utils_tag_navigation_error (file, ERROR_COLOR_1, 2, _("Unrecognized block label"),
                                                     _("???"));
                if (!process_data_subblocks (file, _("Unrecognized data block"), NULL, TRUE))
                {
                    analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
                    break;
                }
            }
        }
        else
        {
            analyzer_utils_tag_navigation_error (file, ERROR_COLOR_1, 1, _("Unrecognized block label"),
                                                 _("???"));
            if (!process_data_subblocks (file, _("Unrecognized data block"), NULL, TRUE))
            {
                analyzer_utils_tag_error (file, ERROR_COLOR_1, -1, _("Unrecognized data"));
                break;
            }
        }
    }

    analyzer_utils_insert_tab (file, &graphics_tab, _("Graphics"));

    analyzer_utils_set_subtitle (file, _("<b>Block count</b>"), NULL);

    for (gint i = Header; i < Trailer; i++)
    {
        if (block_counts[i])
        {
            description_message = g_strdup_printf ("%u", block_counts[i]);
            analyzer_utils_describe (file, blocks[i], description_message);
            g_free (description_message);
        }
    }
}

gboolean
process_gif_field (AnalyzerFile *file,
                   AnalyzerTab *tab,
                   gchar *field_name,
                   gchar *field_name_tag,
                   gchar *field_tooltip,
                   GdkRGBA *color,
                   guint field_length,
                   gchar *description_message,
                   void *read_value)
{
    gchar *field_description;
    guint32 field = 0;

    if (!analyzer_utils_read (&field, file, field_length))
        return FALSE;

    if (field_name_tag)
        analyzer_utils_tag (file, color, field_length, field_name_tag);
    else
        analyzer_utils_tag (file, color, field_length, field_name);

    if (description_message)
    {
        field_description = g_strdup_printf (description_message, field);
        if (tab)
            analyzer_utils_describe_tooltip_tab (tab, field_name, field_description, field_tooltip);
        else
            analyzer_utils_describe_tooltip (file, field_name, field_description, field_tooltip);
        g_free (field_description);
    }

    if (read_value && field_length == 1)
        *(guint8 *) read_value = field;

    return TRUE;
}

gboolean
process_data_subblocks (AnalyzerFile *file,
                        gchar *field_name,
                        GByteArray **array,
                        gboolean error)
{
    guint8 block_size;

    if (array)
        *array = g_byte_array_new ();

    while (FILE_HAS_DATA (file))
    {
        /* Block size */
        analyzer_utils_read (&block_size, file, 1);

        if (block_size)
        {
            if (error)
                analyzer_utils_tag_error (file, ERROR_COLOR_2, 1, _("Data block size"));
            else
                analyzer_utils_tag (file, DATA_SUBBLOCK_START_COLOR, 1, _("Data block size"));
        }
        else
        {
            if (error)
                analyzer_utils_tag_error (file, ERROR_COLOR_2, 1, _("Data block terminator"));
            else
                analyzer_utils_tag (file, DATA_SUBBLOCK_START_COLOR, 1, _("Data block terminator"));
            break;
        }

        if (!FILE_HAS_DATA_N (file, block_size))
            return FALSE;

        if (array)
            *array = g_byte_array_append (*array, file->file_contents + GET_POINTER (file), block_size);

        if (error)
            analyzer_utils_tag_error (file, ERROR_COLOR_1, block_size, field_name);
        else
            analyzer_utils_tag (file, BLOCK_DATA_COLOR_1, block_size, field_name);

        ADVANCE_POINTER (file, block_size);
    }

    return TRUE;
}
