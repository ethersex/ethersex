#!/bin/bash
if grep -q 'VFS_CONFIG_INLINE_SUPPORT=y' .config; then
  grep '^[^#]' .config
  if [ -d .git ]; then
    echo "# git id: `git show | sed -n 1p`"
  fi
  echo "# compile date: `date`"
  echo "# gcc version: `avr-gcc -v 2>&1 | grep version`"
  echo "# ld version: `avr-ld -v`"
else
  exit 1;
fi
