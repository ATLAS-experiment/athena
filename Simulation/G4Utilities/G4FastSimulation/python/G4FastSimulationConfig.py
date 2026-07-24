# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SimpleFastKillerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName" , "BeampipeFwdCut")
    result.setPrivateTools(CompFactory.SimpleFastKillerTool(name="SimpleFastKiller", **kwargs))
    return result


def DeadMaterialShowerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", "DeadMaterial")
    result.setPrivateTools(CompFactory.DeadMaterialShowerTool(name="DeadMaterialShower", **kwargs))
    return result


def FatrasG4Cfg(flags, **kwargs):
    result = ComponentAccumulator()
    # Name of region where FatrasG4 will be triggered
    kwargs.setdefault("RegionName", "InDet")

    # Set the ActsFatrasG4Tool part
    from G4AtlasTools.G4AtlasToolsConfig import ActsFatrasG4ToolCfg
    if "ActsFatrasG4Tool" not in kwargs:        
        kwargs.setdefault("ActsFatrasG4Tool", result.addPublicTool(result.popToolsAndMerge(ActsFatrasG4ToolCfg(flags))))
    
    fatrasG4Tool = CompFactory.FatrasG4Tool(name="FatrasG4", **kwargs)
    # declare produced data
    # ExtraOutputs is now declared in SimHitContainerListCfg in G4AtlasToolsConfig.py, and called from G4AtlasAlgConfig.py
    result.setPrivateTools(fatrasG4Tool)

    return result

def AFatrasG4Cfg(flags, **kwargs):
    result = ComponentAccumulator()
    # Name of region where AFatrasG4 will be triggered
    kwargs.setdefault("RegionName", "InDet")

    # Set the ActsFatrasG4Tool part
    from G4AtlasTools.G4AtlasToolsConfig import ActsFatrasG4ToolCfg
    if "ActsFatrasG4Tool" not in kwargs:
        kwargs.setdefault("ActsFatrasG4Tool", result.addPublicTool(result.popToolsAndMerge(ActsFatrasG4ToolCfg(flags))))

    aFatrasG4Tool = CompFactory.AFatrasG4Tool(name="AFatrasG4", **kwargs)
    # declare produced data
    # ExtraOutputs is now declared in SimHitContainerListCfg in G4AtlasToolsConfig.py, and called from G4AtlasAlgConfig.py
    result.setPrivateTools(aFatrasG4Tool)

    return result

def FastCaloSimCfg(flags, **kwargs):
    result = ComponentAccumulator()
    # Must match the region created by CALOPhysicsRegionToolCfg.
    kwargs.setdefault("RegionName", "CALO")
    kwargs.setdefault("CaloCellContainerSDName", "ToolSvc.SensitiveDetectorMasterTool.CaloCellContainerSD")

    from G4AtlasTools.G4AtlasToolsConfig import FastCaloSimParametrizationToolCfg
    kwargs.setdefault("FastCaloSimParametrizationTool", result.addPublicTool(result.popToolsAndMerge(FastCaloSimParametrizationToolCfg(flags))))

    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RandomSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    kwargs.setdefault("RandomStream", "FastCaloSimRnd")

    # Set the PunchThrough G4 part
    from G4AtlasTools.G4AtlasToolsConfig import PunchThroughSimWrapperCfg
    if "PunchThroughSimWrapper" not in kwargs:
        kwargs.setdefault("PunchThroughSimWrapper", result.addPublicTool(result.popToolsAndMerge(PunchThroughSimWrapperCfg(flags))))
    
    # Config PunchThroughG4Tool
    kwargs.setdefault('doPunchThrough', flags.Sim.FastCalo.doPunchThrough)

    result.setPrivateTools(CompFactory.FastCaloSimTool(name="FastCaloSim", **kwargs))
    return result


def FastCaloSimParamHitAnalysisCfg(flags, name="FastCaloSimParamHitAnalysis",
                                   NTruthParticles=1, saveAllBranches=False,
                                   doG4Hits=False, doClusterInfo=False,
                                   outputGeoFileName=None, **kwargs):
    """Configure the FastCaloSim parametrization-input ntuple algorithm.

    Transport requires an initialized Geant4 particle table and field. The
    simulation-job path provides both; standalone ESD use must do the same.
    """
    result = ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))
    kwargs.setdefault("CaloDetDescrManager", "CaloDetDescrManager")

    from TileConditions.TileSamplingFractionConfig import TileSamplingFractionCondAlgCfg
    result.merge(TileSamplingFractionCondAlgCfg(flags))
    kwargs.setdefault("TileSamplingFraction", "TileSamplingFraction")

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    kwargs.setdefault("TileCablingSvc", result.getPrimaryAndMerge(TileCablingSvcCfg(flags)).name)

    # Preserve the stream name used by downstream regression tools.
    kwargs.setdefault("NtupleFileName", 'ISF_HitAnalysis')
    kwargs.setdefault("GeoFileName", 'ISF_Geometry')
    # Sim jobs have no HIST output stream by default; fall back to a fixed name
    histFileName = flags.Output.HISTFileName or "ISF_HitAnalysis.root"
    histOutputArray = ["ISF_HitAnalysis DATAFILE='%s' OPT='RECREATE'" % (histFileName)]
    if outputGeoFileName:
        histOutputArray += ["ISF_Geometry DATAFILE='%s' OPT='RECREATE'" % (outputGeoFileName)]
    result.addService(CompFactory.THistSvc(Output=histOutputArray))
    kwargs.setdefault("NTruthParticles", NTruthParticles)

    # Use the same transport and extrapolation as the fast-sim model.
    from G4AtlasTools.G4AtlasToolsConfig import FastCaloSimParametrizationToolCfg
    kwargs.setdefault("FastCaloSimParametrizationTool", result.addPublicTool(result.popToolsAndMerge(FastCaloSimParametrizationToolCfg(flags))))

    kwargs.setdefault("CaloBoundaryR", 1148.0)
    kwargs.setdefault("CaloBoundaryZ", 3550.0)
    kwargs.setdefault("SaveAllBranches", saveAllBranches)
    kwargs.setdefault("DoAllCells", False)
    kwargs.setdefault("DoLayers", True)
    kwargs.setdefault("DoLayerSums", True)
    kwargs.setdefault("DoG4Hits", doG4Hits)
    kwargs.setdefault("DoClusterInfo", doClusterInfo)
    kwargs.setdefault("TimingCut", 999999)

    # Pure simulation jobs do not have digitization metadata.
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    result.merge(addFolders(flags, ["/Simulation/Parameters"]))

    result.addEventAlgo(CompFactory.FastCaloSimParamHitAnalysis(name, **kwargs))
    return result
