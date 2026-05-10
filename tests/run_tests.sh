#!/usr/bin/env bash
#
# End-to-end test runner for binterpreter.
# Runs each .lox file under tests/, parses expected output from
# "// expect: <value>" comments, and diffs against actual stdout.
#
# Usage:
#   ./tests/run_tests.sh                            # run all tests
#   ./tests/run_tests.sh tests/closures/basic.lox   # run one test

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BINARY="${BINTERPRETER:-$PROJECT_DIR/out/binterpreter}"

if [[ ! -x "$BINARY" ]]; then
    echo "Binary not found at $BINARY — building..."
    make -C "$PROJECT_DIR" >/dev/null 2>&1
fi

passed=0
failed=0
errors=""

run_test() {
    local file="$1"
    local rel_path="${file#$PROJECT_DIR/}"

    local expected
    expected=$(grep '// expect:' "$file" | sed 's/.*\/\/ expect: //' || true)

    local actual
    actual=$("$BINARY" "$file" 2>/dev/null || true)

    if [[ "$actual" == "$expected" ]]; then
        printf "  \033[32mPASS\033[0m  %s\n" "$rel_path"
        passed=$((passed + 1))
    else
        printf "  \033[31mFAIL\033[0m  %s\n" "$rel_path"
        errors="${errors}--- ${rel_path} ---\nexpected:\n${expected}\nactual:\n${actual}\n\n"
        failed=$((failed + 1))
    fi
}

files=()
if [[ $# -gt 0 ]]; then
    files=("$@")
else
    while IFS= read -r f; do files+=("$f"); done < <(find "$SCRIPT_DIR" -name '*.lox' | sort)
fi

echo ""
echo "Running ${#files[@]} test(s)..."
echo ""

for file in "${files[@]}"; do
    run_test "$file"
done

echo ""
echo "Result: $passed passed, $failed failed"

if [[ $failed -gt 0 ]]; then
    echo ""
    printf -- "%b" "$errors"
    exit 1
fi
