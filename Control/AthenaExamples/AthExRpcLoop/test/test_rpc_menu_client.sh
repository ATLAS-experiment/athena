#!/bin/bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Start the MT menu server on an OS-chosen port and drive it with the
# .proto-only client, which fires eight requests at once and then fourteen more
# one at a time. Twenty-two in total, so the server can be told to serve
# twenty-two and stop by itself -- no signal handling needed to end the test
# cleanly.
#
# The count has to match exactly, and that is a nuisance worth keeping: it is
# how this test noticed that requests still queued when the loop reaches its
# quota are dropped rather than answered. Adding a request to the client
# without adding one here fails loudly instead of quietly timing out.

portfile="${PWD}/rpc_menu_port.txt"
serverlog="${PWD}/rpc_menu_server.log"
clientlog="${PWD}/rpc_menu_client.log"
delay=300
rm -f "${portfile}"

python -m AthExRpcLoop.RpcHiveLoopConfig \
    --threads=4 --concurrent-events=4 --delay=${delay} \
    --port=0 --port-file="${portfile}" --evtMax=22 \
    > "${serverlog}" 2>&1 &
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

rpc_menu_client "${portfile}" ${delay} > "${clientlog}" 2>&1
clientrc=$?
cat "${clientlog}"

# The server must come down on its own once it has served its quota.
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

if ! grep -q "Request loop finished: 22 request(s) accepted" "${serverlog}"; then
    echo "FAIL: the server did not report accepting twenty-two requests; server log:"
    tail -120 "${serverlog}"
    exit 1
fi

# The client already proved overlap by wall clock; this proves it from the
# server's own account of which slot each request went to.
if [ "$(grep -c 'dispatched to slot 1' "${serverlog}")" -lt 1 ]; then
    echo "FAIL: the second slot was never used; server log:"
    tail -120 "${serverlog}"
    exit 1
fi

echo "test_rpc_menu_client: all checks passed"
exit 0
