set -u
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR" || exit 1
COMPILER="bin/mycompiler"

if [ ! -x "$COMPILER" ]; then
    echo "Compiler not found at $COMPILER. Run 'make' first."
    exit 1
fi

pass=0
fail=0

for f in tests/valid/*.mc tests/invalid/*.mc; do
    base="${f%.mc}"
    expected="${base}.expected_output.txt"
    if [ ! -f "$expected" ]; then
        continue
    fi
    actual="$("$COMPILER" "$f" 2>&1)"
    expected_content="$(cat "$expected")"

    if [ "$actual" == "$expected_content" ]; then
        echo "PASS: $(basename "$f")"
        pass=$((pass+1))
    else
        echo "FAIL: $(basename "$f")"
        echo "  --- expected ---"
        diff <(echo "$expected_content") <(echo "$actual") | sed 's/^/  /'
        fail=$((fail+1))
    fi
done

echo
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
