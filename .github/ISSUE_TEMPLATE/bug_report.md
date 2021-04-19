---
name: Bug report
about: Create a bug report to help us improve
title: ''
labels: ''
assignees: ''
---
A clear and concise description of what the bug is.

**To Reproduce**  
Steps to reproduce the behavior, including, but not limited to:
- Value of the environment variables `BOXES`, `HOME`, and `XDG_CONFIG_HOME`. Of these, `BOXES` is the most
  important. For the other two, you can modify the value for privacy reasons. But at least tell us *whether* these
  variables are set.
- How *boxes* is invoked, including input text, ideally as a command which can be copy&pasted, for example

      echo foo | boxes -s 11 -ac -d stone

  If invoked via an editor plugin, give name of editor and plugin, and its configuration
- If a custom box design is involved, its configuration
- The output text produced by *boxes*
- The error message you are getting, if any

**Expected behavior**  
What you *expected* to happen, for example the text that you expected *boxes* to produce

**Screenshots**  
If possible, add screenshots to help explain your problem.

**Environment (please complete the following information):**  
- *Boxes* version, e.g. `v2.1.0 (ba5ce91c)`
- Your operating system and architecture, e.g. `MacOS X 64bit`, or `Windows 10`

*Note:* This is not a form to fill in, just a hint to tell you which information we need to actually help you.
Feel free to add anything you think might be helpful, and don't include this text.
