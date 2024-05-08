#!/bin/bash

CONFIG_FILE="../simple_config"
RESPONSE_PATH="../integration_tests/test_responses"
SERVER_BIN="../build/bin/server"

# Start the server in the background
$SERVER_BIN $CONFIG_FILE &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

# Allow server some time to start
sleep 1

# Function to perform a test and compare responses
perform_test() {
    local request="$1"
    local expected_file="$2"
    local test_num="$3"

    # Perform the test request and capture the response
    RESPONSE=$(echo -ne "$request" | nc localhost 8080)
    CLEANED_RESPONSE=$(echo "$RESPONSE" | sed '/^Date:/d')

    # Normalize and save the actual response
    echo "$CLEANED_RESPONSE" | tr -d '\r\n ' > "test_response${test_num}"

    # Clean and save the expected response
    output_file="cleaned_response${test_num}"
    cat "$expected_file" | sed '/^Date:/d' | tr -d '\r\n ' > "$output_file"

    # Check the response
    echo -n "Echo Test ${test_num}   "
    if diff --ignore-trailing-space "$output_file" "test_response${test_num}"; then
        echo "success"
    else
        echo "failed"
    fi
}

# First test case
perform_test "GET /echo/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "${RESPONSE_PATH}/expected_echo_response1" "1"

# Second test case
perform_test "GET /echo1/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "${RESPONSE_PATH}/expected_echo_response2" "2"

# Kill the server process
kill $SERVER_PID

# Cleanup
rm -f test_response1 test_response2 cleaned_response1 cleaned_response2
rm -f server_log*
exit $?

