# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from ispy import ISObject, IPCPartition
import time
from AthenaCommon.Logging import logging
import os, sys

def GetBFields():
  mlog = logging.getLogger( 'EventDisplays' )

  #BFields are read from initial partition
  partition='initial'
  mlog.debug("Trying to read magnetic field configuration from partition %s", partition)

  #now try and read the information from IS
  try :
    #Get hold of the initial partition
    ipcPart = IPCPartition(partition)
    if not ipcPart.isValid():
      raise UserWarning("Partition %s invalid - cannot access magnetic field setting"%partition)
    #Get the current and valid status
    toroidCurrent = ISObject(ipcPart,'DCS_GENERAL.MagnetToroidsCurrent.value','DdcFloatInfo')
    solenoidCurrent = ISObject(ipcPart,'DCS_GENERAL.MagnetSolenoidCurrent.value','DdcFloatInfo')
    toroidInvalid = ISObject(ipcPart,'DCS_GENERAL.MagnetToroidsCurrent.invalid','DdcIntInfo')
    solenoidInvalid = ISObject(ipcPart,'DCS_GENERAL.MagnetSolenoidCurrent.invalid','DdcIntInfo')
    toroidCurrent.checkout()
    solenoidCurrent.checkout()
    toroidInvalid.checkout()
    solenoidInvalid.checkout()
    #And calculate the flags
    solenoidOn=((solenoidCurrent.value > 1000.) and (solenoidInvalid.value == 0))
    toroidOn=((toroidCurrent.value > 1000.) and (toroidInvalid.value == 0))
  except UserWarning as err:
    mlog.error(err)
    #Should always be able to access initial parititon
    mlog.fatal("Failed to read magnetic field configuration from IS, aborting")
    sys.exit(1)

  #print the result
  mlog.info("Magnetic field in solenoid is %s", (solenoidOn and "ON") or "OFF")
  mlog.info("Magnetic field in toroid is %s", (toroidOn and "ON") or "OFF")

  #finally return our values
  return (solenoidOn,toroidOn)


def WaitForPartition(partitionName=None):
  mlog = logging.getLogger( 'EventDisplays' )
  partitionUp = False
  while not partitionUp:
    try:
      partition = IPCPartition(partitionName)
      runParams = ISObject(partition, "RunParams.RunParams", "RunParams")
      runParams.checkout()
      partitionUp = True

    except Exception:
      mlog.info("%s partition is not up, sleeping for 30 seconds", partitionName)
      time.sleep(30)

def EventCanBeSeenByPublic(projectTags=[]):
# Is the data allowed to be seen by the general public on atlas live an\
d in the CCC

  try:
    partition = IPCPartition('ATLAS')
    RunParams = ISObject(partition, 'RunParams.RunParams', 'RunParams')
    RunParams.checkout()

    ready4physics = ISInfoAny()
    ISInfoDictionary(partition).getValue('RunParams.Ready4Physics', rea\
dy4physics)
    print("physicsReady: %s " % ready4physics.get())

    physicsReady = ISObject(partition, 'RunParams.Ready4Physics','Ready\
4PhysicsInfo')
    physicsReady.checkout()

    ready4physics = ISInfoAny()
    ISInfoDictionary(partition).getValue('RunParams.Ready4Physics', rea\
dy4physics)
    print("physicsReady: %s " % ready4physics.get())

    physicsReady = ISObject(partition, 'RunParams.Ready4Physics','Ready\
4PhysicsInfo')
    physicsReady.checkout()
    print("Ready for physics: %r" % (physicsReady.ready4physics))
    print("RunParams.T0_project_tag", RunParams.T0_project_tag)

    sendToPublicStream = False
    if physicsReady.ready4physics and RunParams.T0_project_tag in proje\
ctTags:
      sendToPublicStream = True

    return sendToPublicStream

  except Exception:
    print('Failed to get bool for EventCanBeSeenByPublic')
    return False

if __name__ == "__main__":
  runType=GetRunType()
  print ("RunType: %s"%runType)
  bFields = GetBFields()
  print ("BFields (Sol,Tor):",bFields)
  isPublicEvent = EventCanBeSeenByPublic(['data24_13p6TeV'])
  print("isPublicEvent:",isPublicEvent)
