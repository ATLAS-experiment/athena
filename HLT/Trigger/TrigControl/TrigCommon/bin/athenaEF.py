#!/bin/sh
# -*- mode: python -*-
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# athenaEF.py - A modified version of athenaHLT.py that runs the HLT configuration
# directly without using HLTMPPy/HLTMPPU. It creates the configuration like
# athenaHLT but executes it like athena.py does.
#
"""date"

# defaults
export USETCMALLOC=1
export USEIMF=1

# parse command line arguments
for a in ${@}
do
    case "$a" in
        --stdcmalloc)    USETCMALLOC=0;;
        --tcmalloc)      USETCMALLOC=1;;
        --stdcmath)      USEIMF=0;;
        --imf)           USEIMF=1;;
        --preloadlib*)   export ATHENA_ADD_PRELOAD=${a#*=};;
        --no-ers-signal-handlers)  export TDAQ_ERS_NO_SIGNAL_HANDLERS=1;;
    esac
done

# Do the actual preloading via LD_PRELOAD
source `which athena_preload.sh `

# Now resurrect ourselves as python script
python_path=`which python`
"exec" "$python_path" "-tt" "$0" "$@";

"""

import sys
import os
import argparse
import json
import pickle
import traceback
from datetime import datetime as dt

from TrigConfStorage.TriggerCrestUtil import TriggerCrestUtil

# Use single-threaded oracle client library to avoid extra
# threads when forking (see ATR-21890, ATDBOPS-115)
os.environ["CORAL_ORA_NO_OCI_THREADED"] = "1"

from TrigCommon import AthHLT
from AthenaCommon.Logging import logging
log = logging.getLogger('athenaEF')


# =============================================================================
# Run Parameters Configuration
# =============================================================================
# Default values for run parameters used by prepareForStart.
# These can be overridden by command-line arguments or fetched from IS.

class RunParams:
   """
   Container for run parameters needed by HltEventLoopMgr::prepareForStart().
   
   This class centralizes all run parameter defaults or their retrieval from IS.
   """
   
   # Default values
   DEFAULT_RUN_NUMBER = 0
   DEFAULT_LB_NUMBER = 0
   DEFAULT_DETECTOR_MASK = 'f' * 32  # All detectors enabled
   DEFAULT_SOR_TIME = None  # Will use 'now' if not set
   DEFAULT_SOLENOID_CURRENT = 7730.0   # (nominal)
   DEFAULT_TOROIDS_CURRENT = 20400.0    # (nominal)
   DEFAULT_BEAM_TYPE = 0
   DEFAULT_BEAM_ENERGY = 0
   DEFAULT_RUN_TYPE = "Physics"
   DEFAULT_TRIGGER_TYPE = 0
   DEFAULT_RECORDING_ENABLED = False
   
   def __init__(self,
                run_number=None,
                lb_number=None,
                detector_mask=None,
                sor_time=None,
                solenoid_current=None,
                toroids_current=None,
                beam_type=None,
                beam_energy=None,
                run_type=None,
                trigger_type=None,
                recording_enabled=None,
                conditions_run=None):
      """Initialize run parameters with defaults for any unspecified values."""
      self.run_number = run_number if run_number is not None else self.DEFAULT_RUN_NUMBER
      self.lb_number = lb_number if lb_number is not None else self.DEFAULT_LB_NUMBER
      self.detector_mask = detector_mask if detector_mask is not None else self.DEFAULT_DETECTOR_MASK
      self.sor_time = sor_time if sor_time is not None else dt.now().strftime('%d/%m/%y %H:%M:%S.%f')
      self.solenoid_current = solenoid_current
      self.toroids_current = toroids_current
      self.beam_type = beam_type if beam_type is not None else self.DEFAULT_BEAM_TYPE
      self.beam_energy = beam_energy if beam_energy is not None else self.DEFAULT_BEAM_ENERGY
      self.run_type = run_type if run_type is not None else self.DEFAULT_RUN_TYPE
      self.trigger_type = trigger_type if trigger_type is not None else self.DEFAULT_TRIGGER_TYPE
      self.recording_enabled = recording_enabled if recording_enabled is not None else self.DEFAULT_RECORDING_ENABLED
      self.conditions_run = conditions_run  # Reference run for conditions lookup (None = use run_number)
   
   def to_dict(self):
      """Return run parameters as a dictionary for prepareForStart."""
      return {
         'run_number': self.run_number,
         'lb_number': self.lb_number,
         'detector_mask': self.detector_mask,
         'sor_time': self.sor_time,
         'solenoid_current': self.solenoid_current,
         'toroids_current': self.toroids_current,
         'beam_type': self.beam_type,
         'beam_energy': self.beam_energy,
         'run_type': self.run_type,
         'trigger_type': self.trigger_type,
         'recording_enabled': self.recording_enabled,
         'conditions_run': self.conditions_run,
      }
   
   @classmethod
   def from_args(cls, args):
      """Create RunParams from argparse args, using defaults for unset values."""
      return cls(
         run_number=args.run_number,
         lb_number=args.lb_number,
         detector_mask=args.detector_mask,
         sor_time=args.sor_time,
         solenoid_current=getattr(args, 'solenoid_current', None),
         toroids_current=getattr(args, 'toroids_current', None),
         conditions_run=getattr(args, 'conditions_run', None),
      )
   
   @classmethod
   def from_is(cls, partition=None, webdaq_base=None, strict=False,
               solenoid_current_override=None, toroids_current_override=None):
      """
      Create RunParams by reading from IS via the WEBDAQ REST API.
      
      This uses the webis_server REST API to fetch run parameters, avoiding
      direct dependencies on TDAQ libraries. The API endpoint is determined by:
      1. The webdaq_base parameter if provided
      2. The TDAQ_WEBDAQ_BASE environment variable
      
      The IS objects accessed are:
      - RunParams.RunParams: run_number, det_mask, timeSOR, trigger_type, etc.
      - Magnets.Magnets: SolenoidCurrent, ToroidsCurrent
      
      Args:
         partition: The partition name (default: from TDAQ_PARTITION env var)
         webdaq_base: Base URL for webis_server (default: from TDAQ_WEBDAQ_BASE env var)
         strict: If True, raise an exception if IS read fails (for --online-environment)
         solenoid_current_override: If provided, skip IS fetch for solenoid (command-line override)
         toroids_current_override: If provided, skip IS fetch for toroids (command-line override)
      
      Returns:
         RunParams instance with values from IS, or defaults if unavailable
      
      Raises:
         RuntimeError: If strict=True and IS read fails
      """
      import requests
      
      # Determine the base URL
      if webdaq_base is None:
         webdaq_base = os.environ.get('TDAQ_WEBDAQ_BASE')
      
      if not webdaq_base:
         msg = "TDAQ_WEBDAQ_BASE not set, cannot read from IS"
         if strict:
            raise RuntimeError(msg + " (required for --online-environment)")
         log.warning(msg + ". Using defaults.")
         return cls()
      
      # Determine partition
      if partition is None:
         partition = os.environ.get('TDAQ_PARTITION', 'ATLAS')
      
      log.info("Reading run parameters from IS via WEBDAQ: %s (partition=%s)", webdaq_base, partition)
      
      params = {}
      
      # Fetch RunParams from IS
      # API: GET /info/current/{partition}/is/{server}/{server}.{name}?format=compact
      # Response format: [name, type, timestamp, data] - we need element [3]
      try:
         url = f"{webdaq_base}/info/current/{partition}/is/RunParams/RunParams.RunParams?format=compact"
         log.debug("Fetching RunParams from: %s", url)
         
         response = requests.get(url, timeout=10)
         if response.status_code == 200:
            response_data = response.json()
            log.debug("RunParams response from IS: %s", response_data)
            
            # Response is [name, type, timestamp, data]
            if isinstance(response_data, list) and len(response_data) >= 4:
               runparams = response_data[3]
            else:
               runparams = response_data  
            
            log.debug("RunParams data: %s", runparams)
            
            # Map IS fields to our RunParams fields
            if 'run_number' in runparams:
               params['run_number'] = int(runparams['run_number'])
            if 'lumiblock' in runparams:
               params['lb_number'] = int(runparams['lumiblock'])
            if 'det_mask' in runparams:
               params['detector_mask'] = runparams['det_mask']
            if 'timeSOR' in runparams:
               sor_time = runparams['timeSOR']
               # Ensure microseconds are present (TrigSORFromPtreeHelper expects format with .%f)
               if '.' not in sor_time:
                  sor_time += '.000000'
               params['sor_time'] = sor_time
            if 'beam_type' in runparams:
               params['beam_type'] = int(runparams['beam_type'])
            if 'beam_energy' in runparams:
               params['beam_energy'] = int(runparams['beam_energy'])
            if 'run_type' in runparams:
               params['run_type'] = runparams['run_type']
            if 'trigger_type' in runparams:
               params['trigger_type'] = int(runparams['trigger_type'])
            if 'recording_enabled' in runparams:
               params['recording_enabled'] = runparams['recording_enabled'] in ('1', 'true', 'True', True, 1)
            
            log.info("Got run parameters from IS: run=%s, lb=%s", 
                     params.get('run_number'), params.get('lb_number'))
         else:
            msg = f"Failed to fetch RunParams from IS: HTTP {response.status_code}"
            if strict:
               raise RuntimeError(msg + " (required for --online-environment)")
            log.warning(msg)
            
      except requests.exceptions.RequestException as e:
         msg = f"Error fetching RunParams from IS: {e}"
         if strict:
            raise RuntimeError(msg + " (required for --online-environment)")
         log.warning(msg)
      except (ValueError, KeyError) as e:
         msg = f"Error parsing RunParams from IS: {e}"
         if strict:
            raise RuntimeError(msg + " (required for --online-environment)")
         log.warning(msg)
      
      # Fetch Magnets from IS
      # In strict mode (online environment), magnets are required unless provided via command line
      # If command-line overrides are provided, use those instead of fetching from IS
      have_solenoid_override = solenoid_current_override is not None
      have_toroids_override = toroids_current_override is not None
      
      if have_solenoid_override:
         params['solenoid_current'] = solenoid_current_override
         log.info("Using solenoid_current=%.1f from command line override", solenoid_current_override)
      if have_toroids_override:
         params['toroids_current'] = toroids_current_override
         log.info("Using toroids_current=%.1f from command line override", toroids_current_override)
      
      # Only fetch from IS if we need at least one value
      if not (have_solenoid_override and have_toroids_override):
         try:
            url = f"{webdaq_base}/info/current/{partition}/is/Magnets/Magnets.Magnets?format=compact"
            log.debug("Fetching Magnets from: %s", url)
            
            response = requests.get(url, timeout=10)
            if response.status_code == 200:
               response_data = response.json()
               log.debug("Magnets response from IS: %s", response_data)
               
               magnets = response_data[3] if isinstance(response_data, list) and len(response_data) >= 4 else response_data
               log.debug("Magnets data: %s", magnets)
               
               # Magnets structure: { "SolenoidCurrent": {"value": ..., "ts": ...}, 
               #                      "ToroidsCurrent": {"value": ..., "ts": ...} }
               if not have_solenoid_override:
                  params['solenoid_current'] = float(magnets['SolenoidCurrent']['value'])
               if not have_toroids_override:
                  params['toroids_current'] = float(magnets['ToroidsCurrent']['value'])
                     
               log.info("Got magnet currents from IS: solenoid=%s, toroids=%s",
                        params.get('solenoid_current'), params.get('toroids_current'))
            elif strict:
               raise RuntimeError(f"Magnets not available from IS: HTTP {response.status_code} "
                                  "(required for --online-environment, use --solenoid-current and --toroids-current to override)")
            else:
               log.debug("Magnets not available from IS: HTTP %d", response.status_code)
               
         except requests.exceptions.RequestException as e:
            if strict:
               raise RuntimeError(f"Error fetching Magnets from IS: {e} "
                                  "(required for --online-environment, use --solenoid-current and --toroids-current to override)")
            log.debug("Error fetching Magnets from IS: %s", e)
         except (ValueError, KeyError, TypeError) as e:
            if strict:
               raise RuntimeError(f"Error parsing Magnets from IS: {e} "
                                  "(required for --online-environment, use --solenoid-current and --toroids-current to override)")
            log.debug("Error parsing Magnets from IS: %s", e)
      
      # In strict mode, verify we got at least run_number from IS
      if strict and 'run_number' not in params:
         raise RuntimeError("Failed to get run_number from IS (required for --online-environment)")
      
      return cls(**params)


