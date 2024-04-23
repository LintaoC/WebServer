#!/bin/bash

CONFIG_FILE="../simple_config"
RESPONSE_PATH="../tests"
SERVER_BIN="../build/bin/server"

# Start the server in the background
$SERVER_BIN $CONFIG_FILE &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

# Allow server some time to start
sleep 1

# Perform a test request and capture the response
RESPONSE=$(echo -ne "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc localhost 8080)
CLEANED_RESPONSE=$(echo "$RESPONSE" | sed '/^Date:/d')

# Normalize and save the actual response
echo "$CLEANED_RESPONSE" | tr -d '\r \n' > test_response2

# Check the response
echo -n "Test 2   "
if diff --ignore-trailing-space "${RESPONSE_PATH}/expected_response2" "test_response2"; then
    echo "success"
else
    echo "failed"
fi

# Kill the server process and exit with the status of the diff command
kill $SERVER_PID
rm -f test_response2
exit $?