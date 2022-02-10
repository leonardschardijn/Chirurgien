# Format Definition XML

Since version 2.0, Chirurgien uses XML files to define recognized formats.

This document describes the structure of these XML files.

## XML attribute types

Most of the configuration is done with mandatory and optional attributes added to specific elements. When describing the attributes that a particular element accepts the following keywords are used:

* **optional**: The attribute is optional, it can be omitted.
* **text**: The attribute's value is a text string.
* **text-pango**: The attribute's value is Pango markup.
* **text-decimal**: The attribute's value is a text string or a decimal number.
* **decimal**: The attribute's value is a decimal number.
* **hex**: The attribute's value is a hexadecimal number.
* **hex-bytes**: The attribute's value is a collection of hexadecimal bytes, so the number of hexadecimal characters must be a multiple of 2.
* **boolean**: The attribute's value is a boolean value. Accepted values for true are "1", "t" and "true". Accepted values for false are "0", "f" and "false".
* **var**: The attribute's value is a variable ID. If no variable is defined under the specified ID, it will be created and initialized to 0.
* **var-decimal**: The attribute's value is a variable ID or a decimal number. If no variable is defined under the specified ID, the value will be interpreted as a decimal number.
* **var-text**: The attribute's value is a variable ID or a text string. If no variable is defined under the specified ID, the value will be interpreted as a text string.
* **color**: The attribute's value is a color ID, as defined in the '**colors**' element.
* **field**: The attribute's value is a field ID, as defined in the '**field-defs**' element.
* **block**: The attribute's value is a block ID, as defined in the '**block-defs**' element.
* **max-n**: The attribute's value is limited to **n** characters.

These keywords are added after the attributes they apply to. As will be seen in the remainder of the document.

Variables are values that are stored and that are accessible through their 'ID'. Values stored in variables are limited to 8 bytes, values longer than that cannot be stored. Variables have an internal "failed" flag that is set when the variable is used as a limit but fails (this is explained more in detail later).

## XML file structure

The root element must be '**format**' and it must/can contain the following elements:

* Mandatory elements: '**endianness**', '**magic**', '**colors**', '**run**' and '**field-defs**'.
* Optional elements: '**block-defs**' and '**details**'.

A template is provided with the following basic structure:

```
<?xml version="1.0" encoding="UTF-8"?>
<format name="?" short-name="?">
  <endianness>
    <!-- <little-endian hex-value="?"/> -->
    <!-- <big-endian hex-value="?"/> -->
  </endianness>
  <magic>
    <signature>
      <!-- <match hex-value="?"/> -->
    </signature>
  </magic>
  <colors>
    <!-- <color id="?" name="?" index="0" background="true"/> -->
  </colors>
  <run>
    <!-- <field id="?"/> -->
  </run>
  <!-- Optional run-blocks -->
  <!-- <block-defs> -->
  <!--   <block-def id="?"> -->
  <!--     <field id="?"/> -->
  <!--   </block-def> -->
  <!-- </block-defs> -->
  <field-defs>
    <!-- <field-def id="?" name="?" color="?" size="?"/> -->
  </field-defs>
  <details><![CDATA[
New format
  ]]></details>
</format>
```

## Root &lt;format&gt; element

The '**format**' root element requires the following attributes:

* **name** (text, max-50): The format's name, as it will appear in the 'Formats' window format information panel and the (analyzed) file description panel.
* **short-name** (text, max-20): The format's short name, as it will appear in the 'Formats' window format list.

## Required &lt;endianness&gt; element

The '**endianness**' element configures the format's byte order.

Only two elements can be included in the '**endianness**' element: '**little-endian**' and '**big-endian**'.

For formats that can be both little- and big-endian (that have variable endianness), proper configuration can be specified.

Attributes of the '**endianness**' element:

* **var-id** (optional, var): For formats with variable endianness, the variable ID that holds the value indicating endianness.

Attributes of the '**little-endian**' and '**big-endian**' elements:

* **hex-value** (optional, hex-bytes): For formats with variable endianness, the value that indicates that the format is little- or big-endian.

### Examples

The format is always little-endian:

```
<endianness>
  <little-endian/>
</endianness>
```

The format has variable endianness:

```
<endianness var-id="my-var">
  <little-endian hex-value="11FF"/>
  <big-endian hex-value="FF11"/>
</endianness>
```

In this example, the variable with ID 'my-var' holds a value that will be compared to the hexadecimal values '11FF' and 'FF11' at runtime to determine if the format is little- or big-endian.

