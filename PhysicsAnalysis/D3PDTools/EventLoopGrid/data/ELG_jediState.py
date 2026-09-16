# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration


def ELG_jediState(sample) :
    """ Returns state of the jobs.

    State returned as an int based on an enum in the PrunDriver.cxx :
    `enum Enum { DONE=0, PENDING=1, FAIL=2 };`
    Make sure these values are kept the same in both files.
    Extra values are added to capture extra states, those start at 90.

    """
    from enum import IntEnum
    class Status(IntEnum):
        DONE = 0
        PENDING = 1
        FAIL = 2
        RUNNING = 90
        OTHER = 91
        SCRIPT_FAIL = 99


    try:
        from pandatools import PandaToolsPkgInfo  # noqa: F401
    except ImportError:
        print ("prun needs additional setup, try:")
        print ("    lsetup panda")
        return Status.SCRIPT_FAIL

    jediTaskID = int(sample.meta().castDouble("nc_jediTaskID", 0))

    if jediTaskID < 100 :
        print ("Sample " + sample.name() + " does not have a jediTaskID")
        return Status.SCRIPT_FAIL

    from pandatools import Client

    taskDict = {}
    taskDict['jediTaskID'] = jediTaskID
    detail = Client.getJediTaskDetails(taskDict, False, True)
    if detail[0] != 0 :
        print ("Problem checking status of task %s with id %s" % (sample.name(), jediTaskID))
        return Status.SCRIPT_FAIL

    status = detail[1]['status']

    if status == "done": return Status.DONE
    elif status == "failed": return Status.FAIL
    # Finished returning FAIL status is original
    # behavior from the PrunDriver.cxx
    elif status == "finished": return Status.FAIL
    elif status == "running": return Status.RUNNING

    # Value for states not considered by PrunDriver.cxx
    return Status.OTHER
