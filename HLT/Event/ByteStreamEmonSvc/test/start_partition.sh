#!/bin/bash

#
# This script is meant to be executed in a full TDAQ environment !
# Run it in a new terminal session without setting up offline, only TDAQ
#

# usage: start_partition [data-file.raw [ port [ partition ]]]

# absolute path, since we change to a temporary directory below
RAW_FILE=${1:+$(realpath -s "${1}")}
PORT=${2:-8880}
PARTITION=${3:-test}

. /cvmfs/atlas.cern.ch/repo/sw/tdaq/tools/cmake_tdaq/bin/cm_setup.sh nightly

# Fixes, if you *really* want to run in an offline environment that still
# has a TDAQ dependency
# export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${TDAQ_INST_PATH}/external/${CMTCONFIG}

set -eo pipefail

cleanup()
{
    for pid in "${pids[@]}"; do
        if kill -0 "$pid" 2>/dev/null; then
            kill "$pid"
        fi
    done
    rm -rf ${d}
}

d=$(mktemp -d)
trap cleanup SIGINT SIGTERM EXIT

pids=()

cd ${d}
export TDAQ_IPC_INIT_REF=file:$(pwd)/init.ref
echo "export TDAQ_IPC_INIT_REF=$TDAQ_IPC_INIT_REF"
ipc_server 2> /dev/null &
pids+=($!)
while ! test_ipc_server -p initial
do
  sleep 1
done
ipc_server -p ${PARTITION} &
while ! test_ipc_server -p ${PARTITION}
do
  sleep 1
done
pids+=($!)
is_server -p ${PARTITION} -n Monitoring &
while ! test_is_server -p ${PARTITION} -n Monitoring
do
  sleep 1
done
pids+=($!)
is_server -p ${PARTITION} -n RunParams &
while ! test_is_server -p ${PARTITION} -n RunParams
do
  sleep 1
done
pids+=($!)
event_sampler -p ${PARTITION} -t eb -n eb-01 --repeatedly ${RAW_FILE:+-f ${RAW_FILE}} ${RAW_FILE:--r 10} &
pids+=($!)
rdb_server -p ${PARTITION} -d ISRepository -S $TDAQ_INST_PATH/share/data/RCInfo/daq_app_is_info.xml &
pids+=($!)
while ! test_rdb_server -p ${PARTITION} -n ISRepository
do
  sleep 1
done

# Create RunParams, taken from the input file if there is one
python3 <<EOF
import datetime
from ispy import *

raw_file = "${RAW_FILE}"

p = IPCPartition("${PARTITION}")
runparams = IS.RunParams(p, "RunParams.RunParams")

if raw_file:
    import eformat
    import libpyevent_storage

    # The detector mask, beam type/energy, recording flag, trigger type
    # and project tag are only in the file metadata, not in the events
    meta = libpyevent_storage.pickDataReader(raw_file)

    # Run number and run type from the first event, the run number is
    # assumed to be the same for all events in the file
    first = next(iter(eformat.istream(raw_file)))

    run_types = {
        "PHYSICS": "Physics",
        "CALIBRATION": "Calibration",
        "COSMIC": "Cosmics",
        "TEST": "Test",
        "SIMULATION": "Simulation",
    }

    runparams.run_number = first.run_no()
    runparams.run_type = run_types.get(str(first.run_type()), "Physics")
    runparams.recording_enabled = int(meta.recEnable())
    runparams.trigger_type = int(meta.triggerType())
    # 128 bit mask as 32 hex characters, most significant word first
    runparams.det_mask = f"{int(meta.detectorMask()):032x}"
    runparams.beam_type = int(meta.beamType())
    runparams.beam_energy = int(meta.beamEnergy())
    runparams.T0_project_tag = meta.projectTag()
    # Start of run is approximated by the time stamp of the first event
    runparams.timeSOR = datetime.datetime.fromtimestamp(
        first.bc_time_seconds() + first.bc_time_nanoseconds() * 1e-9,
        datetime.timezone.utc)

runparams.checkin()
print(runparams)
EOF

# gdb --args webproxy --port ${PORT}
webproxy --port ${PORT} &
pids+=($!)
export TDAQ_WEBDAQ_BASE=http://localhost:${PORT}

#sub=$(curl -s -X POST -H "content-type: application/json" -d '{"sampler_type": "eb"}' $TDAQ_WEBDAQ_BASE/info/current/${PARTITION}/event | jq -r .id)
#echo "Subscription = $sub"
#curl -s -v -o event.data $TDAQ_WEBDAQ_BASE/info/current/${PARTITION}/event/$sub
#echo "Got event"
#curl -s -v -X DELETE $TDAQ_WEBDAQ_BASE/info/current/${PARTITION}/event/$sub
echo "export TDAQ_WEBDAQ_BASE=http://localhost:${PORT}"
echo "export TDAQ_IPC_INIT_REF=$TDAQ_IPC_INIT_REF"
echo "Ctrl-C when you are done"
wait
