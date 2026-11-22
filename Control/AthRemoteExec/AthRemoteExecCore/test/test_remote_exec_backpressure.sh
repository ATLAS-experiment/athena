#!/bin/bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Test server refusal when set limit is reached

portfile="${PWD}/remote_exec_backpressure_port.txt"
serverlog="${PWD}/remote_exec_backpressure_server.log"
clientlog="${PWD}/remote_exec_backpressure_client.log"
rm -f "${portfile}"

test_remote_exec_run_server.py \
  --threads=1 --concurrent-events=1 --delay=200 --queue-limit=1 \
  --port=0 --port-file="${portfile}" --evtMax=-1 \
  >"${serverlog}" 2>&1 &
serverpid=$!

cleanup() {
  kill -TERM ${serverpid} 2>/dev/null
  sleep 2
  kill -KILL ${serverpid} 2>/dev/null
}
trap cleanup EXIT

test_remote_exec_load_client "${portfile}" --requests 100 --concurrency 16 \
  >"${clientlog}" 2>&1
clientrc=$?
cat "${clientlog}"

if [ ${clientrc} -ne 0 ]; then
  echo "FAIL: the client reported failure (${clientrc}); refusals must not"
  echo "      count as faults. Server log:"
  tail -40 "${serverlog}"
  exit 1
fi

refused=$(awk '/^refused/ { print $2 }' "${clientlog}")
if [ -z "${refused}" ] || [ "${refused}" -lt 1 ]; then
  echo "FAIL: nothing was refused, so the queue limit did not bite."
  echo "      Either backpressure is gone or the client stopped recognising"
  echo "      RESOURCE_EXHAUSTED. Server log:"
  tail -40 "${serverlog}"
  exit 1
fi

if grep -q "FAILURES" "${clientlog}"; then
  echo "FAIL: some requests failed outright, not merely refused:"
  grep "FAILURES" "${clientlog}"
  tail -40 "${serverlog}"
  exit 1
fi

if ! grep -q "request queue full" "${serverlog}"; then
  echo "FAIL: the server never reported a full queue; server log:"
  tail -40 "${serverlog}"
  exit 1
fi

echo "test_remote_exec_backpressure: ${refused} request(s) refused, none failed"
exit 0
