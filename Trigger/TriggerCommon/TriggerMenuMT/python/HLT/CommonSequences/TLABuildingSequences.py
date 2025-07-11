#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
import functools
from TriggerMenuMT.HLT.Config.MenuComponents import ChainStep
from AthenaCommon.Logging import logging
from ..Jet.JetChainConfiguration import JetChainConfiguration
from ..Photon.PrecisionPhotonTLAMenuSequenceConfig import PhotonTLAMenuSequenceGenCfg
from ..Jet.JetTLASequenceConfig import JetTLAMenuSequenceGenCfg
from ..Muon.MuonTLASequenceConfig import MuonTLAMenuSequenceGenCfg
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
        tlaSequencesList.append(functools.partial(getTLASignatureSequenceGenCfg, flags, chainDict=chainDict, chainPart=cPart))
            
    log.debug("addTLAStep: About to add a step with: %d parallel sequences.", len(tlaSequencesList))            
    
    # we add one step per TLA chain, with sequences matching the list of signatures
    # and multiplicities matching those of the previous step of the chain (already merged if combined)
    prevStep = chain.steps[-1]
    stepName = 'TLAStep_{:s}'.format(prevStep.name)
    step = ChainStep(name         = stepName,
                    SequenceGens     = tlaSequencesList,
                    chainDicts   = prevStep.stepDicts)	

    log.debug("addTLAStep: About to add step %s ", stepName) 
    chain.steps.append(step)



def getTLASignatureSequenceGenCfg(flags, chainDict, chainPart):
    # Here we simply retrieve the TLA sequence from the existing signature code 
    signature= chainPart['signature']
    
    if signature == 'Photon':    
        photonOutCollectionName = "HLT_egamma_Photons"
        return PhotonTLAMenuSequenceGenCfg(flags, photonsIn=photonOutCollectionName)

    elif signature == 'Muon':    
        return MuonTLAMenuSequenceGenCfg(flags, muChainPart=chainPart)

    elif signature  == 'Jet':
        # Use the jet reco machinery to define the jet collection
        jetChainConfig = JetChainConfiguration(chainDict)
        jetChainConfig.prepareDataDependencies(flags)
        jetInputCollectionName = jetChainConfig.jetName
        log.debug(f"TLA jet input collection = {jetInputCollectionName}")
        return JetTLAMenuSequenceGenCfg(flags, jetsIn=jetInputCollectionName)

    else:
        raise ValueError(f"Unsupported TLA signature: No TLA sequence specified for signature {signature}.")


def findTLAStep(chainConfig):
    tlaSteps = [s for s in chainConfig.steps if 'TLAStep' in s.name and 'EmptyPEBAlign' not in s.name and 'EmptyTLAAlign' not in s.name and 'PEBInfoWriter' not in s.name]
    if len(tlaSteps) == 0:
        return None
    elif len(tlaSteps) > 1:
        raise RuntimeError('Multiple TLA steps in one chain are not supported. ', len(tlaSteps), ' were found in chain ' + chainConfig.name)
    return tlaSteps[0]


def alignTLASteps(chain_configs, chain_dicts):

    TLAEventBuildTypes = ('PhysicsTLA', 'FTagPEBTLA', 'EgammaPEBTLA', 'DarkJetPEBTLA')
    all_tla_chain_configs = [ch for ch in chain_configs if any(ebtype in chain_dicts[ch.name]['eventBuildType'] for ebtype in TLAEventBuildTypes)]

    def getTLAStepPosition(chainConfig):
        tlaStep = findTLAStep(chainConfig)
        log.debug('getTLAStepPosition found step %s and return %d',tlaStep,chainConfig.steps.index(tlaStep) + 1)
        return chainConfig.steps.index(tlaStep) + 1

    # First loop to find the maximal TLA step positions to which we need to align
    maxTLAStepPosition = 0 # {eventBuildType: N}
    for chain in all_tla_chain_configs:
        tlaStepPosition = getTLAStepPosition(chain)
        if tlaStepPosition > maxTLAStepPosition:
            maxTLAStepPosition = tlaStepPosition

    log.debug('maxTLAStepPosition=%d',maxTLAStepPosition)
    
    # Second loop to insert empty steps before the TLA steps where needed
    for chain in all_tla_chain_configs:
        tlaStepPosition = getTLAStepPosition(chain)
        log.debug('Aligning TLA step at step %d for chain %s ', tlaStepPosition, chain.name)
        if tlaStepPosition < maxTLAStepPosition:
            numStepsNeeded = maxTLAStepPosition - tlaStepPosition
            log.debug('Aligning TLA step for chain %s by adding %d empty steps', chain.name, numStepsNeeded)
            chain.insertEmptySteps('EmptyTLAAlign', numStepsNeeded, tlaStepPosition-1)
            chain.numberAllSteps()
