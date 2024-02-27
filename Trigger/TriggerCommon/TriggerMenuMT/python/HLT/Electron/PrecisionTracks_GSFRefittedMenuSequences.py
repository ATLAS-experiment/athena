#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory


def tag(ion):
    return 'precision' + ('HI' if ion is True else '') + 'Tracking_GSFRefitted'

@AccumulatorCache
def precisionTracks_GSFRefittedSequenceCfg(flags, name='Electron', ion=False, variant='_GSF', is_probe_leg = False):
    """ sixth step:  GSF refitting of precision track....."""

    inViewRoIs = "precisionTracks_GSFRefitted"+variant

    roiTool = CompFactory.ViewCreatorPreviousROITool()
    reco = InViewRecoCA(tag(ion)+variant, 
                            RoITool = roiTool,
                            InViewRoIs = inViewRoIs, 
                            RequireParentView = True, 
                            mergeUsingFeature = True,
                            ViewFallThrough = True,
                            isProbe=is_probe_leg)

    # calling GSF refitter
    from TriggerMenuMT.HLT.Electron.PrecisionTracks_GSFRefittedSequence import precisionTracks_GSFRefitted
    precisionTracks_GSFRefittedInViewSequence = precisionTracks_GSFRefitted(flags, inViewRoIs, ion, variant)

    reco.mergeReco(precisionTracks_GSFRefittedInViewSequence)

    selAcc = SelectionCA(name + tag(ion) +variant, isProbe=is_probe_leg)
    selAcc.mergeReco(reco)

    thePrecisionTrack_GSFRefittedHypo = CompFactory.TrigStreamerHypoAlg(name + tag(ion) + "Hypo" + variant)
    thePrecisionTrack_GSFRefittedHypo.FeatureIsROI = False
    selAcc.addHypoAlgo(thePrecisionTrack_GSFRefittedHypo)
    def acceptAllHypoToolGen(chainDict):
        return CompFactory.TrigStreamerHypoTool(chainDict["chainName"], Pass = True)
    return MenuSequenceCA(flags,selAcc,HypoToolGen=acceptAllHypoToolGen,isProbe=is_probe_leg)


def precisionTracks_GSFRefittedSequence_LRTCfg(flags, name='Electron', is_probe_leg=False):
    return precisionTracks_GSFRefittedSequenceCfg(flags, name, is_probe_leg=is_probe_leg, ion=False, variant='_LRTGSF')
