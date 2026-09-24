# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def OutputConditionsAlgCfg(flags, name="OutputConditionsAlg",outputFile='condobjs.root', **kwargs):

    result = ComponentAccumulator(sequence = CompFactory.AthSequencer("AthOutSeq", StopOverride=True))

    from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
    result.merge(PoolWriteCfg(flags))

    kwargs.setdefault("WriteIOV",True)
       
    # create outputStream tool with given filename and pass to myOCA
    condstream=CompFactory.AthenaOutputStreamTool(name+"Tool",
                                                  OutputFile=outputFile,
                                                  PoolContainerPrefix="ConditionsContainer",
                                                  TopLevelContainerName="<type>",
                                                  SubLevelBranchName="<key>"
                                                  )
    kwargs.setdefault("StreamName",condstream)
    oca=CompFactory.OutputConditionsAlg(name,**kwargs)
    result.addEventAlgo(oca)
    
    return result
