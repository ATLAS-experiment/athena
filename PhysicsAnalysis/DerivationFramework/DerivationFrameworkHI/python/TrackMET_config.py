# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from METReconstruction.METRecoCfg import BuildConfig, RefConfig, METConfig, getMETRecoAlg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def Cfg_METTrack(configFlags, ptCut):

    acc = ComponentAccumulator()
    
    cfg_trk = METConfig('Track'+str(ptCut),configFlags,[BuildConfig('SoftTrk','Track')],
                    [RefConfig('TrackFilter','PVTrack')],
                    doTracks=configFlags.MET.UseTracks)
    cfg_trk.refiners['TrackFilter'].DoLepRecovery=True
    cfg_trk.refiners['TrackFilter'].DoVxSep=configFlags.MET.UseTracks
    cfg_trk.refiners['TrackFilter'].DoEoverPSel=True

    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_HITight_Cfg
    
    TrkSelTool_hi_tight = acc.popToolsAndMerge(InDetTrackSelectionTool_HITight_Cfg(configFlags,
                                                            name = "TrackSelectionTool_hi_tight_pt"+str(ptCut),
                                                            minPt = ptCut))

    cfg_trk.refiners['TrackFilter'].TrackSelectorTool=TrkSelTool_hi_tight

    acc.merge(cfg_trk.accumulator)

    recoAlg=getMETRecoAlg(algName='METRecoAlg_Track'+str(ptCut),configs={"Track"+str(ptCut):cfg_trk})
    acc.addEventAlgo(recoAlg)
    return acc

