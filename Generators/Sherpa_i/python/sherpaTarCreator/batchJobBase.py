# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import os, stat

class batchJobBase:
  """A class containing all information necessary to run given bash commands in an arbitrary batch system."""

  def __init__(self, name, hours=0, nCores=1, account=None, queue=None, memMB=0, mounts=[], basedir=""):
    self.name = name
    self.cmds = []
    self.hours = hours
    self.nCores = nCores
    self.account = account
    self.queue = queue
    self.memMB = memMB
    self.mounts = mounts
    if self.memMB == 1:
      self.memMB = 1499
    if self.memMB == 2:
      self.memMB = 2499
    self.env = {}
    self.basedir = basedir
    self.id = None
    self.dependsOnOk = []
    self.dependsOnAny = []

  def write(self, useSingularity=True, useApptainer=False, extraDirs=[]):
    executable =  "#!/bin/sh -\n"

    # COMPILER_PATH
    if useSingularity:
      platform = str(os.environ['COMPILER_PATH']).split('/')[-1].replace('el9', 'almalinux9')
      executable += 'if [ "$1" != "--really" ]; then \n'
      executable += '  exec singularity exec -e --no-home'
      for dir in ["/cvmfs", "/var", self.basedir, "$(pwd | cut -d '/' -f 1-2)"] + extraDirs:
        executable += ' -B '+dir
      executable += ' /cvmfs/atlas.cern.ch/repo/containers/fs/singularity/'
      executable += platform + ' /bin/bash -- "$0" --really "$@";\n'
      executable += 'fi\n'
      executable += "shift;\n\n"
      executable += "export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase\n"
      executable += "source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh\n\n"
      executable += "ulimit -f 1000000;\n"
      executable += "cd "+self.basedir+"\n\n"
      executable += "echo 'ncores="+str(self.nCores)+" nhours="+str(self.hours)+" "+self.basedir+"/"+self.name+".sh';\n"
      for cmd in self.cmds:
        executable += cmd+"\n"
      executable += "exit 0\n"

    elif useApptainer:
      wrapper = ''
      wrapperfilename = self.basedir+"/"+self.name+"_wrapper.sh"
      platform = str(os.environ['COMPILER_PATH']).split('/')[-1].replace('el9', 'almalinux9')
      executable += 'export ALRB_CONT_SWTYPE="apptainer"\n'
      executable += 'export ALRB_CONT_PRESETUP="hostname -f; date; id -a"\n'
      executable += 'export ALRB_testPath=",,,,,,,,,,,,,,,,,,,,,,,,"\n'
      executable += 'export ALRB_CONT_RUNPAYLOAD="'+wrapperfilename+'"\n'
      wrapper += "ulimit -f 1000000;\n"
      wrapper += "cd "+self.basedir+";\n\n"
      wrapper += "echo 'ncores="+str(self.nCores)+" nhours="+str(self.hours)+" "+self.basedir+"/"+self.name+".sh';\n"
      for cmd in self.cmds:
        wrapper += cmd+'\n'
      with open(wrapperfilename, 'w') as f:
        f.write(wrapper)
      st = os.stat(wrapperfilename)
      os.chmod(wrapperfilename, st.st_mode | stat.S_IEXEC)
      executable += "export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase\n"
      executable += "source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh -c "+platform+" -b -q"
      if self.mounts != []:
        executable += "-m"+" ".join(self.mounts)
      else:
        executable += "\n"
      executable += "exit $?\n"
      
    else:
      executable += "export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase\n"
      executable += "source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh\n"
      executable += "ulimit -f 1000000;\n"
      executable += "cd "+self.basedir+"\n\n"
      executable += "echo 'ncores="+str(self.nCores)+" nhours="+str(self.hours)+" "+self.basedir+"/"+self.name+".sh';\n"
      for cmd in self.cmds:
        executable += cmd+"\n"
      executable += "exit 0\n"

    filename = self.basedir+"/"+self.name+".sh"
    with open(filename, 'w') as f:
        f.write(executable)

    #make shell-files executable
    st = os.stat(filename)
    os.chmod(filename, st.st_mode | stat.S_IEXEC)


