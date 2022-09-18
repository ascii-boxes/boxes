#!/usr/bin/env bash
#
# boxes - Command line filter to draw/remove ASCII boxes around text
# Copyright (c) 1999-2021 Thomas Jensen and the boxes contributors
#
# This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
# License, version 3, as published by the Free Software Foundation.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
# You should have received a copy of the GNU General Public License along with this program.
# If not, see <https://www.gnu.org/licenses/>.
#____________________________________________________________________________________________________________________
#
# Test runner for the black-box tests.
#____________________________________________________________________________________________________________________

set -uo pipefail

# Global constants
declare -r SRC_DIR=../src
declare -r OUT_DIR=../out
declare -r BASELINE_FILE=${OUT_DIR}/lcov-baseline.info
declare -r COVERAGE_FILE=${OUT_DIR}/lcov-total.info

# Command Line Options
declare opt_coverage=false
declare opt_suite=false
declare opt_testCase=""


function print_usage()
{
    echo 'Usage: testrunner.sh [--coverage] {--suite | <opt_testCase>}'
    echo '       Returns 0 for success, else non-zero'
}


function parse_arguments()
{
    if [ $# -eq 0 ]; then
        print_usage
        exit 2
    fi

    for i in "$@"; do
        case ${i} in
            --coverage)
                opt_coverage=true
                shift
                ;;
            --suite)
                opt_suite=true
                shift
                ;;
            -h | --help)
                print_usage
                exit 0
                ;;
            *)
                if [ -z ${opt_testCase} ]; then
                    opt_testCase=${i}
                else
                    print_usage
                    exit 2
                fi
                ;;
        esac
    done

    if [[ ${opt_testCase} == "" && ${opt_suite} != "true" ]]; then
        print_usage
        exit 2
    fi
}


