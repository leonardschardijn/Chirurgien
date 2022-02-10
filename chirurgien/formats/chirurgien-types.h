/* chirurgien-types.h
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

#include <glib.h>

G_BEGIN_DECLS

/*** Enumerations ***/

/* Format endianness */
typedef enum
{
    INVALID_ENDIANNESS = 0,
    BIG_ENDIAN_FORMAT,
    LITTLE_ENDIAN_FORMAT,
    VARIABLE_ENDIANNESS

} Endianness;

/* MagicStep types */
typedef enum
{
    MATCH_STEP,
    READ_STEP

} MagicStepType;

/* RunStep types */
typedef enum
{
    FIELD_STEP,
    MATCH_START_STEP,
    MATCH_END_STEP,
    SELECTION_START_STEP,
    SELECTION_END_STEP,
    LOOP_START_STEP,
    LOOP_END_STEP,
    PRINT_STEP,
    EXEC_STEP,
    BLOCK_STEP

} RunStepType;

/* Match step operations */
typedef enum
{
    OP_INVALID = 0,
    OP_DEFINED,
    OP_EQUAL,
    OP_GREATER,
    OP_BIT

} MatchOperation;

/* Field definition size type */
typedef enum
{
    FIXED_SIZE = 0,
    AVAILABLE_SIZE,
    VALUE_SIZE

} FieldSizeType;

/* Field definition print type */
typedef enum
{
    NO_PRINT = 0,
    PRINT_INT,
    PRINT_UINT,
    PRINT_TEXT,
    PRINT_OPTION,
    PRINT_FLAGS,
    PRINT_LITERAL

} FieldPrintType;

/* Field definition text field encoding */
typedef enum
{
    ENCODING_UTF8 = 0,
    ENCODING_UTF16LE,
    ENCODING_UTF32LE,
    ENCODING_UTF16BE,
    ENCODING_UTF32BE,
    ENCODING_ISO_8859_1

} TextEncoding;

/*** Structures ***/

/* A format definition
 * The in-memory representation of an XML definition */
typedef struct
{
    /* Format disabled, indicates that the format should not be used */
    gboolean         disabled;

    /* The format's name */
    gchar           *format_name;
    /* The format's short name */
    gchar           *short_format_name;

    /* Format description/details */
    gchar           *details;

    /* Format endianness */
    Endianness       endianness;
    /* Variable name of the variable holding the endianness value */
    gchar           *endianness_var;
    /* Value that indicates big-endian */
    gpointer         be_value;
    gsize            be_value_size;
    /* Value that indicates little-endian */
    gpointer         le_value;
    gsize            le_value_size;

    /* List of Lists of MagicSteps
     * Used to identify the file format */
    GSList          *magic;

    /* Associative array of FormatColors
     * A collection of all colors used by the format */
    GHashTable      *colors;

    /* List of RunSteps
     * Used to analyze the file format */
    GSList          *run;

    /* Associative array of reusable blocks of RunSteps */
    GHashTable      *blocks;

    /* Associative array of FieldDefinitions
     * A collection of all possible fields in the format */
    GHashTable      *fields;

} FormatDefinition;

/* Possible MagicStep payloads */

/* A match MagicStep */
typedef struct
{
    /* The value used for the match */
    gpointer         value;
    /* The size of the value */
    gsize            value_size;

    /* The step's offset */
    gchar           *offset;

} MatchMStep;

/* A read MagicStep */
typedef struct
{
    /* The variable name used to store the value */
    gchar           *var_id;
    /* The variable's size */
    gsize            size;

    /* The step's offset */
    gchar           *offset;

} ReadMStep;

/* A step in the file format identification process */
typedef struct
{
    /* The step's type
     * Used to identify the payload */
    MagicStepType    step_type;

    /* The MagicStep's payload */
    union
    {
        MatchMStep   match;
        ReadMStep    read;
    };

} MagicStep;

/* A color used by the format */
typedef struct
{
    /* The color's name */
    gchar           *color_name;
    /* The color's index (0-8) */
    guint            color_index;
    /* Color is a background or foreground color */
    gboolean         background; /* TRUE = background, FALSE = foregound */

} FormatColor;

/* Possible RunStep payloads */

/* A field RunStep */
typedef struct
{
    /* Field ID */
    gchar           *field_id;

    /* The name of the variable
     * Store the value of the field, so it can be accessed later */
    gchar           *store_var;
    /* The stored value should be converted to the format's endianness */
    gboolean         convert_endianness;
    /* The stored value is an ASCII-encoded number with the specified base (2..36) */
    guint            ascii_base;

    /* The navigation label, if this field produces one */
    gchar           *navigation;
    /* The navigation string max. size,
     * if the navigation label is a variable holding an offset */
    guint            navigation_limit;

    /* The field's offset value, a variable name with the field's offset */
    gchar           *offset;
    /* The field's additional color,
     * helps identify fields that were created with an offset */
    gchar           *additional_color;

    /* The description tab, if this fields should be added to a tab */
    gchar           *tab;
    /* This field signals the tab insertion */
    gboolean         insert_tab;

    /* The section name, if this fields starts a new description section */
    gchar           *section;

    /* The limit value, a variable name with the field's limit */
    gchar           *limit;

    /* If the field accepts failed limits */
    gboolean         limit_failed;

    /* Print margins, if the field prints a value */
    gint             margin_top;
    gint             margin_bottom;

    /* If the value printed by this field should not be printed */
    gboolean         suppress_print;

} FieldStep;

