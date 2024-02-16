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
# ____________________________________________________________________________________________________________________
#
# Calculate and report the unit test coverage. Invoked by Makefile.
# ____________________________________________________________________________________________________________________


# Expects environment variables OUT_DIR and SRC_DIR.
declare -r tcBaseName=unittest
declare -r testResultsDir=${OUT_DIR}/test-results/${tcBaseName}
declare -r testReportDir=${OUT_DIR}/report-utest

declare branchCoverage=lcov_branch_coverage
if [[ $(uname) == "Darwin" ]]; then
    branchCoverage=branch_coverage
fi

if test -n "$(find "${OUT_DIR}" -maxdepth 1 -name '*.gcda' -print -quit)" \
    && test -n "$(find "${OUT_DIR}" -maxdepth 1 -name '*.gcno' -print -quit)" \
    && test -n "$(lcov --version)"
then
    mkdir -p "${testResultsDir}"
    mkdir -p "${testReportDir}"
    cp "${OUT_DIR}"/*.gc* "${testResultsDir}"
    lcov --capture --directory "${testResultsDir}" --base-directory "${SRC_DIR}" --test-name ${tcBaseName} --quiet \
        --exclude '*/lex.yy.c' --exclude '*/parser.c' --rc "${branchCoverage}=1" \
        --output-file "${testResultsDir}/coverage.info"
    echo -n "[ Coverage ] "
    genhtml --title "Boxes / Unit Tests" --branch-coverage --legend --output-directory "${testReportDir}" \
        "${testResultsDir}/coverage.info" | grep 'lines...' | grep -oP '\d+\.\d*%'
    echo "[  Report  ] Unit test coverage report available at ${testReportDir}/index.html"
fi
