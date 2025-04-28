"""ComponentAccumulator HepMC tools configurations for ISF

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import MeV, mm
from ISF_Services.ISF_ServicesCoreConfig import GeoIDSvcCfg


# GenParticleFilters
def ParticleFinalStateFilterCfg(flags, name="ISF_ParticleFinalStateFilter", **kwargs):
    result = ComponentAccumulator()
    G4NotInUse = not flags.Sim.UsingGeant4
    G4NotInUse = G4NotInUse and flags.Sim.ISFRun
    # use CheckGenInteracting==False to allow GenEvent neutrinos to propagate into the simulation
    kwargs.setdefault("CheckGenSimStable", G4NotInUse)
    kwargs.setdefault("CheckGenInteracting", G4NotInUse)
    result.setPrivateTools(CompFactory.ISF.GenParticleFinalStateFilter(name, **kwargs))
    return result


def ParticleSimAcceptListCfg(flags, name="ISF_ParticleSimAcceptList", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("UseShadowEvent", flags.Sim.UseShadowEvent)
    result.setPrivateTools(CompFactory.ISF.GenParticleSimAcceptList(name, **kwargs))
    return result


def GenParticleSimQuasiStableFilterCfg(flags, name="ISF_GenParticleSimQuasiStableFilter", **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.ISF.GenParticleSimQuasiStableFilter(name, **kwargs))
    return result


@AccumulatorCache
def ParticleSimAcceptList_ExtraParticlesCfg(flags, name="ISF_ParticleSimAcceptList_ExtraParticles", **kwargs):
    result = ComponentAccumulator()
    acceptLists = ["G4particle_acceptlist.txt"]
    # Basically a copy of code from ExtraParticles.ExtraParticlesConfig for now.
    from ExtraParticles import PDGHelpers
    if PDGHelpers.getPDGTABLE('PDGTABLE.MeV'):
        parser = PDGHelpers.PDGParser('PDGTABLE.MeV', '111-556,1112-9090226')
        parser.createList() # NB ignore output here
        acceptLists += ["G4particle_acceptlist_ExtraParticles.txt"]
    else:
        print ('ERROR Failed to find PDGTABLE.MeV file')
    kwargs.setdefault("AcceptLists" , acceptLists )
    kwargs.setdefault("UseShadowEvent", flags.Sim.UseShadowEvent)
    result.setPrivateTools(CompFactory.ISF.GenParticleSimAcceptList(name, **kwargs))
    return result


def ParticlePositionFilterCfg(flags, name="ISF_ParticlePositionFilter", **kwargs):
    result = ComponentAccumulator()
    # ParticlePositionFilter
    kwargs.setdefault("GeoIDService", result.getPrimaryAndMerge(GeoIDSvcCfg(flags)))
    result.setPrivateTools(CompFactory.ISF.GenParticlePositionFilter(name, **kwargs))
    return result


def ParticlePositionFilterIDCfg(flags, name="ISF_ParticlePositionFilterID", **kwargs):
    # importing Reflex dictionary to access AtlasDetDescr::AtlasRegion enum
    import ROOT, cppyy
    cppyy.load_library("libAtlasDetDescrDict")
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion

    kwargs.setdefault("CheckRegion"  , [ AtlasRegion.fAtlasID ] )
    return ParticlePositionFilterCfg(flags, name, **kwargs)


def ParticlePositionFilterCaloCfg(flags, name="ISF_ParticlePositionFilterCalo", **kwargs):
    # importing Reflex dictionary to access AtlasDetDescr::AtlasRegion enum
    import ROOT, cppyy
    cppyy.load_library("libAtlasDetDescrDict")
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion

    kwargs.setdefault("CheckRegion"  , [ AtlasRegion.fAtlasID,
                                            AtlasRegion.fAtlasForward,
                                            AtlasRegion.fAtlasCalo ] )
    return ParticlePositionFilterCfg(flags, name, **kwargs)


def ParticlePositionFilterMSCfg(name="ISF_ParticlePositionFilterMS", **kwargs):
    # importing Reflex dictionary to access AtlasDetDescr::AtlasRegion enum
    import ROOT, cppyy
    cppyy.load_library("libAtlasDetDescrDict")
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion

    kwargs.setdefault("CheckRegion"  , [ AtlasRegion.fAtlasID,
                                            AtlasRegion.fAtlasForward,
                                            AtlasRegion.fAtlasCalo,
                                            AtlasRegion.fAtlasMS ] )
    return ParticlePositionFilterCfg(name, **kwargs)


def ParticlePositionFilterWorldCfg(flags, name="ISF_ParticlePositionFilterWorld", **kwargs):
    # importing Reflex dictionary to access AtlasDetDescr::AtlasRegion enum
    import ROOT, cppyy
    cppyy.load_library("libAtlasDetDescrDict")
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion
    kwargs.setdefault("CheckRegion"  , [ AtlasRegion.fAtlasID,
                                            AtlasRegion.fAtlasForward,
                                            AtlasRegion.fAtlasCalo,
                                            AtlasRegion.fAtlasMS,
                                            AtlasRegion.fAtlasCavern ] )
    return ParticlePositionFilterCfg(flags, name, **kwargs)


def ParticlePositionFilterDynamicCfg(flags, name="ISF_ParticlePositionFilterDynamic", **kwargs):
    # automatically choose the best fitting filter region

    if flags.Detector.EnableMuon:
      return ParticlePositionFilterWorldCfg(flags, name, **kwargs)
    elif flags.Detector.EnableCalo:
      return ParticlePositionFilterCaloCfg(flags, name, **kwargs)
    elif flags.Detector.EnableID:
      return ParticlePositionFilterIDCfg(flags, name, **kwargs)
    else:
      return ParticlePositionFilterWorldCfg(flags, name, **kwargs)


def GenParticleInteractingFilterCfg(flags, name="ISF_GenParticleInteractingFilter", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("AdditionalInteractingParticleTypes", eval(flags.Input.SpecialConfiguration.get("InteractingPDGCodes", "[]")))
    kwargs.setdefault("AdditionalNonInteractingParticleTypes", eval(flags.Input.SpecialConfiguration.get("NonInteractingPDGCodes", "[]")))
    result.setPrivateTools(CompFactory.ISF.GenParticleInteractingFilter(name, **kwargs))
    return result


def EtaPhiFilterCfg(flags, name="ISF_EtaPhiFilter", **kwargs):
    result = ComponentAccumulator()
    # EtaPhiFilter
    EtaRange = 7.0 if flags.Detector.EnableLucid else 6.0
    kwargs.setdefault("MinEta" , -EtaRange)
    kwargs.setdefault("MaxEta" , EtaRange)
    kwargs.setdefault("MaxApplicableRadius", 30*mm)

    result.setPrivateTools(CompFactory.ISF.GenParticleGenericFilter(name, **kwargs))
    return result


def GenParticleFilterToolsCfg(flags):
    result = ComponentAccumulator()
    genParticleFilterList = []
    if flags.Sim.ISF.Simulator.isQuasiStable():
        genParticleFilterList += [result.popToolsAndMerge(ParticleSimAcceptList_ExtraParticlesCfg(flags))]
    else:
        genParticleFilterList += [result.popToolsAndMerge(ParticleFinalStateFilterCfg(flags))]
    if "ATLAS" in flags.GeoModel.Layout or "atlas" in flags.GeoModel.Layout:
        from AthenaConfiguration.Enums import BeamType
        if flags.Beam.Type not in [BeamType.Cosmics, BeamType.TestBeam]:
            genParticleFilterList += [result.popToolsAndMerge(ParticlePositionFilterDynamicCfg(flags))]
            from SimulationConfig.SimEnums import CavernBackground
            if not (flags.Detector.GeometryAFP or flags.Detector.GeometryALFA or flags.Detector.GeometryFwdRegion) \
                and not flags.Detector.GeometryCavern \
                and flags.Sim.CavernBackground in [CavernBackground.Off, CavernBackground.Signal]:
                genParticleFilterList += [result.popToolsAndMerge(EtaPhiFilterCfg(flags))]
    genParticleFilterList += [result.popToolsAndMerge(GenParticleInteractingFilterCfg(flags))]
    result.setPrivateTools(genParticleFilterList)
    return result


def TruthPreselectionToolCfg(flags, name="TruthPreselectionTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault( "GenParticleFilters", result.popToolsAndMerge(GenParticleFilterToolsCfg(flags)) )
    kwargs.setdefault("QuasiStableParticleFilter", result.popToolsAndMerge(GenParticleSimQuasiStableFilterCfg(flags)))
    result.setPrivateTools(CompFactory.ISF.TruthPreselectionTool(name, **kwargs))
    return result

#--------------------------------------------------------------------------------------------------
## Truth Strategies

# Brems: fBremsstrahlung (3)
# Conversion: fGammaConversion (14), fGammaConversionToMuMu (15), fPairProdByCharged (4)
# Decay: 201-298, fAnnihilation(5), fAnnihilationToMuMu (6), fAnnihilationToHadrons (7)
# Ionization: fIonisation (2), fPhotoElectricEffect (12)
# Hadronic: (111,121,131,141,151,161,210)
# Compton: fComptonScattering (13)
# G4 process types:
#  http://www-geant4.kek.jp/lxr/source//processes/management/include/G4ProcessType.hh
# G4 EM sub types:
#  http://www-geant4.kek.jp/lxr/source//processes/electromagnetic/utils/include/G4EmProcessSubType.hh
# G4 HadInt sub types:
#  http://www-geant4.kek.jp/lxr/source//processes/hadronic/management/include/G4HadronicProcessType.hh#L46
def TruthStrategyGroupID_MC15Cfg(flags, name="ISF_MCTruthStrategyGroupID_MC15", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParentMinPt", 100.*MeV)
    kwargs.setdefault("ChildMinPt" , 300.*MeV)
    kwargs.setdefault("VertexTypes", [3, 14, 15, 4, 5, 6, 7, 2, 12, 13])
    kwargs.setdefault("VertexTypeRangeLow", 201)  # All kinds of decay processes
    kwargs.setdefault("VertexTypeRangeHigh", 298)  # ...
    kwargs.setdefault("Regions", [1,2]) # Could import AtlasDetDescr::AtlasRegion enum as in TruthService Cfg methods here
    result.setPrivateTools(CompFactory.ISF.GenericTruthStrategy(name, **kwargs))
    return result


def TruthStrategyGroupIDHadInt_MC15Cfg(flags, name="ISF_MCTruthStrategyGroupIDHadInt_MC15", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParentMinPt", 100.*MeV)
    kwargs.setdefault("ChildMinPt" , 300.*MeV)
    kwargs.setdefault("VertexTypes", [111, 121, 131, 141, 151, 161, 210])
    kwargs.setdefault("AllowChildrenOrParentPassKineticCuts", True)
    kwargs.setdefault("Regions", [1])
    result.setPrivateTools(CompFactory.ISF.GenericTruthStrategy(name, **kwargs))
    return result


def TruthStrategyGroupCaloMuBremCfg(flags, name="ISF_MCTruthStrategyGroupCaloMuBrem", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParentMinEkin", 500.*MeV)
    kwargs.setdefault("ChildMinEkin" , 100.*MeV)
    kwargs.setdefault("VertexTypes"  , [3])
    kwargs.setdefault("ParentPDGCodes", [13, -13])
    kwargs.setdefault("Regions", [3])
    result.setPrivateTools(CompFactory.ISF.GenericTruthStrategy(name, **kwargs))
    return result


def TruthStrategyGroupCaloDecay_MC15Cfg(flags, name="ISF_MCTruthStrategyGroupCaloDecay_MC15", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParentMinEkin", 1000.*MeV)
    kwargs.setdefault("ChildMinEkin" , 500.*MeV)
    kwargs.setdefault("VertexTypes"  , [5, 6, 7])
    kwargs.setdefault("VertexTypeRangeLow" , 201)  # All kinds of decay processes
    kwargs.setdefault("VertexTypeRangeHigh", 298)  # ...
    kwargs.setdefault("Regions", [3])
    result.setPrivateTools(CompFactory.ISF.GenericTruthStrategy(name, **kwargs))
    return result


def TruthStrategyGroupIDCfg(flags, name="ISF_MCTruthStrategyGroupID", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParentMinPt", 100.*MeV)
    kwargs.setdefault("ChildMinPt" , 100.*MeV)
    kwargs.setdefault("VertexTypes", [3, 14, 15, 4, 5, 6, 7, 2, 12, 13])
    kwargs.setdefault("VertexTypeRangeLow"  , 201)  # All kinds of decay processes
    kwargs.setdefault("VertexTypeRangeHigh" , 298)  # ...
    kwargs.setdefault("Regions", [1,2])
    result.setPrivateTools(CompFactory.ISF.GenericTruthStrategy(name, **kwargs))
    return result


def TruthStrategyGroupIDHadIntCfg(flags, name="ISF_MCTruthStrategyGroupIDHadInt", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParentMinPt", 100.*MeV)
    kwargs.setdefault("ChildMinPt" , 100.*MeV)
    kwargs.setdefault("VertexTypes", [111, 121, 131, 141, 151, 161, 210])
    kwargs.setdefault("AllowChildrenOrParentPassKineticCuts", True)
    kwargs.setdefault("Regions", [1])
    result.setPrivateTools(CompFactory.ISF.GenericTruthStrategy(name, **kwargs))
    return result


def TruthStrategyGroupCaloMuBrem_MC15Cfg(flags, name="ISF_MCTruthStrategyGroupCaloMuBrem_MC15", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParentMinEkin", 500.*MeV)
    kwargs.setdefault("ChildMinEkin" , 300.*MeV)
    kwargs.setdefault("VertexTypes"  , [3])
    kwargs.setdefault("ParentPDGCodes", [13, -13])
    kwargs.setdefault("Regions", [3])
    result.setPrivateTools(CompFactory.ISF.GenericTruthStrategy(name, **kwargs))
    return result


def TruthStrategyGroupCaloDecayCfg(flags, name="ISF_MCTruthStrategyGroupCaloDecay", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParentMinPt", 1000.*MeV)
    kwargs.setdefault("ChildMinPt" , 500.*MeV)
    kwargs.setdefault("VertexTypes", [5, 6, 7])
    kwargs.setdefault("VertexTypeRangeLow" , 201)  # All kinds of decay processes
    kwargs.setdefault("VertexTypeRangeHigh", 298)  # ...
    kwargs.setdefault("Regions", [3])
    result.setPrivateTools(CompFactory.ISF.GenericTruthStrategy(name, **kwargs))
    return result

def ValidationTruthStrategyCfg(flags, name="ISF_ValidationTruthStrategy", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParentMinP", 50.*MeV)
    kwargs.setdefault("Regions", [1,2,3,4])
    result.setPrivateTools(CompFactory.ISF.ValidationTruthStrategy(name, **kwargs))
    return result


def FCSParamTruthStrategyCfg(flags, name="ISF_ValidationTruthStrategy", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParentMinP", 150.*MeV)
    kwargs.setdefault("Regions", [3])
    result.setPrivateTools(CompFactory.ISF.ValidationTruthStrategy(name, **kwargs))
    return result


def LLPTruthStrategyCfg(flags, name="ISF_LLPTruthStrategy", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("PassProcessCodeRangeLow",  200)
    kwargs.setdefault("PassProcessCodeRangeHigh", 299)
    # ProcessCategory==9 corresponds to the "fUserDefined" G4ProcessType:
    #   http://www-geant4.kek.jp/lxr/source//processes/management/include/G4ProcessType.hh
    kwargs.setdefault("PassProcessCategory", 9) # ==
    kwargs.setdefault("Regions", [1,2,3,4])
    result.setPrivateTools(CompFactory.ISF.LLPTruthStrategy(name, **kwargs))
    return result


def KeepLLPDecayChildrenStrategyCfg(flags, name="ISF_KeepLLPDecayChildrenStrategy", **kwargs):
    result = ComponentAccumulator()
    # ProcessCategory==9 corresponds to the "fUserDefined" G4ProcessType:
    #   http://www-geant4.kek.jp/lxr/source//processes/management/include/G4ProcessType.hh
    kwargs.setdefault("PassProcessCategory", 9) # ==
    kwargs.setdefault("VertexTypeRangeLow" , 200) # All kinds of decay processes
    kwargs.setdefault("VertexTypeRangeHigh", 299) # ...
    kwargs.setdefault("BSMParent"          , True)
    result.setPrivateTools(CompFactory.ISF.KeepChildrenTruthStrategy(name, **kwargs))
    return result


def KeepLLPHadronicInteractionChildrenStrategyCfg(flags, name="ISF_KeepLLPHadronicInteractionChildrenStrategy", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("VertexTypes", [111, 121, 131, 141, 151, 161, 210])
    kwargs.setdefault("BSMParent"  , True)
    result.setPrivateTools(CompFactory.ISF.KeepChildrenTruthStrategy(name, **kwargs))
    return result


def KeepAllDecayChildrenStrategyCfg(flags, name="ISF_KeepAllDecayChildrenStrategy", **kwargs):
    result = ComponentAccumulator()
    # ProcessCategory==9 corresponds to the "fUserDefined" G4ProcessType:
    #   http://www-geant4.kek.jp/lxr/source//processes/management/include/G4ProcessType.hh
    kwargs.setdefault("PassProcessCategory", 9) # ==
    kwargs.setdefault("VertexTypeRangeLow" , 200) # All kinds of decay processes
    kwargs.setdefault("VertexTypeRangeHigh", 299) # ...
    result.setPrivateTools(CompFactory.ISF.KeepChildrenTruthStrategy(name, **kwargs))
    return result


def KeepHadronicInteractionChildrenStrategyCfg(flags, name="ISF_KeepHadronicInteractionChildrenStrategy", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("VertexTypes", [111, 121, 131, 141, 151, 161, 210])
    result.setPrivateTools(CompFactory.ISF.KeepChildrenTruthStrategy(name, **kwargs))
    return result
