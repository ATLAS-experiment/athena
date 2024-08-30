#!/usr/bin/env athena.py

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ResortVerticesCfg(flags,vxin,vxout,vxsortercfg,algname="ResortVx"):
    cfg = ComponentAccumulator()
    vxsorter = cfg.popToolsAndMerge(vxsortercfg)
    resortvxalg = CompFactory.InDet.InDetPriVxResorter(
        algname,
        VerticesIn=vxin,
        VerticesOut=vxout,
        VertexCollectionSortingTool=vxsorter,
    )
    cfg.addEventAlgo(resortvxalg)
    return cfg

# Executable function, for testing
def main():
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.fillFromArgs()
    flags.Exec.VerboseMessageComponents = ['ResortVx']
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    from TrkConfig.TrkVertexToolsConfig import JetRestrictedSumPt2VertexCollectionSortingToolCfg
    vxsort_jettrks = JetRestrictedSumPt2VertexCollectionSortingToolCfg(flags)
    cfg.merge(ResortVerticesCfg(flags,'PrimaryVertices','PrimaryVertices_resorted',vxsort_jettrks))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(
        OutputStreamCfg(
            flags,
            streamName='xAOD',
            ItemList=[
                'xAOD::VertexContainer#PrimaryVertices',
                'xAOD::VertexAuxContainer#PrimaryVerticesAux.',
                'xAOD::VertexContainer#PrimaryVertices_resorted',
                'xAOD::VertexAuxContainer#PrimaryVertices_resortedAux.',
            ])
    )
    
    cfg.run()

if __name__=='__main__':
    main()
