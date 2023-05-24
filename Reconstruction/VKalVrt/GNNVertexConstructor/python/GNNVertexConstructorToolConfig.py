# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

##---

##NEED???
def GNNVertexConstructorToolCfg(flags, name="LMEdevTool", **kwargs):
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.Rec.GNNVertexConstructorTool(**kwargs))
    return acc
    
    
    
def GNNVertexConstructorAlgCfg(flags, name="LMEdevAlg", **kwargs):
    acc = ComponentAccumulator()
    
    #tool = acc.popToolsAndMerge(GNNSecVtxToolCfg(flags))
        
    acc.addEventAlgo(CompFactory.Rec.GNNVertexConstructorAlg(name, TestTool=acc.popToolsAndMerge(GNNVertexConstructorToolCfg(flags)), **kwargs))
    return acc



##---


def main():
    algClass = CompFactory.Rec.GNNVertexConstructorAlg
    help(algClass)

    toolClass = CompFactory.Rec.GNNVertexConstructorTool
    help(toolClass)
##---

if "__main__" == __name__:
    main()
    