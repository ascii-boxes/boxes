#!/usr/bin/env bash
#
# Low-tech test runner for boxes.
#
# File:             testrunner.sh
# Date created:     September 23, 2014 (Tuesday, 20:06h)
# Author:           Thomas Jensen
# _____________________________________________________________________

if [ $# -ne 1 ]; then
    echo 'Usage: testrunner.sh {-suite | <testCaseFile>}'
    echo '       Returns 0 for success, else non-zero'
    exit 2
fi
if [ ${PWD##*/} != "test" ]; then
    >&2 echo "Please run this script from the test folder."
    exit 2
fi

# Execute the entire test suite
if [ "$1" == "-suite" ]; then
    declare -i overallResult=0
    declare -i countExecuted=0
    declare -i countFailed=0
    declare tc
    for tc in *.txt; do
        $0 $tc
        if [ $? -ne 0 ]; then
            overallResult=1
            ((countFailed++))
        fi
        ((countExecuted++))
    done
    echo "$countExecuted tests executed, $(($countExecuted-$countFailed)) successful, $countFailed failed."
    exit $overallResult
fi

# Execute only a single test
declare -r testCaseFile="$1"
if [ ! -f $testCaseFile ]; then
    >&2 echo "Test Case '$testCaseFile' not found."
    exit 3
fi

echo "Running test case: $testCaseFile"

declare sectionName
for sectionName in :ARGS :INPUT :OUTPUT-FILTER :EXPECTED :EOF; do
    if [ $(grep -c ^$sectionName $testCaseFile) -ne 1 ]; then
        >&2 echo "Missing section $sectionName in test case '$testCaseFile'."
        exit 4
    fi
done

declare -i expectedReturnCode=0
if [ $(grep -c "^:EXPECTED-ERROR " $testCaseFile) -eq 1 ]; then
    expectedReturnCode=$(grep "^:EXPECTED-ERROR " $testCaseFile | sed -e 's/:EXPECTED-ERROR //')
fi

declare -r testInputFile=${testCaseFile/%.txt/.input.tmp}
declare -r testExpectationFile=${testCaseFile/%.txt/.expected.tmp}
declare -r testFilterFile=${testCaseFile/%.txt/.sed.tmp}
declare -r testOutputFile=${testCaseFile/%.txt/.out.tmp}
declare -r boxesArgs=$(cat $testCaseFile | sed -n '/^:ARGS/,+1p' | grep -v ^:INPUT | sed '1d')

cat $testCaseFile | sed -n '/^:INPUT/,/^:OUTPUT-FILTER/p;' | sed '1d;$d' | tr -d '\r' > $testInputFile
cat $testCaseFile | sed -n '/^:OUTPUT-FILTER/,/^:EXPECTED\b.*$/p;' | sed '1d;$d' | tr -d '\r' > $testFilterFile
cat $testCaseFile | sed -n '/^:EXPECTED/,/^:EOF/p;' | sed '1d;$d' | tr -d '\r' > $testExpectationFile

declare boxesBinary=../src/boxes.exe
if [ ! -x $boxesBinary ]; then
    boxesBinary=../src/boxes
fi

echo "    Invoking: $(basename $boxesBinary) $boxesArgs"
export BOXES=../boxes-config

cat $testInputFile | $boxesBinary $boxesArgs >$testOutputFile 2>&1
declare -ir actualReturnCode=$?
cat $testOutputFile | tr -d '\r' | sed -f $testFilterFile | diff - $testExpectationFile
if [ $? -ne 0 ]; then
    >&2 echo "Error in test case: $testCaseFile (top: actual; bottom: expected)"
    exit 5
fi
if [ $actualReturnCode -ne $expectedReturnCode ]; then
    >&2 echo "Error in test case: $testCaseFile (error code was $actualReturnCode, but expected $expectedReturnCode)"
    exit 5
fi

rm $testInputFile
rm $testFilterFile
rm $testExpectationFile
rm $testOutputFile

echo "    OK"
exit 0

#EOF
