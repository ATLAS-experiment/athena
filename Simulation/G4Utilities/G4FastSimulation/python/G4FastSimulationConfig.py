# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SimpleFastKillerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionNames" , ["BeampipeFwdCut"] )
    result.setPrivateTools(CompFactory.SimpleFastKillerTool(name="SimpleFastKiller", **kwargs))
    return result


def DeadMaterialShowerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionNames",        ["DeadMaterial"])
    result.setPrivateTools(CompFactory.DeadMaterialShowerTool(name="DeadMaterialShower", **kwargs))
    return result

def FastCaloSimCfg(flags, **kwargs):
    result = ComponentAccumulator()
    # Set the parametrization service
    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloSimV2ParamSvcCfg
    kwargs.setdefault("ISF_FastCaloSimV2ParamSvc", result.getPrimaryAndMerge(FastCaloSimV2ParamSvcCfg(flags)))
    # Set the FastCaloSim extrapolation tool
    from ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig import FastCaloSimCaloExtrapolationCfg
    kwargs.setdefault("FastCaloSimCaloExtrapolation", result.addPublicTool(result.popToolsAndMerge(FastCaloSimCaloExtrapolationCfg(flags))))
    # Name of region where FastCaloSim will be triggered
    kwargs.setdefault("RegionNames", ["CALO"])
    kwargs.setdefault('CaloCellContainerSDName', "ToolSvc.SensitiveDetectorMasterTool.CaloCellContainerSD")
    
    if flags.Sim.SimplifiedGeoPath:
        # Enable Geant4 track transportation only if simplified geometry is provided
        kwargs.setdefault('doG4Transport', True)
    else:
        kwargs.setdefault('doG4Transport', False)

    # Set the G4CaloTransportTool
    from G4AtlasTools.G4AtlasToolsConfig import G4CaloTransportToolCfg
    kwargs.setdefault("G4CaloTransportTool", result.addPublicTool(result.popToolsAndMerge(G4CaloTransportToolCfg(flags))))

    # Set the PunchThrough G4 part
    from G4AtlasTools.G4AtlasToolsConfig import PunchThroughSimWrapperCfg
    if "PunchThroughSimWrapper" not in kwargs:
        kwargs.setdefault("PunchThroughSimWrapper", result.addPublicTool(result.popToolsAndMerge(PunchThroughSimWrapperCfg(flags))))
    
    # Config PunchThroughG4Tool
    kwargs.setdefault('doPunchThrough', flags.Sim.FastCalo.doPunchThrough)

    # Config FastCaloSim
    if flags.Sim.FastCalo.doEMECFCS:  # AF3 in EMEC and G4 in rest
        kwargs.setdefault('doPhotons', True)
        kwargs.setdefault('doElectrons', True)
        kwargs.setdefault('doHadrons', False)
        kwargs.setdefault('AbsEtaMin', 1.5)
        kwargs.setdefault('AbsEtaMax', 3.2)
        kwargs.setdefault('EkinMinPhotons', 10)
        kwargs.setdefault('EkinMaxPhotons', 2048)
        kwargs.setdefault('EkinMinElectrons', 10)
        kwargs.setdefault('EkinMaxElectrons', 256)
    else: # These are set to AF3 configuration
        kwargs.setdefault('doPhotons', True)
        kwargs.setdefault('doElectrons', True)
        kwargs.setdefault('doHadrons', True)
        kwargs.setdefault('AbsEtaMin', 0)
        kwargs.setdefault('AbsEtaMax', 10)
        kwargs.setdefault('EkinMinPhotons', 0)
        kwargs.setdefault('EkinMaxPhotons', float('inf'))
        kwargs.setdefault('EkinMinElectrons', 0)
        kwargs.setdefault('EkinMaxElectrons', float('inf'))

    result.setPrivateTools(CompFactory.FastCaloSimTool(name="FastCaloSim", **kwargs))
    return result
