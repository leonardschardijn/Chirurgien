/* processor.c
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

#define MAX_STEPS 50000000


gboolean
format_identify (const FormatDefinition *format_definition,
                 ProcessorFile          *file)
{
    ProcessorState state = { 0 };
    FieldDefinition dummy_field_def = { 0 };

    const MagicStep *magic_step;
    ProcessorVariable *processor_var;

    gsize step_offset;
    gboolean magic_failed, format_found;

    format_found = FALSE;

    for (GSList *magic = format_definition->magic;
         magic && !format_found;
         magic = magic->next)
    {
        magic_failed = FALSE;
        state.variables = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                 NULL, processor_variable_destroy);

        for (GSList *magic_iter = magic->data;
             magic_iter;
             magic_iter = magic_iter->next)
        {
            magic_step = magic_iter->data;

            if (magic_step->step_type == MATCH_STEP)
            {
                if (magic_step->match.offset)
                {
                    processor_utils_read_value (&state,
                                                magic_step->match.offset,
                                                READ_VARIABLE | READ_NUMERIC,
                                                NULL,
                                                &step_offset,
                                                FALSE);
                }
                else
                {
                    step_offset = 0;
                }

                if (FILE_HAS_DATA_N (file, step_offset + magic_step->match.value_size))
                {
                    if (!magic_step->match.value ||
                        !magic_step->match.value_size ||
                        memcmp (file->file_contents + step_offset,
                                magic_step->match.value,
                                magic_step->match.value_size))
                    {
                        magic_failed = TRUE;
                        break;
                    }
                }
                else
                {
                    magic_failed = TRUE;
                    break;
                }
            }
            else if (magic_step->step_type == READ_STEP)
            {
                if (magic_step->read.offset)
                {
                    processor_utils_read_value (&state,
                                                magic_step->match.offset,
                                                READ_VARIABLE | READ_NUMERIC,
                                                NULL,
                                                &step_offset,
                                                FALSE);
                }
                else
                {
                    step_offset = 0;
                }

                file->file_contents_index = step_offset;

                if (magic_step->read.var_id && magic_step->read.size <= 8)
                {
                    processor_var = g_slice_new0 (ProcessorVariable);

                    dummy_field_def.size = processor_var->size = magic_step->read.size;

                    if (processor_utils_read (format_definition,
                                              &state,
                                              file,
                                              &dummy_field_def,
                                              TRUE,
                                              processor_var->value))
                    {
                        g_hash_table_insert (state.variables,
                                             magic_step->read.var_id,
                                             processor_var);
                    }
                    else
                    {
                        g_slice_free (ProcessorVariable, processor_var);
                    }
                }

                file->file_contents_index = 0;
            }
        }

        g_hash_table_destroy (state.variables);

        if (!magic_failed)
            format_found = TRUE;
    }

    return format_found;
}

void
format_process (const FormatDefinition *format_definition,
                ProcessorFile          *file)
{
    ProcessorState state = { 0 };
    RunStep *run_step;

    GSList *run_iter;
    guint run_steps_executed;

    if (!format_definition)
    {
        processor_utils_set_title (file, "Unrecognized file format");
        return;
    }

    state.variables = g_hash_table_new_full (g_str_hash, g_str_equal,
                                             NULL, processor_variable_destroy);
    state.tabs = g_hash_table_new_full (g_str_hash, g_str_equal,
                                        NULL, description_tab_destroy);

    /* Process the format */
    processor_utils_set_title (file, format_definition->format_name);

    run_steps_executed = 0;
    run_iter = format_definition->run;

    while (run_steps_executed < MAX_STEPS)
    {
        if (state.block_stack.length)
            run_iter = g_queue_pop_tail (&state.block_stack);
        else if (run_steps_executed)
            break;

        while (run_iter && run_steps_executed < MAX_STEPS)
        {
            run_step = run_iter->data;

            /* RunStep type switch */
            switch (run_step->step_type)
            {
                case FIELD_STEP:
                process_field_step (format_definition, file, run_step, &state);
                run_iter = run_iter->next;

                break;
                case MATCH_START_STEP:
                run_iter = process_match_start_step (format_definition, file, run_step, &state, run_iter);

                break;
                case MATCH_END_STEP:
                run_iter = process_match_end_step (&state, run_iter);

                break;
                case LOOP_START_STEP:
                run_iter = process_loop_start_step (file, run_step, &state, run_iter);

                break;
                case LOOP_END_STEP:
                run_iter = process_loop_end_step (&state, run_iter);

                break;
                case SELECTION_START_STEP:
                process_selection_start_step (&state);
                run_iter = run_iter->next;

                break;
                case SELECTION_END_STEP:
                process_selection_end_step (&state);
                run_iter = run_iter->next;

                break;
                case PRINT_STEP:
                process_print_step (file, run_step, &state);
                run_iter = run_iter->next;

                break;
                case EXEC_STEP:
                process_exec_step (file, run_step, &state);
                run_iter = run_iter->next;

                break;
                case BLOCK_STEP:
                run_iter = process_block_step (format_definition, run_step, &state, run_iter);

                break;
            }
            run_steps_executed++;
        }
    }

    processor_utils_sort_find_unused (format_definition,
                                      file);

    /* Clear state */
    g_queue_clear (&state.loop_stack);
    g_queue_clear_full (&state.selection_stack, selection_scope_destroy);
    g_queue_clear (&state.block_stack);
    g_hash_table_destroy (state.variables);
    g_hash_table_destroy (state.tabs);
}
