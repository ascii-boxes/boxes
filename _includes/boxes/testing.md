# Test Cases

The boxes sources have a low-tech test suite in the [test](https://github.com/{{ site.github }}/tree/master/test) subdirectory.
Travis CI will execute it in order to make sure that a change did not break the program.

Currently, the test suite could use more test cases, so any help with growing the number of tests is highly appreciated.


<a class="bxOffsetAnchor" name="invocation"></a>

## Invocation&nbsp;

Start the entire suite by running `make && make test` from the top level directory.

Run an individual test by calling `./testrunner.sh testcase.txt` from the *test* directory.


<a class="bxOffsetAnchor" name="test-case-format"></a>

## Test&nbsp;Case Format

Each test case is a single file within the *test* subdirectory. It must follow this naming convention:

    nnn_description.txt

where `nnn` is a three-digit number which uniquely identifies the test case. `description` is any short text that describes what the test case does. It must not contain spaces; use underscores instead. The file extension is always `.txt`.

A test case that tests a **successful** invocation of boxes looks like this:

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

Sections may be empty, e.g. if there are no arguments or there is no input.

A test case that makes sure boxes **fails** under certain conditions looks like this:

    :ARGS
    -f nonexistent
    :INPUT
    :OUTPUT-FILTER
    :EXPECTED-ERROR 1
    boxes: Couldn't open config file 'nonexistent' for input.
    :EOF

Note that you write `:EXPECTED-ERROR` instead of just `:EXPECTED`, and the expected return code is given after a space (in this example, it is `1`).

The `:OUTPUT-FILTER` section can be used to give a *sed* script which is run on the actual output before comparing it to the expected output ([example](https://github.com/{{ site.github }}/blob/master/test/065_size_missing_argument.txt)). This way, differences in output that occur because of platform differences can be filtered out. The general advice is to leave this section empty unless you are facing a situation where there is no other solution.
