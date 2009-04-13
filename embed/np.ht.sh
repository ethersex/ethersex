#!/bin/bash
[ -z "$NP_CONFIG" ] && {
  echo NP_CONFIG is unset, too bad   1>&2
  exit 1
}

if grep -q 'NAMED_PIN_INLINE_SUPPORT=y' .config; then
  bash core/portio/np_web.sh "$NP_CONFIG" | m4 core/portio/np_web.m4 -
else
  exit 1;
fi
