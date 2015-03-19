{% comment %}
Release History

    The Release History table is in the releases.html file,
    because we needed to be able to change column headings on small screens.
{% endcomment %}

## When to upgrade

Boxes has been around a long time, and it has been very stable. Sometimes, new releases are still required to ensure that *boxes* continues to compile smoothly on a large number of platforms. Whether or not you should upgrade to a new version depends on what you've got, and what you want to do:

- **Building from Source:** If you want to [build from source]({{ site.baseurl}}/build.html), you should always use the latest sources from the above table. You may also clone the repo and build the bleeding edge. Since we have a stable master branch, this will also get you a good build, but it will be a snapshot version.

- **Using a binary:** The last bugfixes and the last functional enhancements were in version 1.1 (of 2006). So if you've got a working 1.1 binary on your system, you're good and you don't need to upgrade. Staying on the even older 1.0.1 will mean that you're missing features (such as better tab handling and the 'mend' option). Anything prior to 1.0.1 should definitely be upgraded.

Note that you can always copy the [latest box designs](https://github.com/{{ site.github }}/blob/master/boxes-config) from GitHub to replace your older *boxes-config* (Unix) or *boxes.cfg* (Windows) - the designs are backwards compatible.
