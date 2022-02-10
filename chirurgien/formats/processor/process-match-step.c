/* process-match-step.c
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
process_match_start_step (const FormatDefinition *format_definition,
                          ProcessorFile          *file,
                          RunStep                *run_step,
                          ProcessorState         *state,
                          GSList                 *run_iter)
{
    ProcessorVariable *processor_var;
    guint64 op_value;

    gboolean match_success, execute_match;
    gsize match_size;

    guchar match_data[8];

    SelectionScope *scope;

    processor_var = NULL;
    match_success = FALSE;
    execute_match = FALSE;

    /* Match uses a variable */
    if (run_step->match.var_id)
    {
        processor_utils_read_value (state,
                                    run_step->match.var_id,
                                    READ_VARIABLE,
                                    &processor_var,
                                    &op_value,
                                    FALSE);
        if (processor_var)
            execute_match = TRUE;
    }
    else
    {
        execute_match = TRUE;
    }

    if (execute_match)
    {
        /* Match using a processor variable */
        if (processor_var)
        {
            /* Match operation */
            if (run_step->match.op != OP_INVALID)
            {
                switch (run_step->match.op)
                {
                    case OP_EQUAL:
                    match_success = op_value == run_step->match.num_value;

                    break;
                    case OP_GREATER:
                    match_success = op_value > run_step->match.num_value;

                    break;
                    case OP_BIT:
                    match_success = (op_value >> run_step->match.num_value) & 1;

                    break;
                    default: // OP_DEFINED
                    if (op_value)
                        match_success = TRUE;
                }
            }
            /* Match variable against a value */
            else if (run_step->match.value)
            {
                match_size = MIN (processor_var->size, run_step->match.value_size);

                /* Apply value byte order conversion */
                if (run_step->match.convert_endianness)
                {
                    memcpy (match_data, run_step->match.value, match_size);
                    processor_utils_format_byte_order (format_definition,
                                                       state,
                                                       match_data,
                                                       match_size);
                }

                if (!memcmp (processor_var->value,
                             run_step->match.convert_endianness ? match_data : run_step->match.value,
                             match_size))
                {
                    match_success = TRUE;
                }
            }
            /* Match variable against the file */
            else
            {
                match_size = processor_var->size;

                if (FILE_HAS_DATA_N (file, match_size) &&
                    !memcmp (processor_var->value,
                             GET_CONTENT_POINTER (file),
                             match_size))
                {
                    match_success = TRUE;
                }
            }
        }
        /* Match against the file */
        else
        {
            match_size = run_step->match.value_size;

            if (run_step->match.value &&
                run_step->match.convert_endianness)
            {
                memcpy (match_data, run_step->match.value, match_size);
                processor_utils_format_byte_order (format_definition,
                                                   state,
                                                   match_data,
                                                   match_size);
            }

            /* No matching value or value actually matches = match success */
            if (!run_step->match.value ||
                (FILE_HAS_DATA_N (file, match_size) &&
                !memcmp (run_step->match.convert_endianness ? match_data : run_step->match.value,
                         GET_CONTENT_POINTER (file),
                         match_size)))
            {
                match_success = TRUE;
            }
        }
    }

    if (match_success && state->selection_stack.length)
    {
        state->match_depth++;

        scope = g_queue_peek_tail (&state->selection_stack);

        /* If matching within the scope of a selection, update selection state */
        if (!scope->used)
        {
            scope->used = TRUE;
            scope->match_depth = state->match_depth;
        }
    }
    else if (!match_success)
    {
        /* Skip all steps inside the match section */
        run_iter = processor_utils_skip_steps (run_iter->next,
                                               MATCH_START_STEP,
                                               MATCH_END_STEP);
    }

    return run_iter->next;
}

GSList *
process_match_end_step (ProcessorState *state,
                        GSList         *run_iter)
{
    SelectionScope *scope;

    if (state->selection_stack.length)
    {
        scope = g_queue_peek_tail (&state->selection_stack);

        if (scope->used && scope->match_depth == state->match_depth)
        {
           run_iter = processor_utils_skip_steps (run_iter,
                                                  SELECTION_START_STEP,
                                                  SELECTION_END_STEP);
        }
        else
        {
            run_iter = run_iter->next;
        }

        state->match_depth--;
    }
    else
    {
        run_iter = run_iter->next;
    }

    return run_iter;
}
