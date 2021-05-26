---
title: Editor Integration - Sublime Text
permalink: editors-sublime.html
created_at: 2018-05-24 16:01:00 +0200
last_modified_at: 2018-05-24 16:01:00 +0200
---

<a href="{{ site.baseurl }}/editors.html"><button type="button" class="btn btn-outline-primary">Back to All Editors</button></a>


# Integration with Sublime

<span class="atmention">[@drazde](https://github.com/drazde)</span> found a way to integrate *boxes* with Sublime
Text&nbsp;3 (from [#47](https://github.com/{{ site.github }}/issues/47)):

1. Open Sublime Text 3
1. Install [Package Control](https://packagecontrol.io/installation) and restart Sublime.
1. Choose *Tools &rarr; Command Palette...* from the main menu and select *Package Control: Install Package*, then
   choose [FilterPipes](https://packagecontrol.io/packages/FilterPipes).
1. Choose *Tools &rarr; Command Palette...* from the main menu and select *FilterPipes: My Custom Filters Plugin*
1. In the following file open dialog, open
   <code>%APPDATA%\Sublime Text&nbsp;3\&#8203;Packages\&#8203;MyCustomFilterPipes\&#8203;Default.sublime-commands</code>.
   This is the default on Windows, but if you are on other operating systems, the dialog should already be open
   in the correct folder.
1. Before the closing `]`, add the following:
   ```json-doc
   {   /* Boxes stone */
       "caption": "FilterPipes: Boxes stone",
       "command": "filter_pipes_process",
       "args": {
           "command": ["boxes", "-d", "stone", "-a", "c", "-s", "80"]
       }
   }
   ```
   Be sure to add a comma after the previous entry, so that the list is continued properly.
1. Now you have a new command *Boxes stone* which will create a *stone* styled box of 80 characters in width around
   the selected text. In may be accessed via the command palette like above, or you might want to define a hotkey or
   a macro to do it.
1. Add more FilterPipes commands for other designs, or for removing and repairing your ASCII art boxes.
