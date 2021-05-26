---
title: Test Cases
permalink: /testing.html
created_at: 2015-03-17 13:06:24 +0100
last_modified_at: 2021-02-17 10:38:00 +0200
---

# Test Cases

The boxes sources have a low-tech test suite in the
[test](https://github.com/{{ site.github }}/tree/master/test){:target="_blank"} subdirectory. Our continuous
integration pipeline will execute it in order to make sure that a change did not break the program.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Invocation" %}

Start the entire suite by running `make && make test` from the top level directory.

Run an individual test by calling `./testrunner.sh testcase.txt` from the *test* directory.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Test Case Format" %}

Each test case is a single file within the *test* subdirectory. It must follow this naming convention:

    nnn_description.txt

where `nnn` is a three-digit number which uniquely identifies the test case. `description` is any short text that
describes what the test case does. It must not contain spaces; use underscores instead. The file extension is always
`.txt`.

A test case that tests a **successful** invocation of boxes looks like this:

    :DESC
	Tests invoking boxes with a fixed result box size.
    
    :ARGS
    -s 10x4
    :INPUT
    foo
    :OUTPUT-FILTER
    :EXPECTED
    /********/
    /* foo  */
    /*      */
    /********/
    :EOF

Sections may be empty, e.g. if there are no arguments or there is no input. The `:DESC` section is optional, so it is
valid if the test case starts with an `:ARGS` section. The order of sections is fixed.

A test case that makes sure boxes **fails** under certain conditions looks like this:

    :DESC
	Tests that the right error message is shown when the boxes
	config file cannot be found.
    
    :ARGS
    -f nonexistent
    :INPUT
    :OUTPUT-FILTER
    :EXPECTED-ERROR 1
    boxes: Couldn't find config file at 'nonexistent'
    :EOF

Note that you write `:EXPECTED-ERROR` instead of just `:EXPECTED`, and the expected return code is given after a space
(in this example, it is `1`).

The `:OUTPUT-FILTER` section can be used to give a *sed* script which is run on the actual output before comparing it
to the expected output
([example](https://github.com/{{ site.github }}/blob/master/test/065_size_missing_argument.txt){:target="_blank"}).
This way, differences in output that occur because of platform differences can be filtered out. The general advice is
to leave this section empty unless you are facing a situation where there is no other solution.