def get_trigconf_keys_from_oks(partition=None, webdaq_base=None, strict=False):
   """
   Read trigger configuration keys (SMK, L1PSK, HLTPSK) and DB info from OKS via WEBDAQ REST API.
   
   This reads the keys from the partition's TriggerConfiguration object and its
   related L1TriggerConfiguration and TriggerDBConnection objects.
   
   OKS Structure:
   - Partition -> TriggerConfiguration -> L1TriggerConfiguration (Lvl1PrescaleKey)
   - Partition -> TriggerConfiguration -> TriggerDBConnection (SuperMasterKey)
   - Partition -> TriggerConfiguration -> HLTImplementationDB (hltPrescaleKey)
   
   Args:
      partition: The partition name (default: from TDAQ_PARTITION env var)
      webdaq_base: Base URL for webis_server (default: from TDAQ_WEBDAQ_BASE env var)
      strict: If True, raise an exception if OKS read fails (for --online-environment)
   
   Returns:
      dict with keys: SMK, L1PSK, HLTPSK, db_alias (values may be None if not found)
   
   Raises:
      RuntimeError: If strict=True and OKS read fails
   """
   import requests
   
   # Determine the base URL
   if webdaq_base is None:
      webdaq_base = os.environ.get('TDAQ_WEBDAQ_BASE')
   
   if not webdaq_base:
      msg = "TDAQ_WEBDAQ_BASE not set, cannot read from OKS"
      if strict:
         raise RuntimeError(msg + " (required for --online-environment)")
      log.warning(msg)
      return {'SMK': None, 'L1PSK': None, 'HLTPSK': None, 'db_alias': None}
   
   # Determine partition
   if partition is None:
      partition = os.environ.get('TDAQ_PARTITION', 'ATLAS')
   
   log.info("Reading trigger configuration keys from OKS via WEBDAQ: %s (partition=%s)", 
            webdaq_base, partition)
   
   result = {'SMK': None, 'L1PSK': None, 'HLTPSK': None, 'db_alias': None}
   
   def extract_oks_data(response_json):
      """
      Extract data from OKS compact format: [name, type, attributes, relationships]
      Returns tuple (attributes_dict, relationships_dict)
      """
      if isinstance(response_json, list) and len(response_json) >= 4:
         return response_json[2], response_json[3]  # attributes, relationships
      elif isinstance(response_json, list) and len(response_json) >= 3:
         return response_json[2], {}  # attributes only
      return response_json, {}  # fallback
   
   def get_ref_id(ref):
      """Extract object ID from a relationship reference."""
      if isinstance(ref, list) and len(ref) >= 2:
         return ref[0]  # [id, class] format
      elif isinstance(ref, dict) and 'id' in ref:
         return ref['id']
      elif isinstance(ref, str):
         return ref
      return None
   
   # OKS API: GET /info/current/{partition}/oks/{class}/{name}?format=compact
   # Response format: [name, type, attributes, relationships]
   # - attributes: dict of simple values (strings, ints, etc.)
   # - relationships: dict of references to other objects
   try:
      url = f"{webdaq_base}/info/current/{partition}/oks/Partition/{partition}?format=compact"
      log.debug("Fetching Partition from OKS: %s", url)
      
      response = requests.get(url, timeout=10)
      if response.status_code == 200:
         part_attrs, part_rels = extract_oks_data(response.json())
         log.debug("Partition attributes: %s", part_attrs)
         log.debug("Partition relationships: %s", part_rels)
         
         # Get TriggerConfiguration reference from relationships
         trig_conf_id = None
         if 'TriggerConfiguration' in part_rels:
            trig_conf_id = get_ref_id(part_rels['TriggerConfiguration'])
         
         if trig_conf_id:
            log.debug("TriggerConfiguration ID: %s", trig_conf_id)
            
            # Get TriggerConfiguration object
            url = f"{webdaq_base}/info/current/{partition}/oks/TriggerConfiguration/{trig_conf_id}?format=compact"
            response = requests.get(url, timeout=10)
            if response.status_code == 200:
               trig_attrs, trig_rels = extract_oks_data(response.json())
               log.debug("TriggerConfiguration attributes: %s", trig_attrs)
               log.debug("TriggerConfiguration relationships: %s", trig_rels)
               
               # Get L1TriggerConfiguration for L1PSK (relationship 'l1')
               if 'l1' in trig_rels:
                  l1_id = get_ref_id(trig_rels['l1'])
                  if l1_id:
                     url = f"{webdaq_base}/info/current/{partition}/oks/L1TriggerConfiguration/{l1_id}?format=compact"
                     resp = requests.get(url, timeout=10)
                     if resp.status_code == 200:
                        l1_attrs, _ = extract_oks_data(resp.json())
                        log.debug("L1TriggerConfiguration attributes: %s", l1_attrs)
                        if 'Lvl1PrescaleKey' in l1_attrs:
                           result['L1PSK'] = int(l1_attrs['Lvl1PrescaleKey'])
                           log.info("Got L1PSK=%d from OKS", result['L1PSK'])
               
               # Get TriggerDBConnection for SMK and db_alias (relationship 'TriggerDBConnection')
               if 'TriggerDBConnection' in trig_rels:
                  db_id = get_ref_id(trig_rels['TriggerDBConnection'])
                  if db_id:
                     url = f"{webdaq_base}/info/current/{partition}/oks/TriggerDBConnection/{db_id}?format=compact"
                     resp = requests.get(url, timeout=10)
                     if resp.status_code == 200:
                        db_attrs, _ = extract_oks_data(resp.json())
                        log.debug("TriggerDBConnection attributes: %s", db_attrs)
                        if 'SuperMasterKey' in db_attrs:
                           result['SMK'] = int(db_attrs['SuperMasterKey'])
                           log.info("Got SMK=%d from OKS", result['SMK'])
                        if 'Alias' in db_attrs:
                           result['db_alias'] = db_attrs['Alias']
                           log.info("Got db_alias=%s from OKS", result['db_alias'])
               
               # Get HLTImplementationDB for HLTPSK (relationship 'hlt')
               if 'hlt' in trig_rels:
                  hlt_id = get_ref_id(trig_rels['hlt'])
                  if hlt_id:
                     url = f"{webdaq_base}/info/current/{partition}/oks/HLTImplementationDB/{hlt_id}?format=compact"
                     resp = requests.get(url, timeout=10)
                     if resp.status_code == 200:
                        hlt_attrs, _ = extract_oks_data(resp.json())
                        log.debug("HLTImplementationDB attributes: %s", hlt_attrs)
                        if 'hltPrescaleKey' in hlt_attrs:
                           result['HLTPSK'] = int(hlt_attrs['hltPrescaleKey'])
                           log.info("Got HLTPSK=%d from OKS", result['HLTPSK'])
      else:
         msg = f"Failed to fetch Partition from OKS: HTTP {response.status_code}"
         if strict:
            raise RuntimeError(msg + " (required for --online-environment)")
         log.warning(msg)
         
   except requests.exceptions.RequestException as e:
      msg = f"Error fetching trigger keys from OKS: {e}"
      if strict:
         raise RuntimeError(msg + " (required for --online-environment)")
      log.warning(msg)
   except (ValueError, KeyError, TypeError) as e:
      msg = f"Error parsing trigger keys from OKS: {e}"
      if strict:
         raise RuntimeError(msg + " (required for --online-environment)")
      log.warning(msg)
   
   # In strict mode, verify we got the required keys from OKS
   if strict:
      missing = [k for k in ['SMK', 'L1PSK', 'HLTPSK'] if result.get(k) is None]
      if missing:
         raise RuntimeError(f"Failed to get {', '.join(missing)} from OKS (required for --online-environment)")
   
   return result


