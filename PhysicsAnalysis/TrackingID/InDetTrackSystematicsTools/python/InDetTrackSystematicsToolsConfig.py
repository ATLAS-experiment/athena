# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackSystematicsTools package
from AnaAlgorithm.DualUseConfig import isAthena
if isAthena:
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign
from AthenaCommon.Logging import logging

def InDetTrackTruthOriginToolCfg(flags, name="InDetTrackTruthOriginTool", **kwargs):
    acc = ComponentAccumulator()

    log = logging.getLogger("InDetTrackTruthOriginTool")
    # this might fail if we're in AthAnalysis
    try:
        kwargs.setdefault("isFullPileUpTruth", flags.Digitization.PileUp
                          and flags.Digitization.DigiSteeringConf in ['StandardPileUpToolsAlg',
                                                                      'StandardInTimeOnlyTruthPileUpToolsAlg',
                                                                      'StandardInTimeOnlyGeantinoTruthPileUpToolsAlg'])
    except AttributeError:
        # assume this should be false if we're unable to load flags (temporary solution)
        kwargs.setdefault("isFullPileUpTruth", False)
        log.warning("Unable to load digi flags, assuming isFullPileUpTruth=False. Normal if you're in AthAnalysis.")

    acc.setPrivateTools(
        CompFactory.InDet.InDetTrackTruthOriginTool(name, **kwargs))
    return acc

def InDetTrackTruthFilterToolCfg(flags, name="InDetTrackTruthFilterTool", **kwargs):
    acc = ComponentAccumulator()

    if "trackOriginTool" not in kwargs:
        kwargs.setdefault("trackOriginTool", acc.popToolsAndMerge(
            InDetTrackTruthOriginToolCfg(flags)))
        
    # 2022 recommendations (MC23a)
    if flags.Input.MCCampaign in [Campaign.MC23a, Campaign.MC23d, Campaign.MC23e]:
        kwargs.setdefault("calibFileNomEff", "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/TrackingRecommendations_prelim_rel22.root")
        kwargs.setdefault("fFakeLoose", 0.40)
        kwargs.setdefault("fFakeTight", 1.00)
    # Run 2 recommendations (MC20)
    elif flags.GeoModel.Run is LHCPeriod.Run2:
        kwargs.setdefault("calibFileNomEff", "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/TrackingRecommendations_prelim_rel22.root")
        kwargs.setdefault("fFakeLoose", 0.10)
        kwargs.setdefault("fFakeTight", 1.00)
    else:
        raise ValueError(f"InDetTrackTruthFilterTool: Recommendations not yet available for campaign {flags.Input.MCCampaign}! Please check the configuration and contact Tracking CP if you believe this message is in error.")

    acc.setPrivateTools(
        CompFactory.InDet.InDetTrackTruthFilterTool(name, **kwargs))
    return acc

def JetTrackFilterToolCfg(flags, name="JetTrackFilterTool", **kwargs):
    acc = ComponentAccumulator()

    if "trackOriginTool" not in kwargs:
        kwargs.setdefault("trackOriginTool", acc.popToolsAndMerge(
            InDetTrackTruthOriginToolCfg(flags)))

    # TIDE fake rate recommendations:
    # Run 3 (MC23): https://indico.cern.ch/event/1587937/#40-fake-tracks-in-the-jet-core
    if flags.GeoModel.Run >= LHCPeriod.Run3:
        kwargs.setdefault("FakeUncertainty", 0.25)
    # Run 2 (MC20): https://cds.cern.ch/record/2859907
    else:
        kwargs.setdefault("FakeUncertainty", 0.35)

    # TIDE FLost recommendations:
    # Run 3 (MC23)
    if flags.GeoModel.Run >= LHCPeriod.Run3:
        # 2022/23 (MC23a/d): https://indico.cern.ch/event/1531052/#38-flost-update
        if flags.Input.MCCampaign in [Campaign.MC23a, Campaign.MC23d]:
            kwargs.setdefault("FLostUncertainty", 0.24)
        # 2024 (MC23e): https://indico.cern.ch/event/1662051/#46-update-on-2024-flost-measur
        elif flags.Input.MCCampaign is Campaign.MC23e:
            kwargs.setdefault("FLostUncertainty", 0.32)
        else:
            raise ValueError(f"JetTrackFilterTool: Recommendations not yet available for campaign {flags.Input.MCCampaign}! Please check the configuration and contact Tracking CP if you believe this message is in error.")
    # Run 2 (MC20)
    else:
        kwargs.setdefault("FLostUncertainty", 0.24)

    acc.setPrivateTools(CompFactory.InDet.JetTrackFilterTool(name, **kwargs))
    return acc

