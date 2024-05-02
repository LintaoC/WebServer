# Start Docker:
Command:`tools/env/start.sh -u ${USER}` 

# Create and go build Directory under f004 and start compiling
command:
`mkdir build` 
`cd build` 
`cmake ..`
`make` 

# Start a server under the build directory
command: `bin/server ../simple_config` 

# Kill a server
command: 
`sudo lsof -i :8080` 
`sudo kill -9 PID` 

# After start a server listen on port 8080:
# Sending echo request to the server:
command:
`echo -ne "GET /echo1/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" | nc localhost 8080`

# Sending static file request to the server:
command:
`echo -ne "GET /static/a.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" | nc localhost 8080`

# Generate test coverage report:
# build and go to build_coverage directory under f004
# start compiling and generate test coverage report
commnad:
`mkdir build_coverage`
`cd build_coverage`
`cmake -DCMAKE_BUILD_TYPE=Coverage ..`
`make coverage`

# Building container in google cloud
command:
`gcloud builds submit --config docker/cloudbuild.yaml .`