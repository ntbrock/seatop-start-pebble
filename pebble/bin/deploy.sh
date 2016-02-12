#!/bin/sh
. ./bin/pebble_time.ip

pebble clean && pebble build && pebble install --phone ${PEBBLE_TIME_IP}

