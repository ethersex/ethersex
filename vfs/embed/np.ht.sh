#!/bin/bash
if grep -q 'NAMED_PIN_INLINE_SUPPORT=y' .config; then
  bash core/portio/named_pin/np_web.sh core/portio/named_pin/config  | m4 named_pin/np_web.m4 -
else
  exit 1;
fi
