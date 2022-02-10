/* process-loop-step.c
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
process_loop_start_step (ProcessorFile  *file,
                         RunStep        *run_step,
                         ProcessorState *state,
                         GSList         *run_iter)
{
    ProcessorVariable *processor_var;
    guint64 op_value, compare_value;

    gboolean break_loop;

    break_loop = FALSE;

    /* Test limits */
    if (state->file_end_reached)
    {
        break_loop = TRUE;
    }
    else if (run_step->loop.limit)
    {
        processor_utils_read_value (state,
                                    run_step->loop.limit,
                                    READ_VARIABLE,
                                    &processor_var,
                                    &op_value,
                                    FALSE);
        if (processor_var && (!op_value || processor_var->failed))
            break_loop = TRUE;
    }

    /* Evaluate loop conditions */
    if (!break_loop)
    {
        /* Get control variable */
        if (run_step->loop.until_set)
        {
            processor_utils_read_value (state,
                                        run_step->loop.until_set,
                                        READ_VARIABLE,
                                        &processor_var,
                                        &op_value,
                                        FALSE);
            if (processor_var)
            {
                if (run_step->loop.var_value)
                {
                    processor_utils_read_value (state,
                                                run_step->loop.var_value,
                                                READ_VARIABLE,
                                                NULL,
                                                &compare_value,
                                                FALSE);
                    if (op_value == compare_value)
                        break_loop = TRUE;
                }
                else if (run_step->loop.num_value_used)
                {
                    if (op_value == run_step->loop.num_value)
                        break_loop = TRUE;
                }
                else if (op_value)
                {
                    break_loop = TRUE;
                }
            }
        }
        else if (!FILE_HAS_DATA (file))
        {
            break_loop = TRUE;
        }
    }

    if (break_loop)
    {
        /* Skip all steps inside the loop */
        run_iter = processor_utils_skip_steps (run_iter->next,
                                               LOOP_START_STEP,
                                               LOOP_END_STEP);
    }
    else
    {
        g_queue_push_tail (&state->loop_stack, run_iter);
    }

    return run_iter->next;
}

GSList *
process_loop_end_step (ProcessorState *state,
                       GSList         *run_iter)
{
    /* Continue loop */
    if (state->loop_stack.length)
        run_iter = g_queue_pop_tail (&state->loop_stack);
    else
        run_iter = run_iter->next;

    return run_iter;
}
