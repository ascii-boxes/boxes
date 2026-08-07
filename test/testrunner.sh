#!/usr/bin/env bash
#
# boxes - Command line filter to draw/remove ASCII boxes around text
# SPDX-FileCopyrightText: Copyright (c) 1999-2026 Thomas Jensen and the boxes contributors
# SPDX-License-Identifier: GPL-3.0-only
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
declare opt_coverage_per_test=false
declare opt_suite=false
declare opt_testCase=""

declare branchCoverage=lcov_branch_coverage
declare -a lcovArgs=(--ignore-errors unused)
declare -a lcovCaptureArgs=()
declare -ar lcovCaptureRcArgs=(--rc geninfo_unexecuted_blocks=1)
declare -ar lcovExcludeArgs=(--exclude '*/lex.yy.c' --exclude '*/parser.c' --exclude '*/lexer.l' --exclude '*/parser.y')

if [[ $(uname) == "Darwin" ]]; then
    branchCoverage=branch_coverage
    lcovArgs+=(--rc derive_function_end_line=0)
fi

if [[ -n "${GCOV_TOOL:-}" ]]; then
    lcovCaptureArgs+=(--gcov-tool "${GCOV_TOOL}")
fi


function print_usage()
{
    echo 'Usage: testrunner.sh [--coverage] [--coverage-per-test] {--suite | <opt_testCase>}'
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
            --coverage-per-test)
                opt_coverage_per_test=true
                opt_coverage=true
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
                if [ -z "${opt_testCase}" ]; then
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
    if [ "${PWD##*/}" != "test" ]; then
        >&2 echo "Please run this script from the test folder."
        exit 2
    fi
    if [ ! -d ${OUT_DIR} ]; then
        >&2 echo "Please run 'make' from the project root to build an executable before running tests."
        exit 2
    fi
    if [[ ${opt_coverage} == true && $(find ${OUT_DIR} -maxdepth 1 -name '*.gcno' 2>/dev/null | wc -l) -lt 1 ]]; then
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
                "${lcovCaptureArgs[@]}" "${lcovCaptureRcArgs[@]}" "${lcovExcludeArgs[@]}" "${lcovArgs[@]}" \
                --rc "${branchCoverage}=1" --output-file ${BASELINE_FILE} || return 1
            echo -e "Coverage baseline created in ${BASELINE_FILE}\n"
        fi
    fi
}


function cov_args()
{
    if [ ${opt_coverage_per_test} == true ]; then
        echo '--coverage-per-test'
    elif [ ${opt_coverage} == true ]; then
        echo '--coverage'
    fi
}


function execute_suite()
{
    local countExecuted=0
    local countFailed=0
    local tc

    # Note we are getting an encoding error with test 111 which is
    # unique and runs under ISO_8859-15. But this only happens on macOS.
    # So, if we run test 111 on macOS, we should run with LC_ALL=C
    for tc in *.txt; do
        if [[ $(uname) == "Darwin" ]] && [[ ${tc} == "111"* ]]; then
            LC_ALL=C $0 "$(cov_args)" "${tc}"
        else
            $0 "$(cov_args)" "${tc}"
        fi
        if [ $? -ne 0 ]; then
            overallResult=1
            countFailed=$((countFailed + 1))
        fi
        countExecuted=$((countExecuted + 1))
    done
    echo "${countExecuted} tests executed, $((countExecuted - countFailed)) successful, ${countFailed} failed."
}


