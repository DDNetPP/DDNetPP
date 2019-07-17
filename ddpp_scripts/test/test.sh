#!/bin/bash

Reset='\033[0m'
Red='\033[0;31m'
Green='\033[0;32m'
Yellow='\033[0;33m'

function err() {
  echo -e "[${Red}error${Reset}] $1"
}

function log() {
  echo -e "[${Yellow}*${Reset}] $1"
}

function wrn() {
  echo -e "[${Yellow}!${Reset}] $1"
}

function suc() {
  echo -e "[${Green}+${Reset}] $1"
}

run="python3 ../../cat_records.py"
test_dir=$(pwd)

fail=0
errors=0
passed=0

function assert_equals() {
    fail=0
    d=$(diff <($1) <($2))
    if [ "$d" ]
    then
        err "assert does not equal"
        echo "diff: $d"
        errors=$((errors + 1))
        fail=1
    else
        echo -e "[${Green}+${Reset}]"
        passed=$((passed + 1))
    fi
}

function run_test() {
    name=$1
    cd $test_dir
    cd $name
    # echo "running test $name ..."
    assert_equals "$run records1.txt records2.txt" "cat result.txt"
    if [ "$fail" == "1" ]
    then
        err "test '$name' failed"
    fi
}

function run_all() {
    for t in `ls -d */`
    do
        run_test "$t"
    done
}

run_all

echo "---------------------"
echo "tests run:     $((errors + passed))"
echo "tests failed:  $errors"
echo "tests passsed: $passed"

if [ $errors -gt 0 ]
then
    err "test run failed"
else
    suc "all tests passed"
fi
