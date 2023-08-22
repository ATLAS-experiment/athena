# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from FlavorTagDiscriminants.FlavorTagNNConfig import GNNToolCfg
from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
##---

#Tool Config
def GNNVertexConstructorToolCfg(flags, name="LMEdevTool", **kwargs):
    acc = ComponentAccumulator()
    acc.merge(BeamSpotCondAlgCfg(flags))
    gnnTool = acc.getPrimaryAndMerge(
            GNNToolCfg(
                flags,
                NNFile="../network.onnx",
                trackLinkType="IPARTICLE",  #Either IPARTICLE or  TRACK_PARTICLE
                variableRemapping={"BTagTrackToJetAssociator" : "GhostTrack"},
                )
    ) 
    
    kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    kwargs.setdefault("ExtrapolatorName", acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    
    acc.setPrivateTools(CompFactory.Rec.GNNVertexConstructorTool(GNNTool=gnnTool, **kwargs))
    return acc
    

#Algorithm Config    
def GNNVertexConstructorAlgCfg(flags, name="LMEdevAlg", jetkey="AntiKt4EMPFlowJets",  **kwargs):
    acc = ComponentAccumulator()
    

    tool = acc.popToolsAndMerge(GNNVertexConstructorToolCfg(flags)) 
    
    acc.addEventAlgo(CompFactory.Rec.GNNVertexConstructorAlg(name, VtxTool=tool, jetDecoReadKey=jetkey, **kwargs))
    return acc
##---


def main():
    algClass = CompFactory.Rec.GNNVertexConstructorAlg
    help(algClass)

    toolClass = CompFactory.Rec.GNNVertexConstructorTool
    help(toolClass)
    
    gnntoolClass = CompFactory.FlavorTagDiscriminants.GNNTool
    help(gnntoolClass)
    
    VrtFitClass=CompFactory.Trk.TrkVKalVrtFitter
    help(VrtFitClass)
##---

if "__main__" == __name__:
    main()
    
