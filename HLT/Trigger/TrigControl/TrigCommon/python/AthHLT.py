# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Utilities used in athenaHLT.py
#
from typing import Any
from AthenaCommon.Logging import logging
from pycrest.api.crest_api import CrestApi
log = logging.getLogger('athenaHLT')

from functools import cache
import os
import sys

class CondDB:
   _run2 = 236108
   def __init__(self, run):
      self.run = run
   def db_instance(self):
      if self.run>=self._run2:
         return 'CONDBR2'
      else:
         return 'COMP200'
   def sor_folder(self):
      if self.run>=self._run2:
         return '/TDAQ/RunCtrl/SOR'
      else:
         return '/TDAQ/RunCtrl/SOR_Params'

@cache
def get_sor_params(run_number) -> dict[str, Any] | None:
   from CoolConvUtilities import AtlCoolLib

   log.info('Reading SOR record for run %s from COOL', run_number)

   cdb = CondDB(run_number)
   dbcon = AtlCoolLib.readOpen('COOLONL_TDAQ/%s' % cdb.db_instance())
   folder = dbcon.getFolder(cdb.sor_folder())   # type: ignore

   # need to keep sor variable while using payload (cannot do the following in
   # one single line nor overwrite sor). Otherwise: 1) GC comes into play;
   # 2) the object is deleted; 3) since it's a shared_ptr, the internal
   # cool::IObject also gets deleted; 4) payload is not valid any longer
   try:
      sor = folder.findObject(run_number << 32, 0)
   except Exception:
      return None        # This can happen for unknown run numbers

   payload = sor.payload()
   d = {k: payload[k] for k in payload}
   return d

@cache
def get_eor_params_crest(run_number, crest_server:str) -> dict[str, Any] | None:
   from TrigConfStorage.TriggerCrestUtil import TriggerCrestUtil
   log.info('Reading EOR record for run %s from Crest', run_number)
   return TriggerCrestUtil.getEORParams(run_number, server=crest_server)

@cache
def get_trigconf_keys(run_number, lb_number):
   """Read Trigger keys from COOL"""

   from TrigConfStorage.TriggerCoolUtil import TriggerCoolUtil
   confKeys: dict[str, Any] = TriggerCoolUtil.getTrigConfKeys(run_number, lb_number)
   confKeys['DBAlias'] = confKeys.pop('DB', None)
   return confKeys


@cache
def get_trigconf_keys_crest(run_number, lb_number, crest_server):
   """Read Trigger keys from CREST"""
   from TrigConfStorage.TriggerCrestUtil import TriggerCrestUtil   
   log.info("Using CREST server %s", crest_server)
   api: CrestApi = TriggerCrestUtil.getCrestApi(server=crest_server)
   confKeys: dict[str, Any] = TriggerCrestUtil.getTrigConfKeys(run_number, lb_number, api=api)
   confKeys['DBAlias'] = confKeys.pop('DB', None)
   return confKeys

def getCACfg(jopath):
   """Return the CA Cfg function based on joboptions path.
   The format is MODULE[.FNC]. If no FNC is given, 'main' will be tried."""

   import importlib

   sys.path.append('.')   # temporarily add local directory to search path

   # try to import module as given:
   try:
      module = importlib.import_module(jopath)
   except ModuleNotFoundError:
      if '.' not in jopath:
         raise
      # or interpret as module.fnc:
      mod_name, fnc_name = jopath.rsplit('.', maxsplit=1)
      module = importlib.import_module(mod_name)
   else:
      # if the first import worked we are using the 'main(flags)' function:
      fnc_name = 'main'

   sys.path.pop()

   log.info("Loading %s.%s", module.__name__, fnc_name)
   return getattr(module, fnc_name)


def reload_from_json(filename, suppress_args=[]):
   """Re-launch athenaHLT from the given json file. Optionally suppress
   the list of command line args (e.g. flags)."""

   # Remove all command line args that are not compatible with running from JSON:
   argv = []
   for arg_index, arg in enumerate(sys.argv):
      if arg == '--dump-config-reload':
         continue
      if arg in ['--precommand', '-c', '--postcommand', '-C']:
         continue
      if arg_index > 0 and sys.argv[arg_index-1] in ['--precommand', '-c', '--postcommand', '-C']:
         continue
      if arg.startswith('--precommand') or arg.startswith('--postcommand'):
         continue
      if arg in suppress_args:
         continue
      argv.append(arg)

   argv[-1] = filename
   log.info('Restarting %s from %s ...', argv[0], argv[-1])
   sys.stdout.flush()
   sys.stderr.flush()
   os.execvp(argv[0], argv)


#
# Testing (used as ctest)
#
if __name__=='__main__':
   # Can be used as script, e.g.: python -m TrigCommon.AthHLT 327265
   if len(sys.argv)>1:
      log.info('SOR parameters: %s', get_sor_params(int(sys.argv[1])))
      sys.exit(0)

   # Unit testing case:
   d = get_sor_params(327265)  # Run-2
   assert(d is not None)
   if d is not None:
      print(d)
      assert(d['DetectorMask']=='0000000000000000c10069fffffffff7')

   d = get_eor_params_crest(327265, "https://crest.cern.ch/api-v5.0")  # Run-2
   assert(d is not None)
   if d is not None:
      print(d)
      assert(d['DetectorMask']=='0000000000000000c10069fffffffff7')

   d = get_sor_params(216416)  # Run-1
   assert(d is not None)
   if d is not None:
      print(d)
      assert(d['DetectorMask']==281474976710647)

   # Config keys
   d = get_trigconf_keys(360026, 1)
   print(d)
   assert(d['SMK']==2749)
   assert(d['LVL1PSK']==15186)
   assert(d['HLTPSK']==17719)

   # Config keys crest
   d = get_trigconf_keys_crest(360026, 1, "https://crest.cern.ch/api-v5.0")
   print(d)
   assert(d['SMK']==2749)
   assert(d['LVL1PSK']==15186)
   assert(d['HLTPSK']==17719)

   d = get_trigconf_keys(360026, 100)
   print(d)
   assert(d['SMK']==2749)
   assert(d['LVL1PSK']==23504)
   assert(d['HLTPSK']==17792)
