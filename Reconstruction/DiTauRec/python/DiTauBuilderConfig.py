# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from DiTauRec.DiTauToolsConfig import (
    SeedJetBuilderCfg, 
    SubjetBuilderCfg, 
    JetAlgCfg, 
    VertexFinderCfg, 
    DiTauTrackFinderCfg, 
    CellFinderCfg, 
    DiTauConstituentFinderCfg,
    DiTauExtraVarDecoratorCfg, 
    DiTauOnnxScoreCalculatorCfg
)

def DiTauBuilderCfg(flags, name="DiTauBuilder", doLowPt=False):
    acc = ComponentAccumulator()

    tools = [
        acc.popToolsAndMerge(SeedJetBuilderCfg(flags)),
        acc.popToolsAndMerge(SubjetBuilderCfg(flags))
    ]

    if flags.Tracking.doVertexFinding: # Simplified wrt old config
        acc.merge(JetAlgCfg(flags)) # To run TVA tool for VertexFinder
        tools.append(acc.popToolsAndMerge(VertexFinderCfg(flags)))

    tools.append(acc.popToolsAndMerge(DiTauTrackFinderCfg(flags)))
    if doLowPt:
        tools.append(acc.popToolsAndMerge(DiTauConstituentFinderCfg(flags, UseRawConstit=True)))
    else:    
        tools.append(acc.popToolsAndMerge(CellFinderCfg(flags)))

    if flags.DiTau.doExtraVariables:
        tools.append(acc.popToolsAndMerge(DiTauExtraVarDecoratorCfg(flags))) 

    if flags.DiTau.doRunDiTauDiscriminant:
        tools.append(acc.popToolsAndMerge(DiTauOnnxScoreCalculatorCfg(flags)))

    acc.addEventAlgo(CompFactory.DiTauBuilder(name,
                                              DiTauContainer = flags.DiTau.DiTauContainer[1] if doLowPt else flags.DiTau.DiTauContainer[0],
                                              minPt = flags.DiTau.JetSeedPt[1] if doLowPt else flags.DiTau.JetSeedPt[0],
                                              Tools = tools,
                                              SeedJetName = flags.DiTau.SeedJetCollection[0],
                                              maxEta = flags.DiTau.MaxEta,
                                              Rjet = flags.DiTau.Rjet,
                                              Rsubjet = flags.DiTau.Rsubjet,
                                              Rcore = flags.DiTau.Rcore))
    return acc

