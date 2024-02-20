#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# menu components
from ..Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'Tracking'


@AccumulatorCache
def precisionTrackingSequenceCfg(flags, ion=False, variant='', is_probe_leg = False):
    """ fourth step:  precision electron....."""

    inViewRoIs = "precisionTracking" + variant

    from TriggerMenuMT.HLT.Electron.PrecisionTrackingRecoSequences import precisionTracking
    precisionTrackingReco = precisionTracking(flags, inViewRoIs, ion, variant)

    # preparing roiTool
    roiTool = CompFactory.ViewCreatorPreviousROITool()

    viewName = tag(ion)+variant
    precisionInDetReco = InViewRecoCA(viewName,
                                      RoITool=roiTool, # view maker args
                                      ViewFallThrough = True,
                                      RequireParentView=True,
                                      mergeUsingFeature=True,
                                      InViewRoIs=inViewRoIs,
                                      isProbe=is_probe_leg)

    precisionInDetReco.mergeReco(precisionTrackingReco)
    selAcc=SelectionCA(viewName, isProbe=is_probe_leg)
    selAcc.mergeReco(precisionInDetReco)


    precisionElectronHypoAlg = CompFactory.TrigStreamerHypoAlg("Electron"+tag(ion)+"Hypo"+variant)
    precisionElectronHypoAlg.FeatureIsROI = False
    selAcc.addHypoAlgo(precisionElectronHypoAlg)
    def acceptAllHypoToolGen(chainDict):
        return CompFactory.TrigStreamerHypoTool(chainDict["chainName"], Pass = True)
    return MenuSequenceCA(flags,selAcc,HypoToolGen=acceptAllHypoToolGen,isProbe=is_probe_leg)


def precisionTrackingSequence_LRTCfg(flags, is_probe_leg=False):
    return precisionTrackingSequenceCfg(flags, is_probe_leg=is_probe_leg, ion=False, variant='_LRT')
