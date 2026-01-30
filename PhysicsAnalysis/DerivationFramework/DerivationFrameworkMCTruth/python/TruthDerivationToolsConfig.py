# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Provides configs for the tools used for building the common truth collections
# Note that taus are handled separately (see MCTruthCommon.py)
# Note also that navigation info is dropped here and added separately
# Two kinds of config are defined here - for general config of the tools
# and for specific configurations, which implement the former.
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


#==============================================================================
# TruthCollectionMaker instances
#==============================================================================

def TruthCollectionMakerCfg(flags, name, **kwargs):
    """Configure the TruthCollectionMaker tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMaker(name = name,**kwargs),
                      primary = True)
    return acc


def DFCommonTruthMuonToolCfg(flags, name = "DFCommonTruthMuonTool", **kwargs):
    """Muon truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthMuons")
    kwargs.setdefault("KeepNavigationInfo", False)
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMakerMuon(name = name,**kwargs),
                      primary = True)
    return acc


def DFCommonTruthCharmToolCfg(flags, name, **kwargs):
    """Charm truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthCharm")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("Do_Compress", True)
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMakerCharm(name = name,**kwargs),
                      primary = True)
    return acc


def DFCommonTruthElectronToolCfg(flags, name = "DFCommonTruthElectronTool", **kwargs):
    """Electron truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthElectrons")
    kwargs.setdefault("KeepNavigationInfo", False)
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMakerElectron(name = name,**kwargs),
                      primary = True)
    return acc


def DFCommonTruthPhotonToolCfg(flags, name = "DFCommonTruthPhotonTool", **kwargs):
    """Photon truth collection maker (Currently unused?)"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthPhotons")
    kwargs.setdefault("KeepNavigationInfo", False)
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMakerPhoton(name = name,**kwargs),
                      primary = True)
    return acc


# this tool is needed for making TruthPhotons from sim samples, where extra cuts are needed. Origin 42 (pi0) and 23 (light meson) cut way down uninteresting photons
def DFCommonTruthPhotonToolSimCfg(flags, name = "DFCommonTruthPhotonToolSim", **kwargs):
    """Tool for making TruthPhotons from sim samples"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthPhotons")
    kwargs.setdefault("KeepNavigationInfo", False)
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMakerPhotonSim(name = name,**kwargs),
                      primary = True)
    return acc


def DFCommonTruthNeutrinoToolCfg(flags, name = "DFCommonTruthNeutrinoTool", **kwargs):
    """Neutrino truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthNeutrinos")
    kwargs.setdefault("KeepNavigationInfo", False)
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMakerNeutrino(name = name,**kwargs),
                      primary = True)
    return acc


def DFCommonTruthBottomToolCfg(flags, name = "DFCommonTruthBottomTool", **kwargs):
    """B-quark truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthBottom")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("Do_Compress", True)
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMakerBottom(name = name,**kwargs),
                      primary = True)
    return acc


def DFCommonTruthTopToolCfg(flags, name = "DFCommonTruthTopTool", **kwargs):
    """Top-quark truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthTop")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("Do_Compress", True)
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMakerTop(name = name,**kwargs),
                      primary = True)
    return acc


def DFCommonTruthBosonToolCfg(flags, name = "DFCommonTruthBosonTool", **kwargs):
    """Gauge bosons and Higgs truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthBoson")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("BuildingW", True)
    kwargs.setdefault("BuildingZ", True)
    kwargs.setdefault("Do_Compress", True)
    kwargs.setdefault("Do_Sherpa", True)
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMakerBoson(name = name,**kwargs),
                      primary = True)
    return acc


def DFCommonTruthBSMToolCfg(flags, name = "DFCommonTruthBSMTool", **kwargs):
    """BSM particles truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthBSM")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("Do_Compress", True)
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMakerBSM(name = name,**kwargs),
                      primary = True)
    return acc


def DFCommonTruthForwardProtonToolCfg(flags, name = "DFCommonTruthForwardProtonTool", **kwargs):
    """Forward proton truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("BeamEnergy", flags.Beam.Energy)
    kwargs.setdefault("OutputCollectionName", "TruthForwardProtons")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("Do_Compress", True)
    acc.addPublicTool(CompFactory.DerivationFramework.TruthCollectionMakerForwardProton(name, **kwargs), primary = True)
    return acc

#==============================================================================
# Decoration tools
#==============================================================================

