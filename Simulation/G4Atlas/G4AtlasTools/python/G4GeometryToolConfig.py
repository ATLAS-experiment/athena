# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from AthenaCommon import Logging

from SimulationConfig.SimEnums import BeamPipeSimMode, CalibrationRun, CavernBackground, LArParameterization

#the physics region tools
from G4AtlasTools.G4PhysicsRegionConfig import SX1PhysicsRegionToolCfg, BedrockPhysicsRegionToolCfg, CavernShaftsConcretePhysicsRegionToolCfg, PixelPhysicsRegionToolCfg, SCTPhysicsRegionToolCfg, TRTPhysicsRegionToolCfg, TRT_ArPhysicsRegionToolCfg,ITkPixelPhysicsRegionToolCfg,ITkStripPhysicsRegionToolCfg,HGTDPhysicsRegionToolCfg,BeampipeFwdCutPhysicsRegionToolCfg, FWDBeamLinePhysicsRegionToolCfg, CALOPhysicsRegionToolCfg, EMBPhysicsRegionToolCfg, EMECPhysicsRegionToolCfg, HECPhysicsRegionToolCfg, FCALPhysicsRegionToolCfg, FCAL2ParaPhysicsRegionToolCfg, EMECParaPhysicsRegionToolCfg, FCALParaPhysicsRegionToolCfg, PreSampLArPhysicsRegionToolCfg, DeadMaterialPhysicsRegionToolCfg #, FwdRegionPhysicsRegionToolCfg
from G4AtlasTools.G4PhysicsRegionConfig import DriftWallPhysicsRegionToolCfg, DriftWall1PhysicsRegionToolCfg, DriftWall2PhysicsRegionToolCfg, MuonSystemFastPhysicsRegionToolCfg

#the field config tools
from G4AtlasTools.G4FieldConfig import ATLASFieldManagerToolCfg, TightMuonsATLASFieldManagerToolCfg, BeamPipeFieldManagerToolCfg, InDetFieldManagerToolCfg, ITkFieldManagerToolCfg, MuonsOnlyInCaloFieldManagerToolCfg, MuonFieldManagerToolCfg, Q1FwdFieldManagerToolCfg, Q2FwdFieldManagerToolCfg, Q3FwdFieldManagerToolCfg, D1FwdFieldManagerToolCfg, D2FwdFieldManagerToolCfg, Q4FwdFieldManagerToolCfg, Q5FwdFieldManagerToolCfg, Q6FwdFieldManagerToolCfg, Q7FwdFieldManagerToolCfg, Q1HKickFwdFieldManagerToolCfg, Q1VKickFwdFieldManagerToolCfg, Q2HKickFwdFieldManagerToolCfg, Q2VKickFwdFieldManagerToolCfg, Q3HKickFwdFieldManagerToolCfg, Q3VKickFwdFieldManagerToolCfg, Q4VKickAFwdFieldManagerToolCfg, Q4HKickFwdFieldManagerToolCfg, Q4VKickBFwdFieldManagerToolCfg, Q5HKickFwdFieldManagerToolCfg,  Q6VKickFwdFieldManagerToolCfg, FwdRegionFieldManagerToolCfg

from G4AtlasTools.G4AtlasToolsConfig import SensitiveDetectorMasterToolCfg

CylindricalEnvelope, PolyconicalEnvelope, MaterialDescriptionTool,VoxelDensityTool,G4AtlasDetectorConstructionTool,BoxEnvelope=CompFactory.getComps("CylindricalEnvelope","PolyconicalEnvelope","MaterialDescriptionTool","VoxelDensityTool","G4AtlasDetectorConstructionTool","BoxEnvelope")

from AthenaCommon.SystemOfUnits import mm, cm, m

#ToDo - finish migrating this
#from ForwardRegionProperties.ForwardRegionPropertiesToolConfig import ForwardRegionPropertiesCfg

#put it here to avoid circular import?
def G4GeometryNotifierSvcCfg(flags, name="G4GeometryNotifierSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ActivateLVNotifier", True)
    kwargs.setdefault("ActivatePVNotifier", False)
    result.addService(CompFactory.G4GeometryNotifierSvc(name, **kwargs), primary = True)
    return result


def GeoDetectorToolCfg(flags, name='GeoDetectorTool', **kwargs):
    result = ComponentAccumulator()
    from Geo2G4.Geo2G4Config import Geo2G4SvcCfg
    kwargs.setdefault("Geo2G4Svc", result.getPrimaryAndMerge(Geo2G4SvcCfg(flags)).name)
    #add the GeometryNotifierSvc
    kwargs.setdefault("GeometryNotifierSvc", result.getPrimaryAndMerge(G4GeometryNotifierSvcCfg(flags)).name)
    result.setPrivateTools(CompFactory.GeoDetectorTool(name, **kwargs))
    return result


