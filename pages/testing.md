---
title: Test Cases
permalink: /testing.html
created_at: 2015-03-17 13:06:24 +0100
last_modified_at: 2024-02-24 15:00:00 +0200
---

# Test Cases

The boxes sources include three types of tests:

- Unit tests ("white-box tests") test individual functions in isolation.
- Sunny-day tests are a suite of tests which simply creates, mends, and removes every box from our official config
  file. This is to make sure that regular operations work with all of our box designs.
- Black-box tests invoke *boxes* with all kinds of different arguments, inputs, and configurations in order to test
  the application as a whole.

Our continuous integration pipeline based on GitHub Actions will execute all three test suites in order to make sure
that a change did not break the program.

In order to calculate total test coverage across all of these test suites, execute them in the following order:

```bash
make cov             # compile with coverage instrumentation
make utest           # unit tests (no "cov" prefix required)
make covtest-sunny   # sunny-day tests
make covtest         # black-box tests
```
The final coverage output will be the total coverage value (example, will have changed by now):

```
Overall coverage rate:
  lines......: 89.0% (3976 of 4465 lines)
  functions..: 98.6% (275 of 279 functions)
  branches...: 76.3% (2573 of 3371 branches)
```


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Unit Tests" %}

Unit tests, also called "white-box tests", test individual functions in isolation.

In order to invoke the unit test suite, simply call `make utest` (Linux/UNIX/MacOS) or `make win32.utest` (Windows).

New unit tests can be added in the *utest* folder. We use [cmocka](https://cmocka.org/) as a unit test framework.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Sunny-Day Tests" %}

Sunny-day tests are a suite of tests which simply creates, mends, and removes every box from our official config file.
This is to make sure that regular operations work with all of our box designs.

In order to invoke the sunny-day test suite, simply call `make test-sunny`. This is valid for all operating systems.
Calling `make covtest-sunny` will also calculate test coverage, but this requires that the binary was instrumented
before, as per `make cov`.

<div class="alert alert-info">
    For every new box design, a corresponding sunny-day test must be created.
</div>

A new sunny-day test is only required when a box design is added to the official config file. Then, two files must
be created:

- test/sunny-day/*mydesign*.create.txt  
  Here, *mydesign* is the name of your new box design. Create the file with this command:
  ```
  out/boxes -f boxes-config -d mydesign \
      test/sunny-day/_input.txt > test/sunny-day/mydesign.create.txt
  ```
  The new file contains the expected output of the *create* operation.
- test/sunny-day/*mydesign*.mend.txt  
  Again, *mydesign* is the name of your new box design. Create the file with this command:
  ```
  out/boxes -f boxes-config -d mydesign test/sunny-day/_input.txt \
      | sed -e "s/et /__ET__ /g" \
      | out/boxes -f boxes-config -d mydesign -m > test/sunny-day/mydesign.mend.txt
  ```
  The first *boxes* invocation is the same as for "create" above. Then the `sed` command simulates some editing of the
  box content, and the second *boxes* invocation mends the box.  
  Make sure that these results are what you expect them to be, and that's it!


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=2
   text="Black-Box Tests" %}

Black-box tests invoke boxes with all kinds of different arguments, inputs, and configurations in order to test the
application as a whole.

In order to invoke the black-box test suite, simply call `make && make test` from the top level directory. This is
valid for all operating systems. Calling `make cov && make covtest` will also calculate test coverage.

Run an individual test by calling `./testrunner.sh testcase.txt` from the *test* directory.


{% comment %} ---------------------------------------------------------------------------------------- {% endcomment %}
{% include heading.html
   level=3
   text="Test Case Format" %}

The black-box tests are defined in the
[test](https://github.com/{{ site.github }}/tree/master/test){:target="_blank"} subdirectory.

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
