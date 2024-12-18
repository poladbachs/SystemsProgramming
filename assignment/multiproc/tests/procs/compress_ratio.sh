#!/bin/sh

given_file="$1"

# Detect OS and set the correct stat command
if test "$(uname)" = 'Linux'; then
    initial_size="$(stat -c "%s" "$given_file")"
else
    initial_size="$(stat -f "%z" "$given_file")"
fi 

# Compress the file to a temporary gzip
compressed_file="$(mktemp)"
gzip -c "$given_file" > "$compressed_file"

# Get the compressed size
if test "$(uname)" = 'Linux'; then
    compressed_size="$(stat -c "%s" "$compressed_file")"
else
    compressed_size="$(stat -f "%z" "$compressed_file")"
fi

# Clean up temporary compressed file
rm -f "$compressed_file"

# Calculate the compression ratio
compression_ratio=$((initial_size / compressed_size))

# Output the compression ratio
echo "$compression_ratio"