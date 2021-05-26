---
title: Editor Integration - Notepad++
permalink: editors-notepadplusplus.html
created_at: 2018-09-15 21:57:00 +0200
last_modified_at: 2021-05-04 22:00:00 +0200
---

<a href="{{ site.baseurl }}/editors.html"><button type="button" class="btn btn-outline-primary">Back to All Editors</button></a>


# Integration with Notepad++

In order to integrate *boxes* with Notepad++, first make sure that *boxes* is on your `PATH`. On Windows, this usually
means boxes.exe and boxes.cfg must be located in a directory which is on the `PATH` environment variable. This method
of *boxes* integration works only on Windows, because afaik, Notepad++ is available only on Windows.

We use the NppExec plugin. Install NppExec via PluginManager:

1. Display PluginManager via the main menu: *Plugin &rarr; Plugin Manager &rarr; Show Plugin Manager*
1. If NppExec is not on the *Installed* tab already, go to the *Available* tab and select NppExec from the list.
1. Press **Install**.
1. Restart Notepad++

   <img src="{{ site.baseurl }}/images/editor-npp-pluginmanager.png" class="img-fluid" width="670" height="493"
        alt="Plugin Manager" />

Once the NppExec plugin is available, proceed with *boxes* integration:

1. Go to *Plugins &rarr; NppExec &rarr; Execute ...*
1. Paste the following script into the text area:

   <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>NPP_CONSOLE 0  
cls
cmd.exe /c exit %RANDOM%
set tempfile = $(SYS.TEMP)\NppBoxes_$(EXITCODE).out.txt
set ascfile = $(SYS.TEMP)\NppBoxes_$(EXITCODE).in.txt
sel_saveto $(ascfile) :a
cmd.exe /c <span class="mark-hilite">boxes -d nuke -s 80 -pt1 -ac</span> "$(ascfile)" > "$(tempfile)"
sel_loadfrom $(tempfile)
cmd.exe /c del /f /q "$(tempfile)" "$(ascfile)"</code></pre></div></div>

   This script is inspired by a [post by Peter Jones](https://notepad-plus-plus.org/community/topic/16158/2) on the
   Notepad++ forum. It works via temp files in ANSI format. (That's the `:a` argument to the `sel_saveto` command.)
   This is compatible with boxes, and also helps avoiding byte order marks at the beginning of the temp file.  
   The <span class="mark-hilite">green part</span> marks the place where you configure the box. In this case, we'll
   get a *nuke* box 80 characters wide, with the text centered in it and an extra blank line at the top.
1. Press **Save...** and give it a meaningful name, for example `boxes nuke 80`.
1. Press **Save** on the *Script name* input box.

   <img src="{{ site.baseurl }}/images/editor-npp-script.png" class="img-fluid" width="472" height="277"
        alt="NppExec Script" />

1. Go to *Plugins &rarr; NppExec &rarr; Advanced Options ...*
1. In the *Associated Script* dropdown, select the `boxes nuke 80` script entry created previously, then press
   **Add/Modify** to add it to the list of *Menu items*.
1. Optionally select *Place to the Macros submenu*, if you want an entry for this in the *Macros* menu.
1. Press **OK**, and if asked, restart Notepad++.

   <img src="{{ site.baseurl }}/images/editor-npp-options.png" class="img-fluid" width="530" height="511"
        alt="NppExec Advanced Options" />

1. Go to *Macro &rarr; Modify Shortcut/Delete Macro...* to call up the Shortcut mapper.
1. Select the *Plugin commands* tab, and type `boxes` into the filter line at the bottom of the dialog window.
1. Choose our `boxes nuke 80` entry and press **Modify**:

   <img src="{{ site.baseurl }}/images/editor-npp-shortcut.png" class="img-fluid" width="677" height="290"
        alt="Shortcut mapper" />

1. Assign a shortcut key, for example Ctrl+Shift+B.
1. Make sure that the Shortcut mapper reports `No shortcut conflicts for this item` in the status area.

That's it, finally! Now you can select any piece of text in the Notepad++ editor and draw the `nuke 80` box around it
by pressing your assigned shortcut:

                                      _ ._  _ , _ ._
                                    (_ ' ( `  )_  .__)
                                  ( (  (    )   `)  ) _)
                                 (__ (_   (_ . _) _) ,__)
                                     `~~`\ ' . /`~~`
                                     ,::: ;   ; :::,
                                    ':::::::::::::::'
     ___________________________jgs______/_ __ \___________________________________
    |                                                                              |
    |                                LOOK AT THAT!                                 |
    |______________________________________________________________________________|
