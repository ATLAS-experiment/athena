#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def TrigTauMonConfig(inputFlags):
    '''Function to configures some algorithms in the monitoring system.'''

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'TrigTauAthMonitorCfg')

    from TrigTauMonitoring.TrigTauMonitoringConfig import TrigTauMonAlgBuilder
    monAlgCfg = TrigTauMonAlgBuilder( helper ) 
    # build monitor and book histograms
    monAlgCfg.configure()

    ### STEP 6 ###
    # Finalize. The return value should be a tuple of the ComponentAccumulator
    # and the sequence containing the created algorithms. If we haven't called
    # any configuration other than the AthMonitorCfgHelper here, then we can 
    # just return directly (and not create "result" above)
    return helper.result()

if __name__=='__main__':
    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    nightly = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/'
    file = 'data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1'
    flags = initConfigFlags()
    flags.Input.Files = [nightly+file]
    flags.Input.isMC = False
    flags.Output.HISTFileName = 'TrigTauMonitorOutput.root'
    
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    trigTauMonitorAcc = TrigTauMonConfig(flags)
    cfg.merge(trigTauMonitorAcc)

    # If you want to turn on more detailed messages ...
    #trigJetMonitorAcc.getEventAlgo('TrigTauMonAlg').OutputLevel = 2 # DEBUG
    cfg.printConfig(withDetails=True) # set True for exhaustive info

    sc = cfg.run() #use cfg.run(20) to only run on first 20 events
    if not sc.isSuccess():
       import sys
       sys.exit("Execution failed")


