#!/bin/sh

given_file="$1"

# Ensure the file exists
if [ ! -f "$given_file" ]; then
    echo "Error: File not found or not a regular file: $given_file" >&2
    exit 1
fi

# Detect OS and set the correct stat command
OS="$(uname)"
if [ "$OS" = "Linux" ]; then
    stat_cmd='stat -c "%s"'
elif [ "$OS" = "Darwin" ]; then
    stat_cmd='stat -f "%z"'
else
    echo "Unsupported OS: $OS" >&2
    exit 1
fi

# Get the initial size
initial_size=$(eval "$stat_cmd \"$given_file\"")
initial_size="$(echo "$initial_size" | tr -d '[:space:]')"

# Validate the initial size
if ! [ "$initial_size" -gt 0 ] 2>/dev/null; then
    echo "Error: Invalid initial size for $given_file: $initial_size" >&2
    exit 1
fi

# Compress the file to a temporary gzip
compressed_file="$(mktemp)"
gzip -c "$given_file" > "$compressed_file"

# Get the compressed size
compressed_size=$(eval "$stat_cmd \"$compressed_file\"")
compressed_size="$(echo "$compressed_size" | tr -d '[:space:]')"

# Validate the compressed size
if ! [ "$compressed_size" -gt 0 ] 2>/dev/null; then
    echo "Error: Invalid compressed size for $compressed_file: $compressed_size" >&2
    rm -f "$compressed_file"
    exit 1
fi

# Calculate the compression ratio
compression_ratio=$((initial_size / compressed_size))

# Clean up temporary compressed file
rm -f "$compressed_file"

# Output the compression ratio
echo "$compression_ratio"