## Required &lt;magic&gt; element

The '**magic**' element lists all the possible signatures the format has, that is, all the values that would indicate that a file in question is indeed a file in this format.

The only element that can be included in the '**magic**' element is the '**signature**' element. It can be included multiple times.

The '**signature**' element accepts "magic steps" that configure a valid signature for the file format. If multiple steps are included in a signature, all steps must succeed to match the signature.

**NOTE: If a file format defines invalid signatures the file format will not be usable, as it will never match any file.**

Magic steps are configured with the following elements and attributes:

### Magic steps

#### The **&lt;match&gt;** element/step

The file must match a specified value.

Attributes of the (magic) '**match**' element:

* **char-value** (optional, text): The text data that the file should match.
* **hex-value** (optional, hex-bytes): The hexadecimal data that the file should match.
* **offset** (optional, var-decimal): The file offset where the match should be performed.

Only one of '**char-value**' or '**hex-value**' should be defined. If both are defined, '**char-value**' is used.

If no value ('**char-value**' or '**hex-value**') is defined, the step will always fail. This renders the signature useless as it will never succeed.

If the '**match**' element/step fails, the whole signature fails.

The '**match**' element cannot contain other elements.

#### The **&lt;read&gt;** element/step

Read a value from the file and store it in a variable.

Attributes of the '**read**' element:

* **var-id** (var): The variable ID of the variable used to store the value.
* **size** (decimal): The number of bytes to read.
* **offset** (optional, var-decimal): The file offset where the read should be performed.

Variable values are limited to 8 bytes. If '**size**' exceeds this limit, no read is performed.

The '**read**' element/step always succeeds, it will never trigger a signature failure.

The '**read**' element cannot contain other elements.

### Examples

The file format's signature is a file that starts with the characters "A", "B" and "C" and is followed by the hexadecimal values "AB" and "CD".

That is, the file's signature is (in hexadecimal bytes): 41, 42, 43, AB, CD.

```
<magic>
  <signature>
    <match char-value="ABC"/>
    <match hex-value="ABCD" offset="3"/>
  </signature>
</magic>

Alternatively:

<magic>
  <signature>
    <match hex-value="414243ABCD"/>
  </signature>
</magic>

```

The file format's signature is a file that starts with the characters "A", "B" and "C" and is followed by a two byte offset  where the hexadecimal values "01" and "23" should be.

Assuming a little-endian format, the file's signature could be, for instance (in hexadecimal bytes): 41, 42, 43, 05, 00, 01, 23.

```
<magic>
  <signature>
    <match char-value="ABC"/>
    <read var-id="my-var" size="2" offset="3"/>
    <match hex-value="0123" offset="my-var"/>
  </signature>
</magic>
```

## Required &lt;colors&gt; element

The '**colors**' element lists all colors that the format will use.

The only element that can be included in the '**colors**' element is the '**color**' element. It can be included multiple times.

Chirurgien has 9 colors available (numbered from 0 to 8) that can be configured in the '**Colors**' tab of the '**Preferences**' window. The following configuration sets how these colors are named (in the file format) and used.

Attributes of the '**color**' element:

* **id** (color): The color's ID, so it can be used when defining format fields.
* **name** (text, max-50): The color's name, as it will appear in the format information panel.
* **index** (decimal): The color's index. The attribute's value is a decimal number between 0 and 8 and it corresponds to one of the available Chirurgien colors.
* **background** (boolean): If the color is a background (true) or foreground (false) color.

Multiple colors can have the same name. Colors with the same name are grouped together in the format information panel.

Any color can be used as a background or foreground color, but, all system formats will use colors 0 to 5 as background colors and colors 6 to 8 as foreground colors. It is recommended to follow this convention.

### Examples

The file format sets a background and a foreground color.

```
<colors>
  <color id="my-color" name="My first color" index="0" background="true"/>
  <color id="my-other-color" name="My other color" index="6" background="false"/>
</colors>
```

## Required &lt;run&gt; element

The '**run**' element configures the format analysis process using "run steps".

Run steps are configured with the following elements and attributes:

### Run steps

#### The **&lt;field&gt;** element/step

Apply/use a field. This step makes use of the format fields defined in the '**field-defs**' element. Fields can be used to color the hexadecimal/text view of a file, to add line to the description panel or simply read file values.

General attributes of the '**field**' element:

