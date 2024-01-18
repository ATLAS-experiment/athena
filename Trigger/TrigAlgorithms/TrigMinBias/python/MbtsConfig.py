# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from TrigMinBias.TrigMinBiasMonitoring import MbtsFexMonitoring
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def MbtsFexCfg(flags, name="MbtsFex", MbtsBitsKey=None):
    """Configures MBTS Fex with monitoring"""
    acc = ComponentAccumulator()
    from TrigT2CaloCommon.TrigCaloDataAccessConfig import CaloDataAccessSvcDependencies, trigCaloDataAccessSvcCfg


    alg = CompFactory.MbtsFex(name,
        MbtsBitsKey=MbtsBitsKey,
        MonTool = MbtsFexMonitoring(flags), 
        ExtraInputs = CaloDataAccessSvcDependencies)
    acc.addEventAlgo(alg, primary=True)
    acc.merge(trigCaloDataAccessSvcCfg(flags))
    return acc
    
def MbtsSGInputCfg(flags):
    """Configures SG Input needed for MBTS Fex"""
    from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
    return SGInputLoaderCfg(flags, [('TileTBID','DetectorStore+TileTBID' )])


if __name__ == '__main__':
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior=1
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    flags.Input.Files=defaultTestFiles.RAW_RUN2 # or ESD or AOD or ...
    flags.lock()

    acc=ComponentAccumulator()

    acc.merge(MbtsFexCfg(flags, MbtsBitsKey="some"))
    acc.merge(MbtsSGInputCfg(flags))

    acc.printConfig(withDetails=True, summariseProps=True)

    acc.wasMerged()
