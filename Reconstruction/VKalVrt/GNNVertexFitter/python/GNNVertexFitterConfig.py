# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg

def GNNVertexFitterToolCfg(flags, name="GNNVertexFitterTool", **kwargs):
    acc = ComponentAccumulator()

    acc.merge(BeamSpotCondAlgCfg(flags))
    kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    acc.setPrivateTools(CompFactory.Rec.GNNVertexFitterTool(**kwargs))

    return acc
    

def GNNVertexFitterAlgCfg(flags, jet_col="AntiKt4EMPFlowJets",  **kwargs):
    acc = ComponentAccumulator()
    
    tool = acc.popToolsAndMerge(GNNVertexFitterToolCfg(flags)) 
    acc.addEventAlgo(CompFactory.Rec.GNNVertexFitterAlg(VtxTool=tool, inputJetContainer=jet_col, **kwargs))
        
    return acc

def main():
    algClass = CompFactory.Rec.GNNVertexFitterAlg
    help(algClass)

    toolClass = CompFactory.Rec.GNNVertexFitterTool
    help(toolClass)
    
    gnntoolClass = CompFactory.FlavorTagDiscriminants.GNNTool
    help(gnntoolClass)
    
    VrtFitClass=CompFactory.Trk.TrkVKalVrtFitter
    help(VrtFitClass)

if "__main__" == __name__:
    main()