* **id** (field): The field ID, as defined in a '**field-def**' element.
* **offset** (optional, var-decimal):  The file offset where to apply/use the field.
* **navigation** (optional, var-text): The navigation button's label that will navigate to this field.
* **navigation-limit** (optional, decimal): The navigation label's length. This attribute only makes sense when '**navigation**' is a variable ID.
* **limit** (optional, var-decimal): The field's limit or size (more information on this below). 
* **limit-failed** (optional, boolean): If the field accepts failed limits.
* **additional-color** (optional, color): An additional color ID. This color is used to color the first byte of the field. This can help quickly identify field that were created with an offset.

Attributes dealing with reading field values:

* **store-var** (optional, var): The variable ID used to store the field's value. The field's size must be at most of 8 bytes.
* **ascii-base** (optional, decimal): The field's value is an ASCII-encoded number in the base specified by this attribute (from 2 to 36). This is the only instance in which a field with a size exceeding 8 bytes can be stored in a variable because, even though the field's size exceeds 8 bytes, the read value is a (binary) number that can be stored in 8 (or fewer) bytes.
* **convert-endianness** (optional, boolean): If the format's endianness should be taken into account when reading the value of the field. Setting this attribute isn't always needed, as certain field definitions already imply that endianness should be taken into account. It is only needed for fields that do not imply endianness consideration.

Attributes dealing with the description panel, these attributes only make sense on fields that print their value:

* **tab** (optional, text): The description panel tab name, if this field should be printed to a tab.
* **section** (optional, text): The description panel section, if this field starts a new description section. This attribute can be used on fields that do not print their value, but there shouldn't be a good reason to use it on such fields.
* **margin-top** (optional, decimal): The field's description line top margin.
* **margin-bottom** (optional, decimal): The field's description line bottom margin.
* **suppress-print** (optional, boolean): If the field should not print its value. There is never a need to set this attribute to false.
* **insert-tab** (optional, boolean): If the description panel tab should be inserted now. This attribute only makes sense if the "**tab**" attribute is also used.

When the '**navigation**' attribute is a variable ID, the variable's value will be used as an offset to a string within the file. The string size cannot exceed 15 character, otherwise it will be truncated and the characters "[...]" will be appended. Any non-printable character in the string will be considered the end-of-string. Finally, the '**navigation-limit**' attribute can be used to explicitly set a maximum size to the string, this attribute is of no use when '**navigation**' is not a variable ID.

The '**limit**' attribute serves a dual purpose: it can be used to set the size of a field (if the field has variable size) or it can be used to limit the data that the field can use.

The '**limit**' attribute's value is considered the maximum amount of data that the field can consume. If the value is more than what the field needs, the consumed data by the field will be subtracted from the limit. If the value is less than what the field needs, the field will not be used and the variable (this only works if '**limit**' was a variable ID) will be set to a 'failed' state. Fields will not be applied if the limit is in a 'failed' state unless indicated with the '**limit-failed**' attribute.

The idea behind this behavior is simplifying the very common situation in many file formats where a value is used to limit the following data. This can be seen in formats like PNG, where the four-bytes 'Chunk length' limits the size of the following 'Chunk data' or in JPEG where the 2-bytes 'Data length' limits the size of the following segment data.

Once a '**field**' is applied the internal file index is incremented. If a '**field**', used without an '**offset**', cannot be applied because there is not enough available data in the file, the analysis process will be set to an 'end-of-file-reached' state and fields without the '**limit-failed**' attribute will no longer be usable (in this case, the limit that is being ignored is the implicit limit on all fields: the available data in the file).

The '**field**' element cannot contain other elements.

#### The **&lt;match&gt;** element/step

Match a value or meet a certain condition. Match steps implement the very simple conditional logic required to correctly process files.

Attributes of the '**match**' element:

* **var-id** (optional, var): The variable ID used for the match.
* **char-value** (optional, text): The text data for the match.
* **hex-value** (optional, hex-bytes): The hexadecimal data used for the match.
* **num-value** (optional, decimal): The decimal numeric value used for the match.
* **op** (optional, text): The operation to perform. This attribute only makes sense when using '**var-id**' and '**num-value**'.
* **convert-endianness** (optional, boolean): If the format's endianness should be taken into account when performing the match.

Valid values for the '**op**' attribute are:

