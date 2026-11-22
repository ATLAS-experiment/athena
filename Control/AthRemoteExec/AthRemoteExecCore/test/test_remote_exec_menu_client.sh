#!/bin/bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Excersise server with various types of valid and invalid requests

portfile="${PWD}/remote_exec_menu_port.txt"
serverlog="${PWD}/remote_exec_menu_server.log"
clientlog="${PWD}/remote_exec_menu_client.log"
delay=300
rm -f "${portfile}"

test_remote_exec_run_server.py \
  --threads=4 --concurrent-events=4 --delay=${delay} \
  --port=0 --port-file="${portfile}" --evtMax=18 --loglevel DEBUG \
  >"${serverlog}" 2>&1 &
serverpid=$!

cleanup() {
  if kill -0 ${serverpid} 2>/dev/null; then
    echo "server still running, killing it"
    kill -TERM ${serverpid} 2>/dev/null
    sleep 2
    kill -KILL ${serverpid} 2>/dev/null
  fi
}
trap cleanup EXIT

test_remote_exec_menu_client "${portfile}" ${delay} >"${clientlog}" 2>&1
clientrc=$?
cat "${clientlog}"

serverrc=1
for _ in $(seq 1 60); do
  if ! kill -0 ${serverpid} 2>/dev/null; then
    wait ${serverpid}
    serverrc=$?
    break
  fi
  sleep 1
done

if [ ${clientrc} -ne 0 ]; then
  echo "FAIL: the client failed with ${clientrc}; server log:"
  tail -120 "${serverlog}"
  exit 1
fi

if [ ${serverrc} -ne 0 ]; then
  echo "FAIL: the server did not stop cleanly (${serverrc}); server log:"
  tail -120 "${serverlog}"
  exit 1
fi

if ! grep -q "Request loop finished: 18 request(s) accepted" "${serverlog}"; then
  echo "FAIL: the server did not report accepting 18 requests; server log:"
  tail -120 "${serverlog}"
  exit 1
fi

if [ "$(grep -c 'dispatched to slot 1' "${serverlog}")" -lt 1 ]; then
  echo "FAIL: the second slot was never used; server log:"
  tail -120 "${serverlog}"
  exit 1
fi

echo "test_remote_exec_menu_client: all checks passed"
exit 0
