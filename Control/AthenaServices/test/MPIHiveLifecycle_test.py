#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Run real MPI workers through the shared Hive completion lifecycle."""

import os
from pathlib import Path
import re
import signal
import sqlite3
import subprocess
import sys


CASES = {
    # Ranks, slots per worker, requested events, failures, completed count.
    "parallel": (3, 2, 16, [], 16),
    "isolated": (2, 1, 8, [1, 2, 4, 5], 8),
    "consecutive": (2, 1, 8, [0, 1, 2], 3),
    "total": (2, 1, 24, list(range(0, 20, 2)), 19),
}


def worker(case):
    rank = int(os.environ["OMPI_COMM_WORLD_RANK"])
    os.environ["RANK"] = str(rank)
    directory = Path(f"rank-{rank}")
    directory.mkdir()
    os.chdir(directory)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    _, slots, events, failures, _ = CASES[case]
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Exec.MPI = True
    flags.Exec.MaxEvents = events
    flags.Concurrency.NumThreads = 2
    flags.Concurrency.NumConcurrentEvents = slots
    flags.lock()
    cfg = MainServicesCfg(flags)
    # Use Hive's synthetic events; master supplies their global indices.
    cfg.getService("MPIHiveEventLoopMgr").EvtSel = "NONE"
    cfg.addEventAlgo(CompFactory.MPIHiveLifecycleTestAlg(
        "MPILifecycleFixture", FailEvents=failures))
    # cfg.run() returns immediately on an expected event-processing failure.
    # Drive the application explicitly so those cases also stop and finalize
    # the services (including MPI), and preserve the original run status.
    app = cfg.createApp()
    cfg.wasMerged()
    assert app.initialize().isSuccess()
    assert app.start().isSuccess()
    rc = 0 if app.run(events).isSuccess() else 1
    assert app.stop().isSuccess()
    assert app.finalize().isSuccess()
    assert app.terminate().isSuccess()
    return rc


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "--worker":
        return worker(sys.argv[2])

    script = str(Path(__file__).resolve())
    for case, (ranks, _slots, _events, failures, completed) in CASES.items():
        directory = Path(case)
        directory.mkdir()
        with (directory / "mpi.log").open("w") as log:
            process = subprocess.Popen(
                ["mpirun", "--oversubscribe", "--bind-to", "none", "-n",
                 str(ranks), sys.executable, script, "--worker", case],
                cwd=directory, stdout=log, stderr=subprocess.STDOUT,
                start_new_session=True)
            try:
                rc = process.wait(timeout=90)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
                raise AssertionError(f"{case}: MPI job hung")
        output = (directory / "mpi.log").read_text()
        expected_failure = case in ("consecutive", "total")
        assert (rc != 0) == expected_failure, output
        rows = []
        active_workers = 0
        for rank in range(1, ranks):
            db_path = directory / f"rank-{rank}" / "mpilog.db"
            with sqlite3.connect(db_path) as db:
                worker_rows = db.execute(
                    "SELECT id, complete, status FROM event_log ORDER BY id"
                ).fetchall()
                rows.extend(worker_rows)
                active_workers += bool(worker_rows)
        assert sorted(row[0] for row in rows) == list(range(completed)), output
        assert all(row[1] == 1 for row in rows), rows
        # Success is EventStatus::Success (1); failure status must be distinct.
        assert all((status == 1) == (idx not in failures)
                   for idx, _, status in rows), rows
        ends = sorted(int(n) for n in re.findall(
            r"MPI lifecycle end event (\d+)", output))
        assert ends == list(range(completed)), output
        assert output.count("MPI lifecycle slots released") == ranks, output
        if case == "parallel":
            assert active_workers == ranks - 1, output
        print(f"ok: {case}: {completed} events logged and ended "
              f"across {ranks} ranks")
    return 0


if __name__ == "__main__":
    sys.exit(main())
