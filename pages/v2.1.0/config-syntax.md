---
title: Docs - Config File Syntax
permalink: /v2.1.0/config-syntax.html
longContent: true
bxVersion: v2.1.0
created_at: 1999-04-06
last_modified_at: 2021-05-28 22:11:00 +0200
---

# Config File Syntax

{% include version-select.html currentVersion=page.bxVersion contentPiece="config-syntax" %}


The *boxes* config file is a succession of box design definitions.
Everything following a pound sign (`#`) is considered a **comment** (unless, of course, the pound sign is part of a
string or something).
*Boxes* config files are case insensitive, i.e. upper/lower case does not matter.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3
   text="Box Design" %}

<pre><b>BOX</b> <i>design_name</i>
    entries_and_blocks
<b>END</b> <i>design_name</i></pre>

A box design may optionally have **alias names** in addition to the primary design name (since *boxes* v2.1.0).
Alias names are appended to the primary design name as a comma-separated list. For example:

<pre><b>BOX</b> <i>design_name</i>, <i>alias1</i>, <i>alias2</i>
    entries_and_blocks
<b>END</b> <i>design_name</i></pre>

After the `END` statement, only the primary design name is specified. There is no practical limit to the number of
alias names. Every primary design name and alias name must be unique across the entire config file, including any
parent config files.

Every box design definition must have at least a `SHAPE` block, an `ELASTIC` list, and a `SAMPLE` block.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="sample"
   text="Sample Block" %}

<PRE><b>SAMPLE</b>
    sample_image_of_box
<b>ENDS</b></PRE>
This is the box image used for the list of available designs. The reason why an image is not simply generated is that
this way, the box can be shown in an environment in which it might actually be used, e.g. with some C code around (see
the [boxes config file](https://github.com/{{ site.github }}/blob/master/boxes-config){:target="_blank"} for many
examples).

The sample itself should be indented by *4 spaces*. This is because all other samples in the config file are also
indented by this value, which simply looks better on the design list.

The `ENDS` statement must stand on a line of its own, although it may be indented. Such a line cannot occur as part of
the sample itself.\\
The `SAMPLE` block is a required entry in every box design definition.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="shapes"
   text="Shapes Block" %}

<PRE><b>SHAPES</b> {
    <i>shape_name</i> (string_list)
    <i>shape_name</i> (string_list)
    ...
}</PRE>
<i>`shape_name`</i> may be one of `nw`, `nnw`, `n`, `nne`, `ne`, `ene`, `e`, `ese`, `se`, `sse`, `s`, `ssw`, `sw`,
`wsw`, `w`, or `wnw`, corresponding to the [figure shown earlier]({{ site.baseurl }}/config-general.html).
The `string_list` represents the shape itself, line by line. Note that all lines must have equal length. Add extra
spaces if necessary. Not also that double quotes (`"`) which appear as part of a string must be escaped by a preceding
backslash, so as not to terminate the string early. Consequently, all occurrences of backslashes must also be escaped
by adding additional backslashes. The recommended process for creating a string is thus:

1. Type in the string as you think it should look.
1. Make your editor double all backslashes within the string.
1. Escape all double quotes within the string by adding more backslashes.

In Vim, for example, this can be achieved by marking the block, and then typing:

    :s/\\/\\\\/g
    :s/"/\\"/g

Here are a few examples of valid entries in a `SHAPE` block:

    ne ("+++", "+ +", "+++")
    SSE ("///", "\\\\\\")
    ene ()
    s ("\"")

The [`delimiter` statement](#delim) can be used to redefine the string delimiting character and the escape character,
thus eliminating the above backslash problem.
The `SHAPE` block is a required entry in every box design definition.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="elastic"
   text="Elastic List" %}

<PRE><b>ELASTIC</b> (<i>shape_name</i>, <i>shape_name</i>, ...)</PRE>
Simply lists the shapes which are *elastic*. Elastic shapes are repeated so that the box can grow and shrink to meet
its requested size. Corner shapes may not be elastic. Naturally, there must always be at least one elastic shape per
box side. No two neighboring shapes may be elastic. A typical `ELASTIC` entry would look like this:

    elastic (n, e, s, w)

