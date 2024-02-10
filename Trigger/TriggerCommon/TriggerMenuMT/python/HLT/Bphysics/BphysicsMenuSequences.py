# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from ..Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA, InViewRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TrigEDMConfig.TriggerEDM import recordable


@AccumulatorCache
def bmumuxSequence(flags):

    RoIToolCreator = CompFactory.ViewCreatorMuonSuperROITool if flags.Trigger.InDetTracking.bmumux.SuperRoI else CompFactory.ViewCreatorCentredOnIParticleROITool

    roiToolOptions = {
        'RoIEtaWidth' : flags.Trigger.InDetTracking.bmumux.etaHalfWidth,
        'RoIPhiWidth' : flags.Trigger.InDetTracking.bmumux.phiHalfWidth,
        'RoIZedWidth' : flags.Trigger.InDetTracking.bmumux.zedHalfWidth,
        'RoisWriteHandleKey' : recordable(flags.Trigger.InDetTracking.bmumux.roi) }

    viewMakerOptions = {
        'RoITool' : RoIToolCreator(**roiToolOptions),
        'mergeUsingFeature' : True,
        'PlaceMuonInView' : True,
        'InViewMuonCandidates' : 'BmumuxMuonCandidates',
        'InViewMuons' : 'HLT_Muons_Bmumux' }

    reco = InViewRecoCA('Bmumux', **viewMakerOptions)
    from .BphysicsRecoSequences import bmumuxRecoSequenceCfg
    reco.mergeReco(bmumuxRecoSequenceCfg(flags, reco.inputMaker().InViewRoIs, reco.inputMaker().InViewMuons))

    selAcc = SelectionCA('bmumuxSequence')

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    selAcc.mergeReco(reco, robPrefetchCA=ROBPrefetchingAlgCfg_Si(flags, nameSuffix=reco.name))

    hypoAlg = CompFactory.TrigBphysStreamerHypo('BmumuxStreamerHypoAlg')
    selAcc.addHypoAlgo(hypoAlg)

    from TrigBphysHypo.TrigBphysStreamerHypoConfig import TrigBphysStreamerHypoToolFromDict
    return MenuSequenceCA(flags, selAcc, HypoToolGen=TrigBphysStreamerHypoToolFromDict)


def dimuL2Sequence(flags):
    from ..Muon.MuonMenuSequences import muCombAlgSequenceCfg
    from TrigBphysHypo.TrigBphysStreamerHypoConfig import TrigBphysStreamerHypoToolFromDict

    sequence, combinedMuonContainerName = muCombAlgSequenceCfg(flags, "Bphys")
    hypo = CompFactory.TrigBphysStreamerHypo('DimuL2StreamerHypoAlg', 
                                             triggerList = getNoL2CombChainNames(),
                                             triggerLevel = 'L2')
    sequence.addHypoAlgo(hypo)

    return MenuSequenceCA(flags, sequence,
                                  HypoToolGen = TrigBphysStreamerHypoToolFromDict)


@AccumulatorCache
def dimuEFSequence(flags):
    selAcc = SelectionCA('dimuSequence')

    inputMakerAlg = CompFactory.InputMakerForRoI('IM_bphysStreamerDimuEF',
        RoITool = CompFactory.ViewCreatorPreviousROITool(),
        mergeUsingFeature = True)

    reco = InEventRecoCA('bphysStreamerDimuEFReco', inputMaker=inputMakerAlg)
    selAcc.mergeReco(reco)

    hypoAlg = CompFactory.TrigBphysStreamerHypo('DimuEFStreamerHypoAlg', triggerLevel = 'EF')
    selAcc.addHypoAlgo(hypoAlg)

    from TrigBphysHypo.TrigBphysStreamerHypoConfig import TrigBphysStreamerHypoToolFromDict
    return MenuSequenceCA(flags, selAcc, HypoToolGen=TrigBphysStreamerHypoToolFromDict)


def getNoL2CombChainNames():
    from ..Config.GenerateMenuMT import GenerateMenuMT
    menu = GenerateMenuMT()
    chains = [chain.name for chain in menu.chainsInMenu['Bphysics'] if 'noL2Comb' in chain.name]
    return chains
