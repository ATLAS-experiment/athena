#!/bin/bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Does the server refuse rather than queue without bound, and does the refusal
# reach the client in a form it can act on?
#
# Both halves matter, and only the first was ever checked. The queue limit was
# verified by hand; the channel it answers on was not, and for a while it was
# wrong: the handler filled in a reply body AND returned a non-OK gRPC status,
# but gRPC sends only one of those (grpcpp/support/method_handler.h calls
# SendMessagePtr only `if (status.ok())`), so the body was discarded and a
# client could not tell "busy" from "the call broke". Refusals now travel as
# RESOURCE_EXHAUSTED, which is also the form an intermediary can act on -- Envoy
# reads grpc-status from the trailers, and will never parse an ExecuteReply.
#
# The setup is deliberately impossible to serve: one slot, one thread, a 200 ms
# fragment, a queue that holds one, and sixteen concurrent clients. Most
# requests must be refused; none may fail.

portfile="${PWD}/rpc_backpressure_port.txt"
serverlog="${PWD}/rpc_backpressure_server.log"
clientlog="${PWD}/rpc_backpressure_client.log"
rm -f "${portfile}"

python -m AthExRpcLoop.RpcHiveLoopConfig \
    --threads=1 --concurrent-events=1 --delay=200 --queue-limit=1 \
    --port=0 --port-file="${portfile}" --evtMax=-1 \
    > "${serverlog}" 2>&1 &
serverpid=$!

cleanup() {
    kill -TERM ${serverpid} 2>/dev/null
    sleep 2
    kill -KILL ${serverpid} 2>/dev/null
}
trap cleanup EXIT

rpc_load_client "${portfile}" --requests 100 --concurrency 16 \
    > "${clientlog}" 2>&1
clientrc=$?
cat "${clientlog}"

# A run that is mostly refusals is a success here, so the client must not treat
# them as faults -- that is half of what this test pins down.
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

# And the server's own account of it, so a client-side misreading cannot make
# this pass on its own.
if ! grep -q "request queue full" "${serverlog}"; then
    echo "FAIL: the server never reported a full queue; server log:"
    tail -40 "${serverlog}"
    exit 1
fi

echo "test_rpc_backpressure: ${refused} request(s) refused, none failed"
exit 0
