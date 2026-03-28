#!/bin/bash
# Checks student files for prohibited patterns.
# Run before grading: ./check_student.sh

FAIL=0

echo "Checking student files for prohibited patterns..."
echo "=================================================="

# Check for includes of engine headers other than board.h
for f in heuristics.c heuristics.h evaluators/*.c evaluators/*.h; do
    [ -f "$f" ] || continue
    # Skip evaluators.h (it legitimately includes engine headers for the Bot type)
    [ "$(basename "$f")" = "evaluators.h" ] && continue
    # Look for #include lines referencing engine/ but not board.h
    matches=$(grep -n '#include.*engine/' "$f" | grep -v 'engine/board\.h')
    if [ -n "$matches" ]; then
        echo "VIOLATION in $f: prohibited engine #include"
        echo "$matches"
        FAIL=1
    fi
done

# Check for suspicious function names that suggest reimplemented search
# Strip comment-only lines before checking
for f in heuristics.c heuristics.h evaluators/*.c; do
    [ -f "$f" ] || continue
    for pattern in "get_legal_moves" "apply_move" "minimax" "alpha_beta" "alphabeta" "negamax" "transposition"; do
        matches=$(grep -n "$pattern" "$f" | grep -v '^\s*[0-9]*:\s*/[/*]' | grep -v '^\s*[0-9]*:\s*\*')
        if [ -n "$matches" ]; then
            echo "WARNING in $f: suspicious pattern '$pattern'"
            echo "$matches"
            FAIL=1
        fi
    done
done

echo "=================================================="
if [ $FAIL -eq 0 ]; then
    echo "PASSED: No violations found."
else
    echo "FAILED: Review the violations above."
fi

exit $FAIL
