# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import subprocess,sys,os

from . import batchJobBase

def getJobIDfromlastJob():
    last = None
    with open("qsub.log") as f:
        for line in f:
            if line.strip():        
                last = line
    if last is None:
        raise ValueError("log is empty or only blank lines")
    return last              

class batchJob(batchJobBase.batchJobBase):

    def submit(self, dryRun=False):
        
        cmd = "qsub -j oe -o "+self.basedir+"/"+self.name+".log -l walltime="+str(self.hours)+":00:00,select=1:ncpus="+str(self.nCores)+" -A "+self.account+" -q "+self.queue

        if len(self.dependsOnOk)>0 or len(self.dependsOnAny)>0:
            cmd += " -W depend="
            if len(self.dependsOnOk)>0:
                okdeps = "afterok:"+",afterok:".join(self.dependsOnOk)
                cmd += okdeps+","
            if len(self.dependsOnAny)>0:
                anydeps = "afterany:"+",afterany:".join(self.dependsOnAny)
                cmd += anydeps+","
            cmd = cmd[:-1].rstrip()

        if "ecm" in self.basedir:
            jobname = os.path.relpath(self.basedir+"/"+self.name+".log", self.basedir+"/../..")
        else:
            jobname = os.path.relpath(self.basedir+"/"+self.name+".log", self.basedir+"/..")
        jobname = jobname.replace('.','_').replace('/', '_')
        cmd += " -N "+jobname+" "+self.basedir+"/"+self.name+".sh >> qsub.log \n"

        if dryRun:
            print (cmd)
            self.id = "-1"
        else:
            print(cmd)
            p = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            retcode = p.communicate()
            if len(retcode[0]):
                print (retcode[0])
            if len(retcode[1]):
                print (retcode[1])
            if p.returncode:
                print ("ERROR: error while submitting job")
                print ("return code: " + str(p.returncode))
                sys.exit(11)

            p.wait()

            self.id = getJobIDfromlastJob()

        print("Submitted "+self.name+" with id: "+self.id+"\n\n")

def finalizeJobs(dryRun):
    return True

