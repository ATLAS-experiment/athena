#! /usr/bin/env python3

import os.path, os
SCRIPTDIR = os.path.dirname(os.path.realpath(__file__))
DATADIR = os.environ.get('DCSC_DATADIR', SCRIPTDIR)

from time import strftime

from xmlrpc.client import ServerProxy

def get_runs_since(run_number: int) -> list[int]:
    """
    Get runs that occured since `run_number`, excluding it.
    """
    s = ServerProxy('http://atlasdqm.cern.ch:8080')
    run_list: list[int] = s.get_completed_runs({'low_run': run_number})  # type: ignore
    if run_number in run_list:
        run_list.remove(run_number)
    return run_list

def main():
    """
    Read the lastRunNumber file, find runs which happened since it.
    Then create a runList file containing these runs, and overwrite lastRunNumber.
    """
    try:
        last_run = int(open(os.path.join(DATADIR, "lastRunNumber"), 'r+').read())
    except Exception:
        last_run = 450000
    new_runs = get_runs_since(last_run)
    
    open(os.path.join(DATADIR, "runList"), "w").write("\n".join(map(str, new_runs)))
    
    if not new_runs:
        print(strftime("%d/%m/%Y %H:%M:%S"), "Nothing to do")
        return
    
    last_run = new_runs[-1]
    open(os.path.join(DATADIR, "lastRunNumber"), "w").write(str(last_run)+"\n")
    print(strftime("%d/%m/%Y %H:%M:%S"), "Wrote new runList. Last run is", last_run, new_runs)

if __name__ == "__main__":
    main()