* **def**: Test if the variable is defined and its value is anything other than 0 ('**num-value**' can be omitted).
* **eq**: Test if the variable's value is equal to a numeric value  (tests against 0 if '**num-value**' is omitted).
* **gt**: Test if the variable's value is greater than a numeric value (tests against 0 if '**num-value**' is omitted).
* **bit**: Test if a variable's value bit is set. The bit to test is set with the '**num-value**' attribute. Bit positions start at 0  (tests bit 0 if '**num-value**' is omitted).

Depending on the attributes set, the behavior of the match step  varies. The behavior options, in decreasing order of priority, are:

* Both '**var-id**' and '**num-value**' are set: Perform the operation indicated by '**op**' ('**op**' must be a valid operation, otherwise go to next option).
* Both '**var-id**' and a value ('**char-value**' or '**hex-value**') are set: Perform a binary comparison between the variable's value and the indicated value.
* Only '**var-id**' is set: Perform a binary comparison between the variable's value raw binary data and the file.
* Only a value ('**char-value**' or '**hex-value**') is set: Perform a binary comparison between the indicated value and the file.
* Neither '**var-id**' or '**char-/hex-/num-value**' are set and any other cases: Do nothing and succeed (usually used as the last option in a selection scope).

If the '**var-id**' attribute doesn't correspond to a defined variable, the step will execute as if the attribute were unset.

The '**convert-endianness**' attribute converts '**char-value**' or '**hex-value**' (usually used with '**hex-value**') to the format's endianness. The value indicated in '**char-value**' or '**hex-value**' will be assumed to be the value in big-endian order, so if the '**convert-endianness**' attribute is set and the file format's endianness is little-endian, the value will be converted to little-endian.

If the operation indicated in a match step succeeds, all the run steps contained in the '**match**' element are executed. If the operation fails, all steps are skipped.

#### The **&lt;loop&gt;** element/step

Loop until a condition is met.

Attributes of the '**loop**' element:

* **until-set** (optional, var): The control variable. The variable that has to meet a certain condition.
* **limit** (optional, var): The limit variable.
* **var-value** (optional, var): The variable that the control variable should match.
* **num-value** (optional, decimal): The decimal numeric value that the control variable should match.

Depending on the attributes set, the behavior of the loop step  varies. The behavior options, in decreasing order of priority, are:

* Both '**until-set**' and '**var-value**' are set: Compare both variables for equality.
* Both '**until-set**' and '**num-value**' are set: Compare both for equality.
* Only '**until-set**' is set: Test if the variable's value is anything other than 0.
* '**until-set**' is not set: Test if the file has any available data left.

If the '**limit**' attribute is set, it is always the first thing evaluated: Test if the limit variable is in a "failed" state.

If the loop condition isn't met, all the run steps contained in the '**loop**' element are executed continuously.

#### The **&lt;selection&gt;** element/step

Select only one of many.

The '**selection**' element does not accept any attribute.

This element is used to group many '**match**' elements in a "selection scope" where only one must match and be executed. Once a '**match**' element succeeds, the rest of the matches in the selection scope will not be evaluated. Often times, the last '**match**' element is set to always succeed (no attributes set).

#### The **&lt;print&gt;** element/step

Print a line to the description panel.

Attributes of the '**print**' element:

* **line** (optional, text): The description line to print (left part).
* **var-id** (optional, var): The variable's value to print (right right).
* **tooltip** (optional, text-pango): The description line's informative tooltip.
* **signed** (optional, boolean): If the variable's value is a signed value.
* **margin-top** (optional, decimal): The description line top margin.
* **margin-bottom** (optional, decimal): The description line bottom margin.
* **tab** (optional, text): The description panel tab name, if this line should be printed to a tab.
* **section** (optional, text): The description section, if this line starts a new description section.
* **insert-tab** (optional, boolean): If the description panel tab should be inserted now. This attribute only makes sense if the "**tab**" attribute is also used.
* **omit-undefined** (optional, boolean): Do not print anything if '**var-id**' isn't a defined variable.
* **no-section** (optional, boolean): Print the line outside of a description section.

To print a description line the '**line**' attribute must be set. Setting the value to print ('**var-id**') is not required.

Printing a line that is not included in a description section, by setting the '**no-section**' attribute, is only possible on description tabs.

If the variable holds an integer value that is signed, the '**signed**' attribute must be set. If the variable holds a floating-point value setting the '**signed**' attribute is not needed.

The '**print**' element can also be used to simply insert a tab, by setting only the '**tab**' and '**insert-tab**' attributes.

The '**print**' element cannot contain other elements.

#### The **&lt;exec&gt;** element/step

Execute an operation.