The elastic list is a required entry in every box design definition.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="padding"
   text="Padding Block" %}

<PRE><b>PADDING</b> {
    side_or_group_of_sides <i>value</i>
    side_or_group_of_sides <i>value</i>
    ...
}</PRE>
Defines the default [padding area]({{ site.baseurl }}/config-general.html#padding-area). Possible values for
`side_or_group_of_sides` are `all`, `horizontal`, `vertical`, `top`, `left`, `right`, or `bottom`. A padding block may
contain an arbitrary number of entries. Entries are read from top to bottom, so that later entries overwrite earlier
entries. In the following example:

    padding {
        vertical 3
        horiz 2
        left 4
    }

the padding is set to 3 blank lines above the text, 2 spaces to the right of the text, 3 blank lines below the text,
and 4 spaces to the left of the text. These values can be overridden by a different specification on the command line
using the **-p** option.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="regexp"
   text="Replace and Reverse Statements" %}

<PRE><b>REPLACE</b> "<i>search_pattern</i>" <b>WITH</b> "<i>replacement_string</i>"
...
<b>REVERSE</b> "<i>search_pattern</i>" <b>TO</b> "<i>replacement_string</i>"
...</PRE>
These statements are used to perform substitutions on the text surrounded by a box. The `REPLACE` statements are
executed on the input text when a box is being created. `REVERSE` statements have the same effect as `REPLACE`
statements, but they are executed after a box has been removed.
In the simplest case, one string is replaced with another:

    replace "\\*/" with "*-/"
    reverse "\\*-/" to "*/"

The `REPLACE` statement in the above example may be used to quote closing comment tags in the C programming language by
inserting a dash between the asterisk and the slash. The `REVERSE` statement undoes this effect when the box is removed.

The search pattern may be a regular expression, and the replacement string may include backreferences. This gives you
quite a powerful means for text modification. The following example is used to insert a space between all characters of
the input text:

    replace "(.)" with "$1 "
    reverse "(.) " to "$1"

There may be many `REPLACE`/`REVERSE` statements in one design definition. They will be executed one after the other,
starting from the top.

Note that as in all strings, backslashes must be doubled, i.e. *boxes* "consumes one layer of backslashes". This is
why in the first example, there are two backslashes at the beginning of the search pattern, when only one would have
been needed to escape the star operator.

