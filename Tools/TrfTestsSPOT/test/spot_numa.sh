#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# NUMA-aware single-process placement for the MT SPOT tests.
#
# A single AthenaMT process whose threads span two NUMA nodes pays an extra-CPU
# penalty from remote-memory access versus the same job bound to one node. This
# helper binds the MT process to one NUMA node when the job fits, so the SPOT
# numbers reflect local-memory execution.
#
# Usage (source, then call):
#     source spot_numa.sh
#     spot_numa_setup "${NTHREADS}"
#     ATHENA_CORE_NUMBER=${NTHREADS} ${SPOT_NUMA_PREFIX} <transform>_tf.py ...
#
# spot_numa_setup sets two globals and always returns 0. On any uncertainty it
# leaves SPOT_NUMA_PREFIX empty, i.e. the job runs unpinned as before:
#     SPOT_NUMA_PREFIX  command prefix to place before the transform (may be empty)
#     SPOT_NUMA_INFO    one-line summary of the decision
#
# Set SPOT_NUMA_DISABLE=1 to opt out and always run unpinned.

# Count the entries in a CPU list such as "0-15,32-47" -> integer.
_spot_numa_count_cpulist() {
    local list="${1:-}" total=0 part lo hi
    local IFS=','
    for part in ${list}; do
        case "${part}" in
            *-*) lo="${part%-*}"; hi="${part#*-}"; total=$(( total + hi - lo + 1 ));;
            "")  ;;
            *)   total=$(( total + 1 ));;
        esac
    done
    echo "${total}"
}

spot_numa_setup() {
    local nthreads="${1:-0}"
    SPOT_NUMA_PREFIX=""
    SPOT_NUMA_INFO=""

    # Explicit opt-out.
    if [ "${SPOT_NUMA_DISABLE:-0}" = "1" ]; then
        SPOT_NUMA_INFO="SPOT NUMA: disabled (SPOT_NUMA_DISABLE=1), running unpinned"
        echo "${SPOT_NUMA_INFO}" >&2
        return 0
    fi

    # Need a usable positive thread count.
    if ! [ "${nthreads}" -ge 1 ] 2>/dev/null; then
        SPOT_NUMA_INFO="SPOT NUMA: thread count '${nthreads}' not usable, running unpinned"
        echo "${SPOT_NUMA_INFO}" >&2
        return 0
    fi

    # Count NUMA nodes from sysfs. SPOT_NUMA_SYSFS_ROOT is overridable for tests.
    local sysfs_root="${SPOT_NUMA_SYSFS_ROOT:-/sys/devices/system/node}"
    local nnodes=0 node
    if [ -d "${sysfs_root}" ]; then
        for node in "${sysfs_root}"/node[0-9]*; do
            [ -e "${node}" ] || continue
            nnodes=$(( nnodes + 1 ))
        done
    fi
    if [ "${nnodes}" -le 1 ]; then
        SPOT_NUMA_INFO="SPOT NUMA: fewer than 2 NUMA nodes (${nnodes}), running unpinned"
        echo "${SPOT_NUMA_INFO}" >&2
        return 0
    fi

    # Logical CPUs on node 0 (topology assumed symmetric).
    local node0_cpulist_file="${sysfs_root}/node0/cpulist"
    local node0_cpulist="" node0_ncpu=0
    if [ -r "${node0_cpulist_file}" ]; then
        node0_cpulist=$(cat "${node0_cpulist_file}")
        node0_ncpu=$(_spot_numa_count_cpulist "${node0_cpulist}")
    fi
    if [ "${node0_ncpu}" -lt 1 ]; then
        SPOT_NUMA_INFO="SPOT NUMA: could not read node-0 CPU list, running unpinned"
        echo "${SPOT_NUMA_INFO}" >&2
        return 0
    fi

    # Only bind when the job fits inside a single node.
    if [ "${nthreads}" -gt "${node0_ncpu}" ]; then
        SPOT_NUMA_INFO="SPOT NUMA: ${nthreads} threads exceed node-0 CPUs (${node0_ncpu}) on ${nnodes} nodes; running unpinned (spans nodes)"
        echo "${SPOT_NUMA_INFO}" >&2
        return 0
    fi

    # Prefer numactl: binds both CPUs and memory to node 0.
    if command -v numactl >/dev/null 2>&1 && numactl --hardware >/dev/null 2>&1; then
        SPOT_NUMA_PREFIX="numactl --cpunodebind=0 --membind=0 --"
        SPOT_NUMA_INFO="SPOT NUMA: ${nnodes} nodes, ${nthreads}<=${node0_ncpu} CPUs/node; binding to node 0 (CPU+memory): ${SPOT_NUMA_PREFIX}"
        echo "${SPOT_NUMA_INFO}" >&2
        # Record the policy the transform will inherit.
        local resolved IFS=$' \t\n'
        resolved=$(${SPOT_NUMA_PREFIX} numactl --show 2>/dev/null | tr '\n' ';' 2>/dev/null || true)
        if [ -n "${resolved}" ]; then
            SPOT_NUMA_INFO="${SPOT_NUMA_INFO}"$'\n'"SPOT NUMA: resolved policy: ${resolved}"
            echo "SPOT NUMA: resolved policy: ${resolved}" >&2
        fi
        return 0
    fi

    # Fall back to taskset (CPUs only; memory first-touch, may be remote). The
    # probe reads back the mask it applied, so a broken taskset or rejected
    # cpulist falls through to unpinned instead of failing the job at launch.
    local resolved IFS=$' \t\n'
    if command -v taskset >/dev/null 2>&1 &&
       resolved=$(taskset -c "${node0_cpulist}" grep Cpus_allowed_list /proc/self/status 2>/dev/null); then
        SPOT_NUMA_PREFIX="taskset -c ${node0_cpulist}"
        SPOT_NUMA_INFO="SPOT NUMA: numactl absent; taskset pinning CPUs to node 0 (${node0_cpulist}); memory first-touch, may be remote"$'\n'"SPOT NUMA: resolved affinity: ${resolved##*[[:space:]]}"
        echo "${SPOT_NUMA_INFO}" >&2
        return 0
    fi

    SPOT_NUMA_INFO="SPOT NUMA: neither numactl nor taskset available, running unpinned"
    echo "${SPOT_NUMA_INFO}" >&2
    return 0
}