def TruthD2DecoratorCfg(flags, name, **kwargs):
    """Configure the truth D2 decorator tool"""
    acc = ComponentAccumulator()
    TruthD2Decorator = CompFactory.DerivationFramework.TruthD2Decorator
    acc.addPublicTool(TruthD2Decorator(name, **kwargs), primary = True)
    return acc


def TruthClassificationDecoratorCfg(flags, name, **kwargs):
    """Configure the TruthClassificationDecorator tool"""
    acc = ComponentAccumulator()
    from MCTruthClassifier.MCTruthClassifierConfig import DFCommonMCTruthClassifierCfg
    kwargs.setdefault("MCTruthClassifier", acc.addPublicTool(acc.popToolsAndMerge(DFCommonMCTruthClassifierCfg(flags))))
    TruthClassificationDecorator = CompFactory.DerivationFramework.TruthClassificationDecorator
    acc.setPrivateTools(TruthClassificationDecorator(name = name, **kwargs))
    return acc


def MuonTruthClassifierFallbackCfg(flags, name, **kwargs):
    """Config the MuonTruthClassifierFallback tool"""
    acc = ComponentAccumulator()

    if "MCTruthClassifierTool" not in kwargs:
        from MCTruthClassifier.MCTruthClassifierConfig import (
            MCTruthClassifierCfg)
        kwargs.setdefault("MCTruthClassifierTool", acc.popToolsAndMerge(
            MCTruthClassifierCfg(flags, name = "MuonTruthClassifierFallbackMCTruthClassifier")))

    MuonTruthClassifierFallback = CompFactory.DerivationFramework.MuonTruthClassifierFallback
    acc.addPublicTool(MuonTruthClassifierFallback(name = name, **kwargs),
                      primary = True)
    return acc


def TruthDressingToolCfg(flags, name, **kwargs):
    """Configure the TruthDressingTool"""
    acc = ComponentAccumulator()
    TruthDressingTool = CompFactory.DerivationFramework.TruthDressingTool
    acc.addPublicTool(TruthDressingTool( name = name, **kwargs),
                      primary = True)
    return acc


def TruthIsolationToolCfg(flags, name, **kwargs):
    """Configure the truth isolation tool"""
    acc = ComponentAccumulator()
    TruthIsolationTool = CompFactory.DerivationFramework.TruthIsolationTool
    acc.addPublicTool(TruthIsolationTool(name = name, **kwargs),
                      primary = True)
    return acc


def MuonTruthIsolationDecorAlgCfg(flags, name, **kwargs):
    """Configure the MuonTruthIsolationTool"""
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.DerivationFramework.MuonTruthIsolationDecorAlg(name = name, **kwargs),
                      primary = True)
    return acc


def TruthQGDecorationToolCfg(flags, name, **kwargs):
    """Configure the quark/gluon decoration tool"""
    acc = ComponentAccumulator()
    TruthQGDecorationTool = CompFactory.DerivationFramework.TruthQGDecorationTool
    acc.addPublicTool(TruthQGDecorationTool(name = name, **kwargs),
                      primary = True)
    return acc


def TruthNavigationDecoratorCfg(flags, name, **kwargs):
    """Congigure the truth navigation decorator tool"""
    acc = ComponentAccumulator()
    kwargs.setdefault("InputCollections", [])
    kwargs.setdefault("parentDecorKeys", [ key + ".parentLinks" for key in kwargs["InputCollections"] ])
    kwargs.setdefault("childDecorKeys", [ key + ".childLinks" for key in kwargs["InputCollections"] ])
    TruthNavigationDecorator = CompFactory.DerivationFramework.TruthNavigationDecorator
    acc.addPublicTool(TruthNavigationDecorator(name = name, **kwargs),
                      primary = True)
    return acc


def TruthDecayCollectionMakerCfg(flags, name, **kwargs):
    """Configure the truth decay collection maker"""
    acc = ComponentAccumulator()
    TruthDecayCollectionMaker = CompFactory.DerivationFramework.TruthDecayCollectionMaker
    acc.addPublicTool(TruthDecayCollectionMaker(name = name, **kwargs),
                      primary = True)
    return acc


def TruthBornLeptonCollectionMakerCfg(flags, name, **kwargs):
    """Configure the truth Born lepton collection tool"""
    acc = ComponentAccumulator()
    TruthBornLeptonCollectionMaker = CompFactory.DerivationFramework.TruthBornLeptonCollectionMaker
    acc.addPublicTool(TruthBornLeptonCollectionMaker(name = name, **kwargs),
                      primary = True)
    return acc