def get_run_params(args=None, from_is=False, partition=None, webdaq_base=None, strict=False,
                   solenoid_current_override=None, toroids_current_override=None):
   """
   Get run parameters from the appropriate source.
   
   This is the main entry point for obtaining run parameters. It provides
   a single place to modify when adding new sources (like WEBDAQ).
   
   Args:
      args: argparse Namespace with command-line arguments (optional)
      from_is: If True, try to read from WEBDAQ first
      partition: Partition name for IS access (defaults to TDAQ_PARTITION env var)
      webdaq_base: WEBDAQ base URL (defaults to TDAQ_WEBDAQ_BASE env var)
      strict: If True, raise an exception if IS read fails (for --online-environment)
      solenoid_current_override: Command-line override for solenoid current
      toroids_current_override: Command-line override for toroids current
   
   Returns:
      RunParams instance
   
   Raises:
      RuntimeError: If strict=True and IS read fails
   """
   if from_is:
      return RunParams.from_is(partition=partition, webdaq_base=webdaq_base, strict=strict,
                               solenoid_current_override=solenoid_current_override,
                               toroids_current_override=toroids_current_override)
   elif args is not None:
      return RunParams.from_args(args)
   else:
      return RunParams()


class ConfigRunner:
   """
   Runner class that executes Gaudi configuration from JSON file or database.
   Uses TrigConf::JobOptionsSvc with TYPE="FILE" or TYPE="DB" to load configuration.
   Same approach used by PSC (Psc.cxx) - it sets JobOptionsType and
   JobOptionsPath on the ApplicationMgr, and TrigConf::JobOptionsSvc handles both
   FILE and DB modes transparently.
   """
   def __init__(self, job_options_type, job_options_path, run_params=None,
                properties=None, db_server=None, smk=None,
                num_threads=1, num_slots=1, ef_files=None):
      """
      Args:
         job_options_type: "FILE" or "DB"
         job_options_path: JSON file path (for FILE) or DB connection string (for DB)
         run_params: Run parameters dict for prepareForStart
         properties: Pre-loaded properties dict (optional, for FILE mode)
         db_server: DB server alias (for store() in DB mode)
         smk: Super Master Key (for store() in DB mode)
         num_threads: Number of threads for AvalancheSchedulerSvc.ThreadPoolSize
         num_slots: Number of event slots for EventDataSvc.NSlots
         ef_files: List of input files for EFInterfaceSvc
      """
      self.job_options_type = job_options_type
      self.job_options_path = job_options_path
      self.run_params = run_params or {}
      self.properties = properties
      self.db_server = db_server  # For store() in DB mode
      self.smk = smk              # For store() in DB mode
      self.num_threads = num_threads
      self.num_slots = num_slots
      self.ef_files = ef_files or []  # Input files for EFInterfaceSvc
      self._app = None
   
   @classmethod
   def from_json(cls, json_file, run_params=None, properties=None,
                 num_threads=1, num_slots=1, ef_files=None):
      """Create runner for JSON file (TYPE=FILE)"""
      return cls("FILE", os.path.abspath(json_file), run_params, properties,
                 num_threads=num_threads, num_slots=num_slots, ef_files=ef_files)
   
   @classmethod
   def from_database(cls, db_server, smk, l1psk=None, hltpsk=None, run_params=None,
                     num_threads=1, num_slots=1, ef_files=None):
      """Create runner for database (TYPE=DB)"""
      # Build the DB connection string: server=X;smkey=Y;lvl1key=Z;hltkey=W
      db_path = f"server={db_server};smkey={smk}"
      if l1psk is not None:
         db_path += f";lvl1key={l1psk}"
      if hltpsk is not None:
         db_path += f";hltkey={hltpsk}"
      return cls("DB", db_path, run_params, db_server=db_server, smk=smk,
                 num_threads=num_threads, num_slots=num_slots, ef_files=ef_files)
      
   def run(self, maxEvents=None):
      """
      This follows the same pattern as PSC (Psc.cxx):
      1. Create ApplicationMgr via BootstrapHelper
      2. Set JobOptionsSvcType, JobOptionsType, JobOptionsPath
      3. configure() -> initialize() -> prepareForStart() -> start() ->
         hltUpdateAfterFork() -> run() -> stop() -> finalize() -> terminate()
      """
      from Gaudi.Main import BootstrapHelper
      
      # For FILE mode, load properties from JSON if not already provided
      if self.job_options_type == "FILE" and self.properties is None:
         with open(self.job_options_path, 'r') as f:
            jocat = json.load(f)
         self.properties = jocat.get('properties', {})
      
      bsh = BootstrapHelper()
      app = bsh.createApplicationMgr()
      self._app = app
      
      # For FILE mode, set ApplicationMgr properties from JSON before configure
      if self.job_options_type == "FILE" and self.properties:
         app_props = self.properties.get('ApplicationMgr', {})
         for k, v in app_props.items():
            if k not in ('JobOptionsSvcType', 'JobOptionsType', 'JobOptionsPath'):
               log.debug("Setting ApplicationMgr.%s = %s", k, v)
               app.setProperty(k, str(v) if not isinstance(v, str) else v)
      
      # Set JobOptionsSvc properties like PSC does in Psc.cxx
      log.info("Configuring TrigConf::JobOptionsSvc with TYPE=%s, PATH=%s",
               self.job_options_type, self.job_options_path)
      app.setProperty("JobOptionsSvcType", "TrigConf::JobOptionsSvc")
      app.setProperty("JobOptionsType", self.job_options_type)
      app.setProperty("JobOptionsPath", self.job_options_path)
      
      # Configure the application - TrigConf::JobOptionsSvc will load from FILE or DB
      app.configure()
      
      # Override EvtMax AFTER configure() only if explicitly specified by user
      # Otherwise use whatever value is in the DB
      if maxEvents is not None:
         log.info("Setting EvtMax=%d (overriding DB value)", maxEvents)
         app.setProperty('EvtMax', str(maxEvents))
      
      # All property overrides below use iProperty and must be done after configure()
      # but before initialize() - this is the same pattern as PSC (Psc.cxx)
      from GaudiPython.Bindings import iProperty
      
      # Override EFInterfaceSvc.NumEvents if user specified it (overrides DB value)
      if maxEvents is not None:
         log.info("Setting EFInterfaceSvc.NumEvents=%d (overriding DB value)", maxEvents)
         iProperty("EFInterfaceSvc").NumEvents = maxEvents
      
      # Set threading configuration
      log.info("Setting threading: ThreadPoolSize=%d, NSlots=%d", self.num_threads, self.num_slots)
      iProperty("AvalancheSchedulerSvc").ThreadPoolSize = self.num_threads
      iProperty("EventDataSvc").NSlots = self.num_slots
      
      # Set input files for EFInterfaceSvc (overrides what's in DB/JSON config)
      if self.ef_files:
         log.info("Setting EFInterfaceSvc.Files = %s", self.ef_files)
         iProperty("EFInterfaceSvc").Files = self.ef_files
      
      # If HLT PSK is set on command line, read it from DB instead of COOL (ATR-25974)
      # This is the same logic as TrigPSCPythonDbSetup.py
      from TrigPSC import PscConfig
      if PscConfig.forcePSK:
         log.info("PscConfig.forcePSK is set - configuring HLTPrescaleCondAlg to read from DB instead of COOL")
         iProperty("HLTPrescaleCondAlg").Source = "DB"
      
      # Set forceRunNumber on HltEventLoopMgr if conditions_run is specified
      # This overrides the run number used for IOV lookup in conditions loading
      conditions_run = self.run_params.get('conditions_run')
      if conditions_run is not None:
         log.info("Setting HltEventLoopMgr.forceRunNumber=%d for conditions lookup", conditions_run)
         iProperty("HltEventLoopMgr").forceRunNumber = conditions_run
      
      # Initialize
      sc = app.initialize()
      if not sc.isSuccess():
         log.error("Failed to initialize AppMgr")
         return sc
      
      # Initialize TrigServicesHelper for lifecycle calls (prepareForStart, prepareForRun, hltUpdateAfterFork)
      try:
         from TrigServices.TrigServicesHelper import TrigServicesHelper
         helper = TrigServicesHelper()
      except ImportError as e:
         log.error("TrigServicesHelper not available: %s", e)
         log.error("Cannot proceed without TrigServicesHelper - required for HLTEventLoopMgr lifecycle")
         raise RuntimeError("TrigServicesHelper not available") from e
      
      # Call prepareForStart to set up ByteStreamMetadata (like PSC does)
      try:
         run_number = self.run_params['run_number']
         det_mask = self.run_params['detector_mask']
         sor_time = self.run_params['sor_time']
         solenoid_current = self.run_params['solenoid_current']
         toroids_current = self.run_params['toroids_current']
         
         log.info("Calling prepareForStart with run=%d, det_mask=0x%s, sor_time=%s",
                  run_number, det_mask, sor_time)
         
         success = helper.prepareForStart(
            run_number=run_number,
            det_mask=det_mask,
            sor_time=sor_time,
            solenoid_current=solenoid_current,
            toroids_current=toroids_current
         )
         if not success:
            log.error("prepareForStart failed")
            raise RuntimeError("prepareForStart failed")
         log.info("prepareForStart completed successfully")
      except Exception as e:
         log.error("Error calling prepareForStart: %s", e)
         traceback.print_exc()
         raise
      
      # Start
      sc = app.start()
      if not sc.isSuccess():
         log.error("Failed to start AppMgr")
         return sc
      
      # prepareForRun initializes COOL folder helper - must be called after start() 
      # which fires the start incident
      try:
         log.info("Calling prepareForRun to initialize COOL folder helper")
         success = helper.prepareForRun()
         if not success:
            log.error("prepareForRun failed")
            raise RuntimeError("prepareForRun failed")
         log.info("prepareForRun completed successfully")
      except Exception as e:
         log.error("Error calling prepareForRun: %s", e)
         traceback.print_exc()
         raise
      
      # hltUpdateAfterFork initializes the scheduler (like PSC does after fork)
      # worker_id=1 for single-worker, non-forked mode
      try:
         log.info("Calling hltUpdateAfterFork to initialize scheduler (worker_id=1)")
         success = helper.hltUpdateAfterFork(worker_id=1)
         if not success:
            log.error("hltUpdateAfterFork failed")
            raise RuntimeError("hltUpdateAfterFork failed")
         log.info("hltUpdateAfterFork completed successfully")
      except Exception as e:
         log.error("Error calling hltUpdateAfterFork: %s", e)
         traceback.print_exc()
         raise
      
      # Run the event loop
      # Note: Python signal handlers won't work during C++ execution.
      nevt = maxEvents if maxEvents is not None else -1
      sc = app.run(nevt)
      
      if not sc.isSuccess():
         log.error("Failure running application")
         return sc
         
      # Stop
      sc = app.stop()
      if not sc.isSuccess():
         log.error("Failed to stop AppMgr")
         return sc
         
      # Finalize
      sc = app.finalize()
      if not sc.isSuccess():
         log.error("Failed to finalize AppMgr")
         return sc
         
      # Terminate
      sc = app.terminate()
      return sc


