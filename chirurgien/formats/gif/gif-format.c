/* gif-format.c
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

#include "gif-format.h"

#include <glib/gi18n.h>

#include "chirurgien-gif.h"


void
chirurgien_gif (FormatsFile *file)
{
    const gchar *blocks[BLOCKS] = {
        _("Header"),
        _("Logical Screen Descriptor"),
        _("Image Descriptor"),
        _("Graphic Control Extension"),
        _("Plain Text Extension"),
        _("Application Extension"),
        _("Comment Extension"),
        _("Trailer")
    };

    gint block_counts[BLOCKS];

    DescriptionTab graphics_tab;
    guint8 block_label[2];
    gchar *value;

    memset (block_counts, 0, sizeof (gint) * BLOCKS);

    format_utils_set_title (file, "Graphics Interchange Format");

    format_utils_add_field (file, HEADER_BLOCK_COLOR, TRUE, 6, _("GIF header block"), _("Header"));

    format_utils_start_section (file, _("Image details"));
    block_counts[Header]++;

    if (!gif_logical_screen_descriptor_block (file))
    {
        format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                              _("Unrecognized data"), NULL);
        return;
    }
    block_counts[LogicalScreenDescriptor]++;

    format_utils_init_tab (&graphics_tab, NULL);

    /* Block loop */
    while (FILE_HAS_DATA (file))
    {
        /* Loop should have ended at the Trailer */
        if (block_counts[Trailer])
        {
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                      _("Unrecognized data, file ends at Trailer block"), NULL);
            break;
        }

        format_utils_read (file, block_label, 1);

        if (block_label[0] == 0x2C) // Image descriptor
        {
            format_utils_add_field (file, BLOCK_LABEL_COLOR, TRUE, 1,
                                  _("Image Descriptor block label"), _("ID"));
            block_counts[ImageDescriptor]++;

            if (!gif_image_descriptor_block (file, &graphics_tab))
                break;
        }
        else if (block_label[0] == 0x3B) // Trailer
        {
            format_utils_add_field (file, BLOCK_LABEL_COLOR, TRUE, 1,
                                  _("Trailer block label"), _("Trailer"));
            block_counts[Trailer]++;
        }
        else if (block_label[0] == 0x21) // Extension block
        {
            if (!format_utils_read (file, block_label, 2))
            {
                format_utils_add_field (file, ERROR_COLOR_1, FALSE, 1,
                                      _("Unknown block label"), NULL);
                break;
            }

            if (block_label[1] == 0xFF) // Application Extension
            {
                format_utils_add_field (file, BLOCK_LABEL_COLOR, TRUE, 2,
                                      _("Application Extension block label"), _("AE"));
                block_counts[ApplicationExtension]++;

                if (!gif_application_ext_block (file))
                    break;
            }
            else if (block_label[1] == 0xFE) // Comment Extension
            {
                format_utils_add_field (file, BLOCK_LABEL_COLOR, TRUE, 2,
                                      _("Comment Extension block label"), _("CE"));
                block_counts[CommentExtension]++;

                if (!gif_comment_ext_block (file))
                    break;
            }
            else if (block_label[1] == 0xF9) // Graphic Control Extension
            {
                format_utils_add_field (file, BLOCK_LABEL_COLOR, TRUE, 2,
                                      _("Graphic Control Extension block label"), _("GCE"));
                block_counts[GraphicalControlExtension]++;

                if (!gif_graphic_control_ext_block (file, &graphics_tab))
                    break;
            }
            else if (block_label[1] == 0x01) // Plain Text Extension
            {
                format_utils_add_field (file, BLOCK_LABEL_COLOR, TRUE, 2,
                                      _("Plain Text Extension block label"), _("PTE"));
                block_counts[PlainTextExtension]++;

                if (!gif_plain_text_ext_block (file, &graphics_tab))
                    break;
            }
            else
            {
                format_utils_add_field (file, ERROR_COLOR_1, FALSE, 2,
                                      _("Unrecognized block label"), _("???"));

                if (!process_data_subblocks (file, _("Unrecognized data block"), NULL,
                                             ERROR_COLOR_2, ERROR_COLOR_1, FALSE))
                    break;
            }
        }
        else
        {
            format_utils_add_field (file, ERROR_COLOR_1, FALSE, 1,
                                  _("Unknown block label"), _("???"));

            if (!process_data_subblocks (file, _("Unrecognized data block"), NULL,
                                         ERROR_COLOR_2, ERROR_COLOR_1, FALSE))
                break;
        }
    }

    /* If there is still data available after the loop, tag it as unrecognized */
    format_utils_add_field (file, ERROR_COLOR_1, FALSE, G_MAXUINT,
                          _("Unrecognized data"), NULL);

    format_utils_insert_tab (file, &graphics_tab, _("Graphics"));

    if (block_counts[Header])
    {
        format_utils_start_section (file, _("Block count"));

        for (gint i = Header; i <= Trailer; i++)
        {
            if (block_counts[i])
            {
                value = g_strdup_printf ("%u", block_counts[i]);
                format_utils_add_line (file, blocks[i], value, NULL);
                g_free (value);
            }
        }
    }
}

gboolean
process_gif_field (FormatsFile    *file,
                   DescriptionTab *tab,
                   const gchar    *field_name,
                   const gchar    *field_name_tag,
                   const gchar    *field_tooltip,
                   gint            color_index,
                   guint           field_length,
                   const gchar    *value_format,
                   guint8         *read_value)
{
    gchar *field_description;
    guint32 field = 0;

    if (!format_utils_read (file, &field, field_length))
        return FALSE;

    if (field_name_tag)
        format_utils_add_field (file, color_index, TRUE, field_length, field_name_tag, NULL);
    else
        format_utils_add_field (file, color_index, TRUE, field_length, field_name, NULL);

    if (value_format)
    {
        field_description = g_strdup_printf (value_format, field);
        if (tab)
            format_utils_add_line_tab (tab, field_name, field_description, field_tooltip);
        else
            format_utils_add_line (file, field_name, field_description, field_tooltip);
        g_free (field_description);
    }

    if (read_value && field_length == 1)
        *(guint8 *) read_value = field;

    return TRUE;
}

gboolean
process_data_subblocks (FormatsFile *file,
                        const gchar *field_name,
                        GByteArray **array,
                        gint         size_color_index,
                        gint         data_color_index,
                        gboolean     background)
{
    guint8 block_size;

    if (array)
        *array = g_byte_array_new ();

    while (FILE_HAS_DATA (file))
    {
        /* Block size */
        format_utils_read (file, &block_size, 1);

        if (block_size)
        {
            format_utils_add_field (file, size_color_index, background, 1, _("Data block size"), NULL);
        }
        else
        {
            format_utils_add_field (file, size_color_index, background, 1, _("Data block terminator"), NULL);
            break;
        }

        if (!FILE_HAS_DATA_N (file, block_size))
            return FALSE;

        if (array)
            *array = g_byte_array_append (*array, GET_CONTENT_POINTER (file), block_size);

        format_utils_add_field (file, data_color_index, background, block_size, field_name, NULL);
    }

    return TRUE;
}
