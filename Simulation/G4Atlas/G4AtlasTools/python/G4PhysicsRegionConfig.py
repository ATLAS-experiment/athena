# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from SimulationConfig.SimEnums import BeamPipeSimMode
from AthenaCommon.Logging import logging

RegionCreator=CompFactory.RegionCreator


# Beampipe Regions
def BeampipeFwdCutPhysicsRegionToolCfg(flags, name='BeampipeFwdCutPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    theLog = logging.getLogger("BeampipeFwdCutPhysicsRegionToolCfg")
    kwargs.setdefault("RegionName", 'BeampipeFwdCut')
    volumeList = []
    if flags.GeoModel.Run is LHCPeriod.Run1:
        volumeList = ['BeamPipe::SectionF47', 'BeamPipe::SectionF48', 'BeamPipe::SectionF61']
    else:
        volumeList = ['BeamPipe::SectionF198', 'BeamPipe::SectionF199', 'BeamPipe::SectionF200']
        if flags.GeoModel.Run > LHCPeriod.Run4:
            theLog.warning('check that RUN2 beampipe volume names are correct for this geometry tag')
    kwargs.setdefault("VolumeList",  volumeList)

    if flags.Sim.BeamPipeSimMode is BeamPipeSimMode.FastSim:
        kwargs.setdefault("ElectronCut", 10.)
        kwargs.setdefault("PositronCut", 10.)
        kwargs.setdefault("GammaCut", 10.)
        theLog.info('Adding fast sim model to the beampipe!')
    else:
        assert flags.Sim.BeamPipeCut
        if flags.Sim.BeamPipeCut < 1:
            msg = "Setting the forward beam pipe range cuts to %e mm " % flags.Sim.BeamPipeCut
            msg += "-- cut is < 1 mm, I hope you know what you're doing!"
            theLog.info(msg)
        if flags.Sim.BeamPipeSimMode is BeamPipeSimMode.EGammaRangeCuts:
            kwargs.setdefault("ElectronCut", flags.Sim.BeamPipeCut)
            kwargs.setdefault("PositronCut", flags.Sim.BeamPipeCut)
            kwargs.setdefault("GammaCut", flags.Sim.BeamPipeCut)
        elif flags.Sim.BeamPipeSimMode is BeamPipeSimMode.EGammaPRangeCuts:
            kwargs.setdefault("ElectronCut", flags.Sim.BeamPipeCut)
            kwargs.setdefault("PositronCut", flags.Sim.BeamPipeCut)
            kwargs.setdefault("GammaCut", flags.Sim.BeamPipeCut)
            kwargs.setdefault("ProtonCut", flags.Sim.BeamPipeCut)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def FWDBeamLinePhysicsRegionToolCfg(flags, name='FWDBeamLinePhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'FWDBeamLine')
    if flags.GeoModel.Run is LHCPeriod.Run1:
        volumeList = ['BeamPipe::SectionF46']
    else:
        volumeList = ['BeamPipe::SectionF197']
        if flags.GeoModel.Run > LHCPeriod.Run4:
            theLog = logging.getLogger("FWDBeamLinePhysicsRegionToolCfg")
            theLog.warning('Check that RUN2 beampipe volume names are correct for this geometry tag')
    kwargs.setdefault("VolumeList",  volumeList)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


# Forward Regions
def FwdRegionPhysicsRegionToolCfg(flags, name='FwdRegionPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'FwdRegion')
    volumeList = ['FwdRegion::ForwardRegionGeoModel']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 200.)
    kwargs.setdefault("PositronCut", 200.)
    kwargs.setdefault("GammaCut",    200.)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


# Inner Detector Regions
def PixelPhysicsRegionToolCfg(flags, name='PixelPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'Pixel')
    volumeList = ['Pixel::siLog', 'Pixel::siBLayLog']
    if flags.GeoModel.Run in [LHCPeriod.Run2, LHCPeriod.Run3]:
        # TODO: should we support old geometry tags with Run == "UNDEFINED" and flags.GeoModel.IBLLayout not in ["noIBL", "UNDEFINED"]?
        volumeList += ['Pixel::dbmDiamondLog']
    kwargs.setdefault("VolumeList",  volumeList)
    # The range cuts used here are directly linked to the minimum energy of delta rays.
    # The minimum energy of delta rays in an input to the digitisation when using Bichsel charge deposition model.
    # The range cut is equated to an energy threshold in the simulation log file
    # If these change please update the digitisation cuts appropriately.
    kwargs.setdefault("ElectronCut", 0.05)
    kwargs.setdefault("PositronCut", 0.05)
    kwargs.setdefault("GammaCut",    0.05)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def SCTPhysicsRegionToolCfg(flags, name='SCTPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'SCT')
    volumeList = ['SCT::BRLSensor', 'SCT::ECSensor0', 'SCT::ECSensor1',
                  'SCT::ECSensor2','SCT::ECSensor3']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 0.05)
    kwargs.setdefault("PositronCut", 0.05)
    kwargs.setdefault("GammaCut",    0.05)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def ITkPixelPhysicsRegionToolCfg(flags, name='ITkPixelPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'ITkPixel')
    volumeList = ['ITkPixel::InnerBarrelSingleMod_Sensor',
                  'ITkPixel::InnerRingSingleMod_Sensor',
                  'ITkPixel::InnerQuadMod_Sensor',
                  'ITkPixel::OuterQuadMod_Sensor',
                  'ITkPixel::InclinedQuadMod_Sensor']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 0.05)
    kwargs.setdefault("PositronCut", 0.05)
    kwargs.setdefault("GammaCut",    0.05)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def ITkStripPhysicsRegionToolCfg(flags, name='ITkStripPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'ITkStrip')
    volumeList = ['ITkStrip::BRLSensorSS', 'ITkStrip::BRLSensorMS',
                  'ITkStrip::ECSensor0', 'ITkStrip::ECSensor1', 'ITkStrip::ECSensor2',
                  'ITkStrip::ECSensor3', 'ITkStrip::ECSensor4', 'ITkStrip::ECSensor5',
                  'ITkStrip::ECSensorBack0', 'ITkStrip::ECSensorBack1', 'ITkStrip::ECSensorBack2',
                  'ITkStrip::ECSensorBack3', 'ITkStrip::ECSensorBack4', 'ITkStrip::ECSensorBack5']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 0.05)
    kwargs.setdefault("PositronCut", 0.05)
    kwargs.setdefault("GammaCut",    0.05)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def HGTDPhysicsRegionToolCfg(flags, name='HGTDPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'HGTD')
    if flags.HGTD.Geometry.useGeoModelXml:
        volumeList = ['HGTD::HGTDSiSensorPosL0',"HGTD::HGTDSiSensorPosL1",
                      'HGTD::HGTDSiSensorPosL2','HGTD::HGTDSiSensorPosL3',
                      'HGTD::HGTDSiSensorNegL0','HGTD::HGTDSiSensorNegL1',
                      'HGTD::HGTDSiSensorNegL2','HGTD::HGTDSiSensorNegL3',
                      'HGTD::HGTDSiSensor']
    else:
        volumeList = ['HGTD::HGTDSiSensor0', 'HGTD::HGTDSiSensor1',
                      'HGTD::HGTDSiSensor2', 'HGTD::HGTDSiSensor3']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 0.05)
    kwargs.setdefault("PositronCut", 0.05)
    kwargs.setdefault("GammaCut",    0.05)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def TRTPhysicsRegionToolCfg(flags, name='TRTPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    rangeCut = flags.Sim.TRTRangeCut
    kwargs.setdefault("RegionName", 'TRT')
    volumeList = ['TRT::Gas', 'TRT::GasMA']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", rangeCut)
    kwargs.setdefault("PositronCut", rangeCut)
    # The photon range cut is meant to stay small
    kwargs.setdefault("GammaCut",    0.05)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def TRT_ArPhysicsRegionToolCfg(flags, name='TRT_ArPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'TRT_Ar')
    volumeList = ['TRT::Gas_Ar', 'TRT::GasMA_Ar']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 30.0)
    kwargs.setdefault("PositronCut", 30.0)
    kwargs.setdefault("GammaCut",    0.05)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def TRT_KrPhysicsRegionToolCfg(flags, name='TRT_KrPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'TRT_Kr')
    volumeList = ['TRT::Gas_Kr', 'TRT::GasMA_Kr']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 15.0)
    kwargs.setdefault("PositronCut", 15.0)
    kwargs.setdefault("GammaCut",    0.05)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result

def CALOPhysicsRegionToolCfg(flags, name='CALOPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'CALO')
    volumeList = ['CALO::CALO', 'LArMgr::LAr::Endcap::Cryostat::Cylinder::Mixed','LArMgr::LAr::Barrel::Cryostat::MotherVolume','LArMgr::ModeratorTube']
    kwargs.setdefault("VolumeList",  volumeList)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def EMBPhysicsRegionToolCfg(flags, name='EMBPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'EMB')
    volumeList = ['LArMgr::LAr::EMB::STAC']
    kwargs.setdefault("VolumeList",  volumeList)
    rangeEMB = 0.03
    if '_EMV' not in flags.Sim.PhysicsList and '_EMX' not in flags.Sim.PhysicsList:
        rangeEMB = 0.1
    kwargs.setdefault("ElectronCut", rangeEMB)
    kwargs.setdefault("PositronCut", rangeEMB)
    kwargs.setdefault("GammaCut",    rangeEMB)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def EMECPhysicsRegionToolCfg(flags, name='EMECPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'EMEC')
    volumeList = ['LArMgr::LAr::EMEC::Mother']
    kwargs.setdefault("VolumeList",  volumeList)
    rangeEMEC = 0.03
    if '_EMV' not in flags.Sim.PhysicsList and '_EMX' not in flags.Sim.PhysicsList:
        rangeEMEC = 0.1
    kwargs.setdefault("ElectronCut", rangeEMEC)
    kwargs.setdefault("PositronCut", rangeEMEC)
    kwargs.setdefault("GammaCut",    rangeEMEC)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def HECPhysicsRegionToolCfg(flags, name='HECPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'HEC')
    volumeList = ['LArMgr::LAr::HEC::LiquidArgon']
    kwargs.setdefault("VolumeList",  volumeList)
    rangeHEC = 0.03
    if '_EMV' not in flags.Sim.PhysicsList and '_EMX' not in flags.Sim.PhysicsList:
        rangeHEC = 1.0
    kwargs.setdefault("ElectronCut", rangeHEC)
    kwargs.setdefault("PositronCut", rangeHEC)
    kwargs.setdefault("GammaCut",    rangeHEC)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def FCALPhysicsRegionToolCfg(flags, name='FCALPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'FCAL')
    volumeList = ['LArMgr::LAr::FCAL::LiquidArgonC']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 0.03)
    kwargs.setdefault("PositronCut", 0.03)
    kwargs.setdefault("GammaCut",    0.03)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def EMECParaPhysicsRegionToolCfg(flags, name='EMECParaPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'EMECPara')
    volumeList = ['LArMgr::LAr::EMEC::Pos::InnerWheel', 'LArMgr::LAr::EMEC::Pos::OuterWheel',
                  'LArMgr::LAr::EMEC::Neg::InnerWheel', 'LArMgr::LAr::EMEC::Neg::OuterWheel']
    kwargs.setdefault("VolumeList",  volumeList)
    rangeEMEC = 0.03
    if '_EMV' not in flags.Sim.PhysicsList and '_EMX' not in flags.Sim.PhysicsList:
        rangeEMEC = 0.1
    kwargs.setdefault("ElectronCut", rangeEMEC)
    kwargs.setdefault("PositronCut", rangeEMEC)
    kwargs.setdefault("GammaCut",    rangeEMEC)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def FCALParaPhysicsRegionToolCfg(flags, name='FCALParaPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'FCALPara')
    volumeList = ['LArMgr::LAr::FCAL::Module1::Absorber']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 0.03)
    kwargs.setdefault("PositronCut", 0.03)
    kwargs.setdefault("GammaCut",    0.03)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def FCAL2ParaPhysicsRegionToolCfg(flags, name='FCAL2ParaPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'FCAL2Para')
    volumeList = ['LArMgr::LAr::FCAL::Module2::Absorber', 'LArMgr::LAr::FCAL::Module3::Absorber']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 0.03)
    kwargs.setdefault("PositronCut", 0.03)
    kwargs.setdefault("GammaCut",    0.03)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def PreSampLArPhysicsRegionToolCfg(flags, name='PreSampLArPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'PreSampLAr')
    volumeList = ['LArMgr::LAr::Endcap::Presampler::LiquidArgon']
    kwargs.setdefault("VolumeList",  volumeList)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def DeadMaterialPhysicsRegionToolCfg(flags, name='DeadMaterialPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'DeadMaterial')
    volumeList = []
    sectionList = []
    if flags.GeoModel.Run is LHCPeriod.Run1:
        # Avoid overlap with BeampipeFwdCut Region (ATLASSIM-6426)
        endRange = 47 if flags.Sim.BeamPipeSimMode is not BeamPipeSimMode.Normal else 49
        # Avoid overlap with FWDBeamLine region
        if not flags.Detector.GeometryFwdRegion and (flags.Detector.GeometryAFP or flags.Detector.GeometryALFA or flags.Detector.GeometryZDC):
            endRange = 46
        sectionList = list(range(16,endRange)) # does not include endRange
        sectionList += [ 51, 52, 53, 54 ]
    else:
        # Avoid overlap with BeampipeFwdCut Region (ATLASSIM-6426)
        endRange = 198 if flags.Sim.BeamPipeSimMode is not BeamPipeSimMode.Normal else 200
        # Avoid overlap with FWDBeamLine region
        if not flags.Detector.GeometryFwdRegion and (flags.Detector.GeometryAFP or flags.Detector.GeometryALFA or flags.Detector.GeometryZDC):
            endRange = 197
        sectionList = list(range(191,endRange)) # does not include endRange
        if flags.GeoModel.Run > LHCPeriod.Run4:
            print('DeadMaterialPhysicsRegionToolCfg: WARNING check that RUN2 beampipe volume names are correct for this geometry tag')
    for section in sectionList:
        volumeList += ['BeamPipe::SectionF'+str(section)]
    volumeList += ['LArMgr::LAr::Endcap::Cryostat::Cylinder',
                   'LArMgr::LAr::Endcap::Cryostat::Cylinder::Mixed',
                   'LArMgr::LAr::Endcap::Cryostat::Cone::Mixed',
                   'LArMgr::LAr::Endcap::Cryostat::Cone',
                   'DiskShieldingPlugs', 'ToroidShieldingInnerPlugs',
                   'ForwardShieldingMainCylinder']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 1.0)
    kwargs.setdefault("PositronCut", 1.0)
    kwargs.setdefault("GammaCut",    1.0)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


#Muon Regions
def DriftWallPhysicsRegionToolCfg(flags, name='DriftWallPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'DriftWall')
    volumeList = ['Muon::MDTDriftWall' if not flags.Muon.setupGeoModelXML else 'MuonR4::MDTDriftWall' ]
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 0.05)
    kwargs.setdefault("PositronCut", 0.05)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def DriftWall1PhysicsRegionToolCfg(flags, name='DriftWall1PhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'DriftWall1')
    volumeList = ['Muon::Endplug' if not flags.Muon.setupGeoModelXML else 'MuonR4::Endplug']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 1.0)
    kwargs.setdefault("PositronCut", 1.0)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def DriftWall2PhysicsRegionToolCfg(flags, name='DriftWall2PhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'DriftWall2')
    volumeList = ['Muon::SensitiveGas' if not flags.Muon.setupGeoModelXML else 'MuonR4::MDTDriftGas']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 1.0)
    kwargs.setdefault("PositronCut", 1.0)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def MuonSystemFastPhysicsRegionToolCfg(flags, name='MuonSystemFastPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'MuonSystemFastRegion')
    volumeList = []
    from SimulationConfig.SimEnums import CavernBackground
    if flags.Sim.CavernBackground in [CavernBackground.SignalWorld, CavernBackground.WriteWorld]:
        if flags.GeoModel.Run < LHCPeriod.Run4:
            volumeList += ['BeamPipe::BeamPipe', 'IDET::IDET']
        else:
            volumeList += ['BeamPipe::BeamPipe', 'ITK::ITK']
    volumeList = ['Muon::MuonSys']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 1.0)
    kwargs.setdefault("PositronCut", 1.0)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def MuonPhysicsRegionToolCfg(flags, name="MuonPhysicsRegionTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'MuonSys')
    volumeList = ['Muon::MuonSys']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 0.1)
    kwargs.setdefault("PositronCut", 0.1)
    kwargs.setdefault("GammaCut",    0.1)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


# Cavern Regions
def SX1PhysicsRegionToolCfg(flags, name='SX1PhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'SX1')
    volumeList = ['CavernInfra::SX1Air']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 2000.)
    kwargs.setdefault("PositronCut", 2000.)
    kwargs.setdefault("GammaCut",    2000.)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def BedrockPhysicsRegionToolCfg(flags, name='BedrockPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'BEDROCK')
    volumeList = ['CavernInfra::BEDROCK']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 1000000.)
    kwargs.setdefault("PositronCut", 1000000.)
    kwargs.setdefault("GammaCut",    1000000.)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


def CavernShaftsConcretePhysicsRegionToolCfg(flags, name='CavernShaftsConcretePhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'CAV_SHAFTS_CONC')
    volumeList = ['CavernInfra::CAV_SHAFTS_CONC']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 150.)
    kwargs.setdefault("PositronCut", 150.)
    kwargs.setdefault("GammaCut",    150.)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result


## Deliberately leaving this commented out for now as it may be needed in the future
##def CavernShaftsAirPhysicsRegionToolCfg(flags, name='CavernShaftsAirPhysicsRegionTool', **kwargs):
##    result = ComponentAccumulator()
##    kwargs.setdefault("RegionName", 'CAV_SHAFTS_AIR')
##    volumeList = ['CavernInfra::CAV_SHAFTS_AIR']
##    kwargs.setdefault("VolumeList",  volumeList)
##    kwargs.setdefault("ElectronCut", 2000.)
##    kwargs.setdefault("PositronCut", 2000.)
##    kwargs.setdefault("GammaCut",    2000.)
##    result.setPrivateTools(RegionCreator(name, **kwargs))
##    return result


# CTB Regions
def SCTSiliconPhysicsRegionToolCfg(flags, name='SCTSiliconPhysicsRegionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", 'SCTSiliconRegion')
    volumeList = ['SCT::ECSensor0']
    kwargs.setdefault("VolumeList",  volumeList)
    kwargs.setdefault("ElectronCut", 0.01)
    result.setPrivateTools(RegionCreator(name, **kwargs))
    return result
