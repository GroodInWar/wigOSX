#!/bin/sh

set -eu

failed=0

# check_forbidden_include DIRECTORY PATTERN DESCRIPTION
#
# Searches one source directory for includes that would violate the intended
# kernel layering rules. A match records failure but lets the script report
# every violation before exiting.
check_forbidden_include() {
  directory="$1"
  pattern="$2"
  description="$3"

  matches="$(grep -R -n -E "$pattern" "$directory" || true)"

  if [ -n "$matches" ]; then
    echo "Layer violation: $description"
    echo "$matches"
    echo
    failed=1
  fi
}

check_forbidden_include \
  "src/drivers" \
  '#include <kernel/core/(shell|input|console|log|panic|memory)\.h>' \
  "device drivers must not depend on high-level kernel services"

check_forbidden_include \
  "src/mm" \
  '#include <kernel/drivers/(vga|serial|keyboard|pit)\.h>' \
  "memory managers must not depend directly on device drivers"

check_forbidden_include \
  "src/arch/i386/interrupts" \
  '#include <kernel/drivers/(keyboard|pit)\.h>' \
  "architecture interrupt code must use the IRQ dispatcher"

check_forbidden_include \
  "src/kernel/shell.c" \
  '#include <kernel/drivers/' \
  "the shell must use kernel service interfaces instead of drivers"

check_forbidden_include \
  "src/kernel/input.c" \
  '(shell_handle_character|#include <kernel/core/shell\.h>)' \
  "the input IRQ adapter must queue characters instead of invoking the shell"

if [ "$failed" -ne 0 ]; then
  exit 1
fi

echo "Kernel layer checks passed."
