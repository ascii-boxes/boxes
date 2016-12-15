#!/usr/bin/env bash
#
# Low-tech test runner for boxes.
#
# File:             testrunner.sh
# Date created:     September 23, 2014 (Tuesday, 20:06h)
# Author:           Thomas Jensen
# _____________________________________________________________________

# Execute the entire test suite
if [ "$1" == "-suite" ]; then
    declare -i overallResult=0
    declare -i countExecuted=0
    declare -i countFailed=0
    declare -r boxesRoot=$(dirname $0)/..
    echo $boxesRoot
    declare boxesExe="$boxesRoot/boxes.exe"
    [ -x "$boxesExe" ] || boxesExe="$boxesRoot/boxes"
    [ -x "$boxesExe" ] || boxesExe="$boxesRoot/src/boxes.exe"
    [ -x "$boxesExe" ] || boxesExe="$boxesRoot/src/boxes"
    [ -x "$boxesExe" ] || boxesExe="$boxesRoot/build/boxes.exe"
    [ -x "$boxesExe" ] || boxesExe="$boxesRoot/build/boxes"
    declare tc
    for tc in $boxesRoot/test/*_*.txt; do
        ./$0 $boxesExe "$boxesRoot/boxes-config" $tc
        if [ $? -ne 0 ]; then
            overallResult=1
            ((countFailed++))
        fi
        ((countExecuted++))
    done
    echo "$countExecuted tests executed, $(($countExecuted-$countFailed)) successful, $countFailed failed."
    exit $overallResult
fi

if [ $# -ne 3 ]; then
    echo 'Usage: testrunner.sh {-suite | <executable> <configFile> <testCaseFile>}'
    echo '       Returns 0 for success, else non-zero'
    exit 2
fi

declare -r boxesBinary="$1"
if [ ! -x $boxesBinary ]; then
    >&2 echo "Binary '$boxesBinary' not found or not executable."
    exit 3
fi
declare -r boxesConfiguration="$2"
declare -r testCaseFile="$3"
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
declare -r boxesArgs=$(cat $testCaseFile | sed -n '/^:ARGS/,/^:INPUT/p;' | sed '1d;$d' | tr -d '\r')

cat $testCaseFile | sed -n '/^:INPUT/,/^:OUTPUT-FILTER/p;' | sed '1d;$d' | tr -d '\r' > $testInputFile
cat $testCaseFile | sed -n '/^:OUTPUT-FILTER/,/^:EXPECTED/p;' | sed '1d;$d' | tr -d '\r' > $testFilterFile
cat $testCaseFile | sed -n '/^:EXPECTED/,/^:EOF/p;' | sed '1d;$d' | tr -d '\r' > $testExpectationFile

echo "    Invoking: $(basename $boxesBinary) $boxesArgs"
export BOXES="$boxesConfiguration"

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
