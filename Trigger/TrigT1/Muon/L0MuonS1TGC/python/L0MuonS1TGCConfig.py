#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)

def L0MuonTGCSimCfg(flags, name = "L0Muon.TGCSimulation", **kwargs):

    result = ComponentAccumulator()

    alg = CompFactory.L0Muon.TGCSimulation(name = name,
                                           **kwargs)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('nTgcDigits', path='EXPERT', type='TH1F', title=';n_{Digit}^{TGC};Events', xbins=50, xmin=0, xmax=100)

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
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Exec.MaxEvents = 10
    flags.Common.MsgSuppression = False
    flags.lock()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)        
 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # Geometry and TGC Cabling map (basically called in TriggerJobOpts)
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags))
    from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg
    acc.merge(TGCCablingConfigCfg(flags))
    # RDO to Digit (basically called in TriggerJobOpts)
    from AthenaConfiguration.Enums import Format
    if flags.Input.Format is Format.POOL:
        rdoInputs = [
            ('TgcRdoContainer','TGCRDO')
        ]
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, Load=rdoInputs))

    from MuonConfig.MuonByteStreamCnvTestConfig import TgcRdoToTgcDigitCfg
    acc.merge(TgcRdoToTgcDigitCfg(flags, TgcDigitContainer = "TGC_DIGITS", TgcRdoContainer = 'TGCRDO'))


    # example simulation alg
    simAlg = L0MuonTGCSimCfg(flags,
                             OutputLevel = DEBUG)
    acc.merge(simAlg)

    # below is validation
    acc.printConfig(withDetails=True, summariseProps=True)

    # run the job
    status = acc.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())

