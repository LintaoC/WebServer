#!/bin/bash

# Run the first integration test
bash ../integration_tests/integration_test1.sh
result1=$?

# Run the second integration test
bash ../integration_tests/integration_test2.sh
result2=$?

# Run the third integration test
bash ../integration_tests/integration_test3.sh
result3=$?

bash ../integration_tests/integration_test4.sh
result4=$?

# Exit with an error if any test failed
if [ $result1 -eq 0 ] && [ $result2 -eq 0 ] && [ $result3 -eq 0 ] && [ $result4 -eq 0 ]; then
    exit 0
else
    exit 1
fi