# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import GeV

# Copy this to xAODTruthCnvConfig.py
def xAODTruthCnvForFiltersConfig(flags, **kwargs):
    kwargs.setdefault("WriteTruthMetaData", False)
    from xAODTruthCnv.xAODTruthCnvConfig import GEN_EVNT2xAODCfg
    return GEN_EVNT2xAODCfg(flags, **kwargs)


def CreatexAODSlimmedContainerCfg(flags, containerName, **kwargs):
    # TODO Split into separate methods - no need for this to be a single method
    # TODO Need to add all of these to "prefiltSeq"
    cfg = xAODTruthCnvForFiltersConfig(flags)
    if containerName=="TruthElectrons":
        cfg.addEventAlgo(CompFactory.xAODTruthParticleSlimmerElectron('xAODTruthParticleSlimmerElectron'))
    elif containerName=="TruthMuons":
        cfg.addEventAlgo(CompFactory.xAODTruthParticleSlimmerMuon('xAODTruthParticleSlimmerMuon'))
    elif containerName=="TruthTaus":
        cfg.addEventAlgo(CompFactory.xAODTruthParticleSlimmerTau('xAODTruthParticleSlimmerTau'))
    elif containerName=="TruthPhotons":
        cfg.addEventAlgo(CompFactory.xAODTruthParticleSlimmerPhoton('xAODTruthParticleSlimmerPhoton'))
    elif containerName=="TruthMET":
        cfg.addEventAlgo(CompFactory.xAODTruthParticleSlimmerMET('xAODTruthParticleSlimmerMET'))
    elif containerName=="TruthLightLeptons":
        cfg.addEventAlgo(CompFactory.xAODTruthParticleSlimmerLightLepton('xAODTruthParticleSlimmerLightLepton'))
    elif containerName=="TruthGen":
        cfg.addEventAlgo(CompFactory.xAODTruthParticleSlimmerGen('xAODTruthParticleSlimmerGen'))
    else:
        raise NameError("containerName '%s' unknown, bailing out" % (containerName))
    return cfg


def CreateTruthJetsCfg(flags, jetR, mods=""):
    """ FIXME This syntax needs to be updated would probably better live in JetRecConfig.JetRecConfig
    """
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # for compatibility with the rest of GEN config, we just re-map the
    # standard def. Best would be to change all clients in GEN to call us passing a full JetDefinition object
    if jetR < 0.65 :
      from JetRecConfig.StandardSmallRJets import  AntiKt4TruthGEN,AntiKt4TruthGENWZ,AntiKt6TruthGEN,AntiKt6TruthGENWZ
      jetdef = {
          (0.4,"") : AntiKt4TruthGEN,
          (0.4,"WZ") : AntiKt4TruthGENWZ,
          (0.6,"") : AntiKt6TruthGEN,
          (0.6,"WZ") : AntiKt6TruthGENWZ,
      }[ (jetR,mods) ]
    else :
      from JetRecConfig.StandardLargeRJets import  AntiKt10TruthGEN,AntiKt10TruthGENWZ
      jetdef = {
          (1.0,"") : AntiKt10TruthGEN,
          (1.0,"WZ") : AntiKt10TruthGENWZ,
      }[ (jetR,mods) ]

    # run2-config compatibility. (run3 style would use a ComponentAccumulator).
    from JetRecConfig.JetRecConfig import getJetAlgs, reOrderAlgs

    # Get the algs needed by the JetDefinition and schedule them with runII style
    algs, jetdef_i = getJetAlgs(flags, jetdef, True)
    algs, ca = reOrderAlgs( [a for a in algs if a is not None]) ## FIXME need to ensure that these are added to the prefiltSeq
    cfg.merge(ca)
    return cfg


def QCDTruthJetFilterCfg(flags, jetR, mods="", **kwargs):
    """Default truth jet filter setup
    """
    cfg = ComponentAccumulator()
    cfg.merge(CreateTruthJetsCfg(flags, jetR, mods)) # Algs in prefiltSeq
    kwargs.setdefault("TruthJetContainer", 'AntiKt{0}Truth{1}Jets'.format(int(jetR*10),mods))
    kwargs.setdefault("MaxPt", flags.Beam.Energy)
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.QCDTruthJetFilter(name="QCDTruthJetFilter", **kwargs)) ## TODO add to filtSeq
    return cfg


