#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# Pravi — Native Unit Test Runner
# Runs all firmware test suites on the host machine (no hardware required).
# Usage: ./scripts/test.sh [suite]
#   suite  Optional. One of: moving_average | moisture_sensor | state_machine
#          Omit to run all suites.
# ---------------------------------------------------------------------------

set -euo pipefail

cd "$(dirname "$0")/.."   # always run from firmware/

PASS=0
FAIL=0
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

run_suite() {
    local suite="$1"
    echo ""
    echo -e "${YELLOW}▶ Running: test_${suite}${NC}"
    echo "──────────────────────────────────────"

    if "$PIO" test -e native --filter "test_${suite}" 2>&1; then
        echo -e "${GREEN}✔ test_${suite} passed${NC}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}✘ test_${suite} failed${NC}"
        FAIL=$((FAIL + 1))
    fi
}

echo "======================================"
echo "  Pravi Native Test Suite"
echo "======================================"

# Locate pio — check PATH first, then the pipx install location
PIO="pio"
if ! command -v pio &>/dev/null; then
    PIPX_PIO="$HOME/.local/bin/pio"
    if [[ -x "$PIPX_PIO" ]]; then
        PIO="$PIPX_PIO"
    else
        echo -e "${RED}Error: 'pio' not found. Install PlatformIO with:${NC}"
        echo "  pipx install platformio"
        echo "Then open a new terminal (or run: source ~/.zshrc)"
        exit 1
    fi
fi

SUITES=(moving_average moisture_sensor state_machine)

if [[ $# -ge 1 ]]; then
    # Run a single named suite
    run_suite "$1"
else
    # Run all suites
    for suite in "${SUITES[@]}"; do
        run_suite "$suite"
    done
fi

echo ""
echo "======================================"
printf "  Results: ${GREEN}%d passed${NC}  ${RED}%d failed${NC}\n" "$PASS" "$FAIL"
echo "======================================"

[[ $FAIL -eq 0 ]]   # exit 1 if any suite failed
