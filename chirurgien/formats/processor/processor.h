/* processor.h
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

#include "processor-utils.h"

G_BEGIN_DECLS

void        process_field_step              (const FormatDefinition *,
                                             ProcessorFile *,
                                             RunStep *,
                                             ProcessorState *);

GSList *    process_match_start_step        (const FormatDefinition *,
                                             ProcessorFile *,
                                             RunStep *,
                                             ProcessorState *,
                                             GSList *);
GSList *    process_match_end_step          (ProcessorState *,
                                             GSList *);

GSList *    process_loop_start_step         (ProcessorFile *,
                                             RunStep *,
                                             ProcessorState *,
                                             GSList *);
GSList *    process_loop_end_step           (ProcessorState *,
                                             GSList *);

void        process_selection_start_step    (ProcessorState *);
void        process_selection_end_step      (ProcessorState *);

void        process_print_step              (ProcessorFile *,
                                             RunStep *,
                                             ProcessorState *);

void        process_exec_step               (ProcessorFile *,
                                             RunStep *,
                                             ProcessorState *);

GSList *    process_block_step              (const FormatDefinition *,
                                             RunStep *,
                                             ProcessorState *,
                                             GSList *);

G_END_DECLS
