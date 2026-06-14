#!/bin/bash

# A script to batch-process RCPSP files using the Python conversion script.

# --- 1. Check for the correct number of arguments ---
if [ "$#" -ne 2 ]; then
    echo "❌ Incorrect number of arguments."
    echo "Usage: $0 <input_directory> <output_directory>"
    exit 1
fi

# --- 2. Assign arguments to variables for clarity ---
INPUT_DIR="$1"
OUTPUT_DIR="$2"
PYTHON_SCRIPT="rangen2sop.py" # The name of your Python script

# --- 3. Validate the input directory ---
if [ ! -d "$INPUT_DIR" ]; then
    echo "❌ Error: Input directory '$INPUT_DIR' does not exist."
    exit 1
fi

# --- 4. Create the output directory if it doesn't exist ---
# The '-p' flag ensures no error is thrown if the directory already exists.
mkdir -p "$OUTPUT_DIR"
echo "✅ Output will be saved in '$OUTPUT_DIR'"

# --- 5. Loop through files and process them ---
echo "🚀 Starting conversion..."

for input_file in "$INPUT_DIR"/*; do
    # Check if the item is a file (and not a directory)
    if [ -f "$input_file" ]; then
        # Extract just the filename from the full path
        filename=$(basename "$input_file")
        
        # Define the full path for the output file
        output_file="$OUTPUT_DIR/$filename"

        echo "   - Processing '$filename'..."
        
        # Run the Python script
        python3 "$PYTHON_SCRIPT" "$input_file" "$output_file"
    fi
done

echo "🎉 Batch processing complete!"