def load_from_json(json_file, run_params=None, num_threads=1, num_slots=1, ef_files=None):
   """
   Load configuration from a Gaudi joboptions JSON file.
   
   Returns a ConfigRunner with a run() method that executes the configuration
   using TrigConf::JobOptionsSvc with TYPE="FILE".
   """
   with open(json_file, 'r') as f:
      jocat = json.load(f)
   
   if jocat.get('filetype') != 'joboptions':
      raise ValueError(f"Invalid JSON file type: {jocat.get('filetype')}, expected 'joboptions'")
   
   properties = jocat.get('properties', {})
   return ConfigRunner.from_json(json_file, run_params, properties,
                                  num_threads=num_threads,
                                  num_slots=num_slots,
                                  ef_files=ef_files)


def load_from_database(db_server, smk, l1psk=None, hltpsk=None, run_params=None,
                       num_threads=1, num_slots=1, ef_files=None):
   """
   Load configuration from trigger database using the Super Master Key (SMK).
   
   Returns a ConfigRunner that uses TrigConf::JobOptionsSvc with TYPE="DB"
   to load configuration directly from the database, same as athenaHLT.
   """
   log.info("Loading job options from database %s with SMK %d", db_server, smk)
   return ConfigRunner.from_database(db_server, smk, l1psk, hltpsk, run_params,
                                      num_threads=num_threads,
                                      num_slots=num_slots,
                                      ef_files=ef_files)


