/* validator.h
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

#include "validator-utils.h"

G_BEGIN_DECLS

/* Subparser states */
typedef enum
{
    /* Subparser hasn't been executed */
    PARSER_READY = 0,
    /* Subparser is being executed */
    PARSER_EXECUTING,
    /* Subparser has already been executed */
    PARSER_DONE

} SubparserState;

typedef struct
{
    /* The format definition being built */
    FormatDefinition    *definition;

    /* Current magic steps list being built */
    GSList              *magic_steps;
    /* Current run steps list being built */
    GSList              *run_steps;

    /* XML section flags */
    gboolean             format_root_found;
    SubparserState       endianness_state;
    SubparserState       magic_state;
    SubparserState       colors_state;
    SubparserState       run_state;
    SubparserState       block_defs_state;
    SubparserState       field_defs_state;

    /* XML document nesting depth */
    guint                depth;

    /* If the XML tag needs to be closed
     * It cannot have anything nested */
    gboolean             field_closure_needed;
    gboolean             print_closure_needed;
    gboolean             exec_closure_needed;
    gboolean             block_closure_needed;
    /* The depth at which the close tag is needed */
    guint                closure_depth;

    /* Current run block being built */
    gchar               *current_block_id;

    /* Current field definition being built */
    FieldDefinition     *current_field;
    /* If the field has a fixed set of possible values */
    gboolean             field_has_options;

} ParserControl;

/* Format definition parsers */
extern GMarkupParser root_parser;
extern GMarkupParser endianness_parser;
extern GMarkupParser magic_parser;
extern GMarkupParser colors_parser;
extern GMarkupParser run_parser;
extern GMarkupParser run_blocks_parser;
extern GMarkupParser field_defs_parser;

G_END_DECLS
