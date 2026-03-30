# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

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


def DFCommonTruthMuonCfg(flags, name = "DFCommonTruthMuon", **kwargs):
    """Muon truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthMuons")
    kwargs.setdefault("KeepNavigationInfo", False)
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthCollectionMakerMuon(name = name,**kwargs))
    return acc


def DFCommonTruthCharmCfg(flags, name = "DFCommonTruthCharm", **kwargs):
    """Charm truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthCharm")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("Do_Compress", True)
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthCollectionMakerCharm(name = name,**kwargs))
    return acc


def DFCommonTruthElectronCfg(flags, name = "DFCommonTruthElectron", **kwargs):
    """Electron truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthElectrons")
    kwargs.setdefault("KeepNavigationInfo", False)
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthCollectionMakerElectron(name = name,**kwargs))
    return acc


def DFCommonTruthPhotonCfg(flags, name = "DFCommonTruthPhoton", **kwargs):
    """Photon truth collection maker (Currently unused?)"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthPhotons")
    kwargs.setdefault("KeepNavigationInfo", False)
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthCollectionMakerPhoton(name = name,**kwargs))
    return acc


# this tool is needed for making TruthPhotons from sim samples, where extra cuts are needed. Origin 42 (pi0) and 23 (light meson) cut way down uninteresting photons
def DFCommonTruthPhotonToolSimCfg(flags, name = "DFCommonTruthPhotonSim", **kwargs):
    """Tool for making TruthPhotons from sim samples"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthPhotons")
    kwargs.setdefault("KeepNavigationInfo", False)
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthCollectionMakerPhotonSim(name = name,**kwargs))
    return acc


def DFCommonTruthNeutrinoCfg(flags, name = "DFCommonTruthNeutrino", **kwargs):
    """Neutrino truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthNeutrinos")
    kwargs.setdefault("KeepNavigationInfo", False)
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthCollectionMakerNeutrino(name = name,**kwargs))
    return acc


def DFCommonTruthBottomCfg(flags, name = "DFCommonTruthBottom", **kwargs):
    """B-quark truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthBottom")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("Do_Compress", True)
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthCollectionMakerBottom(name = name,**kwargs))
    return acc


def DFCommonTruthTopCfg(flags, name = "DFCommonTruthTop", **kwargs):
    """Top-quark truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthTop")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("Do_Compress", True)
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthCollectionMakerTop(name = name,**kwargs))
    return acc


def DFCommonTruthBosonCfg(flags, name = "DFCommonTruthBoson", **kwargs):
    """Gauge bosons and Higgs truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthBoson")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("BuildingW", True)
    kwargs.setdefault("BuildingZ", True)
    kwargs.setdefault("Do_Compress", True)
    kwargs.setdefault("Do_Sherpa", True)
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthCollectionMakerBoson(name = name,**kwargs))
    return acc


def DFCommonTruthBSMCfg(flags, name = "DFCommonTruthBSM", **kwargs):
    """BSM particles truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionName", "TruthBSM")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("Do_Compress", True)
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthCollectionMakerBSM(name = name,**kwargs))
    return acc


def DFCommonTruthForwardProtonCfg(flags, name = "DFCommonTruthForwardProton", **kwargs):
    """Forward proton truth collection maker"""
    acc = ComponentAccumulator()
    kwargs.setdefault("BeamEnergy", flags.Beam.Energy)
    kwargs.setdefault("OutputCollectionName", "TruthForwardProtons")
    kwargs.setdefault("KeepNavigationInfo", False)
    kwargs.setdefault("Do_Compress", True)
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthCollectionMakerForwardProton(name, **kwargs))
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


def TruthDressingCfg(flags, name, **kwargs):
    """Configure the TruthDressingAlg"""
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthDressingAlg(
        name = name, **kwargs))
    return acc


def TruthIsolationCfg(flags, name, **kwargs):
    """Configure the truth isolation algorithm"""
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthIsolationAlg(
        name = name, **kwargs))
    return acc


def MuonTruthIsolationDecorAlgCfg(flags, name, **kwargs):
    """Configure the MuonTruthIsolationDecorAlg"""
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.DerivationFramework.MuonTruthIsolationDecorAlg(name = name, **kwargs),
                      primary = True)
    return acc


def TruthQGDecorationCfg(flags, name, **kwargs):
    """Configure the quark/gluon decoration algorithm"""
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthQGDecorationTool(
        name = name, **kwargs))
    return acc


def TruthNavigationDecoratorCfg(flags, name, **kwargs):
    """Congigure the truth navigation decorator tool"""
    acc = ComponentAccumulator()
    kwargs.setdefault("InputCollections", [])
    kwargs.setdefault("parentDecorKeys", [ key + ".parentLinks" for key in kwargs["InputCollections"] ])
    kwargs.setdefault("childDecorKeys", [ key + ".childLinks" for key in kwargs["InputCollections"] ])
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthNavigationDecorator
                     (name = name, **kwargs))
    return acc


def TruthDecayCollectionMakerCfg(flags, name, **kwargs):
    """Configure the truth decay collection maker"""
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.DerivationFramework.TruthDecayCollectionMaker(
        name = name, **kwargs))
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
def DFCommonTruthElectronDressingCfg(flags, decorationName = "dressedPhoton"):
    """Configure the electron truth dressing tool"""
    return TruthDressingCfg(flags,
                                name                  = "DFCommonTruthElectronDressingAlg",
                                dressParticlesKey     = "TruthElectrons",
                                usePhotonsFromHadrons = False,
                                dressingConeSize      = 0.1,
                                particleIDsToDress    = [11],
                                decorationName        = decorationName+"_e")


