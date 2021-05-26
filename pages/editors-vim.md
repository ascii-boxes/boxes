---
title: Editor Integration - Vim
permalink: editors-vim.html
created_at: 1999-04-06
last_modified_at: 2021-05-04 22:00:00 +0200
---

<a href="{{ site.baseurl }}/editors.html"><button type="button" class="btn btn-outline-primary">Back to All Editors</button></a>


# Integration with Vim

To call filters from vim, you need to press `!` in visual mode or `!!` in normal mode. So the easiest way to tie in
*boxes* with vim is by adding the following four lines to your *.vimrc*:

```
" sdlfkj comment
vmap ,mc !boxes -d c-cmt<CR>
nmap ,mc !!boxes -d c-cmt<CR>
vmap ,xc !boxes -d c-cmt -r<CR>
nmap ,xc !!boxes -d c-cmt -r<CR>
```

`<CR>` should be there literally; just paste the lines directly from your browser window. This would comment out the
current line or the lines you have marked when you press `,mc` (for *make comment*). Comments can be removed in the
same way by pressing `,xc`. Should you feel that `,mc` is too long a combination to type, feel free to choose a shorter
one. The above example assumes you are using the standard boxes config file, which features the *c-cmt* design. Of
course, the same technique works for any other designs.

While the above example is nice, it does not offer much convenience when you are editing different languages a lot,
because you need to remember the hotkey for each different box design. Fortunately, vim has a feature called
*autocommands*. They can be used to automatically change the meaning of a key combination depending on what file you
edit (any many other things too, of course). Autocommand syntax is

    au[tocmd] [group] {event} {pat} [nested] {cmd}

We can leave out the group. For `{event}`, we choose `BufEnter`, which is generated every time you enter a new buffer,
e.g. when starting vim or when switching between open files. `{pat}` is a file glob, and `{cmd}` is our call to *boxes*.

The lines below are from the author's *.vimrc*. They can be pasted directly from your browser window. Their effect is
that `,mc` and `,xc` always generate the correct comments for many languages, including C, C++, HTML, Java, lex, yacc,
shell scripts, Perl, etc. The default key binding is to generate shell comments using a pound sign (file glob of `*` at
the start).

    autocmd BufEnter * nmap ,mc !!boxes -d pound-cmt<CR>
    autocmd BufEnter * vmap ,mc !boxes -d pound-cmt<CR>
    autocmd BufEnter * nmap ,xc !!boxes -d pound-cmt -r<CR>
    autocmd BufEnter * vmap ,xc !boxes -d pound-cmt -r<CR>
    autocmd BufEnter *.html nmap ,mc !!boxes -d html-cmt<CR>
    autocmd BufEnter *.html vmap ,mc !boxes -d html-cmt<CR>
    autocmd BufEnter *.html nmap ,xc !!boxes -d html-cmt -r<CR>
    autocmd BufEnter *.html vmap ,xc !boxes -d html-cmt -r<CR>
    autocmd BufEnter *.[chly],*.[pc]c nmap ,mc !!boxes -d c-cmt<CR>
    autocmd BufEnter *.[chly],*.[pc]c vmap ,mc !boxes -d c-cmt<CR>
    autocmd BufEnter *.[chly],*.[pc]c nmap ,xc !!boxes -d c-cmt -r<CR>
    autocmd BufEnter *.[chly],*.[pc]c vmap ,xc !boxes -d c-cmt -r<CR>
    autocmd BufEnter *.C,*.cpp,*.java nmap ,mc !!boxes -d java-cmt<CR>
    autocmd BufEnter *.C,*.cpp,*.java vmap ,mc !boxes -d java-cmt<CR>
    autocmd BufEnter *.C,*.cpp,*.java nmap ,xc !!boxes -d java-cmt -r<CR>
    autocmd BufEnter *.C,*.cpp,*.java vmap ,xc !boxes -d java-cmt -r<CR>
    autocmd BufEnter .vimrc*,.exrc nmap ,mc !!boxes -d vim-cmt<CR>
    autocmd BufEnter .vimrc*,.exrc vmap ,mc !boxes -d vim-cmt<CR>
    autocmd BufEnter .vimrc*,.exrc nmap ,xc !!boxes -d vim-cmt -r<CR>
    autocmd BufEnter .vimrc*,.exrc vmap ,xc !boxes -d vim-cmt -r<CR>


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=4 slug="syntax-highlighting"
   text="Syntax Highlighting in Vim" %}

There is a [Vim syntax file](https://github.com/{{ site.github }}/blob/master/boxes.vim) for *boxes*
configuration files, which you can install to have the *boxes* config colorized. On Windows, the file must be placed
in the directory *VIM_INSTALL_DIR\vimfiles\syntax*.  
Activate by `set syn=boxes`.