def InclusiveTrackFilterToolCfg(flags, name="InclusiveTrackFilterTool", **kwargs):
    acc = ComponentAccumulator()

    # 2022 recommendations (MC23a)
    if flags.Input.MCCampaign is Campaign.MC23a:
        kwargs.setdefault("calibFileLRTEff", "InDetTrackSystematicsTools/CalibData_25.2_2025-v00/LargeD0TrackingRecommendations_mc23a.root")
    elif flags.Input.MCCampaign is Campaign.MC23d:
        kwargs.setdefault("calibFileLRTEff", "InDetTrackSystematicsTools/CalibData_25.2_2025-v00/LargeD0TrackingRecommendations_mc23d.root")
    elif flags.Input.MCCampaign is Campaign.MC23e:
        kwargs.setdefault("calibFileLRTEff", "InDetTrackSystematicsTools/CalibData_25.2_2025-v00/LargeD0TrackingRecommendations_mc23e.root")
    # Run 2 recommendations (MC20)
    elif flags.GeoModel.Run is LHCPeriod.Run2:
        kwargs.setdefault("calibFileLRTEff", "InDetTrackSystematicsTools/CalibData_24.0_2023-v00/LargeD0TrackingRecommendations_20230824.root")
    else:
        raise ValueError(f"InclusiveTrackFilterTool: Recommendations not yet available for campaign {flags.Input.MCCampaign}! Please check the configuration and contact Tracking CP if you believe this message is in error.")

    acc.setPrivateTools(
        CompFactory.InDet.InclusiveTrackFilterTool(name, **kwargs))
    return acc

def InDetTrackSmearingToolCfg(flags, name="InDetTrackSmearingTool", **kwargs):
    acc = ComponentAccumulator()

    # 2022 recommendations (MC23a)
    if flags.Input.MCCampaign is Campaign.MC23a:
        kwargs.setdefault("calibFileIP_CTIDE", "InDetTrackSystematicsTools/CalibData_25.2_2025-v00/2022_d0z0_smearing_factors_v2.root")
    # 2023 recommendations (MC23d)
    elif flags.Input.MCCampaign is Campaign.MC23d:
        kwargs.setdefault("calibFileIP_CTIDE", "InDetTrackSystematicsTools/CalibData_25.2_2025-v00/2023_d0z0_smearing_factors_v2.root")
    # 2024 recommendations (MC23e)
    elif flags.Input.MCCampaign is Campaign.MC23e:
        kwargs.setdefault("calibFileIP_CTIDE", "InDetTrackSystematicsTools/CalibData_25.2_2025-v00/2024_d0z0_smearing_factors.root")
    # Run 2 recommendations (MC20)
    elif flags.GeoModel.Run is LHCPeriod.Run2:
        kwargs.setdefault("calibFileIP_CTIDE", "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/d0z0_smearing_factors_Run2_v2.root")
    elif "calibFileIP_CTIDE" not in kwargs:
        raise ValueError(f"InDetTrackSmearingTool: Recommendations not yet available for campaign {flags.Input.MCCampaign}! Please check the configuration and contact Tracking CP if you believe this message is in error.")

    acc.setPrivateTools(
        CompFactory.InDet.InDetTrackSmearingTool(name, **kwargs))
    return acc

