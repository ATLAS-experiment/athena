#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging


#
# Search ConfigFlags.Input.Collections, allow sub-string matches (e.g. handle extra classname mangling in ESD files) 
#
def isCollectionInInputPOOLFile(flags, checkString):
  return any(checkString in entry for entry in flags.Input.Collections)

#
# Return an instance of the TrigNavSlimmingMTAlg to be used in an online environment, either at P1,
# during standalone execution of the trigger on data, or during the RDOtoRDOTrigger step for MC.
#
# Returns a single alg with no dependencies, no need to make this function return a ComponentAccumulator.
#
def getTrigNavSlimmingMTOnlineConfig(flags):
  onlineSlim = CompFactory.TrigNavSlimmingMTAlg('TrigNavSlimmingMTAlg_Online')
  onlineSlim.TrigDecisionTool = "" # We do NOT filter on chains online, no additional tools/services required.
  onlineSlim.OutputCollection = "HLTNav_Summary_OnlineSlimmed"
  onlineSlim.PrimaryInputCollection = "HLTNav_Summary"
  onlineSlim.KeepFailedBranched = True
  onlineSlim.KeepOnlyFinalFeatures = False
  onlineSlim.RemoveEmptySteps = False
  onlineSlim.EdgesToDrop = []
  onlineSlim.NodesToDrop = []
  onlineSlim.ChainsFilter = []
  onlineSlim.RuntimeValidation = flags.Trigger.doRuntimeNaviVal
  return onlineSlim

#
# Return an instance of the TrigNavSlimmingMTAlg to be used during AOD->DAOD
#
# Any additional DAOD->DAOD will not change the slimmed navigation collection.
#
# The navigation graph is reduced to only refer to the final-feature and initial-ROI,
# all nodes from intermediate steps between these two are dropped.
# All branches corresponding to chains which failed the trigger are dropped.
#
# A chainsFilter is optionally supplied. All graph nodes which are not active for at least one chain
# supplied on this filter are dropped. (Assuming a non-empty list was supplied).
#
# Trigger features are reduced to xAOD::Particle representation only, all their 4-vectors
# are packed into a single compact container. Separate containers are used to pack features
# which do not derive from IParticle, e.g. MET. The RoIs are similarly repacked.
#
# NOTE: Unlike all other levels, the content of the DAOD is not controlled by TrigEDMRun3.py
# NOTE: We therefore also need to run AddRun3TrigNavSlimmingCollectionsToEDM to register the outputs to a SlimmingHelper
#
def TrigNavSlimmingMTDerivationCfg(flags, chainsFilter = []):

  log = logging.getLogger("TrigNavSlimmingMTDerivationCfg.py")

  if isCollectionInInputPOOLFile(flags, "HLTNav_Summary_DAODSlimmed"):
    log.info("Will not create a new DAOD Slimmed Trigger Navigation Collection in this job (already present in input file)")
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    return ComponentAccumulator()

  from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg, getRun3NavigationContainerFromInput, possible_keys
  ca = TrigDecisionToolCfg(flags)
  tdt = ca.getPrimary()

  inputCollection = getRun3NavigationContainerFromInput(flags)

  daodSlim = CompFactory.TrigNavSlimmingMTAlg('TrigNavSlimmingMTAlg_DAOD')
  daodSlim.TrigDecisionTool = tdt
  daodSlim.OutputCollection = "HLTNav_Summary_DAODSlimmed"
  daodSlim.PrimaryInputCollection = inputCollection
  daodSlim.AllOutputContainers = possible_keys
  daodSlim.KeepFailedBranched = False
  daodSlim.KeepOnlyFinalFeatures = True
  daodSlim.RemoveEmptySteps = True
  daodSlim.RepackROIs = False # CAUTION: There may be an ROI memory management issue which needs solving in order to enable this functionality (ATLASG-1662).
  daodSlim.RepackROIsOutputCollection = "HLTNav_RepackedROIs"
  daodSlim.RepackMET = True # To check: Is there any analysis need to have online-MET(s) in DAOD?
  daodSlim.RepackFeatures = True

  daodSlim.RepackFeaturesOutputCollection_Particle = "HLTNav_RepackedFeatures_Particle"
  daodSlim.RepackFeaturesOutputCollection_MET = "HLTNav_RepackedFeatures_MET"
  daodSlim.EdgesToDrop = ["view"] # View element links, only useful online.
  daodSlim.NodesToDrop = ["F", "CH"] # Filter nodes, only useful online. CH=ComboHypo nodes, not useful given we run here with KeepFailedBranched=False 
  daodSlim.ChainsFilter = chainsFilter
  ca.addEventAlgo(daodSlim)

  log.info("Producing DAOD Slimmed Trigger Navigation Collection. Reading {} and writing {}".format(daodSlim.PrimaryInputCollection, daodSlim.OutputCollection))

  if daodSlim.OutputCollection not in possible_keys:
    log.error("Producing a collection {} which is not listed in 'possible_keys'! Add this here too.".format(daodSlim.OutputCollection))

  return ca

# Same as the above but adds the branches to the slimming helper. 
# This is the component accumulator version
def AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(slimmingHelper):
  slimmingHelper.AppendToDictionary.update({'HLTNav_Summary_DAODSlimmed':'xAOD::TrigCompositeContainer','HLTNav_Summary_DAODSlimmedAux':'xAOD::TrigCompositeAuxContainer',
                                            'HLTNav_RepackedFeatures_Particle':'xAOD::ParticleContainer','HLTNav_RepackedFeatures_ParticleAux':'xAOD::ParticleAuxContainer',
                                            'HLTNav_RepackedFeatures_MET':'xAOD::TrigMissingETContainer','HLTNav_RepackedFeatures_METAux':'xAOD::TrigMissingETAuxContainer',
                                            'HLTNav_RepackedROIs':'TrigRoiDescriptorCollection'})

  slimmingHelper.AllVariables += ['HLTNav_Summary_DAODSlimmed',
                                  'HLTNav_RepackedFeatures_Particle',
                                  'HLTNav_RepackedFeatures_MET',
                                  'HLTNav_RepackedROIs']