Attributes of the '**exec**' element:

* **var-id** (var): The variable to operate on.
* **set** (optional, var-decimal): The value to set.
* **modulo** (optional, var-decimal): The divisor used to calculate the remainder.
* **add** (optional, var-decimal): The value to add.
* **subtract** (optional, var-decimal): The value to subtract.
* **multiply** (optional, var-decimal): The value to multiply by.
* **divide** (optional, var-decimal): The value to divide by.
* **signed** (optional, boolean): If the values are signed.

If multiple operations are set on a single '**exec**' element, the order of execution is:

* '**set**'
* '**modulo**'
* '**add**'
* '**subtract**'
* '**multiply**'
* '**divide**'

The '**signed**' attribute affects all attributes (operands) in the '**exec**' element.

Both '**var-id**' and '**set**' accept a special reserved value: '**index**'. This allows setting and retrieving the internal file index used by the analysis process. A variable called '**index**' (a variable with ID '**index**') would be shadowed by this special meaning, so such a variable ID should be avoided.

Using the '**divide**' attribute sets '**var-id**' to a floating-point number. If it is then used again as an operand in another '**exec**' step its value will be rounded to an integer.

The '**exec**' element cannot contain other elements.

#### The **&lt;block&gt;** element/step

Execute a reusable block of run steps.

Attributes of the '**block**' element:

* **id** (block): The block ID, as defined in a '**block-def**' element.

If a group of run steps is executed repeatedly in different parts of the analysis process, it is possible to group such run steps in a reusable block using the '**block-defs**' element. The '**block**' element executes the run steps in a block.

The '**block**' element cannot contain other elements.

### Examples

For '**run**' element examples it is best to refer to the complete examples mentioned at the end of the document.

## Required &lt;field-defs&gt; element

The '**field-defs**' element lists all fields that the format will use.

The only element that can be included in the '**field-defs**' element is the '**field-def**' element. It can be included multiple times.

All attributes of the '**field-def**' element (except '**id**') can be set as attributes or as nested elements of the '**field-def**' element. When setting attributes as nested elements, use the element's text to set the attribute's value.

Attributes of the '**field-def**' element:

* **id** (field): The field's ID, so it can be used when defining the format analysis process (the run steps).
* **name** (optional, text): The field's name, as it will appear on the description panel (if printed).
* **tag** (optional, text): The field's tag, as it will appear on the hexadecimal/text view. If this attribute is unset, the '**name**' attribute will be used as the tag.
* **color** (optional, color): The field's color for the hexadecimal/text view.
* **tooltip** (optional, text-pango): The field's descriptive tooltip on the description panel.
* **size** (optional, text-decimal): The field's size, in bytes.
* **value** (optional, hex-bytes, max-2): The byte value that marks the field's size. Only a single byte can be specified. Replaces the '**size**' attribute.
* **print** (optional, text): The field's value print type.
* **encoding** (optional, text): The field's value text encoding, for fields with a text value.
* **mask** (optional, hex): The binary mask applied when reading the field's value. This attribute is meant to be used on fields that do not use all bits in the byte(s) they are part of.
* **shift** (optional, decimal): The right shift applied when reading the field's value. This attribute is meant to be used in combination with '**mask**'. The mask is applied after shifting.

Setting the '**mask**' or the '**shift**' attribute prevents the field from creating a color field in the hexadecimal/text view. So an additional field comprising all data in the byte(s) is needed.

To clarify that this means, suppose there is a byte in the file with two separate 4-bit fields. To read both fields and create a color field in the hexadecimal/text view a total of three fields would need to be defined:

* One to read and print the first 4 bits.
* One to read and print the other 4 bits.
* One including all 8 bits, coloring the view and probably not printing anything.

This is like this because the minimum amount of data that can be colored in the hexadecimal/text view is a single byte and it must be aligned to a byte boundary.

Valid values for the '**print**' attribute are:

* **int**: The field's value in signed integer. Format endianness will be automatically taken into account when reading and printing the field's value.
* **uint**: Same as above but as unsigned integer.
* **text**: The field's value is a text string. Use the '**encoding**' attribute to specify the text encoding used. If the text isn't actually encoded in the expected encoding, the message [INVALID ENCODING] will be shown. Fields with a text value can only be printed to a description tab.
* **option**: The field has a set of valid values that will be specified in the nested '**options**' element.
* **flags**: The field is a flags/bits field. The valid bits and their meaning will be specified in the nested '**flags**' element.
* **[anything else]**: A text string that will be printed as the field's value.

