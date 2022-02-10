/* process-print-step.c
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
process_print_step (ProcessorFile  *file,
                    RunStep        *run_step,
                    ProcessorState *state)
{
    DescriptionTab *tab;

    ProcessorVariable *processor_var;
    guint64 op_value;

    g_autofree gchar *print_value = NULL;

    processor_utils_read_value (state,
                                run_step->print.var_id,
                                READ_VARIABLE,
                                &processor_var,
                                &op_value,
                                FALSE);

    tab = NULL;

    if (run_step->print.tab)
    {
        tab = g_hash_table_lookup (state->tabs,
                                   run_step->print.tab);
        if (!tab)
        {
            tab = processor_utils_new_tab (run_step->print.section);
            g_hash_table_insert (state->tabs,
                                 run_step->print.tab,
                                 tab);
        }
        else if (run_step->print.section)
        {
            processor_utils_start_section_tab (tab, run_step->print.section);
        }
    }
    else if (run_step->print.section)
    {
        processor_utils_start_section (file, run_step->print.section);
    }

    if (processor_var)
    {
        if (processor_var->rational_value)
            print_value = g_strdup_printf ("%f", processor_var->rational);
        else if (run_step->print.signed_val)
            print_value = g_strdup_printf ("%ld", op_value);
        else
            print_value = g_strdup_printf ("%lu", op_value);
    }

    if (print_value || !run_step->print.omit_undefined)
    {
        if (tab)
        {
            if (run_step->print.no_section)
                processor_utils_add_note_tab (tab,
                                              run_step->print.line);
            else
                processor_utils_add_line_tab (tab,
                                              run_step->print.line,
                                              print_value ? print_value : NULL,
                                              run_step->print.tooltip,
                                              run_step->print.margin_top,
                                              run_step->print.margin_bottom);
        }
        else
        {
            processor_utils_add_line (file,
                                      run_step->print.line,
                                      print_value ? print_value : NULL,
                                      run_step->print.tooltip,
                                      run_step->print.margin_top,
                                      run_step->print.margin_bottom);
        }
    }

    if (tab && run_step->print.insert_tab)
    {
        processor_utils_insert_tab (file,
                                    tab,
                                    run_step->print.tab);
        g_hash_table_remove (state->tabs,
                             run_step->print.tab);
    }
}
