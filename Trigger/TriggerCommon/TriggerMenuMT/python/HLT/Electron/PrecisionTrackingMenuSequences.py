#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

# menu components
from ..Config.MenuComponents import MenuSequence, SelectionCA, InViewRecoCA
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory

def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'Tracking'


@AccumulatorCache
def precisionTrackingSequenceGenCfg(flags, ion=False, variant='', is_probe_leg = False):
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
    def acceptAllHypoToolGen(flags, chainDict):
        return CompFactory.TrigStreamerHypoTool(chainDict["chainName"], Pass = True)
    return MenuSequence(flags,selAcc,HypoToolGen=acceptAllHypoToolGen)


def precisionTracking_LRTSequenceGenCfg(flags, is_probe_leg=False):
    return precisionTrackingSequenceGenCfg(flags, is_probe_leg=is_probe_leg, ion=False, variant='_LRT')
