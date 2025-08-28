# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.Enums import HIMode
from TrkConfig.TrkConfigFlags import PrimaryPassConfig

def beamSpotRecoPreIncCommon(flags):
    flags.Detector.EnableAFP = False
    flags.Detector.EnableLucid = False
    flags.Detector.EnableCalo = False
    flags.Detector.EnableL1Calo = False
    flags.Detector.EnableMuon = False
    flags.Detector.EnableTRT = False
    flags.Detector.EnableZDC = False

    flags.Reco.EnableTrigger = False
    flags.Reco.EnableCombinedMuon = False
    flags.Reco.EnableEgamma = False
    flags.Reco.EnableTau = False
    flags.Reco.EnableJet = False
    flags.Reco.EnableMet = False
    flags.Reco.EnableBTagging = False
    flags.Reco.EnableHI = False
    flags.Reco.EnableZDC = False
    flags.Reco.HIMode = HIMode.pp
    flags.Reco.EnableCaloRinger = False
    flags.Reco.EnableTrackCellAssociation = False
    flags.Reco.PostProcessing.InDetForwardTrackParticleThinning = False

    flags.Tracking.PrimaryPassConfig = PrimaryPassConfig.VtxBeamSpot
    flags.Tracking.doTrackSegmentsDisappearing = False

    flags.DQ.enableLumiAccess = False


def beamSpotRecoPreInc(flags):
    beamSpotRecoPreIncCommon(flags)

    flags.Common.doExpressProcessing = True

    flags.GeoModel.Align.Dynamic = True


def beamSpotRecoPreIncMC(flags):
    beamSpotRecoPreIncCommon(flags)


def beamSpotRecoPostInc(flags, cfg):
    from IOVDbSvc.IOVDbSvcConfig import addOverride

    cfg.merge(addOverride(flags, "/Indet/AlignL1/ID", "InDetAlignL1-RUN3-BLK-UPD4-01"))
    cfg.merge(
        addOverride(flags, "/Indet/AlignL2/PIX", "InDetAlignL2PIX-RUN3-BLK-UPD4-01")
    )
    cfg.merge(
        addOverride(flags, "/Indet/AlignL2/SCT", "InDetAlignL2SCT-RUN3-BLK-UPD4-01")
    )
    cfg.merge(addOverride(flags, "/TRT/AlignL1/TRT", "TRTAlignL1-RUN3-BLK-UPD4-01"))
    cfg.merge(addOverride(flags, "/Indet/AlignL3", "IndetAlignL3-RUN3-BLK-UPD4-04"))
    cfg.merge(
        addOverride(flags, "/Indet/IBLDist", "InDetAlignIBLDIST-RUN3-BLK-UPD4-01")
    )
    cfg.merge(addOverride(flags, "/TRT/AlignL2", "TRTAlignL2-RUN3-BLK-UPD4-04"))


def beamSpotRecoPostExecCfg(flags):
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    return OutputStreamCfg(flags, "AOD",["EventInfo#*"])
