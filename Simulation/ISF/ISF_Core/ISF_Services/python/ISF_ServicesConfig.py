"""ComponentAccumulator service configuration for ISF

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SimulationConfig.SimEnums import TruthStrategy
from ISF_HepMC_Tools.ISF_HepMC_ToolsConfig import (
    KeepLLPDecayChildrenStrategyCfg,
    KeepLLPHadronicInteractionChildrenStrategyCfg,
    TruthStrategyGroupID_MC15Cfg,
    TruthStrategyGroupIDHadInt_MC15Cfg,
    #TruthStrategyGroupCaloMuBrem_MC15Cfg,
    TruthStrategyGroupCaloDecay_MC15Cfg,
    LLPTruthStrategyCfg,
    TruthStrategyGroupIDCfg,
    TruthStrategyGroupIDHadIntCfg,
    TruthStrategyGroupCaloMuBremCfg,
    ValidationTruthStrategyCfg,
    FCSParamTruthStrategyCfg
)
from BarcodeServices.BarcodeServicesConfig import BarcodeSvcCfg
from ISF_Geant4CommonTools.ISF_Geant4CommonToolsConfig import (
    EntryLayerToolCfg, ATLFAST_EntryLayerToolCfg
)
from ISF_Tools.ISF_ToolsConfig import ParticleOrderingToolCfg

#include file to access AtlasDetDescr::AtlasRegion enum
import ROOT,cppyy
cppyy.include("AtlasDetDescr/AtlasRegion.h")


def InputConverterCfg(flags, name="ISF_InputConverter", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("QuasiStableParticlesIncluded", flags.Sim.ISF.Simulator.isQuasiStable())
    kwargs.setdefault("UseShadowEvent", flags.Sim.UseShadowEvent)
    kwargs.setdefault("UseGeneratedParticleMass", False)
    if "GenParticleFilters" not in kwargs:
        from ISF_HepMC_Tools.ISF_HepMC_ToolsConfig import GenParticleFilterToolsCfg
        kwargs.setdefault("GenParticleFilters", result.popToolsAndMerge(GenParticleFilterToolsCfg(flags)) )
    result.addService(CompFactory.ISF.InputConverter(name, **kwargs), primary = True)
    return result


def ParticleBrokerSvcNoOrderingCfg(flags, name="ISF_ParticleBrokerSvcNoOrdering", **kwargs):
    result = ComponentAccumulator()
    if "EntryLayerTool" not in kwargs:
        kwargs.setdefault("EntryLayerTool", result.addPublicTool(result.popToolsAndMerge(EntryLayerToolCfg(flags))))
        kwargs.setdefault("GeoIDSvc", result.getService("ISF_GeoIDSvc").name) # FIXME
    # assume "GeoIDSvc" has been set alongside "EntryLayerTool"
    kwargs.setdefault("AlwaysUseGeoIDSvc", False)
    kwargs.setdefault("ValidateGeoIDs", flags.Sim.ISF.ValidationMode)
    kwargs.setdefault("ValidationOutput", flags.Sim.ISF.ValidationMode)
    kwargs.setdefault("ValidationStreamName", "ParticleBroker")

    result.addService(CompFactory.ISF.ParticleBrokerDynamicOnReadIn(name, **kwargs), primary = True)
    return result


def ParticleBrokerSvcCfg(flags, name="ISF_ParticleBrokerSvc", **kwargs):
    # comment copied from old config
    #kwargs.setdefault("ParticleOrderingTool", "ISF_InToOutSubDetOrderingTool")
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleOrderingTool", result.popToolsAndMerge(ParticleOrderingToolCfg(flags)))
    pbsvc = result.getPrimaryAndMerge(ParticleBrokerSvcNoOrderingCfg(flags, name, **kwargs))
    result.addService(pbsvc, primary = True)
    return result


def ATLFAST_ParticleBrokerSvcCfg(flags, name="ISF_ATLFAST_ParticleBrokerSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("EntryLayerTool", result.addPublicTool(result.popToolsAndMerge(ATLFAST_EntryLayerToolCfg(flags))))
    kwargs.setdefault("GeoIDSvc", result.getService("ISF_ATLFAST_GeoIDSvc").name) # FIXME
    pbsvc = result.getPrimaryAndMerge(ParticleBrokerSvcCfg(flags, name, **kwargs))
    result.addService(pbsvc, primary = True)
    return result


# Generic Truth Service Configurations
def TruthServiceCfg(flags, **kwargs):
    """Return the TruthService config flagged by Sim.TruthStrategy"""
    stratmap = {
        TruthStrategy.MC12: MC12TruthServiceCfg,
        TruthStrategy.MC12LLP: MC12LLPTruthServiceCfg,
        TruthStrategy.MC12Plus: MC12PlusTruthServiceCfg,
        TruthStrategy.MC15: MC15TruthServiceCfg,
        TruthStrategy.MC15a: MC15aTruthServiceCfg,
        TruthStrategy.MC15aPlus: MC15aPlusTruthServiceCfg,
        TruthStrategy.MC15aPlusLLP: MC15aPlusLLPTruthServiceCfg,
        TruthStrategy.MC16: MC16TruthServiceCfg,
        TruthStrategy.MC16LLP: MC16LLPTruthServiceCfg,
        TruthStrategy.MC18: MC18TruthServiceCfg,
        TruthStrategy.MC18LLP: MC18LLPTruthServiceCfg,
        # TruthStrategy.PhysicsProcess: PhysicsProcessTruthServiceCfg,
        # TruthStrategy.Global: GlobalTruthServiceCfg,
        TruthStrategy.Validation: ValidationTruthServiceCfg,
        # TruthStrategy.Cosmic: CosmicTruthServiceCfg,
    }
    xCfg = stratmap[flags.Sim.TruthStrategy]
    return xCfg(flags, **kwargs)


def GenericTruthServiceCfg(flags, name="ISF_TruthService", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("BarcodeSvc", result.getPrimaryAndMerge(BarcodeSvcCfg(flags)).name)

    kwargs.setdefault("SkipIfNoChildren", True)
    kwargs.setdefault("SkipIfNoParentId", True)
    kwargs.setdefault("ForceEndVtxInRegions", [])

    if flags.Sim.ISF.Simulator.isQuasiStable():
        kwargs.setdefault("QuasiStableParticlesIncluded", True)
    kwargs.setdefault("QuasiStableParticleOverwrite", not flags.Sim.UseShadowEvent)
    svc = CompFactory.ISF.TruthSvc(name, **kwargs)
    result.addService(svc, primary=True)
    return result


def ValidationTruthServiceCfg(flags, name="ISF_ValidationTruthService", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("TruthStrategies", [result.popToolsAndMerge(ValidationTruthStrategyCfg(flags))] )
    kwargs.setdefault("IgnoreUndefinedBarcodes", True)
    kwargs.setdefault("PassWholeVertices", True)
    truthService = result.getPrimaryAndMerge(GenericTruthServiceCfg(flags, name, **kwargs))
    result.addService(truthService, primary=True)
    return result


# MC12 Truth Service Configurations
def MC12BeamPipeTruthStrategies():
    return ["ISF_MCTruthStrategyGroupID"]


def MC12IDTruthStrategies():
    return ["ISF_MCTruthStrategyGroupID", "ISF_MCTruthStrategyGroupIDHadInt"]


def MC12CaloTruthStrategies():
    return ["ISF_MCTruthStrategyGroupCaloMuBrem"]


def MC12MSTruthStrategies():
    return []


def MC12TruthServiceCfg(flags, name="ISF_MC12TruthService", **kwargs):
    result = ComponentAccumulator()
    if "TruthStrategies" not in kwargs:
        truthCfgs = [
            TruthStrategyGroupIDCfg,
            TruthStrategyGroupIDHadIntCfg,
            TruthStrategyGroupCaloMuBremCfg,
        ]
        truthStrats = [result.popToolsAndMerge(cfg(flags)) for cfg in truthCfgs]
        kwargs.setdefault("TruthStrategies", truthStrats)
    kwargs.setdefault("IgnoreUndefinedBarcodes", False)
    kwargs.setdefault("PassWholeVertices", True)
    truthService = result.getPrimaryAndMerge(GenericTruthServiceCfg(flags, name, **kwargs))
    result.addService(truthService, primary=True)
    return result


def MC12LLPTruthServiceCfg(flags, name="ISF_MC12TruthLLPService", **kwargs):
    result = ComponentAccumulator()
    truthCfgs = [
        TruthStrategyGroupIDCfg,
        TruthStrategyGroupIDHadIntCfg,
        TruthStrategyGroupCaloMuBremCfg,
        LLPTruthStrategyCfg,
    ]
    truthStrats = [result.popToolsAndMerge(cfg(flags)) for cfg in truthCfgs]
    kwargs.setdefault("TruthStrategies", truthStrats)
    truthService = result.getPrimaryAndMerge(MC12TruthServiceCfg(flags, name, **kwargs))
    result.addService(truthService, primary = True)
    return result


def MC12PlusTruthServiceCfg(flags, name="ISF_MC12PlusTruthService", **kwargs):
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion
    kwargs.setdefault("ForceEndVtxInRegions", [AtlasRegion.fAtlasID] )
    return MC12TruthServiceCfg(flags, name, **kwargs)


# MC15 Truth Service Configurations
def MC15BeamPipeTruthStrategies():
    return ["ISF_MCTruthStrategyGroupID_MC15"]


def MC15IDTruthStrategies():
    return ["ISF_MCTruthStrategyGroupID_MC15", "ISF_MCTruthStrategyGroupIDHadInt_MC15"]


def MC15CaloTruthStrategies():
    return ["ISF_MCTruthStrategyGroupCaloMuBrem", "ISF_MCTruthStrategyGroupCaloMuBrem_MC15"]

def MC15MSTruthStrategies():
    return []


def MC15TruthServiceCfg(flags, name="ISF_MC15TruthService", **kwargs):
    result = ComponentAccumulator()
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion

    if "TruthStrategies" not in kwargs:
        truthCfgs = [
            TruthStrategyGroupID_MC15Cfg,
            TruthStrategyGroupIDHadInt_MC15Cfg,
            TruthStrategyGroupCaloMuBremCfg, # FIXME - should be TruthStrategyGroupCaloMuBrem_MC15Cfg but keeping this for consistency with old style
            TruthStrategyGroupCaloDecay_MC15Cfg ]
        if flags.Sim.RecordStepInfo:
            # Override Truth strategies for FCS Parameterization input samples
            truthCfgs = [ FCSParamTruthStrategyCfg ]
        truthStrats = [result.popToolsAndMerge(cfg(flags)) for cfg in truthCfgs]
        kwargs.setdefault("TruthStrategies", truthStrats)

    kwargs.setdefault("IgnoreUndefinedBarcodes", False)
    kwargs.setdefault("PassWholeVertices", False) # new for MC15 - can write out partial vertices.
    kwargs.setdefault("ForceEndVtxInRegions", [AtlasRegion.fAtlasID])
    truthService = result.getPrimaryAndMerge(GenericTruthServiceCfg(flags, name, **kwargs))
    result.addService(truthService, primary=True)
    return result


def MC15aTruthServiceCfg(flags, name="ISF_MC15aTruthService", **kwargs):
    kwargs.setdefault("ForceEndVtxInRegions", [])
    return MC15TruthServiceCfg(flags, name, **kwargs)


def MC15aPlusTruthServiceCfg(flags, name="ISF_MC15aPlusTruthService", **kwargs):
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion

    kwargs.setdefault("ForceEndVtxInRegions", [AtlasRegion.fAtlasID])
    return MC15TruthServiceCfg(flags, name, **kwargs)


def MC15aPlusLLPTruthServiceCfg(flags, name="ISF_MC15aPlusLLPTruthService", **kwargs):
    result = ComponentAccumulator()
    truthCfgs = [
        KeepLLPDecayChildrenStrategyCfg,
        KeepLLPHadronicInteractionChildrenStrategyCfg,
        TruthStrategyGroupID_MC15Cfg,
        TruthStrategyGroupIDHadInt_MC15Cfg,
        TruthStrategyGroupCaloMuBremCfg, # FIXME - should be TruthStrategyGroupCaloDecay_MC15Cfg but keeping this for consistency with old style
        TruthStrategyGroupCaloDecay_MC15Cfg,
        LLPTruthStrategyCfg,
    ]
    truthStrats = [result.popToolsAndMerge(cfg(flags)) for cfg in truthCfgs]
    kwargs.setdefault("TruthStrategies", truthStrats)
    truthService = result.getPrimaryAndMerge(MC15aPlusTruthServiceCfg(flags, name, **kwargs))
    result.addService(truthService, primary = True)
    return result


# MC16 Truth Service Configurations
def MC16TruthServiceCfg(flags, name="ISF_MC16TruthService", **kwargs):
    return MC15aPlusTruthServiceCfg(flags, name, **kwargs)


def MC16LLPTruthServiceCfg(flags, name="ISF_MC16LLPTruthService", **kwargs):
    return MC15aPlusLLPTruthServiceCfg(flags, name, **kwargs)


# MC18 Truth Service Configurations
def MC18TruthServiceCfg(flags, name="ISF_MC18TruthService", **kwargs):
    return MC15aPlusTruthServiceCfg(flags, name, **kwargs)


def MC18LLPTruthServiceCfg(flags, name="ISF_MC18LLPTruthService", **kwargs):
    return MC15aPlusLLPTruthServiceCfg(flags, name, **kwargs)
