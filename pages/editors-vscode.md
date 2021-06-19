---
title: Editor Integration - Visual Studio Code
permalink: editors-vscode.html
created_at: 2021-06-18 22:00:00 +0200
last_modified_at: 2021-06-18 22:00:00 +0200
---

<a href="{{ site.baseurl }}/editors.html"><button type="button" class="btn btn-outline-primary">Back to All Editors</button></a>


# Integration with Visual Studio Code

In order to invoke *boxes* from Visual Studio Code, we need a Visual Studio Code plugin that can invoke filter programs.
Several of those exist. For this example, we use
[Filter Text](https://marketplace.visualstudio.com/items?itemName=yhirose.FilterText) by *yhirose*. It hasn't been
updated for a while, but worked fine for us.

1. Install the [Filter Text](https://marketplace.visualstudio.com/items?itemName=yhirose.FilterText) extension into
   your Visual Studio Code.
2. In your settings.json, add the following configuration:
   ```json
   "filterText.commandList": [
       {
           "name": "Boxes Peek (create)",
           "description": "Add a 'peek' box around selected text",
           "command": "boxes -d peek -p h1 -s 118 -i none -n UTF-8"
       },
       {
           "name": "Boxes Peek (mend)",
           "description": "Repair a 'peek' box",
           "command": "boxes -d peek -p h1 -s 118 -i none -n UTF-8 -m"
       }
   ]
   ```
   This snippet defines two invocations of *boxes*, one for adding a new box, and one for repairing it.
   The [options]({{ site.baseurl }}/boxes-man-1.html#OPTIONS) passed to *boxes* do this:

   <table class="table table-striped">
   <thead>
       <tr><th scope="col">Option</th><th scope="col">Meaning</th></tr>
   </thead>
   <tbody>
       <tr><td><code>-d peek</code></td>
           <td>Choose the <code>peek</code> box design.</td></tr>
       <tr><td><code>-p h1</code></td>
           <td>Add a space to the left and right of the text. Looks better.</td></tr>
       <tr><td><code>-s 118</code></td>
           <td>Always make the box 118 characters wide. The boxes look better when all are the same width.</td></tr>
       <tr><td><code>-i none</code></td>
           <td>Do not indent box or text, always start at the left side.</td></tr>
       <tr><td><code>-n UTF-8</code></td>
           <td>Tell <i>boxes</i> that the input is UTF-8 encoded text. Adjust if you use a different encoding.</td></tr>
       <tr><td><code>-m</code></td>
           <td>Mend box. Tells <i>boxes</i> that we want a repair instead of a new box.</td></tr>
   </tbody>
   </table>

   You can add more commands to the JSON to add different boxes, or add a command for removing a box. By using
   workspace-specific settings.json files, you can even share those commands in your team, and configure different
   boxes for different projects.

3. Define a [key binding](https://code.visualstudio.com/docs/getstarted/keybindings). Press `Ctrl+Shift+P` or
   ⇧⌘P (on Mac) to bring up the command palette, then choose *Preferences: Open Keyboard Shortcuts (JSON)*. Add the
   following JSON:

   ```json
   [
       {
           "key": "ctrl+shift+b",
           "command": "-workbench.action.tasks.build"
       },
       {
           "key": "ctrl+shift+b",
           "command": "extension.filterPredefined"
       }
   ]
   ```

   In my case, `Ctrl+Shift+B` was already defined for a build action, so the first entry removes it. Choose any key
   that suits you.

Done!


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=4
   text="Example" %}

In order to use this, just select some text:

<img src="{{ site.baseurl}}/images/editor-vscode-filter1.png" class="img-fluid" width="384" height="133"
     alt="text before filtering" />

Now you can invoke *boxes* by pressing the hotkey (`Ctrl+Shift+B` in the example). This will show the predefined
commands:

<img src="{{ site.baseurl}}/images/editor-vscode-filter2.png" class="img-fluid" width="403" height="124"
     alt="text before filtering" />

Select *Boxes Peek (create)*, which gets you the box:

<img src="{{ site.baseurl}}/images/editor-vscode-filter3.png" class="img-fluid" width="384" height="193"
     alt="text before filtering" />

If you damage the box later by editing in it:

<img src="{{ site.baseurl}}/images/editor-vscode-filter4.png" class="img-fluid" width="384" height="193"
     alt="text before filtering" />

You can repair it again by selecting the entire broken box:

<img src="{{ site.baseurl}}/images/editor-vscode-filter5.png" class="img-fluid" width="384" height="193"
     alt="text before filtering" />

And using the *Boxes Peek (mend)* command:

<img src="{{ site.baseurl}}/images/editor-vscode-filter3.png" class="img-fluid" width="384" height="193"
     alt="text before filtering" />
