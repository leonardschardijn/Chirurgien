/* process-block-step.c
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

#include "processor.h"


GSList *
process_block_step (const FormatDefinition *format_definition,
                    RunStep                *run_step,
                    ProcessorState         *state,
                    GSList                 *run_iter)
{
    GSList *run_block;

    run_block = g_hash_table_lookup (format_definition->blocks,
                                     run_step->block.block_id);
    if (run_block)
    {
        g_queue_push_tail (&state->block_stack, run_iter->next);
        run_iter = run_block;
    }
    else
    {
        run_iter = run_iter->next;
    }

    return run_iter;
}
