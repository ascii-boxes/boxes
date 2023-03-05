---
layout: default
redirect_from:
  - /docs/index.html
  - /docs/index.shtml
  - /docs/legal.html
  - /docs/legal.shtml
  - /boxes.shtml
  - /links.html
---

<p class="text-center mb-0"><img src="{{ site.baseurl}}/images/boxes.png" class="img-fluid"
   width="240" height="87" alt="boxes"></p>

<p class="text-center mb-4"><i>Boxes</i> is a command line program which draws, removes, and repairs
ASCII art boxes.<br><a href="#more">Learn more</a>,
<a href="{{ site.baseurl }}/download.html">Download/Install</a></p>


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}

<div id="boxesCarouselIndicators" class="carousel slide mx-xl-6" data-bs-ride="carousel">
  <div class="carousel-indicators">
    <button type="button" data-bs-target="#boxesCarouselIndicators" data-bs-slide-to="0" aria-label="Slide 1"
        class="active" aria-current="true"></button>
    <button type="button" data-bs-target="#boxesCarouselIndicators" data-bs-slide-to="1" aria-label="Slide 2"></button>
    <button type="button" data-bs-target="#boxesCarouselIndicators" data-bs-slide-to="2" aria-label="Slide 3"></button>
    <button type="button" data-bs-target="#boxesCarouselIndicators" data-bs-slide-to="3" aria-label="Slide 4"></button>
    <button type="button" data-bs-target="#boxesCarouselIndicators" data-bs-slide-to="4" aria-label="Slide 5"></button>
  </div>
  <div class="carousel-inner">
    <div class="carousel-item active">
      <img src="{{ site.baseurl }}/images/carousel-1.png" class="d-block w-100" alt="boxes-1">
    </div>
    <div class="carousel-item">
      <img src="{{ site.baseurl }}/images/carousel-2.png" class="d-block w-100" alt="boxes-2">
      <div class="carousel-caption d-none d-md-block">
        <h5>Elasticity</h5>
        <p>The boxes adjust to match the size of your content</p>
      </div>
    </div>
    <div class="carousel-item">
      <img src="{{ site.baseurl }}/images/carousel-3.png" class="d-block w-100" alt="boxes-3">
      <div class="carousel-caption d-none d-md-block">
        <h5>Creativity</h5>
        <p>Create and share your own box designs</p>
      </div>
    </div>
    <div class="carousel-item">
      <img src="{{ site.baseurl }}/images/carousel-4.png" class="d-block w-100" alt="boxes-4">
    </div>
    <div class="carousel-item">
      <img src="{{ site.baseurl }}/images/carousel-5.png" class="d-block w-100" alt="boxes-5">
    </div>
  </div>
  <button class="carousel-control-prev" data-bs-target="#boxesCarouselIndicators" type="button" data-bs-slide="prev">
    <span class="carousel-control-prev-icon" aria-hidden="true"></span>
    <span class="visually-hidden">Previous</span>
  </button>
  <button class="carousel-control-next" data-bs-target="#boxesCarouselIndicators" type="button" data-bs-slide="next">
    <span class="carousel-control-next-icon" aria-hidden="true"></span>
    <span class="visually-hidden">Next</span>
  </button>
</div>


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}

{% if site.posts.size > 0 %}
{% include heading.html
   level=3 slug="whats-new"
   text="What's New" %}

<div class="container-fluid">
<div class="row">

{%- for post in site.posts limit:2 -%}
<div class="col-12 col-lg-4 mb-4 mb-lg-1">
<div class="card border-info h-100">
    <div class="card-header">
        {%- if post.release -%}
            <a href="{{ site.baseurl }}{{ post.url }}">{{ post.release }} released</a>
        {%- else -%}
            {%- if post.content contains '<!--break-->' -%}
                <a href="{{ site.baseurl }}{{ post.url }}">{{ post.title }}</a>
            {%- else -%}
                {{- post.title -}}
            {%- endif -%}
        {%- endif -%}
    </div>
    <div class="card-body">
        <p class="card-subtitle text-muted fw-light mb-2">{{ post.date | date: "%b %-d, %Y" }}</p>
        {{- post.content | split:'<!--break-->' | first | markdownify | replace: '<p>', '<p class="card-text">' | replace: '</p>','' -}}
        {%- if post.content contains '<!--break-->' -%}
            <br/><a href="{{ site.baseurl }}{{ post.url }}">... read more</a></p>
        {%- else -%}
            </p>
        {%- endif -%}
    </div>
    {% if post.release -%}
    <div class="card-footer">
        <a href="{{ site.baseurl }}/download.html" class="card-link">Download</a> -
        <a href="{{ site.baseurl }}/releases.html" class="card-link">Release History</a>
    </div>
    {%- endif %}
</div>
</div>
{%- endfor %}

{% if site.posts.size > 2 %}  
<div class="col-12 col-lg-4 mb-1">
<div class="card border-secondary h-100">
    <div class="card-header"><a href="{{ site.baseurl }}/blog/">&raquo; All Blog posts</a></div>
    <div class="card-body">
        <p class="card-subtitle text-muted fw-light mb-2">{{ site.posts.first.date | date: "%b %-d, %Y" }}</p>
        <p class="card-text">The <a href="{{ site.baseurl }}/blog/">What's New</a> page has the comprehensive list
        of all blog posts.</p>
    </div>
</div>
</div>
{% endif %}
</div></div>
{% endif %}


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3 slug="more"
   text="More About Boxes" %}

*Boxes* is a command line program which draws, removes, and repairs ASCII art boxes. It operates as a
[text filter]({{ site.baseurl }}/filters.html). The generated boxes may even be removed and repaired again if they were
badly damaged by editing of the text inside. Since boxes may be open on any side, *boxes* can also be used to create
(regional) comments in any programming language.

*Boxes* is useful for making the function headers in your programming language look better, for spicing up news
postings and emails, or just for decorating documentation files. Since the drawn box is matched in size to your
input text, you can use *boxes* in scripts to dynamically add boxes around stuff.

*Boxes* integrates into your text editor ([learn how]({{ site.baseurl }}/editors.html)).

*Boxes* can be configured via [command line options]({{ site.baseurl }}/boxes-man-1.html#OPTIONS), so that your text
looks *just right* in its new surrounding box. You can even configure regular expressions to change the text when a
box is added or removed.

New box designs of all sorts can easily be added and shared by appending to a free format configuration file. Be
creative!

*Boxes* is free software under the GNU General Public License, version 3 ([GPLv3]({{ site.link_license }})).
