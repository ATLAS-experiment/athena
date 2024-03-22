# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from FlavorTagDiscriminants.FlavorTagNNConfig import GNNToolCfg
from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg

def GNNVertexConstructorToolCfg(flags, name="GNNVertexFitterTool", outfile="HIST.pool.root", **kwargs):
    acc = ComponentAccumulator()
    acc.merge(BeamSpotCondAlgCfg(flags))
    gnnTool = acc.getPrimaryAndMerge(
            GNNToolCfg(
                flags,
                NNFile           ="BTagging/20231205/GN2v01/antikt4empflow/network_fold0.onnx",
                trackLinkType    ="IPARTICLE",  #Either IPARTICLE or  TRACK_PARTICLE
                variableRemapping={"BTagTrackToJetAssociator" : "GhostTrack"},
                )
 
    ) 
    acc.addService(CompFactory.THistSvc(Output=[f"GNNPlots DATAFILE='{outfile}', OPT='RECREATE'"])
    )
  
    kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    acc.setPrivateTools(CompFactory.Rec.GNNVertexConstructorTool(GNNTool=gnnTool, **kwargs))
    return acc
    

def GNNVertexConstructorAlgCfg(flags, jet_col="AntiKt4EMPFlowJets",  **kwargs):
    acc = ComponentAccumulator()
    
    tool = acc.popToolsAndMerge(GNNVertexConstructorToolCfg(flags)) 
    acc.addEventAlgo(CompFactory.Rec.GNNVertexConstructorAlg(VtxTool=tool, inputJetContainer=jet_col, **kwargs))
        
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
