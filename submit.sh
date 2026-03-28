#!/bin/bash
# Package your submission into a zip file.
# Run: ./submit.sh

NAME=${1:-submission}
ZIP="${NAME}.zip"

echo "Packaging submission..."

# Check that required files exist
if [ ! -f heuristics.c ] || [ ! -f heuristics.h ]; then
    echo "ERROR: heuristics.c or heuristics.h not found. Run from the project root."
    exit 1
fi

if [ ! -d evaluators ]; then
    echo "ERROR: evaluators/ folder not found. Run from the project root."
    exit 1
fi

# Create zip with only the submission files
rm -f "$ZIP"
zip -r "$ZIP" heuristics.c heuristics.h evaluators/*.c evaluators/evaluators.h

echo ""
echo "Created: $ZIP"
echo "Send this file along with your report PDF."
