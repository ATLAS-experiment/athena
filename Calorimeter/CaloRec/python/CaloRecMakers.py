# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

#
#
# File: CaloRecMakers.py
# Created: Jul 2006, sss
# Purpose: Create configurables for CaloRec.
#
# This is a prototype for how Configurable instances can be constructed.
#


from CaloRec.CaloRecConf import CaloClusterCopier, CaloClusterMaker, \
     CaloClusterCorrDBWriter, CaloCell2ClusterMapper, \
     CaloTopoClusterSplitter, CaloTopoClusterMaker 


#
# These should probably be in a common library.
#
def _makeconf (cls, name = None, **kwargs):
    if name is not None:
        x = cls(name)
    else:
        x = cls()
    for (k,v) in kwargs.items():
        setattr (x, k, v)
    return x
def _process_tool (alg, tool):
    if not isinstance(tool, str):
        alg += tool
        tool = tool.getFullName()
    return tool
def _process_tools (alg, tools):
    return [_process_tool(alg, t) for t in tools]



def make_CaloClusterCopier (name, output_name, copy_cells = False, use_ClusterPosition=False, etCut=-1.):
    return _makeconf (CaloClusterCopier,
                      name,
                      ClustersName = output_name,
                      CopyCells = copy_cells,
                      UseClusterPosition=use_ClusterPosition,
                      etCut =etCut)


def make_CaloClusterMaker (name,
                           output_key,
                           maker_tools = [],
                           correction_tools = []):
    alg = _makeconf (CaloClusterMaker,
                     name,
                     ClustersOutputName = output_key)
    alg.ClusterMakerTools = _process_tools (alg, maker_tools)
    alg.ClusterCorrectionTools = _process_tools (alg, correction_tools)
    return alg


def make_CaloClusterCorrDBWriter (name,
                                  key,
                                  correction_tools = []):
    alg = _makeconf (CaloClusterCorrDBWriter,
                     name,
                     key = key)
    alg.ClusterCorrectionTools = _process_tools (alg, correction_tools)
    return alg



def make_CaloCell2ClusterMapper (name,
                                 input_key,
                                 output_key):
    return _makeconf (CaloCell2ClusterMapper,
                      name,
                      ClustersName = input_key,
                      MapOutputName = output_key)


def make_CaloTopoClusterMaker (name, **kw):
    return _makeconf (CaloTopoClusterMaker, name, **kw)

def make_CaloTopoClusterSplitter (name, **kw):
    return _makeconf (CaloTopoClusterSplitter, name, **kw)



