#!/bin/sh


given_file="$1"

if test "$(uname)" = 'Linux'; then
    initial_size="$(stat -c "%s" "$given_file")"
else
    initial_size="$(stat -f "%z" "$given_file")"
fi 

compressed_file="$(mktemp)"
gzip -c "$given_file" > "$compressed_file"

if test "$(uname)" = 'Linux'; then
    compressed_size="$(stat -c "%s" "$compressed_file")"
else
    compressed_size="$(stat -f "%z" "$compressed_file")"
fi

rm -f "$compressed_file"

expr "$initial_size" / "$compressed_size"
