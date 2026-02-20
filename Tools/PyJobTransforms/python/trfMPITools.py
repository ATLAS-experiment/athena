# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

## @package PyJobTransforms.trfMPITools
#
# @brief Utilities for handling MPI-Athena jobs
# @author Beojan Stanislaus <beojan.stanislaus@cern.ch>
#

from enum import Enum
from copy import deepcopy
import os
import re
import logging
import pprint
import itertools as it

from PyJobTransforms.trfExitCodes import trfExit
import PyJobTransforms.trfExceptions as trfExceptions

msg = logging.getLogger(__name__)

mpiConfig = None


class MPIType(Enum):
    """MPI master, MPI worker, or not using MPI"""

    NOMPI = 0
    MPIMASTER = 1
    MPIWORKER = 2


def signalError(message):
    msg.error(message)
    raise trfExceptions.TransformSetupException(
        trfExit.nameToCode("TRF_SETUP"), message
    )


def getMPIRank():
    """Return MPI rank"""
    if mpiConfig is not None:
        return int(mpiConfig["rank"])
    if "RANK" not in os.environ:
        return -1
    else:
        try:
            return int(os.environ["RANK"])
        except ValueError:
            signalError("$RANK environment variable is not an integer")
            return -2  # Only here to placate PyRight


def getMPIType():
    """Return MPI type"""
    if mpiConfig is not None:
        return mpiConfig["type"]
    if "RANK" not in os.environ:
        return MPIType.NOMPI
    if getMPIRank() == 0:
        return MPIType.MPIMASTER
    else:
        return MPIType.MPIWORKER


def setupMPIConfig(output, dataDict):
    """Check environment is correct if we are in MPI mode, and setup dictionaries"""
    global mpiConfig
    if "RANK" not in os.environ:
        signalError(
            "Running in MPI mode but the $RANK environment variable is not set!"
        )
    rank = getMPIRank()
    if not os.getcwd().endswith("rank-{}".format(getMPIRank())):
        signalError(
            "Running in MPI mode with rank {0} but working directory is not called rank-{0}".format(
                getMPIRank()
            )
        )
    mpiType = getMPIType()
    mpiConfig = {}
    mpiConfig["rank"] = rank
    mpiConfig["type"] = mpiType
    mpiConfig["outputs"] = {
        dataType: deepcopy(dataDict[dataType]) for dataType in output
    }
    # expand any [  ] lists in output filenames
    output_proc_regex = re.compile(r"(.+)\[(.*)](.*)")
    for v in mpiConfig["outputs"].values():
        v.multipleOK = True
        new_list = []
        list_to_remove = []
        for fn in v.value:
            if ("[" in fn) and ("]" in fn):
                match = output_proc_regex.match(fn)
                new_list.extend(
                    [
                        f"{match.group(1)}{it}{match.group(3)}"
                        for it in match.group(2).split(",")
                    ]
                )
                list_to_remove.append(match.group(1))
            else:
                new_list.append(fn)
                list_to_remove.append(fn)
        v.value = new_list
        v.list_to_remove = list(set(list_to_remove))


def mpiShouldValidate():
    if getMPIType() == MPIType.NOMPI:
        return True  # validate if we're not in MPI mode
    if getMPIRank() == 0:
        return True  # validate in rank 0
    return False  # don't validate in other ranks


def mpiOutputs():
    return mpiConfig["outputs"].values()


def mergeOutputs():
    """Merge outputs into rank 0"""
    if mpiConfig is None:
        msg.warn("trfMPITools.mergeOutputs called when we are not in MPI mode")
        return
    rank_dir_regex = re.compile("rank-([0-9]+)$")
    rank_dirs = {
        int(m.group(1)): m.string
        for m in (rank_dir_regex.search(d.path) for d in os.scandir("..") if d.is_dir())
        if m and int(m.group(1)) > 0
    }
    if getMPIRank() == 0:
        msg.info("Rank output directories are:\n{}".format(pprint.pformat(rank_dirs)))
    all_merge_inputs = list(
        map(
            lambda f: f.path,
            filter(
                lambda f: f.is_file(),
                it.chain.from_iterable(map(os.scandir, rank_dirs.values())),
            ),
        )
    )
    # Remove PoolFileCatalog
    try:
        os.remove("PoolFileCatalog.xml")
    except FileNotFoundError:
        pass
    for dtype, defn in mpiConfig["outputs"].items():
        if getMPIRank() == 0:
            msg.info(f"Output type is {dtype}")
        merge_helper = deepcopy(defn)
        merge_helper.multipleOK = True
        if getMPIRank() == 0:
            for fn in defn.list_to_remove:
                # remove empty files from rank 0
                try:
                    os.remove(fn)
                except FileNotFoundError:
                    pass
        merge_lists = []
        for fn in defn.value:
            msg.info(f"Generating merge list by filtering for {fn} in {all_merge_inputs}")
            merge_inputs = sorted(filter(lambda s: s.endswith(fn), all_merge_inputs))
            # Add to list
            merge_helper.value.extend(merge_inputs)
            merge_lists.append((fn, merge_inputs))
        # Remove non-existent output files from mpiOutputs
        defn.value = [x[0] for x in merge_lists if len(x[1]) >= 1]
        # Merge each final output in a different rank
        if getMPIRank() >= len(merge_lists):
            msg.info(f"In rank {getMPIRank()}, not merging")
            break
        my_merge = merge_lists[getMPIRank()]
        if len(my_merge[1]) < 1:
            msg.info(f"In rank {getMPIRank()}, no inputs for ../rank-0/{my_merge[0]}")
            open("done_merging", "a").close()
            break
        msg.info(
            f"In rank {getMPIRank()}, merging into ../rank-0/{my_merge[0]}. Inputs are \n{pprint.pformat(my_merge[1])}"
        )
        merge_helper.selfMerge(f"../rank-0/{my_merge[0]}", my_merge[1])
    # Create a file to indicate we are done
    open("done_merging", "a").close()
    if getMPIRank() == 0:
        from functools import reduce
        from operator import and_
        from time import sleep
        import sqlite3 as sq3
        from glob import glob

        # Merge log databases
        conn = sq3.connect("mpilog.db")
        cur = conn.cursor()
        tables = ["ranks", "files", "event_log"]
        for db in glob("../rank-[1-9]*/mpilog.db"):
            cur.execute("ATTACH DATABASE ? as db", (db,))
            for table in tables:
                upsert = "INSERT OR IGNORE" if table == "files" else "INSERT"
                cur.execute(f"{upsert} INTO {table} SELECT * from db.{table}")
            conn.commit()
            cur.execute("DETACH DATABASE db")
        conn.close()

        # In rank 0, wait until all other ranks have finished merging
        files_to_check = [
            f"../rank-{rank}/done_merging" for rank in range(0, len(merge_lists))
        ]
        count = 0
        check = [os.path.exists(f) for f in files_to_check]
        while not reduce(and_, check):
            if count % 10 == 0:
                msg.info("Waiting for other ranks to finish merging")
                msg.debug(f"Looking for {files_to_check}")
                msg.debug(f"Result: {check}")
            count = count + 1
            sleep(6)
            check = [os.path.exists(f) for f in files_to_check]
        msg.info("All ranks done merging")
