#!/usr/bin/env python
#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging
log = logging.getLogger('RunTrigCostAnalysis.py')


# Configure Cost Analysis algorithm
def trigCostAnalysisCfg(flags, args, isMC=False):
  from TrigCostAnalysis.ROSToROB import ROSToROBMap

  acc = ComponentAccumulator()

  if len(flags.Input.RunNumbers) > 1:
    log.error('Multiple run numbers from metadata! Only one expected per cost processing')
    return acc

  enhancedBiasWeighter = CompFactory.EnhancedBiasWeighter()
  enhancedBiasWeighter.RunNumber = flags.Input.RunNumbers[0]
  enhancedBiasWeighter.UseBunchCrossingData = False
  enhancedBiasWeighter.IsMC = isMC
  if isMC:
    MCpayload = readMCpayload(args)
    enhancedBiasWeighter.MCCrossSection = MCpayload.get('MCCrossSection')
    enhancedBiasWeighter.MCFilterEfficiency = MCpayload.get('MCFilterEfficiency')
    enhancedBiasWeighter.MCKFactor = MCpayload.get('MCKFactor')
    enhancedBiasWeighter.MCIgnoreGeneratorWeights = MCpayload.get('MCIgnoreGeneratorWeights')

  trigCostAnalysis = CompFactory.TrigCostAnalysis()
  trigCostAnalysis.OutputLevel = args.loglevel
  trigCostAnalysis.BaseEventWeight = args.baseWeight
  trigCostAnalysis.EnhancedBiasTool = enhancedBiasWeighter
  trigCostAnalysis.AlgToChainTool = CompFactory.getComp("TrigCompositeUtils::AlgToChainTool")()
  trigCostAnalysis.UseEBWeights = args.useEBWeights
  trigCostAnalysis.MaxFullEventDumps = 100
  trigCostAnalysis.FullEventDumpProbability = 1 # X. Where probability is 1 in X
  trigCostAnalysis.UseSingleTimeRange = isMC or args.useEBWeights
  trigCostAnalysis.ROSToROBMap = ROSToROBMap().get_mapping()
  trigCostAnalysis.DoMonitorChainAlgorithm = args.monitorChainAlgorithm

  if not isMC:
    trigCostAnalysis.AdditionalHashList = readHashesFromHLTJO(args.joFile, args.smk, args.dbAlias)
  else:
    log.debug("Hashes from the HLTJO won't be retrieved for MC job")
    trigCostAnalysis.AdditionalHashList = list()

  acc.addEventAlgo(trigCostAnalysis)

  return acc

# Prepare dictionary with MC parameters read from arguments, or from AMI
def readMCpayload(args):
  payload = {}

  payload['MCCrossSection'] = args.MCCrossSection
  payload['MCFilterEfficiency'] = args.MCFilterEfficiency
  payload['MCKFactor'] = args.MCKFactor
  payload['MCIgnoreGeneratorWeights'] = args.MCIgnoreGeneratorWeights

  dset = args.MCDatasetName
  if payload['MCCrossSection'] == 0: # If the input file is MC then make sure we have the needed info
    from RatesAnalysis.GetCrossSectionAMITool import GetCrossSectionAMI
    amiTool = GetCrossSectionAMI()
    if dset == '': # Can we get the dataset name from the input file path?
      dset = amiTool.getDatasetNameFromPath(flags.Input.Files[0])
    amiTool.queryAmi(dset)
    payload['MCCrossSection'] = amiTool.crossSection
    payload['MCFilterEfficiency'] = amiTool.filterEfficiency

  return payload

# Read algorithm and class names from HLTJobOptions file
def readHashesFromHLTJO(joFileName="", smk=0, dbAlias=""):
  joData = {}
  try:
    if joFileName:
      log.debug("Reading HLTJobOptions from file {0}".format(joFileName))
      with open(joFileName, "r") as joFile:
        import json
        joData = json.load(joFile)
    elif smk and dbAlias:
      log.debug("Reading HLTJobOptions from database {0} {1}".format(smk, dbAlias))
      from TrigConfIO.HLTTriggerConfigAccess import HLTJobOptionsAccess
      joData = HLTJobOptionsAccess(dbalias = dbAlias, smkey = smk)
    else:
      log.debug("Additional collections' names from HLTJobOptions file are not available")
      return list()
  except Exception as err:
    log.warning("Retrieving additional names fron HLTJO failed: {0}".format(err))
    return list()

  namesList = set()
  for entry in joData["properties"]:
    namesList.add(entry.split('.')[0])

    # Read algorithm names with classes
    entryObj = joData["properties"][entry]
    if "Members" in entryObj:
      membersList = entryObj["Members"].strip('][').replace("'", "").split(', ')
      namesList.update(membersList)


  log.info("Retrieved {0} additional names from HLT JO file".format(len(namesList)))
  return list(namesList)


# Configure deserialisation
def decodingCfg(flags):
  from AthenaCommon.CFElements import seqAND
  from TrigEDMConfig import DataScoutingInfo

  acc = ComponentAccumulator()

  acc.addSequence(seqAND("Decoding"))
  acc.addEventAlgo(CompFactory.HLTResultMTByteStreamDecoderAlg(), "Decoding")

  costDataDeserialiser = CompFactory.TriggerEDMDeserialiserAlg("CostDataTrigDeserialiser")
  costDataDeserialiser.ModuleID = DataScoutingInfo.getDataScoutingResultID("CostMonDS")
  acc.addEventAlgo(costDataDeserialiser, "Decoding")

  return acc


