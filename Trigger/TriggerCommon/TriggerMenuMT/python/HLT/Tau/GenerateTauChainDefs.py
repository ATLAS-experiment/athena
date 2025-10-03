# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

###########################################################################
# SliceDef file for Tau chains
###########################################################################

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
logging.getLogger().info("Importing %s",__name__)

from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDict
from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs
from .TauChainConfiguration import TauChainConfiguration
from ..Ditau.DitauChainConfiguration import DitauChainConfiguration
from ..Jet.JetChainConfiguration import JetChainConfiguration
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def generateJetChainConfigs(flags, subChainDict):
    jet_cfg = JetChainConfiguration(subChainDict)
    jet_cfg.prepareDataDependencies(flags)
    jet = jet_cfg.assembleChain(flags)
    jet_name = jet_cfg.jetName
    return jet, jet_name

def generateChainConfigs(flags, chainDict, perSig_lengthOfChainConfigs):

    
    listOfChainDicts = splitChainDict(chainDict)
    listOfChainDefs=[]

    for subChainDict in listOfChainDicts:
        log.debug('Assembling subChainsDict %s for chain %s', len(listOfChainDefs), subChainDict['chainName'] )        
        if subChainDict['sigDicts']['Tau'][0] == 'Tau':
            Tau = TauChainConfiguration(subChainDict).assembleChain(flags)
            listOfChainDefs += [Tau]
        if subChainDict['sigDicts']['Tau'][0] == 'Ditau':
            Jet, jet_name = generateJetChainConfigs(flags, subChainDict)
            Ditau = DitauChainConfiguration(subChainDict, jet_name).assembleChain(flags) 
            Jet.append_step_to_jet(Ditau.steps)
            listOfChainDefs += [Jet]
        

    if len(listOfChainDefs)>1:
        theChainDef, perSig_lengthOfChainConfigs = mergeChainDefs(listOfChainDefs, chainDict, perSig_lengthOfChainConfigs)

    else:
        theChainDef = listOfChainDefs[0]

    log.debug("theChainDef: %s" , theChainDef)
    return theChainDef, perSig_lengthOfChainConfigs


def prepareDefaultSignatureFlags(inflags : AthConfigFlags) -> AthConfigFlags:
    """
    invoked before generateChainConfigs method to prevent repeated cloning of flags within chain generation
    """
    from TrigInDetConfig.utils import cloneFlagsToActiveConfig
    flags = cloneFlagsToActiveConfig(inflags, "tauIso", log)      #tauIso is most frequently invoked
    return flags