The [`delimiter` statement](#delim) should not be used to change the string delimiting and escape character in
regular expressions. While this is possible, it only affects *boxes*' processing, not the regular expression engine's.
This will most likely cause confusion.

You may choose if the replacement is performed `once` per line or as many times as necessary to replace all occurrences
of the sarch pattern (`global`). Put this indication in front of the search pattern (the default is `global`):

    replace once "foo" with "bar"

We use [PCRE2](https://www.pcre.org/current/doc/html/) regular expression syntax. In order to craft the proper
regular expressions, online tools such as [Regex101](https://regex101.com/) or [Regexper](https://regexper.com/) might
come in handy.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="indent"
   text="Indentation Mode" %}

<PRE><b>INDENT</b> "<i>indentmode</i>"</PRE>
This sets the default indentation mode for a design. Possible values for <i>`indentmode`</i> are `"box"`, `"text"`,
and `"none"`. The indent mode specifies how existing text indentation is treated.

  - `"box"`, which is the default setting, will cause the box to be indented by the same number of spaces as the text
    was. The text itself will not be indented within the box, though.
  - `"text"` will not indent the box, but instead retain the text indentation inside the box.
  - `"none"` will simply throw away all indentation.

For examples on all three indentation modes please refer to the [example page]({{ site.baseurl}}/examples.html#indent).


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="delim"
   text="String Delimiters" %}

<PRE><b>DELIM[ITER]</b> <i>chars</i></PRE>

*`chars`* must consist of exactly two characters:

1. the escape character (first character, one of `@~?!\`)
2. the string delimiter (second character, one of `"~'!|`)

No quotes must be placed around those two characters. The effect is that until the end of the current design or until
the next `DELIMITER` statement, all strings must be enclosed not by the usual double quotes (`"`), but instead by the
character you specify. Also, the delimiter may be escaped not by the usual backslash (`\`), but instead by the escape
character you specify. The escape character and the string delimiter must be different characters, of course.  
For example, consider the literal string `"""""`, which would normally be written like this:

    "\"\"\"\"\""

Using a `DELIMITER` statement, this looks much simpler:

    delimiter \'
    '"""""'

This mechanism is intended for box designs which have a lot of backslash or double quote characters in them.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="credit"
   text="Names of Author and Designer" %}

<PRE><b>AUTHOR</b> &quot;<i>name_of_author</i>&quot;</PRE>
Simply states who the person was who coded the box design definition. This entry is used when displaying the list of
designs (`boxes -l`). The value is free text, but it is good if some contact information is included, for example
`John Doe <john@acme.com>`. These days, such openness with one's email address may not suit everyone, so feel free to
give less or differently formatted author information.

<PRE><b>DESIGNER</b> &quot;<i>name_of_designer</i>&quot;</PRE>
In contrast to `author`, which tells us who created the config file entry, the `designer` keyword tells us who created
the box design itself. In other words, this is the artist who originally created the ASCII artwork.

Please try to give both fields always. Especially in those cases where existing ASCII art from the internet is adapted
for use with *boxes*, it is important (and good manners) to give credit to the original artist.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3
   text="Tags" %}

Since *boxes* v2.1.0:

<PRE><b>TAGS</b> (&quot;<i>tag1</i>&quot;, &quot;<i>tag2</i>&quot;, &quot;<i>tag3</i>&quot;)</PRE>

or, in all versions of *boxes*:

<PRE><b>TAGS</b> &quot;<i>tag1</i>, <i>tag2</i>, <i>tag3</i>&quot;</PRE>

The legacy notation has the advantage of being backwards compatible with all versions of *boxes*, which is why we use
it exclusively in the official config file. But once we're reasonably sure that all the world is at least on v2.1.0,
we can upgrade that.

The `tags` statement applies one or more tags to the box design which can later be used to find and select the design
in a tag query. A tag can only contain the lower-case letters `a-z`, digits, or a dash (`-`). It must not start with
a dash.

At the end of the output of `boxes -l`, *boxes* will print a summary of all tags found in the configuration,
complete with counts of how often the tag was encountered.

A *tag query* can be issued by invoking `boxes -q`. *Boxes* will then print the names of all matching box designs.
Details about tag queries can be found in the
[manual page]({{ site.baseurl }}/{{ page.bxVersion }}/boxes-man-1.html#OPTIONS) for the `-q` option.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3
   text="General Entries" %}

<PRE><b>keyword</b> &quot;<i>string_value</i>&quot;</PRE>

In addition to the author entry, there may be any number of other entries of the above form, giving any kind of
information. The [boxes config file](https://github.com/{{ site.github }}/blob/master/boxes-config){:target="_blank"}
includes the entries `CREATED`, `REVISION`, and `REVDATE`, to indicate the creation timestamp, revision number, and
timestamp of the latest revision, respectively. These other entries are not yet used by *boxes*, though, and will
simply be ignored.

-----


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2 slug="parent"
   text="Config File Inheritance" %}

A config file may specify one or more other config files as a "parents" (since *boxes* v2.1.0). This means that after
the config file is read, *boxes* will also read all the parent configs (in order). If a design is found which we don't
know yet, it is added to the list. In other words, all box designs are "inherited" from the parents, and can be
overwritten.

This mechanism works on the box design level. You can only override an entire box design, but not parts of one.

The syntax is

<PRE><b>PARENT</b> <i>path_to_parent_config</i></PRE>

The path to the parent config should be an absolute pathname specifying a config file (not a directory). Spaces are
permitted in this path, and no quotes are used. A parent reference can only occur outside of any box design definitions.
The special keyword `:global:` is used to refer to the *boxes* global config file.

For example, a user wants to add or change a design from the global config. So she creates a config file in ~/.boxes
and places a `parent :global:` statement at the top to inherit the global config. The local / personal config file
contains only the changes / additions.


Read on in the next part: [A New Box Design Step By Step]({{ site.baseurl }}/config-step.html)
