#!/usr/bin/env bash

# Server process name
SERVER_NAME="server"

# Find process ID
PID=$(pgrep -x "$SERVER_NAME")

if [ -z "$PID" ]; then
  echo "Server is not running."
  exit 1
fi

echo "Sending termination signal to server process (PID: $PID)..."
kill -TERM "$PID"

# Wait up to 5 seconds for shutdown
for i in {1..5}; do
  sleep 1
  if ! kill -0 "$PID" 2>/dev/null; then
    echo "Server shutdown complete."
    exit 0
  fi
done

echo "Server did not shut down gracefully. Attempting force kill..."
kill -KILL "$PID"
exit 1