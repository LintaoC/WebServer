#!/bin/bash

# Run the first integration test
bash ../integration_tests/integration_test1.sh
result1=$?

# Run the second integration test
bash ../integration_tests/integration_test2.sh
result2=$?

# Exit with an error if any test failed
[ $result1 -eq 0 -a $result2 -eq 0 ]