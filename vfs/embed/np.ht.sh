#!/bin/bash
if grep -q 'NAMED_PIN_INLINE_SUPPORT=y' .config; then
  bash core/portio/np_web.sh core/portio/config  | m4 core/portio/np_web.m4 -
else
  exit 1;
fi
