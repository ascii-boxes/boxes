"""
Low-tech test runner for boxes.

File:             testrunner.py
Date created:     December 03, 2016 (Saturday, 23:18h)
Author:           Nic H
_____________________________________________________________________
"""

import sys
import os
import subprocess

def read_case(testcase):
    """Returns a dict of all the test-case fields and the expected exit code of the test case secribed in the file 'testcase'"""
    test_case_sections={':ARGS':'', ':INPUT':'', ':OUTPUT-FILTER':'', ':EXPECTED':'', ':EXPECTED-ERROR':'', ':EOF':''}
    cur_section = ':EOF' # placeholder, any data in this section is later ignored
    expected_return = 0
    with open(testcase, 'rU') as tc:
        for ln in tc.readlines():
            if ln.startswith(':EXPECTED-ERROR'):
                expected_return = int(ln.split(' ')[1])
                cur_section = ':EXPECTED-ERROR'
            elif ln.startswith(':EOF'):
                break
            elif ln[:-1] in test_case_sections.keys():
                cur_section = ln[:-1]
            else:
                test_case_sections[cur_section] += filter(lambda x : x != '\r', ln)
    return (expected_return, test_case_sections)

def execute_test(exe, cfg, args, text):
    """Fork a subprocess for 'exe' with 'args' arguments and 'text' as stdin. 'cfg' is exported as an environment variable to the subprocess."""
    p = subprocess.Popen(
            [exe] + args,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=dict(os.environ, BOXES=cfg)
        )
    try:
        p.stdin.write(text)
        (out, err) = p.communicate()
        return (filter(lambda x : x != '\r', out), filter(lambda x : x != '\r', err), p.wait())
    except IOError as e:
        if e.errno != errno.EPIPE:
            raise e
        return ('', '', p.wait())

def run_test(exe, cfg, testcase):
    """Run 'exe' with configuration 'cfg' on the test case 'testcase'"""
    (expected_return, test_case_sections) = read_case(testcase)
    tc_args = [a for a in test_case_sections[':ARGS'].strip().split(" ") if a]
    (out, err, code) = execute_test(exe, cfg, tc_args, test_case_sections[':INPUT'])
    if code == expected_return and out == test_case_sections[':EXPECTED']:
        print 'Passed:', testcase
    else:
        print 'Failed: %s' % testcase
        print '\nExpected output:\n%s\n\nActual output:\n%s\nError:\n%s\n' % (test_case_sections[':EXPECTED'], out, err)
        print '\n Error codes (expected actual): %d %d' % (expected_return, code)
        print ''
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) == 4:
        run_test(sys.argv[1], sys.argv[2], sys.argv[3])
    else:
        print "Usage: %s <boxes-executable> <boces-configuration> <test case>" % sys.argv[0]

