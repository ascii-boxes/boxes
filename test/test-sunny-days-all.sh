#!/usr/bin/env bash
#
# boxes - Command line filter to draw/remove ASCII boxes around text
# Copyright (c) 1999-2024 Thomas Jensen and the boxes contributors
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
# Tests simple box generation and removal/mending for all box designs in the default config file.
#____________________________________________________________________________________________________________________

set -uo pipefail
shopt -s extglob

# Global constants
declare -r SRC_DIR=../src
declare -r OUT_DIR=../out
declare -r TEST_DIR=${OUT_DIR}/sunny
declare -r EXPECTED_DIR=sunny-day
declare -r INPUT_FILE=${EXPECTED_DIR}/_input.txt
declare -r COVERAGE_DIR=${OUT_DIR}/test-results/sunny
declare -r COVERAGE_FILE=${COVERAGE_DIR}/coverage.info

# Command Line Options
declare opt_coverage=false

# Global Variables
declare result=0
declare success=0
declare -i countExecuted=0
declare -i countFailed=0



function print_usage()
{
    echo 'Usage: test-sunny-days-all.sh [--coverage]'
    echo '       Returns 0 for success, else non-zero'
}


function parse_arguments()
{
    for i in "$@"; do
        case ${i} in
            --coverage)
                opt_coverage=true
                shift
                ;;
            -h | --help)
                print_usage
                exit 0
                ;;
            *)
                print_usage
                exit 2
        esac
    done
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
}


function get_executable_name()
{
    local binaryName=${OUT_DIR}/boxes.exe
    if [ ! -x "$binaryName" ]; then
        binaryName=${OUT_DIR}/boxes
    fi
    echo $binaryName
}


function get_coverage_option()
{
    local coverageOption=lcov_branch_coverage
    if [[ $(uname) == "Darwin" ]]; then
        coverageOption=branch_coverage
    fi
    echo ${coverageOption}
}


function list_designs()
{
    ${boxesBinary} -f ../boxes-config -q '(all)' | grep -v ' (alias)'
}


function clear_gcda_traces()
{
    if [ ${opt_coverage} == true ]; then
        rm -f ${OUT_DIR}/*.gcda
    fi
}


function fail()
{
    success=0
    result=1
    countFailed=$((countFailed + 1))
}


function file_exists()
# Args: $1 - file
{
    if [[ ! -f "$1" ]]; then
        >&2 echo -n "Missing file '$1' "
        fail
        return 1
    fi
    return 0
}


function run_boxes_create()
# Args: $1 - outfile
{
    ${boxesBinary} -f ../boxes-config -d "${design}" "${INPUT_FILE}" | tr -d '\r' > "$1"
}


function run_boxes_mend()
# Args: $1 - outfile
{
    declare rp="et "
    declare rr="__ET__ "
    if [[ "${design}" == "headline" ]]; then
        rp="e t  "
        rr="_ _ E T _ _  "
    fi
    sed -e "s/${rp}/${rr}/g" "${TEST_DIR}/${design}.create.actual.txt" \
            | ${boxesBinary} -f ../boxes-config -d "${design}" -m \
            | tr -d '\r' > "$1"
}


function run_boxes_remove()
# Args: $1 - outfile
{
    ${boxesBinary} -f ../boxes-config -d "${design}" -r "${TEST_DIR}/${design}.create.actual.txt" | tr -d '\r' > "$1"
}


function print_success()
{
    if [[ $success == 0 ]]; then
        echo -e "... \033[31mERROR\033[0m"
    else
        echo -e "... \033[32mOK\033[0m"
    fi
}


function assert_result()
# Args: $1 - verb
#       $2 - expected file
#       $3 - actual file
{
    tr -d '\r' < "$2" > "${TEST_DIR}/${design}.$1.expected.txt"
    if ! diff "$3" "${TEST_DIR}/${design}.$1.expected.txt"; then
        >&2 echo "Error ${1/%?(e)/ing} '${design}' box (top: actual; bottom: expected)"
        fail
    fi
}


function measure_coverage()
{
    if [ ${opt_coverage} == true ]; then
        mkdir -p "${COVERAGE_DIR}"
        cp ${OUT_DIR}/*.gc* "${COVERAGE_DIR}"
        lcov --capture --directory "${COVERAGE_DIR}" --base-directory "${SRC_DIR}" --test-name SunnyDayTests \
            --quiet --exclude '*/lex.yy.c' --exclude '*/parser.c' --rc "${branchCoverage}=1" \
            --output-file "${COVERAGE_FILE}"
        echo -n "    Coverage: "
        lcov --summary "${COVERAGE_FILE}" 2>&1 | grep 'lines...' | grep -oP '\d+\.\d*%'
    fi
}


function report_coverage()
{
    if [ ${opt_coverage} == true ]; then
        local testReportDir=${OUT_DIR}/report-sunny
        mkdir -p ${testReportDir}
        genhtml --title "Boxes / Sunny-Day Tests" --branch-coverage --legend \
            --output-directory ${testReportDir} ${COVERAGE_FILE}
        echo -e "\nTest coverage report available at ${testReportDir}/index.html"
    fi
}


parse_arguments "$@"
check_prereqs
clear_gcda_traces

declare boxesBinary
boxesBinary=$(get_executable_name)

declare branchCoverage
branchCoverage=$(get_coverage_option)

echo "Performing sunny-day tests on all box designs"

mkdir -p ${TEST_DIR}
rm -rf ${TEST_DIR}/*.txt
declare expectedFile
declare actualFile

for design in $(list_designs)
do
    echo -n "Testing design '${design}' "
    success=1

    echo -n "... create "
    expectedFile="${EXPECTED_DIR}/${design}.create.txt"
    actualFile="${TEST_DIR}/${design}.create.actual.txt"
    if file_exists "${expectedFile}"; then
        run_boxes_create "${actualFile}"
        assert_result create "${expectedFile}" "${actualFile}"
    fi
    countExecuted=$((countExecuted + 1))

    echo -n "... mend "
    expectedFile="${EXPECTED_DIR}/${design}.mend.txt"
    actualFile="${TEST_DIR}/${design}.mend.actual.txt"
    if file_exists "${expectedFile}"; then
        run_boxes_mend "${actualFile}"
        assert_result mend "${expectedFile}" "${actualFile}"
    fi
    countExecuted=$((countExecuted + 1))

    echo -n "... remove "
    expectedFile="${INPUT_FILE}"
    actualFile="${TEST_DIR}/${design}.remove.actual.txt"
    run_boxes_remove "${actualFile}"
    assert_result remove "${expectedFile}" "${actualFile}"
    countExecuted=$((countExecuted + 1))

    print_success
done

echo "${countExecuted} tests executed, $((countExecuted - countFailed)) successful, ${countFailed} failed."
measure_coverage
report_coverage

exit ${result}