##
## The following arg_* methods are used as custom types in argparse
##
def arg_sor_time(s) -> str:
   """Convert possible SOR time arguments to an OWLTime compatible string"""
   fmt = '%d/%m/%y %H:%M:%S.%f'
   if s=='now':        return dt.now().strftime(fmt)
   elif s.isdigit():   return dt.fromtimestamp(float(s)/1e9).strftime(fmt)
   else:               return s

def arg_detector_mask(s):
   """Convert detector mask to format expected by eformat"""
   if s=='all':
      return RunParams.DEFAULT_DETECTOR_MASK
   dmask = hex(int(s,16))                                    # Normalize input to hex-string
   dmask = dmask.lower().replace('0x', '').replace('l', '')  # remove markers
   return '0' * (32 - len(dmask)) + dmask                    # (pad with 0s)

def arg_log_level(s):
   """Argument handler for log levels"""
   lvls = s.split(',')
   if len(lvls)==1: lvls.append('ERROR')
   return lvls


def check_args(parser, args):
   """Consistency check of command line arguments (same as athenaHLT.py)"""

   if not args.jobOptions and not args.use_database:
      parser.error("No job options file specified")

   if not args.file and not args.dump_config_exit:
      parser.error("--file is required unless using --dump-config-exit")

   if args.use_crest and not args.use_database:
      parser.error("--use-crest requires --use-database")


def update_run_params(args, flags):
   """Update run parameters from IS, file, or conditions DB"""

   # If --online-environment is specified, try to read from Information Service first
   if getattr(args, 'online_environment', False):
      log.info("Reading run parameters from Information Service via WEBDAQ")
      # Pass command-line magnet values as overrides (if provided)
      # strict=True ensures we fail if IS read fails, rather than falling back to defaults
      # But if user provided magnet values on command line, those take precedence over IS
      solenoid_override = getattr(args, 'solenoid_current', None)
      toroids_override = getattr(args, 'toroids_current', None)
      
      run_params = get_run_params(from_is=True, 
                                  partition=getattr(args, 'partition', None),
                                  webdaq_base=getattr(args, 'webdaq_base', None),
                                  strict=True,
                                  solenoid_current_override=solenoid_override,
                                  toroids_current_override=toroids_override)
      # Update args with values from IS (if not already set on command line)
      if args.run_number is None and run_params.run_number is not None:
         args.run_number = run_params.run_number
         log.info("Using run_number=%d from IS", args.run_number)
      if args.lb_number is None and run_params.lb_number is not None:
         args.lb_number = run_params.lb_number
         log.info("Using lb_number=%d from IS", args.lb_number)
      if args.sor_time is None and run_params.sor_time is not None:
         args.sor_time = run_params.sor_time
         log.info("Using sor_time=%s from IS", args.sor_time)
      if args.detector_mask is None and run_params.detector_mask is not None:
         args.detector_mask = run_params.detector_mask
         log.info("Using detector_mask=%s from IS", args.detector_mask)
      # Update magnet currents from IS (run_params already has command-line overrides if provided)
      args.solenoid_current = run_params.solenoid_current
      args.toroids_current = run_params.toroids_current

   if (args.run_number is not None and args.lb_number is None) or (args.run_number is None and args.lb_number is not None):
      log.error("Both or neither of the options -R (--run-number) and -L (--lb-number) have to be specified")

   if args.run_number is None and args.file:
      from eformat import EventStorage
      dr = EventStorage.pickDataReader(args.file[0])
      args.run_number = dr.runNumber()
      args.lb_number = dr.lumiblockNumber()

   sor_params = None
   if (args.sor_time is None or args.detector_mask is None) and args.run_number is not None:
      sor_params = AthHLT.get_sor_params(args.run_number)
      log.debug('SOR parameters: %s', sor_params)
      if sor_params is None:
         log.error("Run %d does not exist. If you want to use this run-number specify "
                   "remaining run parameters, e.g.: --sor-time=now --detector-mask=all", args.run_number)
         sys.exit(1)

   if args.sor_time is None and sor_params is not None:
      args.sor_time = arg_sor_time(str(sor_params['SORTime']))

   if args.detector_mask is None and sor_params is not None:
      dmask = sor_params['DetectorMask']
      if args.run_number < AthHLT.CondDB._run2:
         dmask = hex(dmask)
      args.detector_mask = arg_detector_mask(dmask)
   
   # Apply defaults for magnet currents if not set (offline mode only)
   # In online mode, magnets must come from IS or command line (handled above)
   if getattr(args, 'solenoid_current', None) is None:
      args.solenoid_current = RunParams.DEFAULT_SOLENOID_CURRENT
      log.debug("Using default solenoid_current=%.1f", args.solenoid_current)
   if getattr(args, 'toroids_current', None) is None:
      args.toroids_current = RunParams.DEFAULT_TOROIDS_CURRENT
      log.debug("Using default toroids_current=%.1f", args.toroids_current)


