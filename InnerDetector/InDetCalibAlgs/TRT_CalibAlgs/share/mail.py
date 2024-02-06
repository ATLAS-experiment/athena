#!/usr/bin/env python
import json
import subprocess
import sys

# Log of DAGMan debugging messages                 : dag.dag.dagman.out
# Log of HTCondor library output                     : dag.dag.lib.out
# Log of HTCondor library error messages             : dag.dag.lib.err
# Log of the life of condor_dagman itself          : dag.dag.dagman.log
# dag status:
# 0: OK
# 1: error; an error condition different than those listed here
# 2: one or more nodes in the DAG have failed
# 3: the DAG has been aborted by an ABORT-DAG-ON specification
# 4: removed; the DAG has been removed by condor_rm
# 5: a cycle was found in the DAG
# 6: the DAG has been halted


def make_status_mail(oiter, usermail, area = None):
    area = area or os.getcwd()
    with open(area + "/iteration" + oiter + ".dag.metrics", "r") as metrics:
        dic = json.load(metrics)

    duration = dic["duration"]

    hours = duration // (60 * 60)
    minutes = (duration - hours * 60 * 60) // 60
    seconds = duration - (hours * 60 * 60) - (minutes * 60)
    time = str(int(hours)) + "h " + str(int(minutes)) + "m " + str(int(seconds)) + "s"

    msg = (
        "This is an automated email from running manual TRT data calibration.\n\nStatus of iteration {}:\n".format(
            oiter
        )
        + "Job graph with id {} terminated after {} with exit code {}.\n".format(
            dic["dagman_id"], time, dic["exitcode"]
        )
        + "Out of {} jobs submitted, {} were ran, of which {} failed and {} succeeded.\n\n".format(
            dic["jobs"],
            dic["total_jobs_run"],
            dic["jobs_failed"],
            dic["jobs_succeeded"],
        )
    )

    failed = None
    with open(area + "/iteration" + oiter + ".dag.dagman.out", "r") as f:
        for line in f.readlines():
            if "Job Submit File:" in line:
                failed = line.split("Job Submit File:")[1].strip().replace(".sub", "")
                msg += "The job that failed was: {}\n".format(failed)
                break
    if failed:
        with open(area + "/TRTcal_error/" + failed + ".0.err", "r") as f:
            msg += "\nThe stderror output of this job is below:\n\n{}".format(f.read())
            msg += "\n\n The main output of this job without errors is also below:\n\n"
        with open(area + "/TRTcal_output/" + failed + ".0.out", "r") as f:
            msg += "".format(f.read())
    with open("mail.txt", "w+") as file:
        file.write(msg)
    proc = subprocess.Popen(
        "cat mail.txt | mail -s 'Manual TRT Calibration -- iteration {} exit status' {} >> mail.txt".format(
            oiter, usermail
        ),
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    ).communicate()
    with open('mailout', 'w+') as f:
        f.write(proc[0])
        f.write(proc[1])


import sys

if __name__ == "__main__":
    assert (
        len(sys.argv) == 4
    ), "incorrect number of arguments -- specify just the iteration number and user mail"
    make_status_mail(sys.argv[1], sys.argv[2], sys.argv[3])