def InDetTrackBiasingCalibKwargs(flags):
    """Return calibFiles/runNumberBounds kwargs for InDetTrackBiasingTool.

    Selects campaign-specific calibration files and (for multi-period
    campaigns) run-number boundaries.  Raises ValueError for unknown
    campaigns or geometries.
    """
    c2 = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00"
    c3 = "InDetTrackSystematicsTools/CalibData_25.2_2025-v00"
    if flags.GeoModel.Run is LHCPeriod.Run2:
        if flags.Input.MCCampaign is Campaign.MC20a:
            # 2015 + 2016 recommendations (MC20a)
            return {
                "calibFiles": [
                    f"{c2}/REL22_REPRO_2015.root",
                    f"{c2}/REL22_REPRO_2016_1stPart.root",
                    f"{c2}/REL22_REPRO_2016_2ndPart.root",
                ],
                "runNumberBounds": [0, 296938, 301912, 999999],
            }
        elif flags.Input.MCCampaign is Campaign.MC20d:
            # 2017 recommendations (MC20d)
            return {
                "calibFiles": [
                    f"{c2}/REL22_REPRO_2017_1stPart.root",
                    f"{c2}/REL22_REPRO_2017_2ndPart.root",
                ],
                "runNumberBounds": [0, 334842, 999999],
            }
        elif flags.Input.MCCampaign is Campaign.MC20e:
            # 2018 recommendations (MC20e)
            return {
                "calibFiles": [
                    f"{c2}/REL22_REPRO_2018_1stPart.root",
                    f"{c2}/REL22_REPRO_2018_2ndPart.root",
                ],
                "runNumberBounds": [0, 353000, 999999],
            }
        else:
            raise ValueError(
                'No biasing recommendations found for campaign "'
                + flags.Input.MCCampaign.value
                + '" in Run 2. Please check the configuration.'
            )
    elif flags.GeoModel.Run is LHCPeriod.Run3:
        if flags.Input.MCCampaign is Campaign.MC23a:
            # 2022 recommendations (MC23a)
            return {
                "calibFiles": [
                    f"{c3}/2022_d0z0qoverp_biasing_factor.root",
                ]
            }
        elif flags.Input.MCCampaign is Campaign.MC23d:
            # 2023 recommendations (MC23d)
            return {
                "calibFiles": [
                    f"{c3}/2023_d0z0qoverp_biasing_factor.root",
                ]
            }
        elif flags.Input.MCCampaign is Campaign.MC23e:
            # 2024 recommendations (MC23e)
            return {
                "calibFiles": [
                    f"{c3}/2024_d0z0qoverp_biasing_factor.root",
                ]
            }
        else:
            raise ValueError(
                'No biasing recommendations found for campaign "'
                + flags.Input.MCCampaign.value
                + '" in Run 3. Please check the configuration.'
            )
    else:
        raise ValueError(
            'No biasing recommendations found for geometry "'
            + flags.GeoModel.Run.value
            + '". Please check the configuration.'
        )


