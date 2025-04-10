# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

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
    TruthCollectionMaker = CompFactory.DerivationFramework.TruthCollectionMaker
    acc.addPublicTool(TruthCollectionMaker(name = name,**kwargs),
                      primary = True)
    return acc

def DFCommonTruthMuonToolCfg(flags):
    """Muon truth collection maker"""
    return TruthCollectionMakerCfg(flags,
                                   name                    = "DFCommonTruthMuonTool",
                                   NewCollectionName       = "TruthMuons",
                                   KeepNavigationInfo      = False,
                                   ParticleSelectionString = f"(abs(TruthParticles.pdgId) == 13) && (TruthParticles.status == 1) && TruthParticles.barcode < {flags.Sim.SimBarcodeOffset}")

def DFCommonTruthElectronToolCfg(flags):
    """Electron truth collection maker"""
    return TruthCollectionMakerCfg(flags,
                                   name                    = "DFCommonTruthElectronTool",
                                   NewCollectionName       = "TruthElectrons",
                                   KeepNavigationInfo      = False,
                                   ParticleSelectionString = f"(abs(TruthParticles.pdgId) == 11) && (TruthParticles.status == 1) && TruthParticles.barcode < {flags.Sim.SimBarcodeOffset}")

def DFCommonTruthPhotonToolCfg(flags):
    """Photon truth collection maker"""
    return TruthCollectionMakerCfg(flags,
                                   name                    = "DFCommonTruthPhotonTool",
                                   NewCollectionName       = "TruthPhotons",
                                   KeepNavigationInfo      = False,
                                   ParticleSelectionString = f"(abs(TruthParticles.pdgId) == 22) && (TruthParticles.status == 1) && TruthParticles.barcode < {flags.Sim.SimBarcodeOffset}")

# this tool is needed for making TruthPhotons from sim samples, where extra cuts are needed. Origin 42 (pi0) and 23 (light meson) cut way down uninteresting photons
def DFCommonTruthPhotonToolSimCfg(flags):
    """Tool for making TruthPhotons from sim samples"""
    return TruthCollectionMakerCfg(flags,
                                   name                    = "DFCommonTruthPhotonToolSim",
                                   NewCollectionName       = "TruthPhotons",
                                   KeepNavigationInfo      = False,
                                   ParticleSelectionString = f"(abs(TruthParticles.pdgId) == 22) && (TruthParticles.status == 1) && ((TruthParticles.classifierParticleOrigin != 42 && TruthParticles.classifierParticleOrigin !=23) || (TruthParticles.pt > 20.0*GeV)) && ( TruthParticles.barcode < {flags.Sim.SimBarcodeOffset} )") 

def DFCommonTruthNeutrinoToolCfg(flags):
    """Neutrino truth collection maker"""
    neutrinoexpression = f"(TruthParticles.isNeutrino && TruthParticles.status == 1) && TruthParticles.barcode < {flags.Sim.SimBarcodeOffset}"
    return TruthCollectionMakerCfg(flags,
                                   name = "DFCommonTruthNeutrinoTool",
                                   NewCollectionName       = "TruthNeutrinos",
                                   KeepNavigationInfo      = False,
                                   ParticleSelectionString = neutrinoexpression)

def DFCommonTruthBottomToolCfg(flags):
    """B-quark truth collection maker"""
    return TruthCollectionMakerCfg(flags,
                                   name                    = "DFCommonTruthBottomTool",
                                   NewCollectionName       = "TruthBottom",
                                   KeepNavigationInfo      = False,
                                   ParticleSelectionString = "(abs(TruthParticles.pdgId) == 5)",
                                   Do_Compress             = True)

def DFCommonTruthTopToolCfg(flags):
    """Top-quark truth collection maker"""
    return TruthCollectionMakerCfg(flags,
                                   name                    = "DFCommonTruthTopTool",
                                   NewCollectionName       = "TruthTop",
                                   KeepNavigationInfo      = False,
                                   ParticleSelectionString = "(abs(TruthParticles.pdgId) == 6)",
                                   Do_Compress             = True)
                                   
