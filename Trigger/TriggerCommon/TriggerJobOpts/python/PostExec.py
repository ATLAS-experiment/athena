# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# This module contains postExec commands that can be used with the CA-based
# runHLT in athena(HLT).
#
# Assumptions:
#  - the final ComponentAccumulator instance is called 'cfg'
#  - the ConfigFlags instance is called 'flags'
#
# Example usage:
#   athenaHLT -C 'from TriggerJobOpts import PostExec; PostExec.foo([args])' ... TriggerJobOpts.runHLT
#   athena --CA --postExec 'from TriggerJobOpts import PostExec; PostExec.foo([args])' TriggerJobOpts/runHLT.py
#

from AthenaCommon.Logging import logging
log = logging.getLogger('TriggerJobOpts.PostExec')

# For convenience we provide access to the globals of the calling frame.
# This is where the `exec` of the post-commmand happens in athena or athenaHLT.
import inspect
__postExec_frame = next(filter(lambda f : ('TrigPSCPythonCASetup.py' in f.filename or
                                           'runHLT.py' in f.filename), inspect.stack()), None)
if __postExec_frame is not None:
   __globals = dict(inspect.getmembers(__postExec_frame[0]))["f_globals"]


#
# PostExec functions
#

def forceConditions(run, lb, iovDbSvc=None):
   """Force all conditions (except prescales) to match the given run and LB number"""

   log.info(forceConditions.__doc__)

   if iovDbSvc is None:
      iovDbSvc = __globals['cfg'].getService('IOVDbSvc')

   # Do not override these folders:
   ignore = ['/TRIGGER/HLT/PrescaleKey']   # see ATR-22143

   # All time-based folders (from IOVDbSvc DEBUG message, see athena!38274)
   timebased = ['/TDAQ/OLC/CALIBRATIONS',
                '/TDAQ/Resources/ATLAS/SCT/Robins',
                '/SCT/DAQ/Config/ChipSlim',
                '/SCT/DAQ/Config/Geog',
                '/SCT/DAQ/Config/MUR',
                '/SCT/DAQ/Config/Module',
                '/SCT/DAQ/Config/ROD',
                '/SCT/DAQ/Config/RODMUR',
                '/SCT/HLT/DCS/HV',
                '/SCT/HLT/DCS/MODTEMP',
                '/MUONALIGN/Onl/MDT/BARREL',
                '/MUONALIGN/Onl/MDT/ENDCAP/SIDEA',
                '/MUONALIGN/Onl/MDT/ENDCAP/SIDEC',
                '/MUONALIGN/Onl/TGC/SIDEA',
                '/MUONALIGN/Onl/TGC/SIDEC']

   from TrigCommon.AthHLT import get_sor_params
   sor = get_sor_params(run)
   timestamp = sor['SORTime'] // int(1e9)

   for i,f in enumerate(iovDbSvc.Folders):
      if any(name in f for name in ignore):
         continue
      if any(name in f for name in timebased):
         iovDbSvc.Folders[i] += f'<forceTimestamp>{timestamp:d}</forceTimestamp>'
      else:
         iovDbSvc.Folders[i] += f'<forceRunNumber>{run:d}</forceRunNumber> <forceLumiblockNumber>{lb:d}</forceLumiblockNumber>'


def reverseViews():
   """Process views in reverse order"""

   log.info(forceConditions.__doc__)

   from TriggerJobOpts.TriggerConfig import collectViewMakers
   viewMakers = collectViewMakers( __globals['cfg'].getSequence() )
   for alg in viewMakers:
      alg.ReverseViewsDebug = True


def dbmod_BFieldAutoConfig():  # DB modifier for debug recovery when using an online SMK
   """Use DCS currents to configure magnetic field"""

   log.info(dbmod_BFieldAutoConfig.__doc__)

   from GaudiPython.Bindings import iProperty
   # Add the DCS folder
   f = '<db>COOLOFL_DCS/CONDBR2</db> /EXT/DCS/MAGNETS/SENSORDATA'
   iProperty('IOVDbSvc').Folders += [f]
   iProperty('CondInputLoader').Load.add(('CondAttrListCollection','/EXT/DCS/MAGNETS/SENSORDATA'))
   # Configure CondAlgs
   iProperty('AtlasFieldCacheCondAlg').UseDCS = True
   iProperty('AtlasFieldMapCondAlg').LoadMapOnStart = False
   iProperty('AtlasFieldMapCondAlg').UseMapsFromCOOL = True
   iProperty('HltEventLoopMgr').setMagFieldFromPtree = False
