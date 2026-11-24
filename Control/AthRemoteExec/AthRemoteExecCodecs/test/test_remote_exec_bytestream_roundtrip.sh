#!/bin/bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run server and client Athena jobs; the client feeds real Run-3 RAW events
# to the server and verifies the digests that come back.

portfile="${PWD}/remote_exec_bytestream_roundtrip_port.txt"
serverlog="${PWD}/remote_exec_bytestream_roundtrip_server.log"
clientlog="${PWD}/remote_exec_bytestream_roundtrip_client.log"
events=10
rm -f "${portfile}"

rawfile="$(python -c 'from AthenaConfiguration.TestDefaults import defaultTestFiles as d; print(d.RAW_RUN3[0])')"

test_remote_exec_run_codecs_server.py \
  --threads=2 --concurrent-events=2 \
  --port=0 --port-file="${portfile}" --evtMax=${events} \
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

test_remote_exec_run_codecs_client.py \
  --filesInput="${rawfile}" --evtMax=${events} \
  --target="${portfile}" --client-cfg ByteStreamClientCfg \
  >"${clientlog}" 2>&1
clientrc=$?

if [ ${clientrc} -ne 0 ]; then
  echo "FAIL: the client job exited with ${clientrc}"
  tail -80 "${clientlog}"
  echo "--- server ---"
  tail -40 "${serverlog}"
  exit 1
fi

# Check if all events were executed
if ! grep -q "Checked ${events} reply/replies" "${clientlog}"; then
  echo "FAIL: the client did not check ${events} replies:"
  grep -i "checked\|ERROR" "${clientlog}" | tail -20
  exit 1
fi

# The server must come down on its own after serving requested number of events
serverrc=1
for _ in $(seq 1 60); do
  if ! kill -0 ${serverpid} 2>/dev/null; then
    wait ${serverpid}
    serverrc=$?
    break
  fi
  sleep 1
done

if [ ${serverrc} -ne 0 ]; then
  echo "FAIL: the server did not stop cleanly (${serverrc}); server log:"
  tail -80 "${serverlog}"
  exit 1
fi

if ! grep -q "Request loop finished: ${events} request(s) accepted, ${events} event(s) run" "${serverlog}"; then
  echo "FAIL: the server did not run ${events} events; server log:"
  tail -40 "${serverlog}"
  exit 1
fi

# Properly check logs from server and client
for log in "${clientlog}" "${serverlog}"; do
  if ! ATLAS_CTEST_TESTNAME="$(basename "${log}" .log)" \
       ATLAS_CTEST_TESTSTATUS=0 noerror.sh; then
    exit 1
  fi
done

echo "test_remote_exec_bytestream_roundtrip: all checks passed"
exit 0
