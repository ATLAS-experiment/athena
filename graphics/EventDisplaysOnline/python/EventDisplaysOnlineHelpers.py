# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from ispy import ISObject, IPCPartition
import time
from AthenaCommon.Logging import logging
import sys

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

def Ready4Physics():
  mlog = logging.getLogger( 'EventDisplays, Ready4Physics' )
  try:
    partition = IPCPartition('ATLAS')
    physicsReady = ISObject(partition, 'RunParams.Ready4Physics','Ready4PhysicsInfo')
    physicsReady.checkout()
    return bool(physicsReady.ready4physics)

  except Exception:
    mlog.warning('Failed to get physicsReady bool, this is expected for offline test and GM test paritions')
    return False

def EventCanBeSeenByPublic(projectTags):
# Is the data allowed to be seen by the general public on atlas live and in the CCC
  mlog = logging.getLogger( 'EventDisplays' )
  try:
    partition = IPCPartition('ATLAS')
    RunParams = ISObject(partition, 'RunParams.RunParams', 'RunParams')
    RunParams.checkout()

    physicsReady = ISObject(partition, 'RunParams.Ready4Physics','Ready4PhysicsInfo')
    physicsReady.checkout()

    sendToPublicStream = False
    if physicsReady.ready4physics and RunParams.T0_project_tag in projectTags:
      sendToPublicStream = True

    return sendToPublicStream

  except Exception:
    mlog.warning('Failed to get bool for EventCanBeSeenByPublic, this is expected for offline test and GM test paritions')
    return False

def GetUniqueJobID():
  # Setup unique output files (so that multiple Athena jobs on the same machine don't interfere)
  import os
  jobId = os.environ.get('TDAQ_APPLICATION_NAME', '').split(':')
  if not len(jobId) == 5:
    from random import randint
    jobId = ['Athena-EventProcessor', 'Athena-EventDisplays-Segment', 'EventDisplays-Rack', 'tmp', '%d' % randint(0, 999)]

  return jobId

def GetRunNumber(partitionName):
  part = IPCPartition(partitionName)
  RunParams = ISObject(part, 'RunParams.RunParams', 'RunParams')
  RunParams.checkout()
  run_number = RunParams.getAttributeValue('run_number')
  return run_number

if __name__ == "__main__":
  runNumber=GetRunNumber()
  print ("RunNumber: %s"%runNumber)
  bFields = GetBFields()
  print ("BFields (Sol,Tor):", bFields)
  isPublicEvent = EventCanBeSeenByPublic(['data24_13p6TeV'])
  print("isPublicEvent:", isPublicEvent)