#
# Return an ComponentAccumulator which configures trigger navigation slimming during 
# RAW->ALL, RAW->ESD or ESD->AOD (and MC equivalents)
#
# The collections should only be created once, if already present in an input POOL file then we do not re-create.
#
# The ESD level data contains a minimal slimming on top of the Online slimming. 
# Some graph nodes and edges which were only of use online are removed.
# This level of slimming is designed to support T0 trigger monitoring.
#
# If the job is configured to produce AODFULL level output, then the ESD level graph is saved also to the AOD.
# Hence T0 trigger monitoring is also supported in AODFULL as it is in ESD.
#
# If the job is configured to produce AODSLIM level output, then the graph is additionally slimmed.
# The navigation graph is reduced to only refer to the final-feature and initial-ROI,
# all nodes from intermediate steps between these two are dropped.
# All branches corresponding to chains which failed the trigger are dropped.
# This is insufficient for T0 monitoring, but retains the information needed for trigger-matching for analyses.
# Unlike at DAOD level, in AODRun3_SMALL slimmed we do still keep reference to the actual trigger features 
# (in case we needed to do offline trigger matching with more than the 4-vector, for example) and no chainsFilter
# is applied.
#
def TrigNavSlimmingMTCfg(flags):

  log = logging.getLogger("TrigNavSlimmingMTCfg.py")
  
  from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg, getRun3NavigationContainerFromInput, possible_keys
  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  
  ca = ComponentAccumulator()

  # We only run this if we are in a job which is decoding the HLT data in reconstruction, and if the slimming has not been switched off
  if flags.Trigger.DecodeHLT is False:
    log.debug("Nothing to do as Trigger.DecodeHLT is False")
    return ca

  if flags.Trigger.doNavigationSlimming is False:
    log.debug("Nothing to do as Trigger.doNavigationSlimming is False")
    return ca

  doESD = flags.Output.doWriteESD
  doAOD = flags.Output.doWriteAOD

  # NOTE: Derivations currently have a different configuration hook, see TrigNavSlimmingMTDerivationCfg above.

  inputCollection = getRun3NavigationContainerFromInput(flags)
  doESDSlim = not isCollectionInInputPOOLFile(flags, "HLTNav_Summary_ESDSlimmed")
  doAODSlim = not isCollectionInInputPOOLFile(flags, "HLTNav_Summary_AODSlimmed")

  if doESD and doESDSlim:
    tdt = ca.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    esdSlim = CompFactory.TrigNavSlimmingMTAlg('TrigNavSlimmingMTAlg_ESD')
    esdSlim.TrigDecisionTool = tdt
    esdSlim.OutputCollection = "HLTNav_Summary_ESDSlimmed"
    esdSlim.PrimaryInputCollection = inputCollection
    esdSlim.AllOutputContainers = possible_keys
    esdSlim.KeepFailedBranched = True
    esdSlim.KeepOnlyFinalFeatures = False
    esdSlim.RemoveEmptySteps = False
    esdSlim.RepackROIs = False
    esdSlim.RepackFeatures = False
    esdSlim.EdgesToDrop = ["view"]
    esdSlim.NodesToDrop = ["F"]
    esdSlim.ChainsFilter = []
    ca.addEventAlgo(esdSlim)
    log.info("Producing ESD Slimmed Trigger Navigation Collection. Reading {} and writing {}".format(esdSlim.PrimaryInputCollection, esdSlim.OutputCollection))
    if esdSlim.OutputCollection not in possible_keys:
      log.error("Producing a collection {} which is not listed in 'possible_keys'! Add this here too.".format(esdSlim.OutputCollection))
  else:
    log.info("Will not create ESD Slimmed Trigger Navigation Collection in this job")

  if doAOD and doAODSlim:
    tdt = ca.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    aodSlim = CompFactory.TrigNavSlimmingMTAlg('TrigNavSlimmingMTAlg_AOD')
    aodSlim.TrigDecisionTool = tdt
    aodSlim.OutputCollection = "HLTNav_Summary_AODSlimmed"
    aodSlim.PrimaryInputCollection = inputCollection
    aodSlim.AllOutputContainers = possible_keys
    aodSlim.RemoveEmptySteps = False
    aodSlim.RepackROIs = False
    aodSlim.RepackFeatures = False
    aodSlim.EdgesToDrop = ["view"]
    aodSlim.NodesToDrop = ["F"]
    aodSlim.ChainsFilter = []
    #
    if flags.Trigger.AODEDMSet == "AODFULL":
      aodSlim.KeepFailedBranched = True
      aodSlim.KeepOnlyFinalFeatures = False
      log.info("Producing AODFULL Slimmed Trigger Navigation Collection. Reading {} and writing {}".format(aodSlim.PrimaryInputCollection, aodSlim.OutputCollection))
    else:
      aodSlim.KeepFailedBranched = False
      aodSlim.KeepOnlyFinalFeatures = True
      log.info("Producing AODSLIM Trigger Navigation Collection. Reading {} and writing {}".format(aodSlim.PrimaryInputCollection, aodSlim.OutputCollection))
    ca.addEventAlgo(aodSlim)
    if aodSlim.OutputCollection not in possible_keys:
      log.error("Producing a collection {} which is not listed in 'possible_keys'! Add this here too.".format(esdSlim.OutputCollection))
  else:
    log.info("Will not create AOD Slimmed Trigger Navigation Collection in this job")

  return ca
