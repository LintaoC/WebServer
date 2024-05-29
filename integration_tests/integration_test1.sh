#!/bin/bash

CONFIG_FILE="../config"
RESPONSE_PATH="../integration_tests/test_responses"
SERVER_BIN="../build/bin/server"

# List the contents of the project directory for debugging
echo "Listing contents of /usr/src/project/:"
ls -l /usr/src/project/

# Build the server executable
echo "Building the server executable..."
cd /usr/src/project
mkdir -p build
cd build
cmake ..
make

# Check if the server executable exists
if [ ! -f "$SERVER_BIN" ]; then
    echo "Server executable not found at $SERVER_BIN!"
    echo "Current directory: $(pwd)"
    echo "Contents of build/bin:"
    ls -l "$(dirname "$SERVER_BIN")"
    exit 1
fi

# Function to start the server
start_server() {
    "$SERVER_BIN" "$CONFIG_FILE" &
    SERVER_PID=$!
    echo "Server PID: $SERVER_PID"
    sleep 1

    # Check if server is running by trying to connect to it
    for i in {1..5}; do
        if nc -z localhost 8080; then
            echo "Server has started successfully"
            return 0
        else
            echo "Waiting for server to start..."
            sleep 1
        fi
    done

    echo "Server failed to start"
    kill $SERVER_PID
    exit 1
}

# Function to stop the server
stop_server() {
    if kill -0 $SERVER_PID > /dev/null 2>&1; then
        kill $SERVER_PID
        echo "Server stopped (PID: $SERVER_PID)"
    else
        echo "Server process not found (PID: $SERVER_PID)"
    fi
}

# Function to perform a test and compare responses
perform_test() {
    local request="$1"
    local expected_file="$2"
    local test_num="$3"

    # Perform the test request and capture the response
    RESPONSE=$(echo -ne "$request" | nc localhost 8080)
    CLEANED_RESPONSE=$(echo "$RESPONSE" | sed '/^Date:/d')

    # Print the actual response for debugging
    echo "Actual Response for Test $test_num:"
    echo "$RESPONSE"

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

# Start the server
start_server

# First test case
perform_test "GET /echo/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "${RESPONSE_PATH}/expected_echo_response1" "1"

# Second test case
perform_test "GET /echo1/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "${RESPONSE_PATH}/expected_echo_response2" "2"

# Third test case
perform_test "GET /none/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "${RESPONSE_PATH}/expected_404_response" "3"

# Stop the server
stop_server

# Cleanup
rm -f test_response1 test_response2 test_response3 cleaned_response1 cleaned_response2 cleaned_response3
rm -f server_log*

exit $?


