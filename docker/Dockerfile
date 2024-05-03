### Build/test container ###
# Define builder stage
FROM f004:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:jammy as deploy

# Copy server output binary to "."
# COPY --from=builder /usr/src/project/build/bin/example_server .
COPY --from=builder /usr/src/project/build/bin/server .
COPY --from=builder /usr/src/project/cloud_config .
COPY --from=builder /usr/src/project/files/* ./files/
COPY --from=builder /usr/src/project/files1/* ./files1/

# Expose some port(s)
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
# Update with real server name
ENTRYPOINT ["./server"]

# Use CMD to specify arguments to ENTRYPOINT
# Update with real server args
CMD ["../cloud_config"]
