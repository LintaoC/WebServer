#!/bin/bash

CONFIG_FILE="../config"
SERVER_BIN="../build/bin/server"

# Start the server in the background
$SERVER_BIN $CONFIG_FILE &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

# Allow server some time to start
sleep 1

# Function to perform a test and check response time
perform_test() {
    local request="$1"
    local test_num="$2"

    start_time=$(date +%s%N | cut -b1-13)

    # Perform the test request
    RESPONSE=$(echo -ne "$request" | nc localhost 8080)

    end_time=$(date +%s%N | cut -b1-13)
    elapsed=$((end_time - start_time))

    # Check the response time
    echo -n "Test ${test_num}   "
    if [ $elapsed -lt 5000 ]; then
        echo "success (response time: ${elapsed}ms)"
    else
        echo "failed (response time: ${elapsed}ms)"
    fi
}

# First test case: /sleep
echo "Starting /sleep test"
{ echo -ne "GET /sleep HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" | nc localhost 8080; echo; } &
SLEEP_PID=$!
sleep 1  # Ensure /sleep request is in progress

# Second test case: immediate response
perform_test "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "1"

# Wait for sleep process to finish
wait $SLEEP_PID

# Kill the server process
kill $SERVER_PID

exit $?

rm -f server_log*
exit $?