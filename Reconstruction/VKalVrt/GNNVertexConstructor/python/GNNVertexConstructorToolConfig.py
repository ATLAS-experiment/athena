# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from FlavorTagDiscriminants.FlavorTagNNConfig import GNNToolCfg
##---

#Tool Config
def GNNVertexConstructorToolCfg(flags, name="LMEdevTool", **kwargs):
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.Rec.GNNVertexConstructorTool(**kwargs))
    return acc
    
    
'''
#GNN Config - taken from the GNN Tool file in Flavour Tag Discrimants
def GNNToolCfg(ConfigFlags, NNFile, **options):
    acc = ComponentAccumulator()

    # this map lets us change the names of EDM inputs with respect to
    # the values we store in the saved NN
    remap = {}

    #Was 20221010 I have changed to suit the model I will be using
    #Assume date of model creation
    if '20230608' in NNFile and 'gn2' in NNFile:
        for aggragate in ['InnermostPixelLayer', 'NextToInnermostPixelLayer',
                          'InnermostPixelLayerShared',
                          'InnermostPixelLayerSplit']:
            remap[f'numberOf{aggragate}Hits'] = (
                f'numberOf{aggragate}Hits21p9')

    mkey = 'variableRemapping'
    options[mkey] = remap | options.get(mkey,{})

    gnntool = CompFactory.FlavorTagDiscriminants.GNNTool(
        name='decorator',
        nnFile=NNFile,
        **options)

    acc.setPrivateTools(CompFactory.Rec.gnntool)

    return acc
'''
#Algorithm Config    
def GNNVertexConstructorAlgCfg(flags, name="LMEdevAlg", **kwargs):
    acc = ComponentAccumulator()
    
    tool = acc.popToolsAndMerge(GNNVertexConstructorToolCfg(flags)) #fix name
    #need imprt and call function name
    gnnTool = acc.popToolsAndMerge(GNNToolCfg(flags, 
                                              NNFile=NNFile))
        
    acc.addEventAlgo(CompFactory.Rec.GNNVertexConstructorAlg(name, TestTool=tool, GNNTool=gnnTool, **kwargs))
    
    
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
    