def JZSliceCfg(flags,x, jetR, mods="", **kwargs):
    # Min and max momenta for the slices
    minDict = {0:-1,1:20,2:60,3:160,4:400,5:800,6:1300,7:1800,8:2500,9:3200,10:3900,11:4600,12:5300}
    maxDict = {0:20,1:60,2:160,3:400,4:800,5:1300,6:1800,7:2500,8:3200,9:3900,10:4600,11:5300,12:7000}
    kwargs.setdefault("MinPt", minDict[x]*GeV)
    kwargs.setdefault("MaxPt", maxDict[x]*GeV)
    return QCDTruthJetFilterCfg(flags, jetR, mods="", **kwargs)


def LeptonPairFilterExampleCfg(flags, name='SS3LFilter', **kwargs):
    """Example configuring LeptonPairFilter to accept SS lepton pairs with
    massive parents, as used in
    MadGraphControl_SimplifiedModel_BB_tC1.py
    """
    cfg = ComponentAccumulator()
    kwargs.setdefault("NLeptons_Min", 0)
    kwargs.setdefault("NLeptons_Max", -1)
    kwargs.setdefault("Ptcut", 10000)
    kwargs.setdefault("Etacut", 2.8)
    kwargs.setdefault("NSFOS_Min", -1)
    kwargs.setdefault("NSFOS_Max", -1)
    kwargs.setdefault("NOFOS_Min", -1)
    kwargs.setdefault("NOFOS_Max", -1)
    kwargs.setdefault("NSFSS_Min", -1)
    kwargs.setdefault("NSFSS_Max", -1)
    kwargs.setdefault("NOFSS_Min", -1)
    kwargs.setdefault("NOFSS_Max", -1)
    kwargs.setdefault("NPairSum_Max", -1)
    kwargs.setdefault("NPairSum_Min", 1)
    kwargs.setdefault("UseSFOSInSum", False)
    kwargs.setdefault("UseSFSSInSum", True)
    kwargs.setdefault("UseOFOSInSum", False)
    kwargs.setdefault("UseOFSSInSum", True)
    kwargs.setdefault("OnlyMassiveParents", True)
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.LeptonPairFilter(name, **kwargs)) ## TODO add to filtSeq
    return cfg


