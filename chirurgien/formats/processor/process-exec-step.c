/* process-exec-step.c
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
process_exec_step (ProcessorFile  *file,
                   RunStep        *run_step,
                   ProcessorState *state)
{
    ProcessorVariable *processor_var;

    guint64 op_value, exec_value;
    gint64 signed_op_value, signed_exec_value;

    gboolean index_used, rational_value;
    gdouble rational;

    gpointer selec_ptr;

    index_used = FALSE;

    selec_ptr = run_step->exec.signed_op ? (gpointer) &signed_op_value : (gpointer) &op_value;

    if (!g_strcmp0 (run_step->exec.var_id, "index"))
    {
        index_used = TRUE;
        op_value = file->file_contents_index;
    }
    else
    {
        processor_utils_read_value (state,
                                    run_step->exec.var_id,
                                    READ_VARIABLE,
                                    &processor_var,
                                    selec_ptr,
                                    run_step->exec.signed_op);
        /* Doesn't existing, create it */
        if (!processor_var)
        {
            processor_var = g_slice_new0 (ProcessorVariable);
            processor_var->size = 8;
            op_value = signed_op_value = processor_var->eight = 0;
            g_hash_table_insert (state->variables,
                                 run_step->exec.var_id,
                                 processor_var);
        }
    }

    rational_value = FALSE;

    if (run_step->exec.set)
    {
        if (!g_strcmp0 (run_step->exec.set, "index"))
        {
            op_value = file->file_contents_index;
        }
        else
        {
            processor_utils_read_value (state,
                                        run_step->exec.set,
                                        READ_VARIABLE | READ_NUMERIC,
                                        NULL,
                                        selec_ptr,
                                        run_step->exec.signed_op);
        }
    }

    selec_ptr = run_step->exec.signed_op ? (gpointer) &signed_exec_value : (gpointer) &exec_value;

    if (run_step->exec.modulo)
    {
        processor_utils_read_value (state,
                                    run_step->exec.modulo,
                                    READ_VARIABLE | READ_NUMERIC,
                                    NULL,
                                    selec_ptr,
                                    run_step->exec.signed_op);
        if (run_step->exec.signed_op)
            signed_op_value = signed_op_value % signed_exec_value;
        else
            op_value = op_value % exec_value;
    }
    if (run_step->exec.add)
    {
        processor_utils_read_value (state,
                                    run_step->exec.add,
                                    READ_VARIABLE | READ_NUMERIC,
                                    NULL,
                                    selec_ptr,
                                    run_step->exec.signed_op);
        if (run_step->exec.signed_op)
            signed_op_value += signed_exec_value;
        else
            op_value += exec_value;
    }
    if (run_step->exec.substract)
    {
        processor_utils_read_value (state,
                                    run_step->exec.substract,
                                    READ_VARIABLE | READ_NUMERIC,
                                    NULL,
                                    selec_ptr,
                                    run_step->exec.signed_op);
        if (run_step->exec.signed_op)
            signed_op_value -= signed_exec_value;
        else
            op_value = op_value > exec_value ? op_value - exec_value : 0;
    }
    if (run_step->exec.multiply)
    {
        processor_utils_read_value (state,
                                    run_step->exec.multiply,
                                    READ_VARIABLE | READ_NUMERIC,
                                    NULL,
                                    selec_ptr,
                                    run_step->exec.signed_op);
        if (run_step->exec.signed_op)
            signed_op_value *= signed_exec_value;
        else
            op_value *= exec_value;
    }
    if (run_step->exec.divide)
    {
        processor_utils_read_value (state,
                                    run_step->exec.divide,
                                    READ_VARIABLE | READ_NUMERIC,
                                    NULL,
                                    selec_ptr,
                                    run_step->exec.signed_op);
        if (run_step->exec.signed_op)
        {
            rational = signed_exec_value ? (gdouble) signed_op_value / signed_exec_value : 0.0;
        }
        else
        {
            rational = exec_value ? (gdouble) op_value / exec_value : 0.0;
        }
        rational_value = TRUE;

        op_value = (guint64) rational;
    }

    if (index_used)
    {
        file->file_contents_index = op_value;
    }
    else if (rational_value)
    {
        processor_var->rational_value = TRUE;
        processor_var->rational = rational;
    }
    else
    {
        processor_var->rational_value = FALSE;
        switch (processor_var->size)
        {
            case 1:
            if (run_step->exec.signed_op)
                processor_var->sone = signed_op_value;
            else
                processor_var->one = op_value;

            break;
            case 2:
            if (run_step->exec.signed_op)
                processor_var->stwo = signed_op_value;
            else
                processor_var->two = op_value;

            break;
            case 3:
            case 4:
            if (run_step->exec.signed_op)
                processor_var->sfour = signed_op_value;
            else
                processor_var->four = op_value;

            break;
            case 5:
            case 6:
            case 7:
            case 8:
            if (run_step->exec.signed_op)
                processor_var->seight = signed_op_value;
            else
                processor_var->eight = op_value;

            break;
        }
    }
}