Valid values for the '**size**' attribute are:

* **available**: The field's size is variable and should be the available size. Meant to be used in conjunction with the '**limit**' attribute of the field run steps using this field.
* **[decimal]**: The field's size is fixed. Use a decimal number to set the fixed size.

Alternatively, the '**value**' attribute can be used to indicate that the field's size is the number of bytes until a specific byte value is found.

Valid values for the '**encoding**' attribute are:

* **UTF-16LE**: The field's text value is encoded using UTF-16LE (UTF-16 little-endian).
* **UTF-32LE**: The field's text value is encoded using UTF-32LE (UTF-32 little-endian).
* **UTF-16BE**: The field's text value is encoded using UTF-16BE (UTF-16 big-endian).
* **UTF-32BE**: The field's text value is encoded using UTF-32BE (UTF-32 big-endian).
* **ISO-8859-1**: The field's text value is encoded using ISO-8859-1.

The '**encoding**' attribute defaults to UTF-8. If that is the text enconding used simply to not set the '**encoding**' attribute.

The '**options**' element can be nested to specify the set of valid values the field has.

The only element that can be included in the '**options**' element is the '**option**' element. It can be included multiple times.

Attributes of the '**options**' element:

* **convert-endianness** (optional, boolean): If the format's endianness should be taken into account when performing the match.

Attributes of the '**option**' element:

* **name** (text): The name of the value.
* **char-value** (optional, text): The value as text data.
* **hex-value** (optional, hex-bytes): The value as hexadecimal data.

The '**convert-endianness**' attribute (of the '**options**' element) converts '**char-value**' or '**hex-value**' (of the '**option**' elements) to the format's endianness. The value indicated in '**char-value**' or '**hex-value**' will be assumed to be the value in big-endian order, so if the '**convert-endianness**' attribute is set and the file format's endianness is little-endian, the value will be converted to little-endian.

If the field's value doesn't match any of the possible values, the value INVALID will be shown.

The '**flags**' element can be nested to specify the set of flags the field includes.

The only element that can be included in the '**flags**' element is the '**flag**' element. It can be included multiple times.

Attributes of the '**flags**' element:

* **convert-endianness** (optional, boolean): If the format's endianness should be taken into account when reading the flags.

Attributes of the '**flag**' element:

* **name** (text): The name of the flag.
* **mask** (hex): The mask used to isolate the flag.
* **meaning** (optional, text): The flag's meaning.

The '**convert-endianness**' attribute (of the '**flags**' element) converts the field's value before reading the flags.

All set flags will be included in the printed description line using the set flag's '**name**'. The '**meaning**' attribute is used in the informative tooltip if it is set to be generated automatically.

When using the '**options**' or the '**flags**' elements, the '**tooltip**' attribute can be set as a nested element to indicate that the tooltip should be automatically generated.

Automatically generated tooltips are created with the following structure:

```
[field name]
[option value]: [option name] 
[...]
```

Or:

```
[field name]
[flag name] ( [flag mask] ): [flag meaning] 
[...]
```

To use automatically generated tooltips set the following attribute (included in the '**tooltip**' element):

* **auto** (optional, boolean): The tooltip should be automatically generated to show the possible values or flags the field has.

On automatically generated tooltips, setting a value to the '**tooltip**' attribute will replace the first line (the field's '**name**') with the '**tooltip**' attribute's value.

Finally, there is one field ID that has a special meaning: '**unused-data**'. Defining a field with this ID will cause all unused bytes (bytes that are not part of any field) to be marked with this field. It must define a '**tag**' (or '**name**') and a '**color**'.

### Examples

For '**field-defs**' element examples it is best to refer to the complete examples mentioned at the end of the document.

## Optional &lt;block-defs&gt; element

The '**block-defs**' element can be used to create reusable blocks of run steps.

The only element that can be included in the '**block-defs**' element is the '**block-def**' element. It can be included multiple times.

Attributes of the '**block-def**' element:

* **id** (block): The block's ID, so it can be used when defining the format analysis process (the run steps).

The '**block-def**' element should contain all the run steps that are part of the block.

## Optional &lt;details&gt; element

The '**details**' element is used to include a description of the format or its implementation in the format description panel.

The '**details**' element text is used as the provided description. The description accepts Pango markup.

## Complete examples

All system-formats are defined using XML files with the structure defined in this document. These files can be found in Chirurgien's source tree.
