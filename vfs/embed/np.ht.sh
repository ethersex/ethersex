#!/bin/bash
if grep -q 'NAMED_PIN_SUPPORT=y' .config; then
  bash named_pin/np_web.sh named_pin/config  | m4 named_pin/np_web.m4 -
else
  exit 1;
fi
