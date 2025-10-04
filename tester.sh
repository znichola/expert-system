#!/bin/bash

# For each file in tests/shouldNotWork: run ./expert-system with that file as input
# If ./expert-system returns 0, print "Test failed: <filename>"
# If ./expert-system returns non-zero, print "Test passed: <filename>"
# At the end, print "All tests passed" if all tests passed, otherwise print the number of tests that failed
set -e
FAILED=0
PASSED=0
TOTAL=0

for file in tests/shouldNotWork/*.txt; do
    echo TESTING FILE $file
    TOTAL=$((TOTAL + 1))
    ./expert-system "$file" && {
        # add color red to the output
        echo -e "\e[31mTest failed (should not work): $file\e[0m"
        FAILED=$((FAILED + 1))
    } || {
        echo -e "\e[32mTest passed (correctly did not work): $file\e[0m"
        PASSED=$((PASSED + 1))
    }
done

echo -e "\n\n=========================================================================="
echo -e "Total tests in tests/shouldNotWork: $TOTAL, Passed: $PASSED, Failed: $FAILED"
echo -e "==========================================================================\n\n"

sleep 1
FAILED=0
PASSED=0
TOTAL=0

for file in tests/shouldWork/*.txt; do
    TOTAL=$((TOTAL + 1))
    # check the output of ./expert-system "$file" against the expected output in tests/shouldWork/expected/<filename>.out
    expected_file="tests/shouldWork/expected/$(basename "$file" .txt).out"
    if ./expert-system "$file" > output.tmp; then
        if diff -q output.tmp "$expected_file" > /dev/null; then
            echo -e "\e[32mTest passed (should work): $file\e[0m"
            PASSED=$((PASSED + 1))
        else
            echo -e "\e[31mTest failed (output mismatch): $file\e[0m"
            echo "Expected output:"
            # cat "$expected_file"
            echo "Actual output:"
            # cat output.tmp
            diff -u "$expected_file" output.tmp || true
            FAILED=$((FAILED + 1))
        fi
    else
        echo -e "\e[31mTest failed (should work but did not): $file\e[0m"
        FAILED=$((FAILED + 1))
    fi
    rm -f output.tmp
done

echo -e "\n\n=========================================================================="
echo -e "Total tests in tests/shouldWork: $TOTAL, Passed: $PASSED, Failed: $FAILED"
echo -e "==========================================================================\n\n"