def HardScatterCollectionMakerCfg(flags, name, **kwargs):
    """Add a mini-collection for the hard scatter and N subsequent generations"""
    acc = ComponentAccumulator()
    return acc


#add the 'decoration' tool to dress the main truth collection with the classification
def DFCommonTruthClassificationToolCfg(flags):
    """dress the main truth collection with the classification"""
    return TruthClassificationDecoratorCfg(flags,
                                          name = "DFCommonTruthClassificationTool",
                                          ParticlesKey = "TruthParticles")


# Hadron origin decoration tools
def HadronOriginClassifierCfg(flags, name, **kwargs):
    """get the hadron origin classification"""
    acc = ComponentAccumulator()
    HadronOriginClassifier = CompFactory.DerivationFramework.HadronOriginClassifier
    kwargs.setdefault("DSID", flags.Input.MCChannelNumber)
    acc.addPublicTool(HadronOriginClassifier(name = name, **kwargs),
                      primary = True)
    return acc


def HadronOriginDecoratorCfg(flags, name, **kwargs):
    """decorate with the hadron origin classification"""
    acc = ComponentAccumulator()
    if "ToolName" not in kwargs:
        kwargs.setdefault("ToolName", acc.getPrimaryAndMerge(HadronOriginClassifierCfg(flags,
                                                                                       name="DFCommonHadronOriginClassifier")))
    acc.addPublicTool(CompFactory.DerivationFramework.HadronOriginDecorator
                      (name = name, **kwargs),
                      primary = True)
    return acc


#add the 'decoration' tools for dressing and isolation
def DFCommonTruthElectronDressingToolCfg(flags, decorationName = "dressedPhoton"):
    """Configure the electron truth dressing tool"""
    return TruthDressingToolCfg(flags,
                                name                  = "DFCommonTruthElectronDressingTool",
                                dressParticlesKey     = "TruthElectrons",
                                usePhotonsFromHadrons = False,
                                dressingConeSize      = 0.1,
                                particleIDsToDress    = [11],
                                decorationName        = decorationName+"_e")


def DFCommonTruthMuonDressingToolCfg(flags, decorationName = "dressedPhoton"):
    """Configure the muon truth dressing tool"""
    return TruthDressingToolCfg(flags,
                                name                  = "DFCommonTruthMuonDressingTool",
                                dressParticlesKey     = "TruthMuons",
                                usePhotonsFromHadrons = False,
                                dressingConeSize      = 0.1,
                                particleIDsToDress    = [13],
                                decorationName        = decorationName+"_mu")


def DFCommonTruthTauDressingToolCfg(flags):
    """Configure the tau truth dressing tool"""
    return TruthDressingToolCfg(flags,
                                name                  = "DFCommonTruthTauDressingTool",
                                dressParticlesKey     = "TruthTaus",
                                usePhotonsFromHadrons = False,
                                dressingConeSize      = 0.2, # Tau special
                                particleIDsToDress    = [15],
                                decoratePhotons = False)


def DFCommonTruthElectronIsolationTool1Cfg(flags):
    """Configure the electron isolation tool, cone=0.2"""
    return TruthIsolationToolCfg(flags,
                                 name                   = "DFCommonTruthElectronIsolationTool1",
                                 isoParticlesKey        = "TruthElectrons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [11],
                                 IsolationConeSizes     = [0.2],
                                 IsolationVarNamePrefix = 'etcone',
                                 ChargedParticlesOnly   = False)


def DFCommonTruthElectronIsolationTool2Cfg(flags):
    """Configure the electron isolation tool, cone=0.3"""
    return TruthIsolationToolCfg(flags,
                                 name                   =  "DFCommonTruthElectronIsolationTool2",
                                 isoParticlesKey        = "TruthElectrons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [11],
                                 IsolationConeSizes     = [0.3],
                                 IsolationVarNamePrefix = 'ptcone',
                                 ChargedParticlesOnly   = True)


def DFCommonTruthMuonIsolationTool1Cfg(flags):
    """Configure the muon isolation tool, cone=0.2"""
    return TruthIsolationToolCfg(flags,
                                 name                   = "DFCommonTruthMuonIsolationTool1",
                                 isoParticlesKey        = "TruthMuons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [13],
                                 IsolationConeSizes     = [0.2],
                                 IsolationVarNamePrefix = 'etcone',
                                 ChargedParticlesOnly   = False)


