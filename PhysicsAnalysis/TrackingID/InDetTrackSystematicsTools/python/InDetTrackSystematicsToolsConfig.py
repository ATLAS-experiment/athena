# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetTrackSystematicsTools package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
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
        
    from AthenaConfiguration.Enums import LHCPeriod
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

    from AthenaConfiguration.Enums import LHCPeriod
    # Run 3 recommendations (MC23): https://indico.cern.ch/event/1424738/#20-run-3-recommendations-fake
    if flags.GeoModel.Run >= LHCPeriod.Run3:
        kwargs.setdefault("FakeUncertainty", 0.30)
    # Run 2 recommendations (MC20): https://cds.cern.ch/record/2859907
    else:
        kwargs.setdefault("FakeUncertainty", 0.35)

    acc.setPrivateTools(CompFactory.InDet.JetTrackFilterTool(name, **kwargs))
    return acc

def InclusiveTrackFilterToolCfg(flags, name="InclusiveTrackFilterTool", **kwargs):
    acc = ComponentAccumulator()

    from AthenaConfiguration.Enums import LHCPeriod
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

    from AthenaConfiguration.Enums import LHCPeriod
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
