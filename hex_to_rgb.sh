#!/bin/bash

# File containing hex codes
file="colors.txt"

# Loop through each line in the file
while IFS= read -r hex; do
    # Remove the leading "#" if present
    hex=${hex#"#"}

    # Extract RGB components and convert to decimal
    r=$((16#${hex:0:2}))
    g=$((16#${hex:2:2}))
    b=$((16#${hex:4:2}))

    # Print the result
    echo "$hex -> R: $r, G: $g, B: $b"
done < "$file"

