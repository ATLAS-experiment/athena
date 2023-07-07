# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
#from AnaAlgorithm.DualUseConfig import createAlgorithm
from FlavorTagDiscriminants.FlavorTagNNConfig import GNNToolCfg
##---

#Tool Config
def GNNVertexConstructorToolCfg(flags, name="LMEdevTool", **kwargs):
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.Rec.GNNVertexConstructorTool(**kwargs))
    return acc
    

#Algorithm Config    
def GNNVertexConstructorAlgCfg(flags, name="LMEdevAlg", jetkey="AntiKt4EMPFlowJets",  **kwargs):
    acc = ComponentAccumulator()
    

    gnnTool = acc.getPrimaryAndMerge(
            GNNToolCfg(
                flags,
                NNFile="../network.onnx",
                trackLinkType="IPARTICLE",  #Either IPARTICLE or  TRACK_PARTICLE
                variableRemapping={"BTagTrackToJetAssociator" : "GhostTrack"},
                )
    ) 
    tool = acc.popToolsAndMerge(GNNVertexConstructorToolCfg(flags)) 
    
    acc.addEventAlgo(CompFactory.Rec.GNNVertexConstructorAlg(name, VtxTool=tool, GNNTool=gnnTool, 
                                                                                  jetContainerKey=jetkey, 
                                                                                  jetDecoReadKey=jetkey, 
                                                                                  **kwargs))
    return acc
##---


def main():
    algClass = CompFactory.Rec.GNNVertexConstructorAlg
    help(algClass)

    toolClass = CompFactory.Rec.GNNVertexConstructorTool
    help(toolClass)
    
    gnntoolClass = CompFactory.FlavorTagDiscriminants.GNNTool
    help(gnntoolClass)
##---

if "__main__" == __name__:
    main()
    
'''# select the good emerging jets
    acc.addEventAlgo(
        CompFactory.Rec.EmergingJetSelectorAlg(
            "EmergingJet_Selector",
            jetContainerInKey=jetkey,
            jetContainerOutKey=f"{jetkey}_sel",
            minPt=50000,
            maxEta=2.50,
        )
    )'''