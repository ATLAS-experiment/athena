#
#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory

def NSWMonitoringCfg(inputFlags):
    '''Function to configures some algorithms in the monitoring system.'''

    ### STEP 1 ###
    # Define one top-level monitoring algorithm. The new configuration 
    # framework uses a component accumulator.
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg

    result.merge(MuonGeoModelCfg(inputFlags))

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'NSWAthMonitorCfg')
    
    # Adding an algorithm to the helper. 

    nswMonAlg = helper.addAlgorithm(CompFactory.NSWDataMonAlg,'NSWMonAlg')
    nswMonAlg.DoESD = True    

    
    acc = helper.result()
    result.merge(acc)
    return result

if __name__=='__main__':
    from AthenaCommon.Constants import DEBUG
    
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Input.Files =[
        "/eos/atlas/atlastier0/rucio/data23_hi/express_express/00463414/data23_hi.00463414.express_express.recon.ESD.x798/data23_hi.00463414.express_express.recon.ESD.x798._lb0501._SFO-ALL._0001.1",
    ]
    flags.Output.HISTFileName = 'monitor_nsw.root'

    flags.Detector.GeometryMM=True
    flags.DQ.useTrigger=False
    flags.Input.isMC = False
    if not flags.Input.isMC:
        flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2022-10"

    flags.lock()
    flags.dump()
    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
              
    nswMonitorAcc  =  NSWMonitoringCfg(flags)
    nswMonitorAcc.OutputLevel=DEBUG
    cfg.merge(nswMonitorAcc)

    cfg.run(100)
