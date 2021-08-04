/* chirurgien-formats.c
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

#include "chirurgien-formats.h"

#include <glib/gi18n.h>
#include "format-utils.h"

#include "cpio/chirurgien-cpio.h"
#include "elf/chirurgien-elf.h"
#include "gif/chirurgien-gif.h"
#include "jpeg/chirurgien-jpeg.h"
#include "pe/chirurgien-pe.h"
#include "png/chirurgien-png.h"
#include "tar/chirurgien-tar.h"
#include "tiff/chirurgien-tiff.h"


gboolean
chirurgien_formats_analyze (FormatsFile *file)
{
    const guchar cpio_magic_number1[] = { 0xC7,0x71 };
    const guchar cpio_magic_number2[] = { 0x71,0xC7 };
    const guchar cpio_magic_number3[] = { 0x30,0x37,0x30,0x37,0x30,0x37 };
    const guchar cpio_magic_number4[] = { 0x30,0x37,0x30,0x37,0x30,0x31 };
    const guchar cpio_magic_number5[] = { 0x30,0x37,0x30,0x37,0x30,0x32 };
    const guchar dos_mz_magic_number[] = { 0x4D,0x5A };
    const guchar elf_magic_number[] = { 0x7F,0x45,0x4C,0x46 };
    const guchar gif_magic_number1[] = { 0x47,0x49,0x46,0x38,0x39,0x61 };
    const guchar gif_magic_number2[] = { 0x47,0x49,0x46,0x38,0x37,0x61 };
    const guchar jpeg_magic_number[] = { 0xFF,0xD8 };
    const guchar pe_magic_number[] = { 0x50,0x45 };
    const guchar png_magic_number[] = { 0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A };
    const guchar tar_magic_number[] = { 0x75,0x73,0x74,0x61,0x72 };
    const guchar tiff_magic_number1[] = { 0x49,0x49,0x2A,0x00 };
    const guchar tiff_magic_number2[] = { 0x4D,0x4D,0x00,0x2A };

    gboolean format_has_unused_bytes = FALSE;

    /* cpio */
    if (FILE_HAS_DATA_N (file, 2) &&
        !memcmp (file->file_contents, cpio_magic_number1, 2))
    {
        chirurgien_cpio (file, BinaryLittleEndian);
    }
    else if (FILE_HAS_DATA_N (file, 2) &&
        !memcmp (file->file_contents, cpio_magic_number2, 2))
    {
        chirurgien_cpio (file, BinaryBigEndian);
    }
    else if (FILE_HAS_DATA_N (file, 6) &&
        !memcmp (file->file_contents, cpio_magic_number3, 6))
    {
        chirurgien_cpio (file, OldASCII);
    }
    else if (FILE_HAS_DATA_N (file, 6) &&
        !memcmp (file->file_contents, cpio_magic_number4, 6))
    {
        chirurgien_cpio (file, NewASCII);
    }
    else if (FILE_HAS_DATA_N (file, 6) &&
        !memcmp (file->file_contents, cpio_magic_number5, 6))
    {
        chirurgien_cpio (file, NewCRC);
    }
    /* DOS MZ & PE */
    else if (FILE_HAS_DATA_N (file, 2) &&
        !memcmp (file->file_contents, dos_mz_magic_number, 2))
    {
        guint32 pe_offset;

        if (FILE_HAS_DATA_N (file, 64))
            memcpy (&pe_offset, file->file_contents + 60, 4);
        else
            pe_offset = 0;

        if (pe_offset && FILE_HAS_DATA_N (file, pe_offset + 4) &&
            !memcmp (file->file_contents + pe_offset, pe_magic_number, 2))
        {
            chirurgien_pe (file, pe_offset);
            format_has_unused_bytes = TRUE;
        }
    }
    /* ELF */
    else if (FILE_HAS_DATA_N (file, 4) &&
        !memcmp (file->file_contents, elf_magic_number, 4))
    {
        chirurgien_elf (file);
        format_has_unused_bytes = TRUE;
    }
    /* GIF */
    else if (FILE_HAS_DATA_N (file, 6) &&
             (!memcmp (file->file_contents, gif_magic_number1, 6) ||
              !memcmp (file->file_contents, gif_magic_number2, 6)))
    {
        chirurgien_gif (file);
    }
    /* JPEG */
    else if (FILE_HAS_DATA_N (file, 2) &&
             !memcmp (file->file_contents, jpeg_magic_number, 2))
    {
        chirurgien_jpeg (file);
    }
    /* PNG */
    else if (FILE_HAS_DATA_N (file, 8) &&
             !memcmp (file->file_contents, png_magic_number, 8))
    {
        chirurgien_png (file);
    }
    /* tar */
    else if (FILE_HAS_DATA_N (file, 262) &&
             !memcmp (file->file_contents + 257, tar_magic_number, 5))
    {
        chirurgien_tar (file);
    }
    /* TIFF */
    else if (FILE_HAS_DATA_N (file, 4) &&
             (!memcmp (file->file_contents, tiff_magic_number1, 4) ||
              !memcmp (file->file_contents, tiff_magic_number2, 4)))
    {
        chirurgien_tiff (file);
        format_has_unused_bytes = TRUE;
    }
    else
    {
        format_utils_set_title (file, _("Unrecognized file format"));
    }

    return format_has_unused_bytes;
}
