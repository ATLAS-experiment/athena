#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)

def L0MuonRPCSimCfg(flags, name = "L0MuonRPCSim", **kwargs):

    result = ComponentAccumulator()

    alg = CompFactory.L0Muon.L0MuonRPCSim(name = name,
                                          **kwargs)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.HistPath = 'L0MuonRPCSim'
    monTool.defineHistogram('track_input_eta', path='EXPERT', type='TH1F', title=';#eta_{#mu}^{truth};Muons', xbins=50, xmin=-3, xmax=3)

    alg.MonTool = monTool

    histSvc = CompFactory.THistSvc(Output=["EXPERT DATAFILE='" + name + ".root' OPT='RECREATE'"])

    result.addEventAlgo(alg)
    result.addService(histSvc)
    return result
  

if __name__ == "__main__":
    
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Constants import DEBUG
    flags = initConfigFlags()    
    flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
    flags.Exec.MaxEvents = 20
    flags.Common.MsgSuppression = False
    flags.lock()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)        
 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # example simulation alg
    simAlg = L0MuonRPCSimCfg(flags,
                             name = "L0MuonRPCSim",
                             OutputLevel = DEBUG)
    acc.merge(simAlg)

    # below is validation
    acc.printConfig(withDetails=True, summariseProps=True)

    # run the job
    status = acc.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())

