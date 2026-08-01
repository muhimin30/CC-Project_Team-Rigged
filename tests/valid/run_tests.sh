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