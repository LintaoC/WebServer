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
RESPONSE=$(echo -ne "POST /submit-form HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 27\r\n\r\nusername=johndoe&password=1234" | nc localhost 8080)
CLEANED_RESPONSE=$(echo "$RESPONSE" | sed '/^Date:/d')

# Normalize and save the actual response
echo "$CLEANED_RESPONSE" | tr -d '\r \n' > test_response1

# Check the response
echo -n "Test 1   "
if diff --ignore-trailing-space "${RESPONSE_PATH}/expected_response1" "test_response1"; then
    echo "success"
else
    echo "failed"
fi

# Kill the server process and exit with the status of the diff command
kill $SERVER_PID
rm -f test_response1
exit $?


