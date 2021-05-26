---
title: Contributing
permalink: /contributing.html
created_at: 2014-12-23 22:50:09 +0100
last_modified_at: 2021-05-04 21:36:00 +0200
---

# Contributing

You are considering a contribution to *boxes*? That's fantastic!

All *boxes* development happens in our [GitHub project](https://github.com/{{ site.github }}). Here are a few links
with more details:

  - [How to build from source]({{ site.baseurl }}/build.html)
  - [How to write test cases]({{ site.baseurl }}/testing.html)
  - [Bug Reports and Feature Requests](https://github.com/{{ site.github }}/issues)

Boxes has been around a long time. Great care was taken to make the *boxes* source code as compatible as possible
with all kinds of different platforms.

The planned functionality has long been implemented, and it is very stable. Development has thus slowed down and
concentrates on maintenance and contributor-driven changes. (The year 2021 is a bit of an exception, as the original
author has taken up the project again to implement all open feature requests.)

This is why we attach great importance to contributions.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Contribution Process" %}

Changes must be introduced by creating pull requests on GitHub. Note that you can improve the *boxes* code as well as
the website content.

Ideas for new box designs should be submitted via GitHub, but since the most beautiful designs sometimes come from
non-technical people, emails are acceptable for those.

It is best to discuss the planned change before actually doing work on it by [creating an issue for
it](https://github.com/{{ site.github }}/issues){:target="_blank"}. This is really not a big deal, and it helps to
ensure that your work will end up on the *boxes* master branch.

In order to create a pull request, you must first
[fork the *boxes* repo](https://docs.github.com/en/github/getting-started-with-github/fork-a-repo){:target="_blank"}.
Then create a new feature branch for your change. Commit the changes to your feature branch. After that, you can
[create a pull
request](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/about-pull-requests){:target="_blank"}
against the *boxes* master.

The pull request must meet the criteria mentioned below. If additional changes are identified during the review, you
can simply push them to your forked repo, and the pull request will update automatically.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Pull Request Acceptance Criteria" %}

Since *boxes* is not actively being developed anymore, changes to the code base must be "production ready" from the start. The following check list may help:

  - The [GitHub workflow](https://github.com/{{ site.github }}/actions){:target="_blank"} must pass for the pull
    request. This is indicated by a small green check mark next to your pull request.
  - Provide [test cases]({{ site.baseurl}}/testing.html) for any code that you add. This helps make sure that nobody
    else breaks it later.
  - Cross-platform compatibility is very important for *boxes*. Since *boxes* does not (and cannot really) make use of
    *autoconf*, the C code itself must work on every platform. In other words, you can use only such C commands that
    are known on every platform. When in doubt, restrict yourself to commands that you already find in the current
    code base.

These criteria apply only to code changes. (Although other changes should not break the build, either.)


<h2 id="contributors" class="text-muted">Contributors</h2>

<p class="text-muted">Information on the people behind <i>boxes</i> has moved to
    <a href="{{ site.baseurl }}/team.html">its own page</a>.</p>