def DFCommonTruthBosonToolCfg(flags):
    """Gauge bosons and Higgs truth collection maker"""
    return TruthCollectionMakerCfg(flags,
                                   name                    = "DFCommonTruthBosonTool",
                                   NewCollectionName       = "TruthBoson",
                                   KeepNavigationInfo      = False,
                                   ParticleSelectionString = "(abs(TruthParticles.pdgId) == 23 || abs(TruthParticles.pdgId) == 24 || abs(TruthParticles.pdgId) == 25)",
                                   Do_Compress             = True,
                                   Do_Sherpa               = True)

def DFCommonTruthBSMToolCfg(flags):
    """BSM particles truth collection maker"""
    return TruthCollectionMakerCfg(flags,
                                   name                    = "DFCommonTruthBSMTool",
                                   NewCollectionName       = "TruthBSM",
                                   KeepNavigationInfo      = False,
                                   ParticleSelectionString = "(TruthParticles.isBSM)",
                                   Do_Compress             = True)

def DFCommonTruthForwardProtonToolCfg(flags):
    """Forward proton truth collection maker"""
    beam_energy = flags.Beam.Energy
    return TruthCollectionMakerCfg(flags,
                                   name                    = "DFCommonTruthForwardProtonTool",
                                   NewCollectionName       = "TruthForwardProtons",
                                   KeepNavigationInfo      = False,
                                   ParticleSelectionString = "(TruthParticles.status==1) && (abs(TruthParticles.pdgId)==2212) && (TruthParticles.e>0.8*"+str(beam_energy)+")",
                                   Do_Compress             = True) 

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
    TruthClassificationDecorator = CompFactory.DerivationFramework.TruthClassificationDecorator
    acc.addPublicTool(TruthClassificationDecorator(name = name, **kwargs),
                      primary = True)
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
    from MCTruthClassifier.MCTruthClassifierConfig import DFCommonMCTruthClassifierCfg
    accMCTC = DFCommonMCTruthClassifierCfg(flags)
    DFCommonTruthClassifier = accMCTC.getPrimary()
    acc = TruthClassificationDecoratorCfg(flags,
                                          name = "DFCommonTruthClassificationTool",
                                          ParticlesKey = "TruthParticles",
                                          MCTruthClassifier = DFCommonTruthClassifier)
    acc.merge(accMCTC)
    return acc

# Hadron origin decoration tools
def HadronOriginClassifierCfg(flags, name, **kwargs):
    """get the hadron origin classification"""
    acc = ComponentAccumulator()
    HadronOriginClassifier = CompFactory.DerivationFramework.HadronOriginClassifier
    acc.addPublicTool(HadronOriginClassifier(name = name, **kwargs),
                      primary = True)
    return acc

def HadronOriginDecoratorCfg(flags, name, **kwargs):
    """decorate with the hadron origin classification"""
    acc = ComponentAccumulator()
    HadronOriginDecorator = CompFactory.DerivationFramework.HadronOriginDecorator
    acc.addPublicTool(HadronOriginDecorator(name = name, **kwargs),
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
                                decorationName        = decorationName)

def DFCommonTruthMuonDressingToolCfg(flags, decorationName = "dressedPhoton"):
    """Configure the muon truth dressing tool"""
    return TruthDressingToolCfg(flags,
                                name                  = "DFCommonTruthMuonDressingTool",
                                dressParticlesKey     = "TruthMuons",
                                usePhotonsFromHadrons = False,
                                dressingConeSize      = 0.1,
                                particleIDsToDress    = [13],
                                decorationName        = decorationName)

def DFCommonTruthTauDressingToolCfg(flags):
    """Configure the tau truth dressing tool"""
    return TruthDressingToolCfg(flags,
                                name                  = "DFCommonTruthTauDressingTool",
                                dressParticlesKey     = "TruthTaus",
                                usePhotonsFromHadrons = False,
                                dressingConeSize      = 0.2, # Tau special
                                particleIDsToDress    = [15])

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
