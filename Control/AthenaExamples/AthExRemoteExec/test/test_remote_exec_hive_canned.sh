#!/bin/bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Serve a fixed list of requests through the scheduler with no client process.
# Everything except the socket is exercised: slots, gates, control flow,
# replies, and every error path -- so a failure here is never a networking
# problem.
#
# Canned requests carry no payloads, and cannot: a payload's bytes are its
# fragment's own schema, and the loop manager has no way to build one without
# knowing that schema, which is exactly what it must not know. So the list below
# is sequence names, and what it covers is the control flow around a payload
# rather than the payload. RemoteExecSeqPing takes no inputs and is the success case;
# RemoteExecSeqSum takes one and is therefore the "client sent nothing usable" case;
# RemoteExecSeqFail fails; NoSuchSequence does not exist. The payload paths are covered
# by test_remote_exec_menu_client.sh and test_remote_exec_athena_client.sh, which have clients.

set -o pipefail

log=remote_exec_hive_canned.log

python -m AthExRemoteExec.RemoteExecHiveLoopConfig \
    --threads=2 --concurrent-events=2 --delay=200 \
    --test-requests='RemoteExecSeqPing;RemoteExecSeqChain;RemoteExecSeqPing;RemoteExecSeqFail;NoSuchSequence;RemoteExecSeqSum;RemoteExecSeqPing' \
    > "${log}" 2>&1
rc=$?

if [ ${rc} -ne 0 ]; then
    echo "FAIL: the job exited with ${rc}"
    tail -50 "${log}"
    exit 1
fi

failures=0
expect() {
    local what="$1"
    local pattern="$2"
    local count="$3"
    local seen
    seen=$(grep -c -F -- "${pattern}" "${log}")
    if [ "${seen}" != "${count}" ]; then
        echo "FAIL: ${what} (expected ${count} line(s) matching '${pattern}', saw ${seen})"
        failures=$((failures + 1))
    else
        echo "ok: ${what}"
    fi
}

# Seven requests taken off the queue; only the four that could possibly run
# were given a slot. Both halves matter: a request refused up front must still
# be counted as handled, and must not have cost an event.
expect "only the servable requests reached the scheduler" \
    "Request loop finished: 7 request(s) accepted, 4 event(s) run" 1

expect "an unknown sequence is refused without a slot" \
    "no sequence named 'NoSuchSequence'" 1
expect "a request with no payload for a declared input is refused" \
    "sequence 'RemoteExecSeqSum' requires input 'addends'" 1
expect "and so is one for a fragment whose input it did not send" \
    "sequence 'RemoteExecSeqChain' requires input 'terms'" 1
expect "a failing algorithm becomes an error reply" \
    "RemoteExec request 4 (RemoteExecSeqFail) -> ALG_FAILURE" 1
expect "and the reply names the algorithm that failed" \
    "sequence 'RemoteExecSeqFail' failed" 1

# The reply the client would have received, for each Ping. Three separate
# requests through the same two slots: a slot whose store was not cleared fails
# to record the second request's outputs, which is what the repeats catch.
for tick in 1 3 7; do
    expect "ping ${tick} was answered" \
        "RemoteExec request ${tick} (RemoteExecSeqPing) -> OK" 1
done

# ...and each answer carries the fragment's own message, not an empty reply.
expect "each ping carried its output" \
    "output pong = athexremoteexec.demo.v1.Ints" 3

if [ ${failures} -ne 0 ]; then
    echo "FAIL: ${failures} check(s) failed; log follows"
    tail -80 "${log}"
    exit 1
fi

echo "test_remote_exec_hive_canned: all checks passed"
exit 0
