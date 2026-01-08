"""Enable dependencies of tau reconstruction

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

def StandaloneTauRecoFlags(flags):

    flags.Reco.EnableTrigger = False
    flags.Reco.EnableCombinedMuon = True
    flags.Reco.EnablePFlow = True
    flags.Reco.EnableTau = True
    flags.Reco.EnableJet = True
    flags.Reco.EnableBTagging = False
    flags.Reco.EnableCaloRinger = False
    flags.Reco.PostProcessing.GeantTruthThinning = False
    flags.Reco.PostProcessing.TRTAloneThinning = False

# special seed jet collections
def GetSeedCollection(flags):

    # Schedule the custom jets needed for tau seeding
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    from JetRecConfig.JetDefinition import  JetDefinition
    from JetRecConfig.StandardSmallRJets import flavourghosts, calibmods_noCut, standardmods, truthmods
    minimalghosts = ["Track","MuonSegment","Truth"]

    if flags.Tau.TauRec.SeedJetCollection == "AntiKt4MLTopoJets":
        from JetRecConfig.StandardSmallRJets import AntiKt4MLTopo
        return AntiKt4MLTopo
    if flags.Tau.TauRec.SeedJetCollection == "AntiKt4EMPFlowMLJets":
        from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlowML
        return AntiKt4EMPFlowML
    if flags.Tau.TauRec.SeedJetCollection == "AntiKt4EMPFlow10GeVCutTauSeedJets":
        AntiKt4EMPFlow10GeVCutTauSeed = JetDefinition("AntiKt",0.4,cst.GPFlow,
                                      infix = "10GeVCutTauSeed",
                                      ghostdefs = minimalghosts+flavourghosts,
                                      modifiers = calibmods_noCut+("Filter:1",)+truthmods+standardmods+("JetPtAssociation","CaloEnergiesClus"),
                                      ptmin = 10000.,
                                      lock = True)
        return AntiKt4EMPFlow10GeVCutTauSeed
    if flags.Tau.TauRec.SeedJetCollection == "AntiKt4EMPFlow5GeVCutTauSeedJets":
        AntiKt4EMPFlow5GeVCutTauSeed = JetDefinition("AntiKt",0.4,cst.GPFlow,
                                      infix = "5GeVCutTauSeed",
                                      ghostdefs = minimalghosts+flavourghosts,
                                      modifiers = calibmods_noCut+("Filter:1",)+truthmods+standardmods+("JetPtAssociation","CaloEnergiesClus"),
                                      ptmin = 5000.,
                                      lock = True)
        return AntiKt4EMPFlow5GeVCutTauSeed
    if flags.Tau.TauRec.SeedJetCollection == "AntiKt4EMPFlowNoPtCutTauSeedJets":
        AntiKt4EMPFlowNoPtCutTauSeed = JetDefinition("AntiKt",0.4,cst.GPFlow,
                                      infix = "NoPtCutTauSeed",
                                      ghostdefs = minimalghosts+flavourghosts,
                                      modifiers = calibmods_noCut+("Filter:1",)+truthmods+standardmods+("JetPtAssociation","CaloEnergiesClus"),
                                      ptmin = 1,
                                      lock = True)
        return AntiKt4EMPFlowNoPtCutTauSeed


# seed jet for EleRM taus
def GetEleRMSeedCollection(flags):

    if 'PFlow' in flags.Tau.TauRec.SeedJetCollection:
        from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow_tauSeedEleRM
        return AntiKt4EMPFlow_tauSeedEleRM
    else:
        from JetRecConfig.StandardSmallRJets import AntiKt4LCTopo
        AntiKt4LCTopo_EleRM = AntiKt4LCTopo.clone(suffix="_EleRM")
        AntiKt4LCTopo_EleRM.inputdef.name = flags.Tau.ActiveConfig.LCTopoOrigin_EleRM
        AntiKt4LCTopo_EleRM.inputdef.inputname = flags.Tau.ActiveConfig.CaloCalTopoClusters_EleRM
        AntiKt4LCTopo_EleRM.inputdef.containername = flags.Tau.ActiveConfig.LCOriginTopoClusters_EleRM
        AntiKt4LCTopo_EleRM.standardRecoMode = True
        AntiKt4LCTopo_EleRM.context = "EleRM"
        return AntiKt4LCTopo_EleRM