def DFCommonTruthMuonIsolationTool2Cfg(flags):
    """Configure the muon isolation tool, cone=0.3"""
    return TruthIsolationToolCfg(flags,
                                 name                   = "DFCommonTruthMuonIsolationTool2",
                                 isoParticlesKey        = "TruthMuons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [13],
                                 IsolationConeSizes     = [0.3],
                                 IsolationVarNamePrefix = 'ptcone',
                                 ChargedParticlesOnly   = True)


def DFCommonTruthPhotonIsolationTool1Cfg(flags):
    """Configure the photon isolation tool, etcone"""
    return TruthIsolationToolCfg(flags,
                                 name                   = "DFCommonTruthPhotonIsolationTool1",
                                 isoParticlesKey        = "TruthPhotons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [22],
                                 IsolationConeSizes     = [0.2],
                                 IsolationVarNamePrefix = 'etcone',
                                 ChargedParticlesOnly   = False)


def DFCommonTruthPhotonIsolationTool2Cfg(flags):
    """Configure the photon isolation tool, ptcone"""
    return  TruthIsolationToolCfg(flags,
                                  name                   = "DFCommonTruthPhotonIsolationTool2",
                                  isoParticlesKey        = "TruthPhotons",
                                  allParticlesKey        = "TruthParticles",
                                  particleIDsToCalculate = [22],
                                  IsolationConeSizes     = [0.2],
                                  IsolationVarNamePrefix = 'ptcone',
                                  ChargedParticlesOnly   = True)


def DFCommonTruthPhotonIsolationTool3Cfg(flags):
   """Configure the photon isolation tool, etcone=0.4"""
   return  TruthIsolationToolCfg(flags,
                                 name                   = "DFCommonTruthPhotonIsolationTool3",
                                 isoParticlesKey        = "TruthPhotons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [22],
                                 IsolationConeSizes     = [0.4],
                                 IsolationVarNamePrefix = 'etcone',
                                 ChargedParticlesOnly   = False)


# Quark/gluon decoration for jets
def DFCommonTruthDressedWZQGLabelToolCfg(flags):
    """Configure the QG decoration tool for AntiKt4TruthDressedWZJets"""
    return TruthQGDecorationToolCfg(flags,
                                    name          = "DFCommonTruthDressedWZQGLabelTool",
                                    JetCollection = "AntiKt4TruthDressedWZJets")

#==============================================================================
# Truth thinning
#==============================================================================

# Menu truth thinning: removes truth particles on the basis of a menu of
# options (rather than a string)
def MenuTruthThinningCfg(flags, name, **kwargs):
    """Configure the menu truth thinning tool"""
    acc = ComponentAccumulator()
    MenuTruthThinning = CompFactory.DerivationFramework.MenuTruthThinning
    acc.addPublicTool(MenuTruthThinning(name, **kwargs),
                      primary = True)
    return acc

#==============================================================================
# Other tools
#==============================================================================
# Truth links on some objects point to the main truth particle container.
# This re-points the links from the old container to the new container
def TruthLinkRepointToolCfg(flags, name, **kwargs):
    """Configure the truth link repointing tool"""
    acc = ComponentAccumulator()
    TruthLinkRepointTool = CompFactory.DerivationFramework.TruthLinkRepointTool
    acc.addPublicTool(TruthLinkRepointTool(name, **kwargs),
                      primary = True)
    return acc


# Makes a small collection of 'primary' vertices, one per event
# A bit like a collection of 'reconstructable' vertices
def TruthPVCollectionMakerCfg(flags, name, **kwargs):
    """Configure the truth PV collection maker tool"""
    acc = ComponentAccumulator()
    TruthPVCollectionMaker = CompFactory.DerivationFramework.TruthPVCollectionMaker
    acc.addPublicTool(TruthPVCollectionMaker(name, **kwargs),
                      primary = True)
    return acc


# Tool for thinning TruthParticles
def GenericTruthThinningCfg(flags, name, **kwargs):
    """Configure the GenericTruthThinning tool"""
    acc = ComponentAccumulator()
    GenericTruthThinning = CompFactory.DerivationFramework.GenericTruthThinning
    acc.addPublicTool(GenericTruthThinning(name, **kwargs),
                      primary = True)
    return acc
