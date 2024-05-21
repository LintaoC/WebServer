#!/bin/bash

CONFIG_FILE="../config"
SERVER_BIN="../build/bin/server"

# Start the server in the background
$SERVER_BIN $CONFIG_FILE &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

# Allow server some time to start
sleep 1

# Function to perform a test and compare responses
check_response() {
    local response="$1"
    local expected="$2"
    local test_num="$3"

    # Clean and normalize the response
    cleaned_response=$(echo "$response" | sed '/^Date:/d' | tr -d '\r\n ')

    # Clean and normalize the expected response
    cleaned_expected=$(echo "$expected" | sed '/^Date:/d' | tr -d '\r\n ')

    # Check if the response matches the expected value
    if [[ "$cleaned_response" == "$cleaned_expected" ]]; then
        echo "Test ${test_num} success"
    else
        echo "Test ${test_num} failed"
    fi
}

# 1. Create an entity
echo "Creating entity..."
create_response=$(curl -s -X POST http://localhost:8080/api/example_entity -H "Content-Type: application/json" -d '{"make": "Toyota", "model": "Corolla", "year": 2020}')
entity_id=$(echo $create_response | grep -oP '(?<="id": ")[^"]*')
expected_create_response='{"id":"'"$entity_id"'"}'
check_response "$create_response" "$expected_create_response" "1"

# 2. Retrieve the entity
echo "Retrieving entity..."
get_response=$(curl -s -X GET http://localhost:8080/api/example_entity/$entity_id)
expected_get_response='{"make": "Toyota", "model": "Corolla", "year": 2020}'
check_response "$get_response" "$expected_get_response" "2"

# 3. Update the entity
echo "Updating entity..."
update_response=$(curl -s -X PUT http://localhost:8080/api/example_entity/$entity_id -H "Content-Type: application/json" -d '{"make": "Toyota", "model": "Corolla", "year": 2021}')
expected_update_response="Entity updated successfully"
check_response "$update_response" "$expected_update_response" "3"

# 4. Retrieve the updated entity
echo "Retrieving updated entity..."
get_updated_response=$(curl -s -X GET http://localhost:8080/api/example_entity/$entity_id)
expected_get_updated_response='{"make": "Toyota", "model": "Corolla", "year": 2021}'
check_response "$get_updated_response" "$expected_get_updated_response" "4"

# 5. Delete the entity
echo "Deleting entity..."
delete_response=$(curl -s -X DELETE http://localhost:8080/api/example_entity/$entity_id)
expected_delete_response="Entity deleted successfully"
check_response "$delete_response" "$expected_delete_response" "5"

# 6. Verify deletion
echo "Verifying deletion..."
verify_delete_response=$(curl -s -X GET http://localhost:8080/api/example_entity/$entity_id)
expected_verify_delete_response="Entity not found"
check_response "$verify_delete_response" "$expected_verify_delete_response" "6"

# Kill the server process
kill $SERVER_PID

# Cleanup
rm -rf ../database/example_entity
rm -f test_response1 test_response2 test_response3 test_response4 test_response5 test_response6
rm -f cleaned_response1 cleaned_response2 cleaned_response3 cleaned_response4 cleaned_response5 cleaned_response6
rm -f server_log*
exit $?
