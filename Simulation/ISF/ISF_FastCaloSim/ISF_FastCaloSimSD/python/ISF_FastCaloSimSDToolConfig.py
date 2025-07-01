# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FCS_StepInfoSDToolCfg(flags, name="FCS_StepInfoSensitiveDetector", **kwargs):
    kwargs.setdefault("StacVolumes", ["LArMgr::LAr::EMB::STAC"])
    kwargs.setdefault("PresamplerVolumes", ["LArMgr::LAr::Barrel::Presampler::Module"])
    kwargs.setdefault("NegIWVolumes", ["LArMgr::LAr::EMEC::Neg::InnerWheel"])
    kwargs.setdefault("NegOWVolumes", ["LArMgr::LAr::EMEC::Neg::OuterWheel"])
    kwargs.setdefault("NegBOBarretteVolumes",["LArMgr::LAr::EMEC::Neg::BackOuterBarrette::Module::Phidiv"])
    kwargs.setdefault("PosIWVolumes", ["LArMgr::LAr::EMEC::Pos::InnerWheel"])
    kwargs.setdefault("PosOWVolumes", ["LArMgr::LAr::EMEC::Pos::OuterWheel"])
    kwargs.setdefault("PosBOBarretteVolumes",["LArMgr::LAr::EMEC::Pos::BackOuterBarrette::Module::Phidiv"])
    kwargs.setdefault("PresVolumes",  ["LArMgr::LAr::Endcap::Presampler::LiquidArgon"])
    kwargs.setdefault("SliceVolumes", ["LArMgr::LAr::HEC::Module::Depth::Slice"])
    kwargs.setdefault("FCAL1Volumes", ["LArMgr::LAr::FCAL::Module1::Gap"])
    kwargs.setdefault("FCAL2Volumes", ["LArMgr::LAr::FCAL::Module2::Gap"])
    kwargs.setdefault("FCAL3Volumes", ["LArMgr::LAr::FCAL::Module3::Gap"])
    kwargs.setdefault("TileVolumes",  ["Tile::Scintillator"])
    kwargs.setdefault("OutputCollectionNames", ["EventSteps"])

    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.FCS_Param.FCS_StepInfoSDTool(name, **kwargs))
    return result


def PostIncludeParametrizationInputSim_1mm(flags, cfg):
    
    # // LAr barrel
    # CALOSAMPLING(PreSamplerB, 1, 0) //  0
    # CALOSAMPLING(EMB1,        1, 0) //  1
    # CALOSAMPLING(EMB2,        1, 0) //  2
    # CALOSAMPLING(EMB3,        1, 0) //  3

    # // LAr EM endcap
    # CALOSAMPLING(PreSamplerE, 0, 1) //  4
    # CALOSAMPLING(EME1,        0, 1) //  5
    # CALOSAMPLING(EME2,        0, 1) //  6
    # CALOSAMPLING(EME3,        0, 1) //  7

    # // Hadronic endcap
    # CALOSAMPLING(HEC0,        0, 1) //  8
    # CALOSAMPLING(HEC1,        0, 1) //  9
    # CALOSAMPLING(HEC2,        0, 1) // 10
    # CALOSAMPLING(HEC3,        0, 1) // 11

    # // Tile barrel
    # CALOSAMPLING(TileBar0,    1, 0) // 12
    # CALOSAMPLING(TileBar1,    1, 0) // 13
    # CALOSAMPLING(TileBar2,    1, 0) // 14

    # // Tile gap (ITC & scint)
    # CALOSAMPLING(TileGap1,    1, 0) // 15
    # CALOSAMPLING(TileGap2,    1, 0) // 16
    # CALOSAMPLING(TileGap3,    1, 0) // 17

    # // Tile extended barrel
    # CALOSAMPLING(TileExt0,    1, 0) // 18
    # CALOSAMPLING(TileExt1,    1, 0) // 19
    # CALOSAMPLING(TileExt2,    1, 0) // 20

    # // Forward EM endcap
    # CALOSAMPLING(FCAL0,       0, 1) // 21
    # CALOSAMPLING(FCAL1,       0, 1) // 22
    # CALOSAMPLING(FCAL2,       0, 1) // 23

    
    stepInfoSDTool = cfg.getPublicTool("SensitiveDetectorMasterTool").SensitiveDetectors['FCS_StepInfoSensitiveDetector']
    stepInfoSDTool.shift_lar_subhit=True #default
    stepInfoSDTool.shorten_lar_step=True
    
    stepInfoSDTool.maxRadiusLateral =   [ 0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5,  0.5] # For finer merging
    stepInfoSDTool.maxRadiusLongitudinal = [11.0,  5.0, 31.0,  5.0,  5.0,  7.0, 32.0,  9.0, 35.0, 67.0, 62.0, 58.0, 38.0, 49.0, 48.0, 56.0, 48.0,  5.0, 38.0, 68.0, 85.0, 56.0, 55.0, 55.0] # For finer merging
    
    stepInfoSDTool.maxTime=25. #default
    stepInfoSDTool.maxTimeTile=100. #default
