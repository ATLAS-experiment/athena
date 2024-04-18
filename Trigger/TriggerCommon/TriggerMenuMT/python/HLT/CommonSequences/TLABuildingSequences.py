#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
import functools
from TriggerMenuMT.HLT.Config.MenuComponents import ChainStep
from AthenaCommon.Logging import logging
from ..Jet.JetChainConfiguration import JetChainConfiguration
from ..Photon.PrecisionPhotonTLAMenuSequenceConfig import PhotonTLAMenuSequenceCfg
from ..Jet.JetTLASequenceConfig import JetTLAMenuSequenceCfg
from ..Muon.MuonTLASequenceConfig import MuonTLAMenuSequenceCfg
log = logging.getLogger(__name__)


def addTLAStep(flags, chain, chainDict):
    '''
    Add one extra chain step for TLA Activities
    '''

    tlaSequencesList = []
    log.debug("addTLAStep: processing chain: %s", chainDict['chainName'])
    

    for cPart in chainDict['chainParts']:
        
        log.debug("addTLAStep: processing signature: %s", cPart['signature'] )
        # call the sequence from their respective signatures
        tlaSequencesList.append(functools.partial(getTLASignatureSequence, flags, chainDict=chainDict, chainPart=cPart))
            
    log.debug("addTLAStep: About to add a step with: %d parallel sequences.", len(tlaSequencesList))            
    
    # we add one step per TLA chain, with sequences matching the list of signatures
    # and multiplicities matching those of the previous step of the chain (already merged if combined)
    prevStep = chain.steps[-1]
    stepName = 'Step_merged{:d}_TLAStep_{:s}'.format(len(prevStep.legIds), prevStep.name)
    step = ChainStep(name         = stepName,
                    Sequences     = tlaSequencesList,
                    multiplicity = prevStep.multiplicity,
                    chainDicts   = prevStep.stepDicts)	

    log.debug("addTLAStep: About to add step %s ", stepName) 
    chain.steps.append(step)



def getTLASignatureSequence(flags, chainDict, chainPart):
    # Here we simply retrieve the TLA sequence from the existing signature code 
    signature= chainPart['signature']
    
    if signature == 'Photon':    
        photonOutCollectionName = "HLT_egamma_Photons"
        return PhotonTLAMenuSequenceCfg(flags, photonsIn=photonOutCollectionName)

    elif signature == 'Muon':    
        return MuonTLAMenuSequenceCfg(flags, muChainPart=chainPart)

    elif signature  == 'Jet' or signature  == 'Bjet':   
        jetDef = JetChainConfiguration(chainDict)
        jetInputCollectionName = jetDef.jetName
        log.debug(f"TLA jet input collection = {jetInputCollectionName}")
        # Turn off b-tagging for jets that have no tracks anyway - we want to avoid 
        # adding a TLA AntiKt4EMTopoJets_subjetsIS BTagging container in the EDM.
        # We do not switch off BTag recording for Jet signatures as both Jet and Bjet signature
        # will use the same hypo alg, so it needs to be configured the same!
        # Thus, BTag recording will always run for PFlow jets, creating an empty container if no btagging exists. 
        attachBtag = True
        if jetDef.recoDict["trkopt"] == "notrk": attachBtag = False
        return JetTLAMenuSequenceCfg(flags, jetsIn=jetInputCollectionName, attachBtag=attachBtag)


def findTLAStep(chainConfig):
    tlaSteps = [s for s in chainConfig.steps if 'TLAStep' in s.name and 'EmptyPEBAlign' not in s.name and 'EmptyTLAAlign' not in s.name and 'PEBInfoWriter' not in s.name]
    if len(tlaSteps) == 0:
        return None
    elif len(tlaSteps) > 1:
        raise RuntimeError('Multiple TLA steps in one chain are not supported. ', len(tlaSteps), ' were found in chain ' + chainConfig.name)
    return tlaSteps[0]


def alignTLASteps(chain_configs, chain_dicts):

    def is_tla_dict(chainNameAndDict):
        return  'PhysicsTLA' in chainNameAndDict[1]['eventBuildType'] 

    all_tla_chain_dicts = dict(filter(is_tla_dict, chain_dicts.items()))
    all_tla_chain_names = list(all_tla_chain_dicts.keys())

    
    def is_tla_config(chainNameAndConfig):
        return chainNameAndConfig[0] in all_tla_chain_names

    all_tla_chain_configs = dict(filter(is_tla_config, chain_configs.items()))


    maxTLAStepPosition = 0 # {eventBuildType: N}

    def getTLAStepPosition(chainConfig):
        tlaStep = findTLAStep(chainConfig)

        log.debug('getTLAStepPosition found step %s and return %d',tlaStep,chainConfig.steps.index(tlaStep) + 1)
        return chainConfig.steps.index(tlaStep) + 1

    # First loop to find the maximal TLA step positions to which we need to align
    for chainName, chainConfig in all_tla_chain_configs.items():
        tlaStepPosition = getTLAStepPosition(chainConfig)
        if tlaStepPosition > maxTLAStepPosition:
            maxTLAStepPosition = tlaStepPosition

    log.debug('maxTLAStepPosition=%d',maxTLAStepPosition)
    
    # Second loop to insert empty steps before the TLA steps where needed
    for chainName, chainConfig in all_tla_chain_configs.items():        
        tlaStepPosition = getTLAStepPosition(chainConfig)
        log.debug('Aligning TLA step at step %d for chain %s ', tlaStepPosition, chainName)
        if tlaStepPosition < maxTLAStepPosition:
            numStepsNeeded = maxTLAStepPosition - tlaStepPosition
            log.debug('Aligning TLA step for chain %s by adding %d empty steps', chainName, numStepsNeeded)
            chainConfig.insertEmptySteps('EmptyTLAAlign', numStepsNeeded, tlaStepPosition-1)
            chainConfig.numberAllSteps()
