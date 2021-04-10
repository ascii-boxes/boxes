#!/usr/bin/env bash
#
# boxes - Command line filter to draw/remove ASCII boxes around text
# Copyright (c) 1999-2021 Thomas Jensen and the boxes contributors
#
# This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public
# License, version 2, as published by the Free Software Foundation.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
# You should have received a copy of the GNU General Public License along with this program; if not, write to the Free
# Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
# _____________________________________________________________________________________________________________________
#
# Low-tech test runner for boxes.
# _____________________________________________________________________________________________________________________

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

declare boxesEnv=""
if [ $(grep -c "^:ENV" $testCaseFile) -eq 1 ]; then
    boxesEnv=$(cat $testCaseFile | sed -n '/^:ENV/,/^:ARGS/p;' | sed '1d;$d' | tr -d '\r')
fi

declare -i expectedReturnCode=0
if [ $(grep -c "^:EXPECTED-ERROR " $testCaseFile) -eq 1 ]; then
    expectedReturnCode=$(grep "^:EXPECTED-ERROR " $testCaseFile | sed -e 's/:EXPECTED-ERROR //')
fi

declare -r testInputFile=${testCaseFile/%.txt/.input.tmp}
declare -r testExpectationFile=${testCaseFile/%.txt/.expected.tmp}
declare -r testFilterFile=${testCaseFile/%.txt/.sed.tmp}
declare -r testOutputFile=${testCaseFile/%.txt/.out.tmp}
declare -r boxesArgs=$(cat $testCaseFile | sed -n '/^:ARGS/,+1p' | grep -v ^:INPUT | sed '1d' | tr -d '\r')

cat $testCaseFile | sed -n '/^:INPUT/,/^:OUTPUT-FILTER/p;' | sed '1d;$d' | tr -d '\r' > $testInputFile
cat $testCaseFile | sed -n '/^:OUTPUT-FILTER/,/^:EXPECTED\b.*$/p;' | sed '1d;$d' | tr -d '\r' > $testFilterFile
cat $testCaseFile | sed -n '/^:EXPECTED/,/^:EOF/p;' | sed '1d;$d' | tr -d '\r' > $testExpectationFile

declare boxesBinary=../out/boxes.exe
if [ ! -x $boxesBinary ]; then
    boxesBinary=../out/boxes
fi

if [ ! -z "$boxesEnv" ]; then
    echo $boxesEnv | sed -e 's/export/\n    export/g' | sed '1d'
    unset BOXES
    eval $boxesEnv
else
    export BOXES=../boxes-config
fi

echo "    Invoking: $(basename $boxesBinary) $boxesArgs"
if [ ! -z "$BOXES_TEST_XXD" ]; then
    cat $testInputFile | $boxesBinary $boxesArgs | xxd >$testOutputFile 2>&1
else
    cat $testInputFile | $boxesBinary $boxesArgs >$testOutputFile 2>&1
fi
declare -ir actualReturnCode=$?
cat $testOutputFile | tr -d '\r' | sed -E -f $testFilterFile | diff - $testExpectationFile
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