function measure_coverage()
{
    if [ ${opt_coverage} == true ]; then
        local testResultsDir=${OUT_DIR}/test-results/${tcBaseName}
        mkdir -p "${testResultsDir}"
        cp ${OUT_DIR}/*.gc* "${testResultsDir}"
        lcov --capture --directory "${testResultsDir}" --base-directory ${SRC_DIR} --test-name "${tcBaseName}" --quiet \
            "${lcovCaptureArgs[@]}" "${lcovCaptureRcArgs[@]}" "${lcovExcludeArgs[@]}" "${lcovArgs[@]}" \
            --rc "${branchCoverage}=1" --output-file "${testResultsDir}/coverage.info" || return 1
        echo -n "    Coverage: "
        lcov --summary "${testResultsDir}/coverage.info" 2>&1 | grep 'lines...' | grep -oP '\d+\.\d*%' || return 1
    fi
}


function consolidate_coverage()
{
    if [[ ${opt_coverage} == true ]]; then
        local status=0

        echo -e "\nConsolidating test coverage ..."
        pushd ${OUT_DIR}/test-results || exit 1
        # LCOV 2.5 cannot reread branch data merged under multiple test names (lcov#522).
        find . -name "*.info" | xargs printf -- '--add-tracefile %s\n' | xargs --exit \
            lcov --forget-test-names --rc "${branchCoverage}=1" "${lcovExcludeArgs[@]}" "${lcovArgs[@]}" \
                --output-file ../${COVERAGE_FILE} --add-tracefile ../${BASELINE_FILE} || status=$?
        popd || exit 1
        if [ ${status} -ne 0 ]; then
            return ${status}
        fi
        echo ""
    fi
}


function report_coverage()
{
    local testReportDir=${OUT_DIR}/report
    mkdir -p ${testReportDir}
    genhtml --title "Boxes / All Tests" --branch-coverage --legend \
        --output-directory ${testReportDir} ${COVERAGE_FILE} || return 1
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
    if [[ $(grep -c "^:ENV" "${opt_testCase}") -eq 1 ]]; then
        boxesEnv=$(sed -n '/^:ENV/,/^:ARGS/p;' < "${opt_testCase}" | sed '1d;$d' | tr -d '\r')
    fi
    if [ -n "$boxesEnv" ]; then
        echo "$boxesEnv" | sed -e 's/export/\n    export/g' | sed '1d'
        unset BOXES
        eval "$boxesEnv"
    else
        export BOXES=../boxes-config
    fi
}


function arrange_test_fixtures()
{
    if [ $(grep -c "^:EXPECTED-ERROR " ${opt_testCase}) -eq 1 ]; then
        expectedReturnCode=$(grep "^:EXPECTED-ERROR " "${opt_testCase}" | sed -e 's/:EXPECTED-ERROR //')
    fi
    if [ $(grep -c "^:EXPECTED discard-stderr" ${opt_testCase}) -eq 1 ]; then
        discardStderr=true
    fi
    if [ $(grep -c "^:ASSERT-OUTPUT-FILE " "${opt_testCase}") -eq 1 ]; then
        assertOutputFile=$(grep "^:ASSERT-OUTPUT-FILE " "${opt_testCase}" | sed -e 's/^:ASSERT-OUTPUT-FILE //')
        rm -f "${assertOutputFile}"
    fi

    cat "${opt_testCase}" | sed -n '/^:INPUT/,/^:OUTPUT-FILTER/p;' | sed '1d;$d' | tr -d '\r' > "${testInputFile}"
    cat "${opt_testCase}" | sed -n '/^:OUTPUT-FILTER/,/^:EXPECTED\b.*$/p;' | sed '1d;$d' | tr -d '\r' > "${testFilterFile}"
    cat "${opt_testCase}" | sed -n '/^:EXPECTED/,/^:EOF/p;' | sed '1d;$d' | tr -d '\r' > "${testExpectationFile}"
}


function run_boxes()
{
    local boxesBinary=${OUT_DIR}/boxes.exe
    if [ ! -x $boxesBinary ]; then
        boxesBinary=${OUT_DIR}/boxes
    fi

    echo "    Invoking: $(basename $boxesBinary) $boxesArgs"
    if [ -z "${BOXES_TEST_XXD:-}" ]; then
        if [ ${discardStderr} == true ]; then
            eval "$boxesBinary $boxesArgs" < "$testInputFile" > "$testOutputFile" 2> "$testErrorFile"
        else
            eval "$boxesBinary $boxesArgs" < "$testInputFile" > "$testOutputFile" 2>&1
        fi
    else
        if [ ${discardStderr} == true ]; then
            eval "$boxesBinary $boxesArgs" < "$testInputFile" | xxd > "$testOutputFile" 2> "$testErrorFile"
        else
            eval "$boxesBinary $boxesArgs" < "$testInputFile" | xxd > "$testOutputFile" 2>&1
        fi
    fi
    actualReturnCode=$?
}


function assert_outcome()
{
    local actualOutputForComparison=${testOutputFile}
    if [ -n "${assertOutputFile}" ]; then
        if [ -s "${testOutputFile}" ]; then
            >&2 echo "Error in test case: ${opt_testCase} (stdout was not empty while asserting output file ${assertOutputFile})"
            exit 5
        fi
        actualOutputForComparison=${assertOutputFile}
    fi
    if [ ! -f "${actualOutputForComparison}" ]; then
        >&2 echo "Error in test case: ${opt_testCase} (output file '${actualOutputForComparison}' not found)"
        exit 5
    fi

    tr -d '\r' < "${actualOutputForComparison}" | sed -E -f "${testFilterFile}" | diff - "${testExpectationFile}"
    if [ $? -ne 0 ]; then
        >&2 echo "Error in test case: ${opt_testCase} (top: actual; bottom: expected)"
        exit 5
    fi
    if [ ${actualReturnCode} -ne "${expectedReturnCode}" ]; then
        >&2 echo -n "Error in test case: ${opt_testCase}"
        >&2 echo " (error code was ${actualReturnCode}, but expected ${expectedReturnCode})"
        exit 5
    fi
}


parse_arguments "$@"
check_prereqs
cov_baseline || exit $?

declare tcBaseName=${opt_testCase%.txt}

# Execute the entire test suite
if [ ${opt_suite} == true ]; then
    declare -i overallResult=0
    clear_gcda_traces
    execute_suite

    if [ ${opt_coverage_per_test} == false ]; then
        tcBaseName=black_box_all
        if ! measure_coverage; then
            overallResult=1
        fi
    fi
    if [ ${opt_coverage} == true ]; then
        if ! consolidate_coverage || ! report_coverage; then
            overallResult=1
        fi
    fi
    exit ${overallResult}
fi

# Execute only a single test
echo "Running test case: ${opt_testCase}"
if [ ${opt_coverage_per_test} == true ]; then
    clear_gcda_traces
fi

check_mandatory_sections

declare -i expectedReturnCode=0
declare -r testInputFile=${opt_testCase/%.txt/.input.tmp}
declare -r testExpectationFile=${opt_testCase/%.txt/.expected.tmp}
declare -r testFilterFile=${opt_testCase/%.txt/.sed.tmp}
declare -r testOutputFile=${opt_testCase/%.txt/.out.tmp}
declare -r testErrorFile=${opt_testCase/%.txt/.err.tmp}
declare -r boxesArgs=$(sed -n '/^:ARGS/,+1p' < "${opt_testCase}" | grep -v ^:INPUT | sed '1d' | tr -d '\r')
declare discardStderr=false
declare assertOutputFile=""

arrange_environment
arrange_test_fixtures

declare -i actualReturnCode=100
run_boxes

if [ ${opt_coverage_per_test} == true ]; then
    measure_coverage || exit $?
fi
assert_outcome

rm "${testInputFile}"
rm "${testFilterFile}"
rm "${testExpectationFile}"
rm "${testOutputFile}"
rm -f "${testErrorFile}"
if [ -n "${assertOutputFile}" ]; then
    rm -f "${assertOutputFile}"
fi

echo "    OK"
exit 0
