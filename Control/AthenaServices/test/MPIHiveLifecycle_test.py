#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Run real MPI workers through a complete Hive job."""

import os
from pathlib import Path
import re
import signal
import sqlite3
import subprocess
import sys


RANKS = 3  # One master and two workers.


def worker(flags):
    rank = int(os.environ["OMPI_COMM_WORLD_RANK"])
    directory = Path(f"rank-{rank}")
    directory.mkdir()
    os.chdir(directory)

    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    cfg = MainServicesCfg(flags)
    # Use Hive's synthetic events; master supplies their global indices.
    cfg.getService("MPIHiveEventLoopMgr").EvtSel = "NONE"
    cfg.addEventAlgo(CompFactory.MPIHiveLifecycleTestAlg("MPILifecycleFixture"))
    return 0 if cfg.run().isSuccess() else 1


def main():
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Exec.MPI = True
    flags.Exec.MaxEvents = 16
    flags.Concurrency.NumThreads = 2
    flags.Concurrency.NumConcurrentEvents = 2
    parser = flags.getArgumentParser()
    parser.add_argument("--worker", action="store_true",
                        help="run as an MPI rank (set by the launcher)")
    args = flags.fillFromArgs(parser=parser)
    flags.lock()
    if args.worker:
        return worker(flags)

    script = str(Path(__file__).resolve())
    with Path("mpi.log").open("w") as log:
        # Fail a hung shutdown well before the ctest timeout, and run mpirun in
        # its own process group so that all ranks are killed with it.
        process = subprocess.Popen(
            ["mpirun", "--oversubscribe", "--bind-to", "none", "-n",
             str(RANKS), sys.executable, script, "--worker", *sys.argv[1:]],
            stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
        try:
            rc = process.wait(timeout=90)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
            raise AssertionError("MPI job hung")
    events = flags.Exec.MaxEvents
    output = Path("mpi.log").read_text()
    assert rc == 0, output

    # MPIClusterSvc records one event_log row per event in each worker's
    # mpilog.db. Together the workers must have logged every event exactly
    # once, as complete and successful, and each worker must have taken part.
    rows = []
    active_workers = 0
    for rank in range(1, RANKS):
        with sqlite3.connect(f"rank-{rank}/mpilog.db") as db:
            worker_rows = db.execute(
                "SELECT id, complete, status FROM event_log ORDER BY id"
            ).fetchall()
            rows.extend(worker_rows)
            active_workers += bool(worker_rows)
    assert sorted(row[0] for row in rows) == list(range(events)), output
    assert all(row[1] == 1 for row in rows), rows
    # EventStatus::Success is 1.
    assert all(status == 1 for _, _, status in rows), rows
    assert active_workers == RANKS - 1, output

    ends = sorted(int(n) for n in re.findall(
        r"MPI lifecycle end event (\d+)", output))
    assert ends == list(range(events)), output
    assert output.count("MPI lifecycle slots released") == RANKS, output
    print(f"ok: {events} events logged and ended across {RANKS} ranks")
    return 0


if __name__ == "__main__":
    sys.exit(main())
