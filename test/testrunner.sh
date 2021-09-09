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
# Test runner for the black-box tests.
# _____________________________________________________________________________________________________________________

measureCoverage=false
if [ $# -lt 1 -o $# -gt 2 ]; then
    echo 'Usage: testrunner.sh [--coverage] {--suite | <testCaseFile>}'
    echo '       Returns 0 for success, else non-zero'
    exit 2
elif [ $# -eq 1 ]; then
    if [ "$1" == "--coverage" ]; then
        echo 'Usage: testrunner.sh [--coverage] {--suite | <testCaseFile>}'
        echo '       Returns 0 for success, else non-zero'
        exit 2
    fi
elif [ $# -eq 2 ]; then
    if [ "$1" != "--coverage" -a "$2" != "--coverage" ]; then
        echo 'Usage: testrunner.sh [--coverage] {--suite | <testCaseFile>}'
        echo '       Returns 0 for success, else non-zero'
        exit 2
    else
        measureCoverage=true
    fi
fi

if [ ${PWD##*/} != "test" ]; then
    >&2 echo "Please run this script from the test folder."
    exit 2
fi
if [ ! -d ../out ]; then
    >&2 echo "Please run 'make' from the project root to build the executable before running the tests."
    exit 2
fi

if [ $measureCoverage == true ]; then
    if [ $(ls ../out/*.gcno 2>/dev/null | wc -l) -lt 1 ]; then
        >&2 echo "Binaries not instrumented. Run 'make cov' from the project root."
        exit 5
    fi
    if [ "$1" == "--suite" -o "$2" == "--suite" ]; then
        rm -f ../out/lcov-baseline.info
    fi
    if [ ! -f ../out/lcov-baseline.info ]; then
        echo "Creating coverage baseline ..."
        lcov --capture --initial --no-recursion --directory ../out --base-directory ../src \
            --exclude '*/lex.yy.c' --exclude '*/parser.c' --output-file ../out/lcov-baseline.info
        echo -e "Coverage baseline created in ../out/lcov-baseline.info\n"
    fi
fi

# Execute the entire test suite
if [ "$1" == "--suite" -o "$2" == "--suite" ]; then
    declare -i overallResult=0
    declare -i countExecuted=0
    declare -i countFailed=0
    declare tc
    for tc in *.txt; do
        if [ $measureCoverage == true ]; then
            $0 --coverage $tc
        else
            $0 $tc
        fi
        if [ $? -ne 0 ]; then
            overallResult=1
            ((countFailed++))
        fi
        ((countExecuted++))
    done
    echo "$countExecuted tests executed, $(($countExecuted-$countFailed)) successful, $countFailed failed."

    if [ $measureCoverage == true ]; then
        echo -e "\nConsolidating test coverage ..."
        pushd ../out/test-results
        find . -name *.info | xargs printf -- '--add-tracefile %s\n' | xargs --exit \
            lcov --rc lcov_branch_coverage=1 --exclude '*/lex.yy.c' --exclude '*/parser.c' \
                 --output-file ../lcov-blackbox.info --add-tracefile ../lcov-baseline.info
        popd
        echo ""

        declare -r testReportDir=../out/coverage
        mkdir -p ${testReportDir}
        genhtml --title "Boxes / black-box tests" --branch-coverage --legend \
            --output-directory ${testReportDir} ../out/lcov-blackbox.info
        echo -e "\nTest coverage report created in ${testReportDir}/index.html"
    fi
    exit $overallResult
fi

# Execute only a single test
declare testCaseFile="$1"
if [ "$1" == "--coverage" ]; then
    testCaseFile="$2"
fi
if [ ! -f $testCaseFile ]; then
    >&2 echo "Test Case '$testCaseFile' not found."
    exit 3
fi

echo "Running test case: $testCaseFile"

declare -r testCaseName=${testCaseFile:0:-4}
if [ $measureCoverage == true ]; then
    rm -f ../out/*.gcda    # remove any left-over coverage data files
fi

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
    cat $testInputFile | eval "$boxesBinary $boxesArgs" | xxd >$testOutputFile 2>&1
else
    cat $testInputFile | eval "$boxesBinary $boxesArgs" >$testOutputFile 2>&1
fi
declare -ir actualReturnCode=$?

declare -r testResultsDir=../out/test-results/${testCaseName}
if [ $measureCoverage == true ]; then
    mkdir -p ${testResultsDir}
    cp ../out/*.gc* ${testResultsDir}
    lcov --capture --directory ${testResultsDir} --base-directory ../src --test-name ${testCaseName} --quiet \
        --exclude '*/lex.yy.c' --exclude '*/parser.c' --rc lcov_branch_coverage=1 \
        --output-file ${testResultsDir}/coverage.info
    echo -n "    Coverage: "
    lcov --summary ${testResultsDir}/coverage.info 2>&1 | grep 'lines...' | grep -oP '\d+\.\d*%'
fi

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