/* A match RunStep */
typedef struct
{
    /* The variable used for the match */
    gchar           *var_id;

    /* The operation to perform */
    MatchOperation   op;

    /* The numeric value used for the match */
    guint64          num_value;

    /* The value used for the match */
    gpointer         value;
    /* The size of the value */
    gsize            value_size;

    /* The value should be converted (from big-endian) to the format's endianness */
    gboolean         convert_endianness;

} MatchStep;

/* A loop step */
typedef struct
{
    /* The variable used to control the loop */
    gchar           *until_set;

    /* The variable value used for matching against the control variable */
    gchar           *var_value;

    /* The numeric value used for matching against the control variable */
    guint64          num_value;
    /* If the numeric value is set */
    gboolean         num_value_used;

    /* The limit value, a variable with the loop's limit
     * If the fields inside a loop are constrained by a limit,
     * the loop should be aware of such a limit, in order to stop
     * looping on limit failure */
    gchar           *limit;

} LoopStep;

/* A print step */
typedef struct
{
    /* The line to print */
    gchar           *line;

    /* The line's tooltip */
    gchar           *tooltip;

    /* Print line margins */
    gint             margin_top;
    gint             margin_bottom;

    /* The variable value to be printed */
    gchar           *var_id;
    /* The variable value is signed */
    gboolean         signed_val;

    /* Prevents the line from being printed if the variable is undefined */
    gboolean         omit_undefined;

    /* Print the line outside of a description section */
    gboolean         no_section;

    /* The section name, if this print starts a new description section */
    gchar           *section;

    /* The description tab, if this print should be added to a tab */
    gchar           *tab;
    /* This print signals the tab insertion */
    gboolean         insert_tab;

} PrintStep;

/* An exec step */
typedef struct
{
    /* The variable to operate on */
    gchar           *var_id;

    /* The value to set the variable to */
    gchar           *set;

    /* The value to use as modulo */
    gchar           *modulo;

    /* The value to add */
    gchar           *add;

    /* The value to substract */
    gchar           *substract;

    /* The value to multiply by */
    gchar           *multiply;

    /* The value to divide by */
    gchar           *divide;

    /* The operands are signed */
    gboolean         signed_op;

} ExecStep;

/* A block step */
typedef struct
{
    /* The block to execute */
    gchar           *block_id;

} BlockStep;

/* A step in the file format analysis process */
typedef struct
{
    /* The step's type
     * Used to identify the payload */
    RunStepType      step_type;

    /* The RunStep's payload */
    union
    {
        FieldStep    field;
        MatchStep    match;
        LoopStep     loop;
        PrintStep    print;
        ExecStep     exec;
        BlockStep    block;
    };

} RunStep;

/* A field option definition */
typedef struct
{
    /* The option's name */
    gchar           *name;
    /* The option's value */
    gpointer         value;

} FieldDefinitionOption;

/* A field flag definition */
typedef struct
{
    /* The flag's name */
    gchar           *name;
    /* The flag's mask */
    guint64          mask;

    /* The flag's meaning */
    gchar           *meaning;

} FieldDefinitionFlag;

/* A field definition */
typedef struct
{
    /* Field name */
    gchar           *name;
    /* Field tag
     * Used in the Hex/Text view of a file
     * If unset, the field's name is used */
    gchar           *tag;

    /* The field's tooltip
     * Used if the field is printed to the description panel */
    gchar           *tooltip;
    /* Should the tooltip be automatically generated */
    gboolean         auto_tooltip;

    /* The field's color */
    gchar           *color;

    /* The field's size type */
    FieldSizeType    size_type;
    /* The field's fixed size */
    gsize            size;
    /* The byte value used to find the field's size */
    guchar           value;

    /* The field's mask, if the field covers only a selection of bits */
    guint64          mask;
    /* The field's left shift */
    guint            shift;

    /* How the field should be printed to the description panel
     * (should this field be printed) */
    FieldPrintType   print;
    /* The string to print (PRINT_LITERAL) */
    gchar           *print_literal;
    /* Field encoding (only used in text fields, PRINT_TEXT) */
    TextEncoding     encoding;

    /* For fields with fixed values: a list of FieldDefinitionOptions
     * For flags fields: a list of FieldDefinitionFlags */
    GSList          *value_collection;
    /* Format endianness should be taken into account when matching values */
    gboolean         convert_endianness;

} FieldDefinition;


/* Processor definitions */

typedef struct
{
    /* A match step has claimed the selection */
    gboolean         used;
    /* The match depth at which the selection was claimed */
    guint            match_depth;

} SelectionScope;

/* Processor variable */
typedef struct
{
    /* If the variable failed when used as a limit */
    gboolean         failed;

    /* Variable size */
    gsize            size;

    /* The variable holds a rational value */
    gboolean         rational_value;

    /* Variable value - max. 8 bytes */
    union
    {
        guint8       one;
        gint8        sone;
        guint16      two;
        gint16       stwo;
        guint32      four;
        gint32       sfour;
        guint64      eight;
        gint64       seight;
        gdouble      rational;
        guchar       value[8];
   };

} ProcessorVariable;

typedef struct
{
    GQueue           loop_stack;
    GQueue           selection_stack;
    GQueue           block_stack;

    guint            match_depth;

    GHashTable      *variables;
    GHashTable      *tabs;

    gboolean         file_end_reached;

} ProcessorState;

G_END_DECLS
