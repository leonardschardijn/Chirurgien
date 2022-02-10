/* process-selection-step.c
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


void
process_selection_start_step (ProcessorState *state)
{
    SelectionScope *scope;

    scope = g_slice_new0 (SelectionScope);

    /* Start selection scope */
    g_queue_push_tail (&state->selection_stack, scope);
}

void
process_selection_end_step (ProcessorState *state)
{
    /* End selection scope */
    if (state->selection_stack.length)
        selection_scope_destroy (g_queue_pop_tail (&state->selection_stack));
}
