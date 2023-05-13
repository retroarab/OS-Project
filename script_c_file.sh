#!/bin/bash

# Define regular expressions to match error and warning messages
regexError='error:'
regexWarning='warning:'

# Check if file exists and is not a symbolic link
if test -f "$1" && ! test -h "$1"; then
    echo "File exists" >> log.txt
else
    echo "File does not exist or is a symbolic link"
    exit 1
fi

# Compile the file with gcc and redirect stderr to a log file
out="log.txt"
gcc -Wall "$1" 2> "$out"

# Count the number of error and warning messages in the log file
errorCount=$(grep -c "$regexError" "$out")
warningCount=$(grep -c "$regexWarning" "$out")

# Print the number of error and warning messages
echo "$errorCount $warningCount"

