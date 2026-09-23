#!/bin/bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Athena to Athena: an ordinary client job whose SumAlg happens to run in
# another process.
#
# What makes this test worth having, over and above the .proto-only client, is
# that both ends are built from the same declarations -- the same two adapter
# algorithms, the same schema name, the same key strings -- so a change that
# breaks the agreement between them cannot pass here. And it checks arithmetic
# rather than status codes: the client knows what the answer should be, and a
# reply that decoded but arrived empty is a clean StatusCode away from looking
# like success.
#
# Ten events, each sending different numbers, so a server answering from a stale
# slot is caught by the numbers rather than by a crash.

portfile="${PWD}/remote_exec_athena_client_port.txt"
serverlog="${PWD}/remote_exec_athena_client_server.log"
clientlog="${PWD}/remote_exec_athena_client_client.log"
events=10
rm -f "${portfile}"

python -m AthExRemoteExec.RemoteExecHiveLoopConfig \
    --threads=2 --concurrent-events=2 \
    --port=0 --port-file="${portfile}" --evtMax=${events} \
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

python -m AthExRemoteExec.RemoteExecDemoClientConfig \
    --target="${portfile}" --evtMax=${events} \
    > "${clientlog}" 2>&1
clientrc=$?

if [ ${clientrc} -ne 0 ]; then
    echo "FAIL: the client job exited with ${clientrc}"
    tail -80 "${clientlog}"
    echo "--- server ---"
    tail -40 "${serverlog}"
    exit 1
fi

# The count, said out loud by DemoCheckAlg. A job that ran no events also fails
# no checks, which is the way this test would otherwise pass by accident.
if ! grep -q "Checked ${events} reply/replies" "${clientlog}"; then
    echo "FAIL: the client did not check ${events} replies:"
    grep -i "checked\|ERROR" "${clientlog}" | tail -20
    exit 1
fi

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

echo "test_remote_exec_athena_client: all checks passed"
exit 0