function check_prereqs()
{
    if [ ${PWD##*/} != "test" ]; then
        >&2 echo "Please run this script from the test folder."
        exit 2
    fi
    if [ ! -d ${OUT_DIR} ]; then
        >&2 echo "Please run 'make' from the project root to build an executable before running tests."
        exit 2
    fi
    if [[ ${opt_coverage} == true && $(ls ${OUT_DIR}/*.gcno 2>/dev/null | wc -l) -lt 1 ]]; then
        >&2 echo "Binaries not instrumented. Run 'make cov' from the project root."
        exit 5
    fi
    if [ ! -f ${opt_testCase} ]; then
        >&2 echo "Test Case '${opt_testCase}' not found."
        exit 3
    fi
}


function cov_baseline()
{
    if [ ${opt_coverage} == true ]; then
        if [ ${opt_suite} == true ]; then
            rm -f ${BASELINE_FILE}
        fi
        if [ ! -f ${BASELINE_FILE} ]; then
            echo "Creating coverage baseline ..."
            lcov --capture --initial --no-recursion --directory ${OUT_DIR} --base-directory ${SRC_DIR} \
                --exclude '*/lex.yy.c' --exclude '*/parser.c' --output-file ${BASELINE_FILE}
            echo -e "Coverage baseline created in ${BASELINE_FILE}\n"
        fi
    fi
}


function execute_suite()
{
    local countExecuted=0
    local countFailed=0
    local tc
    for tc in *.txt; do
        if [ ${opt_coverage} == true ]; then
            $0 --coverage ${tc}
        else
            $0 ${tc}
        fi
        if [ $? -ne 0 ]; then
            overallResult=1
            countFailed=$((countFailed + 1))
        fi
        countExecuted=$((countExecuted + 1))
    done
    echo "${countExecuted} tests executed, $(($countExecuted - $countFailed)) successful, ${countFailed} failed."
}


function measure_coverage()
{
    local testResultsDir=${OUT_DIR}/test-results/${tcBaseName}
    if [ ${opt_coverage} == true ]; then
        mkdir -p ${testResultsDir}
        cp ${OUT_DIR}/*.gc* ${testResultsDir}
        lcov --capture --directory ${testResultsDir} --base-directory ${SRC_DIR} --test-name ${tcBaseName} --quiet \
            --exclude '*/lex.yy.c' --exclude '*/parser.c' --rc lcov_branch_coverage=1 \
            --output-file ${testResultsDir}/coverage.info
        echo -n "    Coverage: "
        lcov --summary ${testResultsDir}/coverage.info 2>&1 | grep 'lines...' | grep -oP '\d+\.\d*%'
    fi
}


function consolidate_coverage()
{
    echo -e "\nConsolidating test coverage ..."
    pushd ${OUT_DIR}/test-results
    find . -name *.info | xargs printf -- '--add-tracefile %s\n' | xargs --exit \
        lcov --rc lcov_branch_coverage=1 --exclude '*/lex.yy.c' --exclude '*/parser.c' \
             --output-file ../${COVERAGE_FILE} --add-tracefile ../${BASELINE_FILE}
    popd
    echo ""
}


function report_coverage()
{
    local testReportDir=${OUT_DIR}/report
    mkdir -p ${testReportDir}
    genhtml --title "Boxes / All Tests" --branch-coverage --legend \
        --output-directory ${testReportDir} ${COVERAGE_FILE}
    echo -e "\nTest coverage report available at ${testReportDir}/index.html"
}


function clear_gcda_traces()
{
    if [ ${opt_coverage} == true ]; then
        rm -f ${OUT_DIR}/*.gcda
    fi
}


function check_mandatory_sections()
{
    local sectionName
    for sectionName in :ARGS :INPUT :OUTPUT-FILTER :EXPECTED :EOF; do
        if [ $(grep -c ^$sectionName $opt_testCase) -ne 1 ]; then
            >&2 echo "Missing section $sectionName in test case '$opt_testCase'."
            exit 4
        fi
    done
}


function arrange_environment()
{
    local boxesEnv=""
    if [ $(grep -c "^:ENV" ${opt_testCase}) -eq 1 ]; then
        boxesEnv=$(cat ${opt_testCase} | sed -n '/^:ENV/,/^:ARGS/p;' | sed '1d;$d' | tr -d '\r')
    fi
    if [ ! -z "$boxesEnv" ]; then
        echo $boxesEnv | sed -e 's/export/\n    export/g' | sed '1d'
        unset BOXES
        eval $boxesEnv
    else
        export BOXES=../boxes-config
    fi
}


function arrange_test_fixtures()
{
    if [ $(grep -c "^:EXPECTED-ERROR " ${opt_testCase}) -eq 1 ]; then
        expectedReturnCode=$(grep "^:EXPECTED-ERROR " ${opt_testCase} | sed -e 's/:EXPECTED-ERROR //')
    fi

    cat ${opt_testCase} | sed -n '/^:INPUT/,/^:OUTPUT-FILTER/p;' | sed '1d;$d' | tr -d '\r' > ${testInputFile}
    cat ${opt_testCase} | sed -n '/^:OUTPUT-FILTER/,/^:EXPECTED\b.*$/p;' | sed '1d;$d' | tr -d '\r' > ${testFilterFile}
    cat ${opt_testCase} | sed -n '/^:EXPECTED/,/^:EOF/p;' | sed '1d;$d' | tr -d '\r' > ${testExpectationFile}
}


function run_boxes()
{
    local boxesBinary=${OUT_DIR}/boxes.exe
    if [ ! -x $boxesBinary ]; then
        boxesBinary=${OUT_DIR}/boxes
    fi

    echo "    Invoking: $(basename $boxesBinary) $boxesArgs"
    if [ -z "${BOXES_TEST_XXD:-}" ]; then
        cat $testInputFile | eval "$boxesBinary $boxesArgs" >$testOutputFile 2>&1
    else
        cat $testInputFile | eval "$boxesBinary $boxesArgs" | xxd >$testOutputFile 2>&1
    fi
    actualReturnCode=$?
}


function assert_outcome()
{
    cat ${testOutputFile} | tr -d '\r' | sed -E -f ${testFilterFile} | diff - ${testExpectationFile}
    if [ $? -ne 0 ]; then
        >&2 echo "Error in test case: ${opt_testCase} (top: actual; bottom: expected)"
        exit 5
    fi
    if [ ${actualReturnCode} -ne ${expectedReturnCode} ]; then
        >&2 echo -n "Error in test case: ${opt_testCase}"
        >&2 echo " (error code was ${actualReturnCode}, but expected ${expectedReturnCode})"
        exit 5
    fi
}


parse_arguments "$@"
check_prereqs
cov_baseline

# Execute the entire test suite
if [ ${opt_suite} == true ]; then
    declare -i overallResult=0
    execute_suite

    if [ ${opt_coverage} == true ]; then
        consolidate_coverage
        report_coverage
    fi
    exit ${overallResult}
fi

# Execute only a single test
echo "Running test case: ${opt_testCase}"
declare -r tcBaseName=${opt_testCase%.txt}
clear_gcda_traces

check_mandatory_sections

declare -i expectedReturnCode=0
declare -r testInputFile=${opt_testCase/%.txt/.input.tmp}
declare -r testExpectationFile=${opt_testCase/%.txt/.expected.tmp}
declare -r testFilterFile=${opt_testCase/%.txt/.sed.tmp}
declare -r testOutputFile=${opt_testCase/%.txt/.out.tmp}
declare -r boxesArgs=$(cat ${opt_testCase} | sed -n '/^:ARGS/,+1p' | grep -v ^:INPUT | sed '1d' | tr -d '\r')

arrange_environment
arrange_test_fixtures

declare -i actualReturnCode=100
run_boxes
measure_coverage

assert_outcome

rm ${testInputFile}
rm ${testFilterFile}
rm ${testExpectationFile}
rm ${testOutputFile}

echo "    OK"
exit 0

#EOF