def xAODBHadronFilterCommonCfg(flags, **kwargs):
    """Make a HeavyFlavorHadronFilter and steer it to select B Hadrons
    with any pT within |eta| < 4"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    ## Default cut params
    kwargs.setdefault("RequestBottom", True)
    kwargs.setdefault("RequestCharm", False)
    kwargs.setdefault("Request_cQuark", False)
    kwargs.setdefault("Request_bQuark", False)
    kwargs.setdefault("RequestSpecificPDGID", False)
    kwargs.setdefault("RequireTruthJet", False)
    kwargs.setdefault("BottomPtMin", 0*GeV)
    kwargs.setdefault("BottomEtaMax", 4.0)
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODHeavyFlavorHadronFilter("xAODHeavyFlavorBHadronFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODBSignalFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODBSignal filter conversion to xAOD, creation
    of slimmed container containing electrons connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODBSignalFilter("xAODBSignalFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


# To modify cuts make a new Cfg method depending on this one e.g.
#def xAODBSignalFilterModifiedExampleCfg(flags, **kwargs):
#    kwargs.setdefault("Cuts_Final_hadrons_switch", True)
#    kwargs.setdefault("Cuts_Final_hadrons_pT", 900.0)
#    kwargs.setdefault("Cuts_Final_hadrons_eta", 2.6)
#    kwargs.setdefault("B_PDGCode", 521)
#    kwargs.setdefault("LVL1MuonCutOn", True)
#    kwargs.setdefault("LVL2MuonCutOn", True)
#    kwargs.setdefault("LVL1MuonCutPT", 3500)
#    kwargs.setdefault("LVL1MuonCutEta", 2.6)
#    kwargs.setdefault("LVL2MuonCutPT", 3500)
#    kwargs.setdefault("LVL2MuonCutEta", 2.6)
#    return xAODBSignalFilterCommonCfg(flags, **kwargs)


def xAODCHadronFilterCommonCfg(flags, **kwargs):
    """Make a HeavyFlavorHadronFilter and steer it to select C Hadrons
    with any pT within |eta| < 4"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    ## Default cut params
    kwargs.setdefault("RequestBottom", False)
    kwargs.setdefault("RequestCharm", True)
    kwargs.setdefault("Request_cQuark", False)
    kwargs.setdefault("Request_bQuark", False)
    kwargs.setdefault("RequestSpecificPDGID", False)
    kwargs.setdefault("RequireTruthJet", False)
    kwargs.setdefault("CharmPtMin", 0*GeV)
    kwargs.setdefault("CharmEtaMax", 4.0)
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODHeavyFlavorHadronFilter("xAODHeavyFlavorCHadronFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODChargedTracksFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODChargedTracks filter conversion to xAOD,
    creation of slimmed container containing truth events connecting the
    filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    cfg.addEventAlgo(CompFactory.xAODChargedTracksFilter("xAODChargedTracksFilter", **kwargs)) # TODO Add to filtSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    return cfg


def xAODChargedTracksWeightFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODChargedTracksWeight filter conversion to
    xAOD, creation of slimmed container containing truth events
    connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    cfg.addEventAlgo(CompFactory.xAODChargedTracksWeightFilter("xAODChargedTracksWeightFilter", **kwargs)) # TODO Add to filtSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    return cfg


def xAODDecayTimeFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODDecayTime filter conversion to xAOD,
    creation of slimmed container containing truth events connecting
    the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODDecayTimeFilter("xAODDecayTimeFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODDecaysFinalStateFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODDecaysFinalState filter conversion to xAOD,
    creation of slimmed container containing truth events connecting
    the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODDecaysFinalStateFilter("xAODDecaysFinalStateFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODDiLeptonMassFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODDiLeptonMass filter conversion to xAOD,
    creation of slimmed container containing electrons connecting the
    filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthLightLeptons") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODDiLeptonMassFilter("xAODDiLeptonMassFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODDirectPhotonFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODDirectPhoton filter conversion to xAOD,
    connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODDirectPhotonFilter("xAODDirectPhotonFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODElectronFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODElectron filter conversion to xAOD,
    creation of slimmed container containing electrons connecting the
    filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthElectrons") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODElectronFilter("xAODElectronFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODForwardProtonFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODForwardProton filter conversion to xAOD,
    creation of slimmed container containing truth events connecting
    the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODForwardProtonFilter("xAODForwardProtonFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODFourLeptonMassFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODDiLeptonMass filter conversion to xAOD,
    creation of slimmed container containing electrons connecting the
    filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthLightLeptons") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODFourLeptonMassFilter("xAODFourLeptonMassFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODHTFilterCommonCfg(flags, **kwargs):
    """HT filter setup for anti-kT R=0.4 truth jets"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    cfg.merge(CreateTruthJetsCfg(flags, 0.4,"WZ")) # Algs in prefiltSeq
    from MCTruthClassifier.MCTruthClassifierConfig import DFCommonMCTruthClassifierCfg
    cfg.merge(DFCommonMCTruthClassifierCfg(flags)) # FIXME this Cfg method creates a public MCTruthClassifier tool.
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODHTFilter("xAODHTFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODHeavyFlavorHadronFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODHeavyFlavorHadron filter conversion to
    xAOD, creation of slimmed container containing truth events
    connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODHeavyFlavorHadronFilter("xAODHeavyFlavorHadronFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODJetFilterCommonCfg(flags, **kwargs):
    """conversion to xAOD, connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODJetFilter("xAODJetFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODLeptonFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODLepton filter conversion to xAOD, creation
    of slimmed container containing electrons and muons connecting the
    filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthElectrons") # Algs in prefiltSeq
    cfg.merge(CreatexAODSlimmedContainerCfg(flags, containerName="TruthMuons")) # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODLeptonFilter("xAODLeptonFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODLeptonPairFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODLeptonPair filter conversion to xAOD,
    creation of slimmed container containing truth events connecting
    the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    #for more settable parameters see:
    #https://gitlab.cern.ch/atlas/athena/-/blob/master/Generators/GeneratorFilters/GeneratorFilters/xAODLeptonPairFilter.h
    cfg.addEventAlgo(CompFactory.xAODLeptonPairFilter("xAODLeptonPairFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODM4MuIntervalFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODM4MuInterval filter conversion to xAOD,
    creation of slimmed container containing truth events connecting
    the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODM4MuIntervalFilter("xAODM4MuIntervalFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODMETFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODMissingEt filter conversion to xAOD,
    creation of slimmed container containing MET connecting the
    filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthMET") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODMETFilter("xAODMissingEtFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODMuDstarFilterCommonCfg(flags, **kwargs):
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    cfg.merge(CreateTruthJetsCfg(flags, 0.4)) # Algs in prefiltSeq
    ## Default cut params - TODO Why do these differ from the C++ header?
    kwargs.setdefault("PtMinMuon",  0.)
    kwargs.setdefault("PtMaxMuon",  1e9)
    kwargs.setdefault("EtaRangeMuon",  10.0)
    kwargs.setdefault("PtMinDstar",  0.)
    kwargs.setdefault("PtMaxDstar",  1e9)
    kwargs.setdefault("EtaRangeDstar",  10.0)
    kwargs.setdefault("RxyMinDstar",  -1e9)
    kwargs.setdefault("PtMinPis",  0.)
    kwargs.setdefault("PtMaxPis",  1e9)
    kwargs.setdefault("EtaRangePis",  10.0)
    kwargs.setdefault("D0Kpi_only", False)
    kwargs.setdefault("PtMinKpi",  0.)
    kwargs.setdefault("PtMaxKpi",  1e9)
    kwargs.setdefault("EtaRangeKpi",  10.0)
    kwargs.setdefault("mKpiMin",  0.)
    kwargs.setdefault("mKpiMax",  1e9)
    kwargs.setdefault("delta_m_Max",  1e9)
    kwargs.setdefault("DstarMu_m_Max",  1e9)
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODMuDstarFilter("xAODMuDstarFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODMultiBjetFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODMultiBjet filter conversion to xAOD,
    creation of slimmed container containing truth events creating truh
    jets container connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    cfg.merge(CreateTruthJetsCfg(flags, 0.4)) # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODMultiBjetFilter("xAODMultiBjetFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODMultiCjetFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODMultiCjet filter conversion to xAOD,
    creation of slimmed container containing truth events creating truth
    jets container connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    cfg.merge(CreateTruthJetsCfg(flags, 0.4)) # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODMultiCjetFilter("xAODMultiCjetFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODMultiElecMuTauFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODMultiLepton filter conversion to xAOD,
    creation of slimmed container containing electrons and muons
    connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODMultiElecMuTauFilter("xAODMultiElecMuTauFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODMultiElectronFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODMultiElectron filter conversion to xAOD,
    connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthElectrons") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODMultiElectronFilter("xAODMultiElectronFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODMultiLeptonFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODMultiLepton filter conversion to xAOD,
    creation of slimmed container containing electrons and muons
    connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthElectrons") # Algs in prefiltSeq
    cfg.merge(CreatexAODSlimmedContainerCfg(flags, containerName="TruthMuons")) # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODMultiLeptonFilter("xAODMultiLeptonFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODMultiMuonFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODMultiMuon filter conversion to xAOD,
    connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthMuons") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODMultiMuonFilter("xAODMultiMuonFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODMuonFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODMuon filter conversion to xAOD, creation of
    slimmed container containing muons connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthMuons") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODMuonFilter("xAODMuonFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODParentChildFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODParentChildLepton filter conversion to
    xAOD, connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODParentChildFilter("xAODParentChildFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODParentTwoChildrenFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODParentTwoChildren filter conversion to
    xAOD, connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    cfg.merge(CreateTruthJetsCfg(flags, 0.4)) # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODParentTwoChildrenFilter("xAODParentTwoChildrenFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODParticleDecayFilterCommonCfg(flags, **kwargs):
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODParticleDecayFilter("xAODParticleDecayFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODParticleFilterCommonCfg(flags, **kwargs):
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODParticleFilter("xAODParticleFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODPhotonFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODPhoton filter conversion to xAOD, creation
    of slimmed container containing photons connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthPhotons") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODPhotonFilter("xAODPhotonFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODSameParticleHardScatteringFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODSameParticleHardScattering filter
    conversion to xAOD, connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODSameParticleHardScatteringFilter("xAODSameParticleHardScatteringFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODSplitPhotonFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODSplitPhoton filter conversion to xAOD,
    connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODSplitPhotonFilter("xAODSplitPhotonFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODTTbarWToLeptonFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODTTbarWToLepton filter conversion to xAOD,
    connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODTTbarWToLeptonFilter("xAODTTbarWToLeptonFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODTTbarWithJpsimumuFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODTTbarWithJpsimumu filter conversion to
    xAOD, connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODTTbarWithJpsimumuFilter("xAODTTbarWithJpsimumuFilter", **kwargs)) # TODO Add to filtSeq
    return cfg


def xAODTauFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODTau filter conversion to xAOD, creation of
    slimmed container containing taus connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthTaus") # Algs
    # in prefiltSeq To modify tau selection pT and eta cuts (on the
    # Slimmer level) make a new Cfg method depending on
    # CreatexAODSlimmedContainerCfg where you override the required parameters
    # cfg.getEventAlgo("xAODTruthParticleSlimmerTau").tau_pt_selection = 1000.0
    # cfg.getEventAlgo("xAODTruthParticleSlimmerTau").abseta_selection = 4.5

    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODTauFilter("xAODTauFilter", **kwargs)) ## TODO add to filtSeq
    return cfg


def xAODVBFForwardJetsFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODVBFForwardJetsFilter conversion to xAOD,
    connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    cfg.merge(CreateTruthJetsCfg(flags, 0.4)) # Algs in prefiltSeq
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODVBFForwardJetsFilter("xAODVBFForwardJetsFilter", **kwargs)) ## TODO add to filtSeq
    return cfg


def xAODVBFMjjIntervalFilterCommonCfg(flags, **kwargs):
    """common fragment for xAODVBFMjjInterval filter conversion to xAOD,
    connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    cfg.merge(CreateTruthJetsCfg(flags, 0.4)) # Algs in prefiltSeq
    ## Default cut params - TODO Why do these differ from the defaults in the C++ header?
    kwargs.setdefault("NoJetProbability", 1000.0*0.0000252)
    kwargs.setdefault("OneJetProbability", 1000.0*0.0000252)
    kwargs.setdefault("LowMjjProbability", 1000.0*0.0000252)
    kwargs.setdefault("HighMjj", 2500.*GeV)
    kwargs.setdefault("PhotonJetOverlapRemoval", True)
    kwargs.setdefault("ElectronJetOverlapRemoval", True)
    kwargs.setdefault("TauJetOverlapRemoval", True)
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODVBFMjjIntervalFilter("xAODVBFMjjIntervalFilter", **kwargs)) ## TODO add to filtSeq
    return cfg


def xAODXtoVVDecayFilterExtendedCfg(flags, **kwargs):
    """common fragment for xAODtoVVDecayExtended filter conversion to
    xAOD, connecting the filter"""
    cfg = CreatexAODSlimmedContainerCfg(flags, containerName="TruthGen") # Algs in prefiltSeq
    ## Default cut params - TODO Why do these differ from the defaults in the C++ header?
    kwargs.setdefault("PDGGrandParent", 25)
    kwargs.setdefault("PDGParent", 15)
    kwargs.setdefault("PDGChild1", [211,213,215,311,321,323,10232,10323,20213,20232,20323,30213,100213,100323,1000213])
    kwargs.setdefault("PDGChild2", [211,213,215,311,321,323,10232,10323,20213,20232,20323,30213,100213,100323,1000213])
    # To modify cuts make a new Cfg method depending on this one, where you set the required kwargs
    cfg.addEventAlgo(CompFactory.xAODXtoVVDecayFilterExtended("xAODXtoVVDecayFilterExtended", **kwargs)) ## TODO add to filtSeq
    return cfg


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Logging import log
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.EVNT_RUN3_50
    flags.lock()

    acc = ComponentAccumulator()
    acc.merge( LeptonPairFilterExampleCfg(flags))
    acc.merge( xAODBHadronFilterCommonCfg(flags))
    acc.merge( xAODBSignalFilterCommonCfg(flags))
    acc.merge( xAODCHadronFilterCommonCfg(flags))
    acc.merge( xAODChargedTracksFilterCommonCfg(flags))
    acc.merge( xAODChargedTracksWeightFilterCommonCfg(flags))
    acc.merge( xAODDecayTimeFilterCommonCfg(flags))
    acc.merge( xAODDecaysFinalStateFilterCommonCfg(flags))
    acc.merge( xAODDiLeptonMassFilterCommonCfg(flags))
    acc.merge( xAODDirectPhotonFilterCommonCfg(flags))
    acc.merge( xAODElectronFilterCommonCfg(flags))
    acc.merge( xAODForwardProtonFilterCommonCfg(flags))
    acc.merge( xAODFourLeptonMassFilterCommonCfg(flags))
    acc.merge( xAODHTFilterCommonCfg(flags))
    acc.merge( xAODHeavyFlavorHadronFilterCommonCfg(flags))
    acc.merge( xAODJetFilterCommonCfg(flags))
    acc.merge( xAODLeptonFilterCommonCfg(flags))
    acc.merge( xAODLeptonPairFilterCommonCfg(flags))
    acc.merge( xAODM4MuIntervalFilterCommonCfg(flags))
    acc.merge( xAODMETFilterCommonCfg(flags))
    acc.merge( xAODMuDstarFilterCommonCfg(flags))
    acc.merge( xAODMultiBjetFilterCommonCfg(flags))
    acc.merge( xAODMultiCjetFilterCommonCfg(flags))
    acc.merge( xAODMultiElecMuTauFilterCommonCfg(flags))
    acc.merge( xAODMultiElectronFilterCommonCfg(flags))
    acc.merge( xAODMultiLeptonFilterCommonCfg(flags))
    acc.merge( xAODMultiMuonFilterCommonCfg(flags))
    acc.merge( xAODMuonFilterCommonCfg(flags))
    acc.merge( xAODParentChildFilterCommonCfg(flags))
    acc.merge( xAODParentTwoChildrenFilterCommonCfg(flags))
    acc.merge( xAODParticleDecayFilterCommonCfg(flags))
    acc.merge( xAODParticleFilterCommonCfg(flags))
    acc.merge( xAODPhotonFilterCommonCfg(flags))
    acc.merge( xAODSameParticleHardScatteringFilterCommonCfg(flags))
    acc.merge( xAODSplitPhotonFilterCommonCfg(flags))
    acc.merge( xAODTTbarWToLeptonFilterCommonCfg(flags))
    acc.merge( xAODTTbarWithJpsimumuFilterCommonCfg(flags))
    acc.merge( xAODTauFilterCommonCfg(flags))
    acc.merge( xAODVBFForwardJetsFilterCommonCfg(flags))
    acc.merge( xAODVBFMjjIntervalFilterCommonCfg(flags))
    acc.merge( xAODXtoVVDecayFilterExtendedCfg(flags))
    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('GeneratorFiltersConfig.pkl','wb') )

    print('All OK')