def DFCommonTruthMuonDressingCfg(flags, decorationName = "dressedPhoton"):
    """Configure the muon truth dressing tool"""
    return TruthDressingCfg(flags,
                                name                  = "DFCommonTruthMuonDressingAlg",
                                dressParticlesKey     = "TruthMuons",
                                usePhotonsFromHadrons = False,
                                dressingConeSize      = 0.1,
                                particleIDsToDress    = [13],
                                decorationName        = decorationName+"_mu")


def DFCommonTruthTauDressingCfg(flags):
    """Configure the tau truth dressing tool"""
    return TruthDressingCfg(flags,
                                name                  = "DFCommonTruthTauDressingAlg",
                                dressParticlesKey     = "TruthTaus",
                                usePhotonsFromHadrons = False,
                                dressingConeSize      = 0.2, # Tau special
                                particleIDsToDress    = [15],
                                decoratePhotons = False)


def DFCommonTruthElectronIsolation1Cfg(flags):
    """Configure the electron isolation algorithm, cone=0.2"""
    return TruthIsolationCfg(flags,
                                 name                   = "DFCommonTruthElectronIsolation1",
                                 isoParticlesKey        = "TruthElectrons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [11],
                                 IsolationConeSizes     = [0.2],
                                 IsolationVarNamePrefix = 'etcone',
                                 ChargedParticlesOnly   = False)


def DFCommonTruthElectronIsolation2Cfg(flags):
    """Configure the electron isolation algorithm, cone=0.3"""
    return TruthIsolationCfg(flags,
                                 name                   =  "DFCommonTruthElectronIsolation2",
                                 isoParticlesKey        = "TruthElectrons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [11],
                                 IsolationConeSizes     = [0.3],
                                 IsolationVarNamePrefix = 'ptcone',
                                 ChargedParticlesOnly   = True)


def DFCommonTruthMuonIsolation1Cfg(flags):
    """Configure the muon isolation algorithm, cone=0.2"""
    return TruthIsolationCfg(flags,
                                 name                   = "DFCommonTruthMuonIsolation1",
                                 isoParticlesKey        = "TruthMuons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [13],
                                 IsolationConeSizes     = [0.2],
                                 IsolationVarNamePrefix = 'etcone',
                                 ChargedParticlesOnly   = False)


def DFCommonTruthMuonIsolation2Cfg(flags):
    """Configure the muon isolation algorithm, cone=0.3"""
    return TruthIsolationCfg(flags,
                                 name                   = "DFCommonTruthMuonIsolation2",
                                 isoParticlesKey        = "TruthMuons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [13],
                                 IsolationConeSizes     = [0.3],
                                 IsolationVarNamePrefix = 'ptcone',
                                 ChargedParticlesOnly   = True)


def DFCommonTruthPhotonIsolation1Cfg(flags):
    """Configure the photon isolation algorithm, etcone"""
    return TruthIsolationCfg(flags,
                                 name                   = "DFCommonTruthPhotonIsolation1",
                                 isoParticlesKey        = "TruthPhotons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [22],
                                 IsolationConeSizes     = [0.2],
                                 IsolationVarNamePrefix = 'etcone',
                                 ChargedParticlesOnly   = False)


def DFCommonTruthPhotonIsolation2Cfg(flags):
    """Configure the photon isolation algorithm, ptcone"""
    return  TruthIsolationCfg(flags,
                                  name                   = "DFCommonTruthPhotonIsolation2",
                                  isoParticlesKey        = "TruthPhotons",
                                  allParticlesKey        = "TruthParticles",
                                  particleIDsToCalculate = [22],
                                  IsolationConeSizes     = [0.2],
                                  IsolationVarNamePrefix = 'ptcone',
                                  ChargedParticlesOnly   = True)


def DFCommonTruthPhotonIsolation3Cfg(flags):
   """Configure the photon isolation algorithm, etcone=0.4"""
   return  TruthIsolationCfg(flags,
                                 name                   = "DFCommonTruthPhotonIsolation3",
                                 isoParticlesKey        = "TruthPhotons",
                                 allParticlesKey        = "TruthParticles",
                                 particleIDsToCalculate = [22],
                                 IsolationConeSizes     = [0.4],
                                 IsolationVarNamePrefix = 'etcone',
                                 ChargedParticlesOnly   = False)


# Quark/gluon decoration for jets
def DFCommonTruthDressedWZQGLabelCfg(flags):
    """Configure the QG decoration algorithm for AntiKt4TruthDressedWZJets"""
    return TruthQGDecorationCfg(flags,
                                name          = "DFCommonTruthDressedWZQGLabel",
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


# Tool for thinning TruthParticles
def GenericTruthThinningCfg(flags, name, **kwargs):
    """Configure the GenericTruthThinning tool"""
    acc = ComponentAccumulator()
    GenericTruthThinning = CompFactory.DerivationFramework.GenericTruthThinning
    acc.addPublicTool(GenericTruthThinning(name, **kwargs),
                      primary = True)
    return acc