def update_trigconf_keys(args, flags):
   """Update trigger configuration keys from OKS, COOL, or CREST.
   
   Priority order:
   1. Command-line arguments (always take precedence)
   2. OKS via WEBDAQ (if --online-environment is set)
   3. CREST (if --use-crest is set)
   4. COOL (default)
   """

   if args.smk is None or args.l1psk is None or args.hltpsk is None:
      trigconf = None
      
      # Try OKS first if --online-environment is set
      if getattr(args, 'online_environment', False):
         log.info("Reading trigger configuration keys from OKS (online environment)")
         # strict=True ensures we fail if OKS read fails, rather than falling back to COOL
         oks_keys = get_trigconf_keys_from_oks(
            partition=getattr(args, 'partition', None),
            webdaq_base=getattr(args, 'webdaq_base', None),
            strict=True
         )
         log.info("Retrieved trigger keys from OKS: %s", oks_keys)
         
         # With strict=True, we're guaranteed to have all keys or an exception was raised
         trigconf = {
            'SMK': oks_keys.get('SMK'),
            'LVL1PSK': oks_keys.get('L1PSK'),
            'HLTPSK': oks_keys.get('HLTPSK')
         }
         # Also update db_server if provided by OKS and not set on command line
         if oks_keys.get('db_alias') and args.db_server == 'TRIGGERDB_RUN3':
            args.db_server = oks_keys['db_alias']
            log.info("Using db_server=%s from OKS", args.db_server)
      
      # Fall back to CREST or COOL only if NOT in online-environment mode
      if trigconf is None:
         if args.use_crest:
            crest_server = args.crest_server or flags.Trigger.crestServer
            log.info("Reading trigger configuration keys from CREST for run %s", args.run_number)
            trigconf = AthHLT.get_trigconf_keys_crest(args.run_number, args.lb_number, crest_server)
            log.info("Retrieved trigger keys from CREST: %s", trigconf)
         else:
            log.info("Reading trigger configuration keys from COOL for run %s", args.run_number)
            trigconf = AthHLT.get_trigconf_keys(args.run_number, args.lb_number)
            log.info("Retrieved trigger keys from COOL: %s", trigconf)

      try:
         if args.smk is None:
            args.smk = trigconf['SMK']
            log.debug("Using SMK=%d from conditions DB/OKS", args.smk)
         else:
            log.debug("Using SMK=%d from command line (ignoring DB/OKS value %s)", args.smk, trigconf.get('SMK'))
         if args.l1psk is None:
            args.l1psk = trigconf['LVL1PSK']
            log.debug("Using L1PSK=%d from conditions DB/OKS", args.l1psk)
         else:
            log.debug("Using L1PSK=%d from command line (ignoring DB/OKS value %s)", args.l1psk, trigconf.get('LVL1PSK'))
         if args.hltpsk is None:
            args.hltpsk = trigconf['HLTPSK']
            log.debug("Using HLTPSK=%d from conditions DB/OKS", args.hltpsk)
         else:
            log.debug("Using HLTPSK=%d from command line (ignoring DB/OKS value %s)", args.hltpsk, trigconf.get('HLTPSK'))
      except KeyError:
         log.error("Cannot read trigger configuration keys from the conditions database for run %d", args.run_number)
         sys.exit(1)
   else:
      log.info("Using trigger configuration keys from command line: SMK=%d, L1PSK=%d, HLTPSK=%d",
               args.smk, args.l1psk, args.hltpsk)


class MyHelp(argparse.Action):
   """Custom help to hide/show expert groups"""
   def __call__(self, parser, namespace, values, option_string=None):

      for g in parser.expert_groups:
         for a in g._group_actions:
            if values!='all':
               a.help = argparse.SUPPRESS

      parser.print_help()
      if values!='all':
         print('\nUse --help=all to show all (expert) options')
      sys.exit(0)


