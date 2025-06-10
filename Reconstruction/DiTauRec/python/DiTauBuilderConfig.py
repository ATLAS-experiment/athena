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
    DiTauIDVarDecoratorCfg, 
    DiTauOnnxScoreCalculatorCfg
)

def DiTauBuilderCfg(flags, name="DiTauBuilder", doLowPt=False, **kwargs):
    acc = ComponentAccumulator()

    tools = [
        acc.popToolsAndMerge(SeedJetBuilderCfg(flags, JetCollection=flags.DiTau.SeedJetCollection[0])),
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
    if flags.DiTau.doRunDiTauDiscriminant:
        tools.append(acc.popToolsAndMerge(DiTauIDVarDecoratorCfg(flags)))
        tools.append(acc.popToolsAndMerge(DiTauOnnxScoreCalculatorCfg(flags)))

    if doLowPt:
        kwargs.setdefault("DiTauContainer", flags.DiTau.DiTauContainer[1])
        kwargs.setdefault("minPt", flags.DiTau.JetSeedPt[1])
    else:
        kwargs.setdefault("DiTauContainer", flags.DiTau.DiTauContainer[0])
        kwargs.setdefault("minPt", flags.DiTau.JetSeedPt[0])

    kwargs.setdefault("Tools", tools)
    kwargs.setdefault("SeedJetName", flags.DiTau.SeedJetCollection[0])
    kwargs.setdefault("maxEta", flags.DiTau.MaxEta)
    kwargs.setdefault("Rjet", flags.DiTau.Rjet)
    kwargs.setdefault("Rsubjet", flags.DiTau.Rsubjet)
    kwargs.setdefault("Rcore", flags.DiTau.Rcore)

    acc.addEventAlgo(CompFactory.DiTauBuilder(name, **kwargs))
    return acc

