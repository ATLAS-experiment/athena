# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def DiTauOutputCfg(flags):

   from OutputStreamAthenaPool.OutputStreamConfig import addToESD,addToAOD
   result=ComponentAccumulator()

   DiTauOutputList  = [ "xAOD::DiTauJetContainer#DiTauJets" ]
   DiTauOutputList += [ "xAOD::DiTauJetAuxContainer#DiTauJetsAux." ]

   result.merge(addToESD(flags,DiTauOutputList))
   result.merge(addToAOD(flags,DiTauOutputList))
   return result

def DiTauReconstructionCfg(flags):

    result = ComponentAccumulator()

    from DiTauRec.DiTauBuilderConfig import DiTauBuilderCfg
    result.merge(DiTauBuilderCfg(flags))

    if (flags.Output.doWriteESD or flags.Output.doWriteAOD):
        result.merge(DiTauOutputCfg(flags))

    return result


if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc21_13p6TeV/ESDFiles/mc21_13p6TeV.421450.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep_fct.recon.ESD.e8445_e8447_s3822_r13565/ESD.28877240._000046.pool.root.1"]

    # Use latest MC21 tag to pick up latest muon folders apparently needed
    flags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN3-10"
    flags.Output.ESDFileName = "ESD.pool.root"
    flags.Output.AODFileName = "AOD.pool.root"

    nThreads=1
    flags.Concurrency.NumThreads = nThreads
    if nThreads>0:
        flags.Scheduler.ShowDataDeps = True
        flags.Scheduler.ShowDataFlow = True
        flags.Scheduler.ShowControlFlow = True
        flags.Concurrency.NumConcurrentEvents = nThreads

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg=MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    # this delcares to the scheduler that EventInfo object comes from the input
    loadFromSG = [('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                  ( 'AthenaAttributeList' , 'StoreGateSvc+Input' ),
                  ( 'CaloCellContainer' , 'StoreGateSvc+AllCalo' )]
    cfg.addEventAlgo(CompFactory.SGInputLoader(Load=loadFromSG), sequenceName="AthAlgSeq")

    cfg.merge(DiTauReconstructionCfg(flags))

    cfg.run(1000)
