#!/bin/bash

# usage: run.sh [port] [password]

# runs ircserv with the first available port starting from 6667

if [ -z "$1" ]; then
    port=6667
else
    port=$1
fi
if [ -z "$2" ]; then
    password="horse"
else
    password=$2
fi

failcount=0
exit_code=1

C_LOGCOLOR='\033[1;34m'
C_RESET='\033[0m'

log () {
    echo -e "$C_LOGCOLOR[RUNLOG] $@$C_RESET"
}

log "Welcome to run.sh."
while true; do
    if [ -z "$(lsof -i :$port)" ]; then
        start_time=$(date +%s)
        ./ircserv $port $password
        exit_code=$?
        end_time=$(date +%s)
        elapsed_time=$((end_time-start_time))
        if [ $elapsed_time -le 1 ]; then
            failcount=$((failcount+1))
            if [ $failcount -gt 3 ]; then
                log "Failed to start ircserv $failcount times in a row."
                log "Exiting run.sh."
                exit $exit_code
            fi
            log "Failed to start ircserv. Retrying..."
        else
            break
        fi
    else
        log "Port $port is already in use. Trying next port..."
    fi
    port=$((port+1))
done

log "Successfully exited ircserv (survived longer than 1s)."
log "Exiting run.sh."
exit $exit_code