# Configure HLTConfigSvc with JSON Menu file
def hltConfigSvcCfg(flags, smk, dbAlias):
  acc = ComponentAccumulator()

  hltConfigSvc = CompFactory.getComp("TrigConf::HLTConfigSvc")("HLTConfigSvc")

  menuFile = getHltMenu()
  # If local file not found - read HLTMenu from database
  if menuFile:
    log.debug("Reading HLTMenu from file {0}".format(menuFile))

    hltConfigSvc.InputType = "FILE"
    hltConfigSvc.HLTJsonFileName = menuFile
  elif smk and dbAlias:
    log.debug("Reading HLTMenu from database {0} {1}".format(smk, dbAlias))

    hltConfigSvc.InputType = "DB"
    hltConfigSvc.HLTJsonFileName = ""
    hltConfigSvc.TriggerDB = dbAlias
    hltConfigSvc.SMK = smk
  else:
    log.error("Cannot read the HLTMenu! Provide file or relevant keys.")

  acc.addService(hltConfigSvc, False, True)

  return acc


def readConfigFromCool(flags, smk, dbAlias):
  # Try to read keys from COOL (for P1 data)
  from TrigConfStorage.TriggerCoolUtil import TriggerCoolUtil
  dbconn = TriggerCoolUtil.GetConnection("CONDBR2")
  runNumber = flags.Input.RunNumbers[0]
  configKeys = TriggerCoolUtil.getHLTConfigKeys(dbconn, [[runNumber, runNumber]])

  log.debug("Getting keys from COOL for run {0}".format(runNumber))
  if configKeys and runNumber in configKeys.keys():
    if not smk:
      smk = configKeys[runNumber]['SMK']

    if not dbAlias:
      dbAlias = configKeys[runNumber]['DB']

    log.debug("Config keys are SMK: {0} DB alias: {1}".format(smk, dbAlias))

  else:
    log.debug("Configuration keys for run {0} not found!".format(runNumber))
    dbAlias = None if not dbAlias else dbAlias
    smk = None if not smk else smk

  return (smk, dbAlias)


# Get HLT Menu from json file
def getHltMenu():
  # Try to find local menu file
  menuFileName = 'HLTMenu_.*json'

  import re, os
  r = re.compile(menuFileName)
  menuFiles = list(filter(r.match, os.listdir('.')))

  if len(menuFiles) > 1:
    log.info("Found more than one menu file! Saving first match %s", menuFiles[0])

  return menuFiles[0] if len(menuFiles) > 0 else ""


if __name__=='__main__':
  # Set the Athena configuration flags
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags = initConfigFlags()

  # Add specific command-line arguments
  parser = flags.getArgumentParser
  parser.add_argument('--outputHist', type=str, default='TrigCostRoot_Results.root', help='Histogram output ROOT file')
  parser.add_argument('--monitorChainAlgorithm', action='store_true', help='Turn on Chain Algorithm monitoring')
  parser.add_argument('--baseWeight', type=float, default=1.0, help='Base events weight')
  parser.add_argument('--useEBWeights', type=bool, default=False, help='Apply Enhanced Bias weights')
  parser.add_argument('--joFile', type=str, help='Optional HLTJobOptions file to add more hashes')

  parser.add_argument('--smk', type=int, help='SuperMasterKey to retrieve menu file')
  parser.add_argument('--dbAlias', type=str, help='Database alias to retrieve menu file')

  parser.add_argument('--MCDatasetName', default='', type=str, help='For MC input: Name of the dataset, can be used instead of MCCrossSection, MCFilterEfficiency')
  parser.add_argument('--MCCrossSection', default=0.0, type=float, help='For MC input: Cross section of process in nb')
  parser.add_argument('--MCFilterEfficiency', default=1.0, type=float, help='For MC input: Filter efficiency of any MC filter (0.0 - 1.0)')
  parser.add_argument('--MCKFactor', default=1.0, type=float, help='For MC input: Additional multiplicitive fudge-factor to the supplied cross section.')
  parser.add_argument('--MCIgnoreGeneratorWeights', action='store_true', help='For MC input: Flag to disregard any generator weights.')

  args = parser.parse_args()

  log.level = args.loglevel

  # verbosity defined in Control/AthenaCommon/python/Constants.py
  flags.Exec.OutputLevel = args.loglevel
  flags.fillFromArgs(args.flags)
  flags.lock()

  # Initialize configuration object, add accumulator, merge, and run.
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
  cfg = MainServicesCfg(flags)

  if flags.Input.isMC:
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))
  else:
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge(ByteStreamReadCfg(flags))
    cfg.merge(decodingCfg(flags))

  histSvc = CompFactory.THistSvc()
  histSvc.Output += ["COSTSTREAM DATAFILE='" + args.outputHist + "' OPT='RECREATE'"]
  cfg.addService(histSvc)

  # Retrieve config from cool database
  if not flags.Input.isMC and (not args.smk or not args.dbAlias):
    (args.smk, args.dbAlias) = readConfigFromCool(flags, args.smk, args.dbAlias)

  cfg.merge(hltConfigSvcCfg(flags, args.smk, args.dbAlias))
  cfg.merge(trigCostAnalysisCfg(flags, args, flags.Input.isMC))

  # If you want to turn on more detailed messages ...
  # exampleMonitorAcc.getEventAlgo('ExampleMonAlg').OutputLevel = 2 # DEBUG
  cfg.printConfig(withDetails=False) # set True for exhaustive info
  sc = cfg.run(args.maxEvents)
  import sys
  sys.exit(0 if sc.isSuccess() else 1)