def BeamPipeGeoDetectorToolCfg(flags, name='BeamPipe', **kwargs):
    #set up geometry
    from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
    result = BeamPipeGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "BeamPipe")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def PixelGeoDetectorToolCfg(flags, name='Pixel', **kwargs):
    #set up geometry
    from PixelGeoModel.PixelGeoModelConfig import PixelSimulationGeometryCfg
    result = PixelSimulationGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "Pixel")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def SCTGeoDetectorToolCfg(flags, name='SCT', **kwargs):
    #set up geometry
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_SimulationGeometryCfg
    result = SCT_SimulationGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "SCT")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def PLRGeoDetectorToolCfg(flags, name='PLR', **kwargs):
    #set up geometry
    from PLRGeoModelXml.PLR_GeoModelConfig import PLR_GeometryCfg
    result = PLR_GeometryCfg(flags)
    kwargs.setdefault("DetectorName", "PLR")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def ITkPixelGeoDetectorToolCfg(flags, name='ITkPixel', **kwargs):
    #set up geometry
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelSimulationGeometryCfg
    result = ITkPixelSimulationGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "ITkPixel")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def ITkStripGeoDetectorToolCfg(flags, name='ITkStrip', **kwargs):
    #set up geometry
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripSimulationGeometryCfg
    result = ITkStripSimulationGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "ITkStrip")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def HGTDGeoDetectorToolCfg(flags, name='HGTD', **kwargs):
    #set up geometry
    if flags.HGTD.Geometry.useGeoModelXml:
        from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_SimulationGeometryCfg
    else:
        from HGTD_GeoModel.HGTD_GeoModelConfig import HGTD_SimulationGeometryCfg

    result = HGTD_SimulationGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "HGTD")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def HGTDServiceToolCfg(flags, name='HGTDServices', **kwargs):
    #set up geometry
    from HGTD_GeoModelXml.HGTDServiceGeoModelConfig import HGTDServiceGeoModelCfg
    result = HGTDServiceGeoModelCfg(flags)
    kwargs.setdefault("DetectorName", "HGTDServices")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def TRTGeoDetectorToolCfg(flags, name='TRT', **kwargs):
    #set up geometry
    from TRT_GeoModel.TRT_GeoModelConfig import TRT_SimulationGeometryCfg
    result = TRT_SimulationGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "TRT")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def IDetServicesMatGeoDetectorToolCfg(flags, name='IDetServicesMat', **kwargs):
    #set up geometry
    from InDetServMatGeoModel.InDetServMatGeoModelConfig import InDetServiceMaterialCfg
    result = InDetServiceMaterialCfg(flags)
    kwargs.setdefault("DetectorName", "InDetServMat")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def LArMgrGeoDetectorToolCfg(flags, name='LArMgr', **kwargs):
    #set up geometry
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result = LArGMCfg(flags)
    kwargs.setdefault("DetectorName", "LArMgr")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def TileGeoDetectorToolCfg(flags, name='Tile', **kwargs):
    #set up geometry
    from TileGeoModel.TileGMConfig import TileGMCfg
    result = TileGMCfg(flags)
    kwargs.setdefault("DetectorName", "Tile")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def LucidGeoDetectorToolCfg(flags, name='Lucid', **kwargs):
    #set up geometry
    from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
    result=ForDetGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "LUCID")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def ALFAGeoDetectorToolCfg(flags, name='ALFA', **kwargs):
    #set up geometry
    from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
    result = ForDetGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "ALFA")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def ZDCGeoDetectorToolCfg(flags, name='ZDC', **kwargs):
    #set up geometry
    from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
    result = ForDetGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "ZDC")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def AFPGeoDetectorToolCfg(flags, name='AFP', **kwargs):
    #set up geometry
    from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
    result = ForDetGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "AFP")
    kwargs.setdefault("GeoDetectorName", "AFP_GeoModel")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def FwdRegionGeoDetectorToolCfg(flags, name='FwdRegion', **kwargs):
    #set up geometry
    from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
    result = ForDetGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "FwdRegion")
    kwargs.setdefault("GeoDetectorName", "ForwardRegionGeoModel")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def MuonGeoDetectorToolCfg(flags, name='Muon', **kwargs):
    #set up geometry
    result = ComponentAccumulator()
    if not flags.Muon.usePhaseIIGeoSetup:
        from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
        result.merge(MuonGeoModelCfg(flags))
        kwargs.setdefault("DetectorName", "Muon")
    else:
        kwargs.setdefault("DetectorName", "MuonR4")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def ITKEnvelopeCfg(flags, name="ITK", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("DetectorName", "ITK")
    kwargs.setdefault("InnerRadius", 28.8*mm)
    kwargs.setdefault("OuterRadius", 1.148*m)
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        # ITk should include the HGTD (3420 mm < |z| < 3545 mm) for now
        kwargs.setdefault("dZ", 354.5*cm)
    else:
        kwargs.setdefault("dZ", 347.5*cm)

    SubDetectorList=[]
    if flags.Detector.GeometryITkPixel:
        toolITkPixel = result.popToolsAndMerge(ITkPixelGeoDetectorToolCfg(flags))
        SubDetectorList += [toolITkPixel]
    if flags.Detector.GeometryITkStrip:
        toolITkStrip = result.popToolsAndMerge(ITkStripGeoDetectorToolCfg(flags))
        SubDetectorList += [toolITkStrip]
    if flags.Detector.GeometryPLR:
        toolPLR = result.popToolsAndMerge(PLRGeoDetectorToolCfg(flags))
        SubDetectorList += [toolPLR]
    # TODO: for now HGTD is also here
    if flags.Detector.GeometryHGTD:
        toolHGTD = result.popToolsAndMerge(HGTDGeoDetectorToolCfg(flags))
        SubDetectorList += [toolHGTD]

    kwargs.setdefault("SubDetectors", SubDetectorList)
    result.setPrivateTools(CylindricalEnvelope(name, **kwargs))
    return result


def IDETEnvelopeCfg(flags, name="IDET", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "IDET")
    innerRadius = 37.*mm # RUN1 default
    if flags.GeoModel.Run in [LHCPeriod.Run2, LHCPeriod.Run3]:
        innerRadius = 28.9*mm #29.15*mm
    kwargs.setdefault("InnerRadius", innerRadius)
    kwargs.setdefault("OuterRadius", 1.148*m)
    kwargs.setdefault("dZ", 347.5*cm)

    SubDetectorList=[]
    if flags.Detector.GeometryPixel:
        toolPixel = result.popToolsAndMerge(PixelGeoDetectorToolCfg(flags))
        SubDetectorList += [toolPixel]
    if flags.Detector.GeometrySCT:
        toolSCT = result.popToolsAndMerge(SCTGeoDetectorToolCfg(flags))
        SubDetectorList += [toolSCT]
    if flags.Detector.GeometryTRT:
        toolTRT = result.popToolsAndMerge(TRTGeoDetectorToolCfg(flags))
        SubDetectorList += [toolTRT]

    toolIDetServices = result.popToolsAndMerge(IDetServicesMatGeoDetectorToolCfg(flags))
    SubDetectorList += [toolIDetServices]
    kwargs.setdefault("SubDetectors", SubDetectorList)
    result.setPrivateTools(CylindricalEnvelope(name, **kwargs))
    return result


def CALOEnvelopeCfg(flags, name="CALO", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("DetectorName", "CALO")
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        # Make room for HGTD (3420 mm < |z| < 3545 mm) but include JMTube and JMPlug
        kwargs.setdefault("NSurfaces", 22)
        kwargs.setdefault("InnerRadii", [41.,41.,41.,41.,41.,41.,64.,64.,120.,120.,1148.,1148.,120.,120.,64.,64.,41.,41.,41.,41.,41.,41.]) #FIXME Units?
        kwargs.setdefault("OuterRadii", [415.,415.,3795.,3795.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,3795.,3795.,415.,415.]) #FIXME Units?
        kwargs.setdefault("ZSurfaces", [-6781.,-6747.,-6747.,-6530.,-6530.,-4587.,-4587.,-4472.,-4472.,-3545.,-3545.,3545.,3545.,4472.,4472.,4587.,4587.,6530.,6530.,6747.,6747.,6781.]) #FIXME Units?
    else:
        kwargs.setdefault("NSurfaces", 18)
        kwargs.setdefault("InnerRadii", [41.,41.,41.,41.,41.,41.,120.,120.,1148.,1148.,120.,120.,41.,41.,41.,41.,41.,41.]) #FIXME Units?
        kwargs.setdefault("OuterRadii", [415.,415.,3795.,3795.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,4251.,3795.,3795.,415.,415.]) #FIXME Units?
        kwargs.setdefault("ZSurfaces", [-6781.,-6747.,-6747.,-6530.,-6530.,-4587.,-4587.,-3475.,-3475.,3475.,3475.,4587.,4587.,6530.,6530.,6747.,6747.,6781.]) #FIXME Units?
    SubDetectorList=[]
    if flags.Detector.GeometryLAr:
        toolLArMgr = result.popToolsAndMerge(LArMgrGeoDetectorToolCfg(flags))
        SubDetectorList += [ toolLArMgr ]
    if flags.Detector.GeometryTile:
        toolTile = result.popToolsAndMerge(TileGeoDetectorToolCfg(flags))
        SubDetectorList += [ toolTile ]
    kwargs.setdefault("SubDetectors", SubDetectorList)
    result.setPrivateTools(PolyconicalEnvelope(name, **kwargs))
    return result


def ForwardRegionEnvelopeCfg(flags, name='ForwardRegion', **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("DetectorName", "ForDetEnvelope")
    SubDetectorList=[]

    if flags.Detector.GeometryFwdRegion: # I.e. fully simulate the FwdRegion rather than using BeamTransport to get to Forward Detectors
        toolFwdRegion = result.popToolsAndMerge(FwdRegionGeoDetectorToolCfg(flags))
        SubDetectorList += [ toolFwdRegion ]

        #TODO - migrate this over (WIP at the moment) (dnoel)
        #toolFwdRegionProperties = ForwardRegionPropertiesCfg(flags)
        #result.addPublicTool(toolFwdRegionProperties) #add this as a service later?
    if flags.Detector.GeometryZDC:
        toolZDC = result.popToolsAndMerge(ZDCGeoDetectorToolCfg(flags))
        SubDetectorList += [ toolZDC ]
    if flags.Detector.GeometryALFA:
        toolALFA = result.popToolsAndMerge(ALFAGeoDetectorToolCfg(flags))
        SubDetectorList += [ toolALFA ]
    if flags.Detector.GeometryAFP:
        toolAFP = result.popToolsAndMerge(AFPGeoDetectorToolCfg(flags))
        SubDetectorList += [ toolAFP ]
    kwargs.setdefault("SubDetectors", SubDetectorList)
    ##FIXME Should this really be a GeoDetectorTool???
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def MUONEnvelopeCfg(flags, name="MUONQ02", **kwargs): #FIXME rename to MUON when safe (IS IT SAFE?))
    result = ComponentAccumulator()

    kwargs.setdefault("DetectorName", "MUONQ02") #FIXME rename to MUON when safe
    kwargs.setdefault("NSurfaces", 34)
    kwargs.setdefault("InnerRadii", [1050.,1050.,1050.,1050.,436.7,436.7,279.,279.,70.,70.,420.,420.,3800.,3800.,4255.,4255.,4255.,4255.,4255.,4255.,3800.,3800.,420.,420.,70.,70.,279.,279.,436.7,436.7,1050.,1050.,1050.,1050.]) #FIXME Units?
    kwargs.setdefault("OuterRadii", [1500.,1500.,2750.,2750.,12650.,12650.,13400.,13400.,14200.,14200.,14200.,14200.,14200.,14200.,14200.,14200.,13000.,13000.,14200.,14200.,14200.,14200.,14200.,14200.,14200.,14200.,13400.,13400.,12650.,12650.,2750.,2750.,1500.,1500.]) #FIXME Units?
    kwargs.setdefault("ZSurfaces", [-26046.,-23001.,-23001.,-22030.,-22030.,-18650.,-18650.,-12900.,-12900.,-6783.,-6783.,-6748.,-6748.,-6550.,-6550.,-4000.,-4000.,4000.,4000.,6550.,6550.,6748.,6748.,6783.,6783.,12900.,12900.,18650.,18650.,22030.,22030.,23001.,23001.,26046.]) #FIXME Units?
    SubDetectorList=[]
    if flags.Detector.GeometryMuon:
        toolMuon = result.popToolsAndMerge(MuonGeoDetectorToolCfg(flags))
        SubDetectorList += [ toolMuon ]

    kwargs.setdefault("SubDetectors", SubDetectorList)
    result.setPrivateTools(PolyconicalEnvelope(name, **kwargs))
    return result


def CosmicShortCutCfg(flags, name="CosmicShortCut", **kwargs):
    kwargs.setdefault("DetectorName", "TTR_BARREL")
    kwargs.setdefault("NSurfaces", 14)
    kwargs.setdefault("InnerRadii", [70.,70.,12500.,12500.,12500.,12500.,13000.,13000.,12500.,12500.,12500.,12500.,70.,70.]) #FIXME Units?
    kwargs.setdefault("OuterRadii", [12501.,12501.,12501.,12501.,13001.,13001.,13001.,13001.,13001.,13001.,12501.,12501.,12501.,12501.]) #FIXME Units?
    kwargs.setdefault("ZSurfaces", [-22031.,-22030.,-22030.,-12901.,-12901.,-12900.,-12900., 12900.,12900.,12901.,12901.,22030.,22030.,22031.]) #FIXME Units?
    SubDetectorList=[]
    kwargs.setdefault("SubDetectors", SubDetectorList)
    return PolyconicalEnvelope(name, **kwargs)


def generateSubDetectorList(flags):
    result = ComponentAccumulator()
    SubDetectorList=[]

    if flags.Beam.Type is BeamType.Cosmics or flags.Sim.CavernBackground not in [CavernBackground.Off, CavernBackground.Signal]:
        if flags.Beam.Type is BeamType.Cosmics and hasattr(flags, "Sim.ReadTR"):
            SubDetectorList += [ CosmicShortCutCfg(flags) ]

    if flags.Detector.GeometryMuon:
        accMuon = MUONEnvelopeCfg(flags)
        toolMuon = accMuon.popPrivateTools()
        SubDetectorList += [ toolMuon ] #FIXME rename to MUON when safe
    if flags.Detector.GeometryID:
        toolIDET = result.popToolsAndMerge(IDETEnvelopeCfg(flags))
        SubDetectorList += [ toolIDET ]
    if flags.Detector.GeometryITk or flags.Detector.GeometryHGTD:  # TODO: HGTD is also here for now
        toolITK = result.popToolsAndMerge(ITKEnvelopeCfg(flags))
        SubDetectorList += [ toolITK ]
    if flags.Detector.GeometryCalo:
        toolCALO = result.popToolsAndMerge(CALOEnvelopeCfg(flags))
        SubDetectorList += [ toolCALO ]
    if flags.Detector.GeometryMuon:
        result.merge(accMuon) #add the acc later to match the old style config
    if flags.Detector.GeometryBpipe:
        toolBpipe = result.popToolsAndMerge(BeamPipeGeoDetectorToolCfg(flags))
        SubDetectorList += [ toolBpipe ]
    if flags.Detector.GeometryLucid:
        toolLucid = result.popToolsAndMerge(LucidGeoDetectorToolCfg(flags))
        SubDetectorList += [ toolLucid ]
    if flags.Detector.GeometryFwdRegion or flags.Detector.GeometryAFP or flags.Detector.GeometryALFA or flags.Detector.GeometryZDC:
        toolFwdRegion = result.popToolsAndMerge(ForwardRegionEnvelopeCfg(flags))
        SubDetectorList += [ toolFwdRegion ]

    #if DetFlags.Muon_on(): #HACK
    #    SubDetectorList += ['MUONQ02'] #FIXME rename to MUON when safe #HACK
    #SubDetectorList += generateFwdSubDetectorList() #FIXME Fwd Detectors not supported yet.
    result.setPrivateTools(SubDetectorList)
    return result


def ATLASEnvelopeCfg(flags, name="Atlas", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("DetectorName", "Atlas")
    kwargs.setdefault("NSurfaces", 18)
    ## InnerRadii
    innerRadii = [0.0] * 18
    kwargs.setdefault("InnerRadii", innerRadii)

    ## Shrink the global ATLAS envelope to the activated detectors,
    ## except when running on special setups.

    ## OuterRadii
    AtlasForwardOuterR = 2751.
    AtlasOuterR1 = 14201.
    AtlasOuterR2 = 14201.
    # if flags.Beam.Type is not BeamType.Cosmics and not flags.Detector.GeometryMuon and not \
    #    (flags.Sim.CavernBackground is not CavernBackground.Signal):
    if not (flags.Detector.GeometryMuon or flags.Detector.GeometryCavern):
        AtlasOuterR1 = 4251.
        AtlasOuterR2 = 4251.
        if not flags.Detector.GeometryCalo:
            AtlasOuterR1 = 1150.
            AtlasOuterR2 = 1150.

    outerRadii = [0.0] * 18
    for i in (0, 1, 16, 17):
        outerRadii[i] = 1501.
    for i in (2, 3, 14, 15):
        outerRadii[i] = AtlasForwardOuterR
    for i in (4, 5, 12, 13):
        outerRadii[i] = AtlasOuterR2
    for i in range(6, 12):
        outerRadii[i] = AtlasOuterR1

    ## World R range
    if flags.Sim.WorldRRange:
        routValue = flags.Sim.WorldRRange
        if flags.Sim.WorldRRange > max(AtlasOuterR1, AtlasOuterR2):
            for i in range(4, 14):
                outerRadii[i] = routValue
        else:
            raise RuntimeError('getATLASEnvelope: ERROR flags.Sim.WorldRRange must be > %f. Current value %f' % (max(AtlasOuterR1, AtlasOuterR2), routValue) )
    kwargs.setdefault("OuterRadii", outerRadii)

    ## ZSurfaces
    zSurfaces = [-26046., -23001., -23001., -22031., -22031., -12899., -12899., -6741., -6741.,  6741.,  6741.,  12899., 12899., 22031., 22031., 23001., 23001., 26046.] # FIXME units mm??

    if flags.Detector.GeometryFwdRegion or flags.Detector.GeometryAFP or flags.Detector.GeometryALFA or flags.Detector.GeometryZDC:
        zSurfaces[0]  = -400000.
        zSurfaces[17] =  400000.

    #leave a check in for WorldRrange and WorldZrange?
    if flags.Sim.WorldZRange:
        if flags.Sim.WorldZRange < 26046.:
              raise RuntimeError('getATLASEnvelope: ERROR flags.Sim.WorldZRange must be > 26046. Current value: %f' % flags.Sim.WorldZRange)
        zSurfaces[17] =  flags.Sim.WorldZRange + 100.
        zSurfaces[16] =  flags.Sim.WorldZRange + 50.
        zSurfaces[15] =  flags.Sim.WorldZRange + 50.
        zSurfaces[14] =  flags.Sim.WorldZRange
        zSurfaces[13] =  flags.Sim.WorldZRange
        zSurfaces[0] =  -flags.Sim.WorldZRange - 100.
        zSurfaces[1] =  -flags.Sim.WorldZRange - 50.
        zSurfaces[2] =  -flags.Sim.WorldZRange - 50.
        zSurfaces[3] =  -flags.Sim.WorldZRange
        zSurfaces[4] =  -flags.Sim.WorldZRange

    kwargs.setdefault("ZSurfaces", zSurfaces)
    SubDetectorList = result.popToolsAndMerge(generateSubDetectorList(flags))
    kwargs.setdefault("SubDetectors", SubDetectorList)
    result.setPrivateTools(PolyconicalEnvelope(name, **kwargs))
    return result


def MaterialDescriptionToolCfg(flags, name="MaterialDescriptionTool", **kwargs):
    ## kwargs.setdefault("SomeProperty", aValue)
    result = ComponentAccumulator()
    kwargs.setdefault("TestBeam", flags.Beam.Type is BeamType.TestBeam)
    result.setPrivateTools(MaterialDescriptionTool(name, **kwargs))
    return result


def VoxelDensityToolCfg(flags, name="VoxelDensityTool", **kwargs):
    ## kwargs.setdefault("SomeProperty", aValue)
    voxelDensitySettings = {}
    if flags.Detector.GeometryITkPixel:
        voxelDensitySettings["ITkPixelDetector"] = 0.05
    if flags.Detector.GeometryITkStrip:
        voxelDensitySettings["ITkStrip::Barrel"] = 0.05
        voxelDensitySettings["ITkStrip::ITkStrip_Forward"] = 0.05
        ##The below is only needed temporarily, while we wait for
        ##improved naming to be propagated to all necessary geo tags
        voxelDensitySettings["ITkStrip::SCT_Forward"] = 0.05
    kwargs.setdefault("VolumeVoxellDensityLevel",voxelDensitySettings)
    result = ComponentAccumulator()
    result.setPrivateTools(VoxelDensityTool(name, **kwargs))
    return result


def ATLAS_RegionCreatorListCfg(flags):
    result = ComponentAccumulator()
    regionCreatorList = []

    if flags.Detector.GeometryCavern or flags.Sim.CavernBackground not in [CavernBackground.Off, CavernBackground.Signal]:
        regionCreatorList += [
            result.popToolsAndMerge(SX1PhysicsRegionToolCfg(flags)),
            result.popToolsAndMerge(BedrockPhysicsRegionToolCfg(flags)),
            result.popToolsAndMerge(CavernShaftsConcretePhysicsRegionToolCfg(flags))]
        #regionCreatorList += ['CavernShaftsAirPhysicsRegionTool'] # Not used currently
    if flags.Detector.GeometryID:
        if flags.Detector.GeometryPixel:
            regionCreatorList += [result.popToolsAndMerge(PixelPhysicsRegionToolCfg(flags))]
        if flags.Detector.GeometrySCT:
            regionCreatorList += [result.popToolsAndMerge(SCTPhysicsRegionToolCfg(flags))]
        if flags.Detector.GeometryTRT:
            regionCreatorList += [result.popToolsAndMerge(TRTPhysicsRegionToolCfg(flags))]
            if flags.GeoModel.Run in [LHCPeriod.Run2, LHCPeriod.Run3]:
                # TODO: should we support old geometry tags with Run == "UNDEFINED" and flags.GeoModel.IBLLayout not in ["noIBL", "UNDEFINED"]?
                regionCreatorList += [result.popToolsAndMerge(TRT_ArPhysicsRegionToolCfg(flags))] #'TRT_KrPhysicsRegionTool'
        # FIXME dislike the ordering here, but try to maintain the same ordering as in the old configuration.
        if flags.Detector.GeometryBpipe:
            if flags.Sim.BeamPipeSimMode is not BeamPipeSimMode.Normal:
                regionCreatorList += [result.popToolsAndMerge(BeampipeFwdCutPhysicsRegionToolCfg(flags))]
            if not flags.Detector.GeometryFwdRegion and (flags.Detector.GeometryAFP or flags.Detector.GeometryALFA or flags.Detector.GeometryZDC):
                regionCreatorList += [result.popToolsAndMerge(FWDBeamLinePhysicsRegionToolCfg(flags))]
    if flags.Detector.GeometryITk:
        if flags.Detector.GeometryITkPixel:
            regionCreatorList += [result.popToolsAndMerge(ITkPixelPhysicsRegionToolCfg(flags))]
        if flags.Detector.GeometryITkStrip:
            regionCreatorList += [result.popToolsAndMerge(ITkStripPhysicsRegionToolCfg(flags))]
    if flags.Detector.GeometryHGTD:
        regionCreatorList += [result.popToolsAndMerge(HGTDPhysicsRegionToolCfg(flags))]
    if flags.Detector.GeometryITk or flags.Detector.GeometryHGTD:  # TODO: I do not know why this is only for ITk (and HGTD)
        # FIXME dislike the ordering here, but try to maintain the same ordering as in the old configuration.
        if flags.Detector.GeometryBpipe:
            if flags.Sim.BeamPipeSimMode is not BeamPipeSimMode.Normal:
                regionCreatorList += [result.popToolsAndMerge(BeampipeFwdCutPhysicsRegionToolCfg(flags))]
            if not flags.Detector.GeometryFwdRegion and (flags.Detector.GeometryAFP or flags.Detector.GeometryALFA or flags.Detector.GeometryZDC):
                regionCreatorList += [result.popToolsAndMerge(FWDBeamLinePhysicsRegionToolCfg(flags))]
    if flags.Detector.GeometryCalo:
        if flags.Detector.GeometryLAr:
            # Shower parameterization overrides the calibration hit flag
            if flags.Sim.LArParameterization is not LArParameterization.NoFrozenShowers \
               and flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile, CalibrationRun.LArTileZDC, CalibrationRun.DeadLAr]:
                Logging.log.info('You requested both calibration hits and frozen showers / parameterization in the LAr.')
                Logging.log.info('  Such a configuration is not allowed, and would give junk calibration hits where the showers are modified.')
                Logging.log.info('  Please try again with a different value of either flags.Sim.LArParameterization (' + str(flags.Sim.LArParameterization.value) + ') or flags.Sim.CalibrationRun ('+str(flags.Sim.CalibrationRun.value)+')')
                raise RuntimeError('Configuration not allowed')
            regionCreatorList += [
                result.popToolsAndMerge(EMBPhysicsRegionToolCfg(flags)),
                result.popToolsAndMerge(EMECPhysicsRegionToolCfg(flags)),
                result.popToolsAndMerge(HECPhysicsRegionToolCfg(flags)),
                result.popToolsAndMerge(FCALPhysicsRegionToolCfg(flags))]
            fullCommandList = '\t'.join(flags.Sim.G4Commands)
            if flags.Sim.LArParameterization is LArParameterization.FrozenShowers or 'EMECPara' in fullCommandList:
                # EMECPara Physics region is used by Woodcock tracking
                # and by EMEC Frozen Showers (the latter is not part
                # of production configurations).  NB The 'EMB'
                # PhysicsRegion seems to be used by the Frozen Showers
                # parametrization also. Unclear if this is correct -
                # not a big issue as Frozen Showers are not used in
                # the EMB in production configurations.
                regionCreatorList += [
                    result.popToolsAndMerge(EMECParaPhysicsRegionToolCfg(flags))]
            if flags.Sim.LArParameterization is not LArParameterization.NoFrozenShowers:
                regionCreatorList += [result.popToolsAndMerge(FCALParaPhysicsRegionToolCfg(flags)),
                                      result.popToolsAndMerge(FCAL2ParaPhysicsRegionToolCfg(flags))]
                if flags.Sim.LArParameterization in [LArParameterization.DeadMaterialFrozenShowers, LArParameterization.FrozenShowersFCalOnly, LArParameterization.FastCaloSim]: # TODO Check this makes sense
                    pass
                    regionCreatorList += [
                        result.popToolsAndMerge(PreSampLArPhysicsRegionToolCfg(flags)),
                        result.popToolsAndMerge(DeadMaterialPhysicsRegionToolCfg(flags))]
            if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
                regionCreatorList += [result.popToolsAndMerge(CALOPhysicsRegionToolCfg(flags))]
    ## TODO Follow-up about whether this is still needed
    #if flags.Detector.GeometryFwdRegion:
    #    regionCreatorList +=[result.popToolsAndMerge(FwdRegionPhysicsRegionToolCfg(flags))]
    if flags.Detector.GeometryMuon:
        #todo - add the line below
        regionCreatorList += [
            result.popToolsAndMerge(DriftWallPhysicsRegionToolCfg(flags)),
            result.popToolsAndMerge(DriftWall1PhysicsRegionToolCfg(flags)),
            result.popToolsAndMerge(DriftWall2PhysicsRegionToolCfg(flags))]
        if flags.Sim.CavernBackground not in [CavernBackground.Off, CavernBackground.Read] and not flags.Sim.RecordFlux:
            regionCreatorList += [result.popToolsAndMerge(MuonSystemFastPhysicsRegionToolCfg(flags))]
    result.setPrivateTools(regionCreatorList)
    return result


def TB_RegionCreatorListCfg(flags):
    regionCreatorList = []
    result = ComponentAccumulator()
    # Deliberately left commented out for now
    #TODO - migrate below>>
    #if (flags.GeoModel.AtlasVersion=="tb_LArH6_2003"):
    #    if (flags.Detector.GeometryLAr):
    #        regionCreatorList += [FCALPhysicsRegionTool(flags)]
    #elif (flags.GeoModel.AtlasVersion=="tb_LArH6_2002"):
    #    if (flags.Detector.GeometryLAr):
    #        regionCreatorList += [HECPhysicsRegionTool(flags)]
    #elif (flags.GeoModel.AtlasVersion=="tb_LArH6EC_2002"):
    #    if (flags.Detector.GeometryLAr):
    #        regionCreatorList += [EMECPhysicsRegionTool(flags)]
    #elif (flags.GeoModel.AtlasVersion=="tb_LArH6_2004"):
    #    if (flags.Sim.LArTB_H6Hec.get_Value()):
    #        regionCreatorList += [HECPhysicsRegionTool(flags)]
    #    if (flags.Sim.LArTB_H6Emec.get_Value()):
    #        regionCreatorList += [EMECPhysicsRegionTool(flags)]
    #    if (flags.Sim.LArTB_H6Fcal.get_Value()):
    #        regionCreatorList += [FCALPhysicsRegionTool(flags)]
    #<<migrate above
    result.setPrivateTools(regionCreatorList)
    return result


#########################################################################
def ATLAS_FieldMgrListCfg(flags):
    result = ComponentAccumulator()
    fieldMgrList = []

    if flags.Sim.TightMuonStepping:
        tool  = result.popToolsAndMerge(TightMuonsATLASFieldManagerToolCfg(flags))
        fieldMgrList += [tool]
    else:
        tool  = result.popToolsAndMerge(ATLASFieldManagerToolCfg(flags))
        fieldMgrList += [tool]
    if flags.Detector.GeometryBpipe:
        tool  = result.popToolsAndMerge(BeamPipeFieldManagerToolCfg(flags))
        fieldMgrList += [tool]
    if flags.Detector.GeometryID:
        tool  = result.popToolsAndMerge(InDetFieldManagerToolCfg(flags))
        fieldMgrList += [tool]
    if flags.Detector.GeometryITk or flags.Detector.GeometryHGTD:  # TODO: while HGTD is included in the ITK envelope
        tool  = result.popToolsAndMerge(ITkFieldManagerToolCfg(flags))
        fieldMgrList += [tool]
    if flags.Detector.GeometryCalo and flags.Sim.MuonFieldOnlyInCalo:
        tool  = result.popToolsAndMerge(MuonsOnlyInCaloFieldManagerToolCfg(flags))
        fieldMgrList += [tool]
    if flags.Detector.GeometryMuon:
        tool  = result.popToolsAndMerge(MuonFieldManagerToolCfg(flags))
        fieldMgrList += [tool]

    #sort these forward ones later
    if flags.Detector.GeometryFwdRegion: #or forward?
      fieldMgrList+=[
          result.popToolsAndMerge(Q1FwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q2FwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q3FwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(D1FwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(D2FwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q4FwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q5FwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q6FwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q7FwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q1HKickFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q1VKickFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q2HKickFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q2VKickFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q3HKickFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q3VKickFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q4VKickAFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q4HKickFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q4VKickBFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q5HKickFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q6VKickFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(Q1HKickFwdFieldManagerToolCfg(flags)),
          result.popToolsAndMerge(FwdRegionFieldManagerToolCfg(flags))]

    result.setPrivateTools(fieldMgrList)
    return result


def TB_FieldMgrListCfg(flags):
    fieldMgrList = []
    result = ComponentAccumulator()
    result.setPrivateTools(fieldMgrList)
    return result


def GeometryConfigurationToolsCfg(flags):
    geoConfigToolList = []
    # The methods for these tools should be defined in the
    # package containing each tool, so G4AtlasTools in this case
    result =ComponentAccumulator()
    geoConfigToolList += [result.popToolsAndMerge(MaterialDescriptionToolCfg(flags))]
    geoConfigToolList += [result.popToolsAndMerge(VoxelDensityToolCfg(flags))]
    result.setPrivateTools(geoConfigToolList)
    return result


@AccumulatorCache
def G4AtlasDetectorConstructionToolCfg(flags, name="G4AtlasDetectorConstructionTool", **kwargs):
    result = ComponentAccumulator()

    ## For now just have the same geometry configurations tools loaded for ATLAS and TestBeam
    kwargs.setdefault("GeometryConfigurationTools", result.popToolsAndMerge(GeometryConfigurationToolsCfg(flags)))

    if "SenDetMasterTool" not in kwargs:
        kwargs.setdefault("SenDetMasterTool",
                          result.addPublicTool(result.popToolsAndMerge(SensitiveDetectorMasterToolCfg(flags))))

    if flags.Beam.Type is BeamType.TestBeam:
        # Tile test beam
        from G4AtlasTools.G4TestBeamGeometryConfig import TileTB_WorldEnvelopeCfg
        kwargs.setdefault("World", result.popToolsAndMerge(TileTB_WorldEnvelopeCfg(flags)))
        kwargs.setdefault("RegionCreators", []) # Empty for Tile test beam
        kwargs.setdefault("FieldManagers", []) # Empty for Tile test beam
    elif False: # This block is in case we ever decide to support LAr Test Beam again in Athena in the future
        kwargs.setdefault("World", 'LArTB_World')
        kwargs.setdefault("RegionCreators", result.popToolsAndMerge(TB_RegionCreatorListCfg(flags)))
        kwargs.setdefault("FieldManagers", result.popToolsAndMerge(TB_FieldMgrListCfg(flags)))
    else:
        if flags.Detector.GeometryCavern:
            kwargs.setdefault("World", result.popToolsAndMerge(CavernWorldCfg(flags)))
        else:
            kwargs.setdefault("World", result.popToolsAndMerge(ATLASEnvelopeCfg(flags)))
        kwargs.setdefault("RegionCreators", result.popToolsAndMerge(ATLAS_RegionCreatorListCfg(flags)))
        if flags.BField.solenoidOn or flags.BField.barrelToroidOn or flags.BField.endcapToroidOn:
            kwargs.setdefault("FieldManagers", result.popToolsAndMerge(ATLAS_FieldMgrListCfg(flags)))
    result.setPrivateTools(G4AtlasDetectorConstructionTool(name, **kwargs))
    return result


def CavernInfraGeoDetectorToolCfg(flags, name='CavernInfra', **kwargs):
    from AtlasGeoModel.CavernGMConfig import CavernGeometryCfg
    result = CavernGeometryCfg(flags)
    kwargs.setdefault("DetectorName", "CavernInfra")
    result.setPrivateTools(result.popToolsAndMerge(GeoDetectorToolCfg(flags, name, **kwargs)))
    return result


def CavernWorldCfg(flags, name="Cavern", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "World")
    bedrockDX = 302700
    bedrockDZ = 301000
    if flags.Sim.CavernBackground is CavernBackground.Off:
        ## Be ready to resize bedrock if the cosmic generator needs more space
        if flags.Sim.ISFRun:
            # for ISF cosmics simulation, set world volume to biggest possible case
            bedrockDX = 1000.*3000 # 3 km
            bedrockDZ = 1000.*3000 # 3 km
        else:
            from CosmicGenerator.CosmicGeneratorConfig import CavernPropertyCalculator
            theCavernProperties = CavernPropertyCalculator()
            if theCavernProperties.BedrockDX(flags) > bedrockDX:
                bedrockDX = theCavernProperties.BedrockDX(flags)
            if theCavernProperties.BedrockDZ(flags) > bedrockDZ:
                bedrockDZ = theCavernProperties.BedrockDZ(flags)

    kwargs.setdefault("dX", bedrockDX) #FIXME Units?
    kwargs.setdefault("dY", 57300 + 41000 + 1000) # 1 extra metre to help voxelization... #FIXME Units?
    kwargs.setdefault("dZ", bedrockDZ) #FIXME Units?
    # Subtraction Solid - has to be a better way to do this!!
    kwargs.setdefault("NumberOfHoles", 1)
    kwargs.setdefault("HoleNames", ['BelowCavern'])
    kwargs.setdefault("Hole_dX",   [bedrockDX])
    kwargs.setdefault("Hole_dY",   [41000])
    kwargs.setdefault("Hole_dZ",   [bedrockDZ])

    kwargs.setdefault("HolePosX",  [0])
    kwargs.setdefault("HolePosY",  [-58300])
    kwargs.setdefault("HolePosZ",  [0])

    subDetectorList = []
    subDetectorList += [ result.popToolsAndMerge(CavernInfraGeoDetectorToolCfg(flags))]
    subDetectorList += [ result.popToolsAndMerge(ATLASEnvelopeCfg(flags))]

    kwargs.setdefault("SubDetectors", subDetectorList)
    result.setPrivateTools(BoxEnvelope(name, **kwargs))
    return result