def InDetTrackBiasingToolCfg(flags, name="InDetTrackBiasingTool",
                              **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("isMC", flags.Input.isMC)
    if "calibFiles" not in kwargs:
        kwargs.update(InDetTrackBiasingCalibKwargs(flags))
    acc.setPrivateTools(
        CompFactory.InDet.InDetTrackBiasingTool(name, **kwargs))
    return acc

def TrackSystematicsAlgCfg(flags, name="InDetTrackSystematicsAlg", **kwargs):
    acc = ComponentAccumulator()

    if "TrackFilterToolLRT" not in kwargs:
        kwargs.setdefault("TrackFilterToolLRT", acc.popToolsAndMerge(
            InclusiveTrackFilterToolCfg(flags)))

    if "TrackFilterToolSTD" not in kwargs:
        kwargs.setdefault("TrackFilterToolSTD", acc.popToolsAndMerge(
            InDetTrackTruthFilterToolCfg(flags)))

    acc.addEventAlgo(CompFactory.InDet.TrackSystematicsAlg(name, **kwargs))
    return acc


def TrackSmearingAlgCfg(flags, syst, input_tracks, output_tracks,
                        bias_kwargs={}):
    """Shallow-copy input_tracks and apply smearing/biasing for one syst.

    syst is a single smearing variation string (TRK_RES_*, TRK_BIAS_*), or
    empty for a nominal (un-smeared) copy.  Produces output_tracks as a
    shallow copy with modified d0/z0/qoverp values.

    Any extra keyword arguments are forwarded to InDetTrackBiasingToolCfg
    as calibration kwargs (calibFiles, runNumberBounds, ...).  If none are
    supplied, InDetTrackBiasingCalibKwargs(flags) is called to derive them;
    exceptions from that call propagate to the caller.
    """
    ca = ComponentAccumulator()

    smearingTool = ca.popToolsAndMerge(InDetTrackSmearingToolCfg(flags))
    ca.addPublicTool(smearingTool)

    # Only configure the biasing tool for TRK_BIAS_* systematics.
    # The tool reads RandomRunNumber unconditionally (needs PRW), so
    # configuring it for nominal or RES systematics would require PRW
    # to be scheduled even when no biasing is needed.
    biasingTool = None
    if 'BIAS' in syst or bias_kwargs:
        if not bias_kwargs:
            bias_kwargs = InDetTrackBiasingCalibKwargs(flags)
        if bias_kwargs:
            biasingTool = ca.popToolsAndMerge(
                InDetTrackBiasingToolCfg(flags, **bias_kwargs))
            ca.addPublicTool(biasingTool)

    alg = CompFactory.InDet.TrackSmearingAlg(
        f'TrackSmearingAlg_{output_tracks}',
        SmearingTool=smearingTool,
        InputTrackContainer=input_tracks,
        OutputTrackContainer=output_tracks,
        SystematicVariation=syst,
    )
    if biasingTool is not None:
        alg.BiasingTool = biasingTool

    ca.addEventAlgo(alg)
    return ca


def JetTrackFilteringAlgCfg(
        flags, syst, jet_collection,
        in_ghost_tracks, out_ghost_tracks):
    """Filter ghost-track links on jets for one filter systematic.

    syst is a single filter variation string (TRK_EFF_*, TRK_FAKE_RATE_*).
    Reads in_ghost_tracks decoration from jet_collection, applies filter
    tools per-track (dispatching LRT vs STD by patternRecoInfo bit 49),
    and writes surviving links to out_ghost_tracks.
    """
    assert syst, "JetTrackFilteringAlgCfg called with empty syst"

    ca = ComponentAccumulator()

    is_larged0 = 'LARGED0' in syst
    is_tide = 'TIDE' in syst

    alg = CompFactory.InDet.JetTrackFilteringAlg(
        f'JetTrackFilteringAlg_{out_ghost_tracks}',
        JetCollection=jet_collection,
        InGhostTracks=in_ghost_tracks,
        OutGhostTracks=out_ghost_tracks,
        SystematicVariation=syst,
    )
    if not is_larged0:
        alg.STDFilterTool = ca.popToolsAndMerge(
            InDetTrackTruthFilterToolCfg(flags)
        )
        ca.addPublicTool(alg.STDFilterTool)
    if is_larged0:
        alg.LRTFilterTool = ca.popToolsAndMerge(
            InclusiveTrackFilterToolCfg(flags)
        )
        ca.addPublicTool(alg.LRTFilterTool)
    if is_tide:
        alg.JetFilterTool = ca.popToolsAndMerge(
            JetTrackFilterToolCfg(flags)
        )
        ca.addPublicTool(alg.JetFilterTool)

    ca.addEventAlgo(alg)
    return ca
