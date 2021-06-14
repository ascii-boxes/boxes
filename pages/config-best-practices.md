---
title: Docs - Best Practices
permalink: /config-best-practices.html
created_at: 2021-05-27 20:55:00 +0200
last_modified_at: 2021-05-27 20:55:00 +0200
---

# Config Best Practices

Finally, here is a small list of best practices to consider when creating your own box design. Especially when you
consider having your box design added to the [official configuration
file](https://github.com/{{ site.github }}/blob/master/boxes-config){:target="_blank"}, it makes sense to quickly run
through this list.

- **Align Shape Lines** – Multi-line shapes should be arranged in the configuration file so that their lines align.
  This makes it much easier for a human to recognize the shapes. For example,

      n ("/'''\",
         "|   |",
         "\___/")

  is easier to read than just `n("/'''\","|   |","\___/")`, although *boxes* considers both the same.

- **Apply Proper Tags** – In our official config file, all box designs should be properly tagged using the `tags`
  statement. This normally means that you choose some of the existing tags and set them on your box design. The list
  can be seen at the end of `boxes -l` output. Currently, the most important tags are:

  - `artwork` - It's an ASCII art box, not a simple thing like a box of asterisks or hash characters. When in doubt,
    don't use this tag.
  - `box` - It's a box that goes around the input text, i.e. at most one open side.
  - `comment` - A regional comment in a programming language.
  - `large` - A box intended for large content, like figlet fonts, or just lots of text. It wants to be big!
  - `programming` - All comments in a programming language, including boxes which are valid comments. Anything tagged
    `comment` would also be tagged `programming`.
  - `sign` - An ASCII art figure holding or showing a sign which contains the input text.
  - `simple` - A simple box, normally excluding `artwork`.
  - `speech` - A box which includes a speech bubble.

- **Give Credit Where Credit is Due** – This point is really important. If your box is based on someone else's artwork,
  the original artist must be credited in a `designer` entry, and you must make sure that the person allowed you to
  use his artwork. Very often, the ASCII artists already say so on their website, which is great, but as the author
  of a design, it is your responsibility to make sure that no intellectual property is violated. Some artists also
  want their initials placed on the box, in which cases your box design must also include them. This is not a hard
  thing to do at all, just something to think of. 

- **Indent Sample Block by 4 Spaces** – It's nice when the `sample` block gives a real-world example of your box.
  Currently it is also nice if the sample is indented by exactly 4&nbsp;spaces. Then it appears nicely in line in the
  box listing produced by `boxes -l`, as we are currently still too lazy to align the samples automatically.

- **Maximize Text Area** – Try to design every box so that there is as little whitespace on its inside as possible.
  Many boxes don't really look very good unless there is some space between the text area and the box, but this can be
  addressed by a default padding value. The advantage is that when a really small box size is specified by the user,
  padding will be sacrificed by *boxes* in order to meet the size. This wouldn't be possible if the spaces were
  hard-coded into the box shapes.

- **Prefer Small Shapes** – Try to find the smallest possible shapes which draw your box. For example, a row of dashes
  (`"------"`) should be drawn by repeating a shape that consists of a single dash (`"-"`) not multiple ones (`"--"`).
  The advantage is that this way, *boxes* has the most freedom when sizing your box, and can produce boxes which
  fit better around their input text. There is no need to make shapes on one side of a box the same size as shapes on
  an opposing side. *Boxes* will handle that.

- **Support Nested Comments** – If your box is supposed to be used in code, then consider adding `replace` and
  `reverse` statements to it. When the box is applied to a piece of code which already contains comments, those
  can be escaped. When the box is removed, the comments are restored
  ([example]({{ site.baseurl }}/examples.html#comments)).

- **Use DELIM Judiciously** – If possible, avoid using the `delimiter` statement. It's an advanced feature which helps
  when a box design contains a lot of backslashes or quotation marks, but it otherwise makes a box design harder to
  read for humans - we are simply not used to weird characters that function as escape characters. But if you must use
  it, then by all means do - the `delimiter` statement has no technical disadvantages. It's all about readability.


That's it for now! We may add to this list when new topics come up. Enjoy creating your very own box designs!