def main():
   parser = argparse.ArgumentParser(prog='athenaEF.py', formatter_class=
                                    lambda prog : argparse.ArgumentDefaultsHelpFormatter(prog, max_help_position=32, width=100),
                                    usage = '%(prog)s [OPTION]... -f FILE jobOptions',
                                    add_help=False)
   parser.expert_groups = []   # Keep list of expert option groups

   ## Global options
   g = parser.add_argument_group('Options')
   g.add_argument('jobOptions', nargs='?', help='job options: CA module (package.module:function), pickle file (.pkl), or JSON file (.json)')
   g.add_argument('--threads', metavar='N', type=int, default=1, help='number of threads')
   g.add_argument('--concurrent-events', metavar='N', type=int, help='number of concurrent events if different from --threads')
   g.add_argument('--log-level', '-l', metavar='LVL', type=arg_log_level, default='INFO,ERROR', help='OutputLevel of athena,POOL')
   g.add_argument('--precommand', '-c', metavar='CMD', action='append', default=[],
                  help='Python commands executed before job options')
   g.add_argument('--postcommand', '-C', metavar='CMD', action='append', default=[],
                  help='Python commands executed after job options')
   g.add_argument('--interactive', '-i', action='store_true', help='interactive mode')
   g.add_argument('--help', '-h', nargs='?', choices=['all'], action=MyHelp, help='show help')

   g = parser.add_argument_group('Input/Output')
   g.add_argument('--file', '--filesInput', '-f', action='append', help='input RAW file')
   g.add_argument('--save-output', '-o', metavar='FILE', help='output file name')
   g.add_argument('--number-of-events', '--evtMax', '-n', metavar='N', type=int, default=-1, help='processes N events (default: -1, means all)')
   g.add_argument('--skip-events', '--skipEvents', '-k', metavar='N', type=int, default=0, help='skip N first events')
   g.add_argument('--loop-files', action='store_true', help='loop over input files if no more events')

   ## Performance and debugging
   g = parser.add_argument_group('Performance and debugging')
   g.add_argument('--perfmon', action='store_true', help='enable PerfMon')
   g.add_argument('--tcmalloc', action='store_true', default=True, help='use tcmalloc')
   g.add_argument('--stdcmalloc', action='store_true', help='use stdcmalloc')
   g.add_argument('--stdcmath', action='store_true', help='use stdcmath library')
   g.add_argument('--imf', action='store_true', default=True, help='use Intel math library')
   g.add_argument('--show-includes', '-s', action='store_true', help='show printout of included files')

   ## Conditions
   g = parser.add_argument_group('Conditions')
   g.add_argument('--run-number', '-R', metavar='RUN', type=int,
                  help='run number (if None, read from first event)')
   g.add_argument('--lb-number', '-L', metavar='LBN', type=int,
                  help='lumiblock number (if None, read from first event)')
   g.add_argument('--conditions-run', metavar='RUN', type=int, default=None,
                  help='reference run number for conditions lookup (use when IS run number has no COOL data)')
   g.add_argument('--sor-time', type=arg_sor_time,
                  help='The Start Of Run time. Three formats are accepted: '
                  '1) the string "now", for current time; '
                  '2) the number of nanoseconds since epoch (e.g. 1386355338658000000 or int(time.time() * 1e9)); '
                  '3) human-readable "20/11/18 17:40:42.3043". If not specified the sor-time is read from the conditions DB')
   g.add_argument('--detector-mask', metavar='MASK', type=arg_detector_mask,
                  help='detector mask (if None, read from the conditions DB), use string "all" to enable all detectors')

   ## Database
   g = parser.add_argument_group('Database')
   g.add_argument('--use-database', '-b', action='store_true',
                  help='configure from trigger database using SMK')
   g.add_argument('--db-server', metavar='DB', default='TRIGGERDB_RUN3', help='DB server name (alias)')
   g.add_argument('--smk', type=int, default=None, help='Super Master Key')
   g.add_argument('--l1psk', type=int, default=None, help='L1 prescale key')
   g.add_argument('--hltpsk', type=int, default=None, help='HLT prescale key')
   g.add_argument('--use-crest', action='store_true', default=False,
                  help='Use CREST for trigger configuration')
   g.add_argument('--crest-server', metavar='URL', default=None,
                  help='CREST server URL (defaults to flags.Trigger.crestServer)')
   g.add_argument('--dump-config', action='store_true', help='Dump joboptions JSON file')
   g.add_argument('--dump-config-exit', action='store_true', help='Dump joboptions JSON file and exit')

   ## Magnet settings
   g = parser.add_argument_group('Magnets')
   g.add_argument('--solenoid-current', type=float, default=None,
                  help='Solenoid current in Amperes (default: nominal current for offline running, required from IS online)')
   g.add_argument('--toroids-current', type=float, default=None,
                  help='Toroids current in Amperes (default: nominal current for offline running, required from IS online)')

   ## Online / Information Service
   g = parser.add_argument_group('Online')
   g.add_argument('--online-environment', action='store_true',
                  help='Enable online environment: read run parameters from IS and trigger '
                       'configuration keys (SMK, L1PSK, HLTPSK) from OKS via WEBDAQ REST API')
   g.add_argument('--partition', metavar='NAME', default=None,
                  help='TDAQ partition name (defaults to TDAQ_PARTITION environment variable)')
   g.add_argument('--webdaq-base', metavar='URL', default=None,
                  help='WEBDAQ base URL (defaults to TDAQ_WEBDAQ_BASE environment variable)')

   ## Online Histogramming
   g = parser.add_argument_group('Online Histogramming')
   g.add_argument('--oh-monitoring', '-M', action='store_true', default=False,
                  help='enable online histogram publishing via WebdaqHistSvc')

   ## Expert options
   g = parser.add_argument_group('Expert')
   parser.expert_groups.append(g)
   (args, unparsed_args) = parser.parse_known_args()
   check_args(parser, args)

   # set ROOT to batch mode (ATR-21890)
   from ROOT import gROOT
   gROOT.SetBatch()

   # set default OutputLevels and file inclusion
   import AthenaCommon.Logging
   AthenaCommon.Logging.log.setLevel(getattr(logging, args.log_level[0]))
   AthenaCommon.Logging.log.setFormat("%(asctime)s  Py:%(name)-31s %(levelname)7s %(message)s")
   if args.show_includes:
      from AthenaCommon.Include import include
      include.setShowIncludes( True )

   # consistency checks for arguments
   if not args.concurrent_events:
      args.concurrent_events = args.threads

   # Update args and set athena flags
   from TrigPSC import PscConfig
   from TrigPSC.PscDefaultFlags import defaultOnlineFlags
   
   # Get flags with online defaults (same as athenaHLT)
   flags = defaultOnlineFlags()

   # Enable WebdaqHistSvc for online histogram publishing if requested
   if args.oh_monitoring:
      flags.Trigger.Online.useOnlineWebdaqHistSvc = True
      log.info("Enabled WebdaqHistSvc for online histogram publishing")

   # CREST configuration (same as athenaHLT)
   log.info("Using CREST for trigger configuration: %s", args.use_crest)
   if args.use_crest:
      flags.Trigger.useCrest = True
      if args.crest_server:
         flags.Trigger.crestServer = args.crest_server
      else:
         args.crest_server = flags.Trigger.crestServer

   update_run_params(args, flags)

   if args.use_database:
      # If HLTPSK was given on the command line OR from OKS (--online-environment),
      # we ignore what is stored in COOL and use the specified key directly from the DB.
      # This is needed because COOL may point to a different HLTPSK for the forced run number.
      PscConfig.forcePSK = (args.hltpsk is not None) or args.online_environment
      # Read trigger config keys from COOL/OKS if not specified
      update_trigconf_keys(args, flags)

   # Fill flags from command line (if not running from DB/JSON)
   if not args.use_database and args.jobOptions and not args.jobOptions.endswith('.json'):
      PscConfig.unparsedArguments = unparsed_args
      for flag_arg in unparsed_args:
         flags.fillFromString(flag_arg)

   PscConfig.interactive = args.interactive
   PscConfig.exitAfterDump = args.dump_config_exit

   # NOTE: Do NOT set flags.Input.Files here!
   # Like athenaHLT, we keep Input.Files=[] during configuration to ensure the
   # configuration is portable and doesn't depend on specific input file metadata.
   # Input files are passed to EFInterface for runtime use only.

   # Set conditions run number override (for test partitions with fake run numbers)
   if args.conditions_run is not None:
      log.info("Using conditions from reference run %d (overriding run %s for IOV lookup)",
               args.conditions_run, args.run_number)
      flags.Input.ConditionsRunNumber = args.conditions_run

   # Set number of events
   if args.number_of_events > 0:
      flags.Exec.MaxEvents = args.number_of_events

   # Set skip events  
   if args.skip_events > 0:
      flags.Exec.SkipEvents = args.skip_events

   # NOTE: Do NOT set flags.Concurrency.NumThreads or NumConcurrentEvents here.
   # Threading is set at runtime via iProperty after configure() - see ConfigRunner.run()

   # Enable PerfMon if requested
   flags.PerfMon.doFastMonMT = args.perfmon

   # Configure EF ByteStream services (mandatory to run without HLTMPPU)
   # This provides the data flow interface that would normally come from HLTMPPU
   flags.Trigger.Online.useEFByteStreamSvc = True
   ef = flags.Trigger.Online.EFInterface
   ef_files = args.file if args.file else []
   ef.Files        = ef_files
   ef.LoopFiles    = args.loop_files
   ef.NumEvents    = args.number_of_events
   ef.SkipEvents   = args.skip_events
   ef.RunNumber    = args.run_number

   # Execute precommands
   if args.precommand:
      log.info("Executing precommand(s)")
      for cmd in args.precommand:
         log.info("  %s", cmd)
         exec(cmd, globals(), {'flags': flags})

   # Determine input type
   is_database = args.use_database
   is_pickle = False
   is_json = False
   
   if not is_database and args.jobOptions:
      jobOptions = args.jobOptions
      is_pickle = jobOptions.endswith('.pkl')
      is_json = jobOptions.endswith('.json')

   if is_database:
      # Load configuration from trigger database
      # Handle CREST vs standard DB access (same as athenaHLT)
      if args.use_crest:
         crestconn = TriggerCrestUtil.getCrestConnection(args.db_server)
         db_alias = f"{args.crest_server}/{crestconn}"
         log.info("Loading configuration via CREST from %s with SMK %d", db_alias, args.smk)
      else:
         db_alias = args.db_server
         log.info("Loading configuration from database %s with SMK %d", db_alias, args.smk)
      
      # Get run parameters for prepareForStart
      run_params = get_run_params(args).to_dict()
      acc = load_from_database(db_alias, args.smk, args.l1psk, args.hltpsk, run_params,
                               num_threads=args.threads, num_slots=args.concurrent_events,
                               ef_files=ef_files)
      log.info("Configuration loaded from database")

   elif is_pickle:
      # Load ComponentAccumulator from pickle file
      log.info("Loading configuration from pickle file: %s", jobOptions)
      with open(jobOptions, 'rb') as f:
         acc = pickle.load(f)
      log.info("Configuration loaded from pickle")

   elif is_json:
      # Load configuration from JSON file
      log.info("Loading configuration from JSON file: %s", jobOptions)
      # Get run parameters for prepareForStart
      run_params = get_run_params(args).to_dict()
      acc = load_from_json(jobOptions, run_params,
                           num_threads=args.threads, num_slots=args.concurrent_events,
                           ef_files=ef_files)
      log.info("Configuration loaded from JSON")

   else:
      # Load from CA module - follow the same pattern as athenaHLT/TrigPSCPythonCASetup:
      # 1. Build the full configuration with services
      # 2. Dump to JSON file
      # 3. Use TrigConf::JobOptionsSvc to load from JSON
      # This preserves the ability to use the same JobOptionsSvc as athenaHLT
      log.info("Loading CA configuration from: %s", jobOptions)
      
      # Clone and lock flags for services configuration (as done in TrigPSCPythonCASetup)
      from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
      from AthenaConfiguration.MainServicesConfig import addMainSequences
      from TrigServices.TriggerUnixStandardSetup import commonServicesCfg
      from AthenaConfiguration.ComponentFactory import CompFactory
      
      locked_flags = flags.clone()
      locked_flags.lock()
      
      # Create base CA with framework services (like TrigPSCPythonCASetup)
      cfg = ComponentAccumulator(CompFactory.AthSequencer("AthMasterSeq", Sequential=True))
      cfg.setAppProperty('ExtSvcCreates', False)
      cfg.setAppProperty("MessageSvcType", "TrigMessageSvc")
      cfg.setAppProperty("JobOptionsSvcType", "TrigConf::JobOptionsSvc")
      
      # Add main sequences and common services (includes TrigServicesCfg)
      addMainSequences(locked_flags, cfg)
      cfg.merge(commonServicesCfg(locked_flags))
      
      # Now merge user CA config (with unlocked flags, as in TrigPSCPythonCASetup)
      cfg_func = AthHLT.getCACfg(jobOptions)
      cfg.merge(cfg_func(flags))
      
      # Execute postcommands before dumping (like TrigPSCPythonCASetup)
      if args.postcommand:
         log.info("Executing postcommand(s)")
         for cmd in args.postcommand:
            log.info("  %s", cmd)
            exec(cmd, globals(), {'flags': flags, 'cfg': cfg})
         args.postcommand = []  # Clear so we don't run them again later
      
      # Dump configuration to JSON (like TrigPSCPythonCASetup)
      fname = "HLTJobOptions_EF"
      log.info("Dumping configuration to %s.pkl and %s.json", fname, fname)
      with open(f"{fname}.pkl", "wb") as f:
         cfg.store(f)
      
      from TrigConfIO.JsonUtils import create_joboptions_json
      create_joboptions_json(f"{fname}.pkl", f"{fname}.json")
      
      # Check for dump-and-exit
      if args.dump_config_exit:
         log.info("Configuration dumped to %s.json. Exiting...", fname)
         sys.exit(0)
      
      # Now load the JSON using JsonConfigRunner with TrigConf::JobOptionsSvc
      log.info("Loading configuration from %s.json via TrigConf::JobOptionsSvc", fname)
      # Get run parameters for prepareForStart
      run_params = get_run_params(args).to_dict()
      acc = load_from_json(f"{fname}.json", run_params,
                           num_threads=args.threads, num_slots=args.concurrent_events,
                           ef_files=ef_files)
      
      log.info("Configuration loaded with HLT online services")
   
   # Execute postcommands
   if args.postcommand:
      log.info("Executing postcommand(s)")
      for cmd in args.postcommand:
         log.info("  %s", cmd)
         exec(cmd, globals(), {'flags': flags, 'acc': acc})

   # Dump configuration if requested
   if args.dump_config or args.dump_config_exit:
      fname = "HLTJobOptions_EF"
      
      if is_database:
         # For DB mode, fetch properties via Python API
         from TrigConfIO.HLTTriggerConfigAccess import HLTJobOptionsAccess
         log.info("Fetching configuration from database for dump...")
         jo_access = HLTJobOptionsAccess(dbalias=acc.db_server, smkey=acc.smk)
         props = jo_access.algorithms()
         
         log.info("Dumping configuration to %s.json", fname)
         hlt_json = {'filetype': 'joboptions', 'properties': props}
         with open(f"{fname}.json", "w") as f:
            json.dump(hlt_json, f, indent=4, sort_keys=True, ensure_ascii=True)
            
      elif is_json:
         # For JSON mode, properties were already loaded
         props = acc.properties
         if props:
            log.info("Dumping configuration to %s.json", fname)
            hlt_json = {'filetype': 'joboptions', 'properties': props}
            with open(f"{fname}.json", "w") as f:
               json.dump(hlt_json, f, indent=4, sort_keys=True, ensure_ascii=True)
         else:
            log.warning("No properties available to dump")
            
      elif is_pickle:
         # For pickle-loaded ComponentAccumulator, gather properties
         app_props, msg_props, comp_props = acc.gatherProps()
         props = {"ApplicationMgr": app_props, "MessageSvc": msg_props}
         for comp, name, value in comp_props:
            props.setdefault(comp, {})[name] = value
         
         log.info("Dumping configuration to %s.json", fname)
         hlt_json = {'filetype': 'joboptions', 'properties': props}
         with open(f"{fname}.json", "w") as f:
            json.dump(hlt_json, f, indent=4, sort_keys=True, ensure_ascii=True)
      
      # Note: For CA module, dumping is already handled earlier 
      # before converting to ConfigRunner
      
      if args.dump_config_exit:
         log.info("Configuration dumped. Exiting...")
         sys.exit(0)

   # Run the application directly (like athena.py does)
   log.info("Starting Athena execution...")
   
   # Create worker directory structure that HLT services expect
   # (normally created by HLTMPPU/PSC). Worker ID 1 means single-worker, non-forked mode
   # and must match what we pass to hltUpdateAfterFork(worker_id=1) in ConfigRunner.run()
   worker_dir = os.path.join(os.getcwd(), "athenaHLT_workers", "athenaHLT-01")
   if not os.path.exists(worker_dir):
      log.info("Creating worker directory: %s", worker_dir)
      os.makedirs(worker_dir, exist_ok=True)
   
   if args.interactive:
      log.info("Interactive mode - call acc.run() to execute")
      import code
      code.interact(local={'acc': acc, 'flags': flags})
   else:
      # Run the application
      from AthenaCommon import ExitCodes
      exitcode = 0
      try:
         # Pass maxEvents if explicitly set (including -1 for all events)
         sc = acc.run(args.number_of_events)
         if sc.isFailure():
            exitcode = ExitCodes.EXE_ALG_FAILURE
      except SystemExit as e:
         exitcode = ExitCodes.EXE_ALG_FAILURE if e.code == 1 else e.code
      except Exception:
         traceback.print_exc()
         exitcode = ExitCodes.UNKNOWN_EXCEPTION

      log.info('Leaving with code %d: "%s"', exitcode, ExitCodes.what(exitcode))
      sys.exit(exitcode)


if "__main__" in __name__:
   sys.exit(main())
