#! /usr/bin/env bash

#=========================================================================#
#                               bashtest.sh                               #
#                                                                         #
# Author: nic                                                             #
# Date: 2016-Dec-07                                                       #
#=========================================================================#

set -e
set -u

function usage(){
    grep "^#.*#$" $0
}

while getopts "h" opt; do
    case $opt in
        h)
            usage
            exit 0
            ;;
        \?)
            usage
            exit 1
            ;;
    esac
done
shift $(($OPTIND -1))

echo $@
