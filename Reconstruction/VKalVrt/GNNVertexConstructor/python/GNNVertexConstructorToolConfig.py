# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AnaAlgorithm.DualUseConfig import createAlgorithm
from FlavorTagDiscriminants.FlavorTagNNConfig import GNNToolCfg
from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg

def GNNVertexConstructorToolCfg(flags, name="LMEdevTool", **kwargs):
    acc = ComponentAccumulator()
    acc.merge(BeamSpotCondAlgCfg(flags))
    gnnTool = acc.getPrimaryAndMerge(
            GNNToolCfg(
                flags,
                NNFile           ="GNNVertexConstructor/network.onnx",
                trackLinkType    ="IPARTICLE",  #Either IPARTICLE or  TRACK_PARTICLE
                variableRemapping={"BTagTrackToJetAssociator" : "GhostTrack"},
                )
    ) 
    
    kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    
    acc.setPrivateTools(CompFactory.Rec.GNNVertexConstructorTool(GNNTool=gnnTool, **kwargs))
    return acc
    

def GNNVertexConstructorAlgCfg(flags, name="LMEdevAlg", jetkey="AntiKt4EMPFlowJets",  **kwargs):
    acc = ComponentAccumulator()
    
    tool = acc.popToolsAndMerge(GNNVertexConstructorToolCfg(flags)) 
    acc.addEventAlgo(CompFactory.Rec.GNNVertexConstructorAlg(name, VtxTool=tool, inputJetContainer=jetkey, **kwargs))
    return acc

def main():
    algClass = CompFactory.Rec.GNNVertexConstructorAlg
    help(algClass)

    toolClass = CompFactory.Rec.GNNVertexConstructorTool
    help(toolClass)
    
    gnntoolClass = CompFactory.FlavorTagDiscriminants.GNNTool
    help(gnntoolClass)
    
    VrtFitClass=CompFactory.Trk.TrkVKalVrtFitter
    help(VrtFitClass)

if "__main__" == __name__:
    main()
