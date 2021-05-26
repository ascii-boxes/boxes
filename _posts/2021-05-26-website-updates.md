---
layout: post
title: "Website Updates"
date: 2021-05-26 21:00:00 +0200
---

*Boxes* got a new website design, including versioned documentation and streamlined navigation<!--break-->.

The changes to the website include:

- Updated frameworks for better browser support
- Improved responsive design
- Versioned documentation: The [config file syntax]({{ site.baseurl }}/config-syntax.html) and
  [manual page]({{ site.baseurl }}/boxes-man-1.html) now feature a version dropdown, where the *boxes* version
  can be selected. This will give you the documentation for the exact version of *boxes* you are using.
- Better social media support via the Jekyll SEO plugin
- New looks, but you be the judge of whether it's better than before
- Start page has some more relevant info
- Documentation was sorted to be more easily accessible
- Internally upgraded from plain CSS to SASS, which helps because we can access values from frameworks and themes
  using their SASS variables. Also better structure of CSS.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="website-on-local-machine"
   text="The website can be run on your local machine" %}

Running the website locally is important in order to be able to test changes before they go online. The {{ site.name }}
website is based on [Jekyll](https://jekyllrb.com/){:target="_blank"}, GitHub's lightweight but powerful CMS, and
several other [open source components]({{ site.baseurl }}/website-attribution.html).

In order to run the website locally, ...

1. Check the prerequisites:
   - [Git](https://git-scm.com/){:target="_blank"}
   - Ruby 2.4.4 or higher ([Windows Installer](https://rubyinstaller.org/downloads/){:target="_blank"},
     [Unix/Linux](https://www.ruby-lang.org/en/documentation/installation/){:target="_blank"}), needed because Jekyll
     is a Ruby Gem. Choose the 64bit or the 32bit version depending on your operating system. Always choose a
     variant that *includes DevKit* (normally something like *Ruby+Devkit 2.4.X (x64)*).
   - Make sure Ruby is on the `PATH`.
1. `gem install bundler`
1. Clone the {{ site.name }} repo: `git clone https://github.com/{{ site.github }}.git`
1. `cd {{ site.name }}`
1. `git checkout gh-pages`
1. `bundle install`  
   This command should take case of installing everything else as per the Gemfile.
1. `bundle exec jekyll serve --watch --incremental --trace`

Now you can browse the website at *http://localhost:4000*
