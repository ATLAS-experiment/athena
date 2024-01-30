#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
def L1CaloCTPMonitoringConfig(flags):
    '''Function to configure LVL1 L1CaloCTP algorithm in the monitoring system.'''

    #import math 
    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # any things that need setting up for job e.g.
    #from AtlasGeoModel.AtlasGeoModelConfig import AtlasGeometryCfg
    #result.merge(AtlasGeometryCfg(flags))

    # make the athena monitoring helper
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'L1CaloCTPMonitoringCfg')

    # get any algorithms
    L1CaloCTPMonAlg = helper.addAlgorithm(CompFactory.L1CaloCTPMonitorAlgorithm,'L1CaloCTPMonAlg')

    # add any steering
    groupName = 'L1CaloCTPMonitor' # the monitoring group name is also used for the package name
    L1CaloCTPMonAlg.PackageName = groupName

    mainDir = 'LVL1_Interfaces'
    trigPath = 'CTP/'

    # add monitoring algorithm to group, with group name and main directory 
    myGroup = helper.addGroup(L1CaloCTPMonAlg, groupName , mainDir)

    multiplicity_labels = ['EM1', 'EM2', 'Tau1', 'Tau2', 'Jet1 (3-bit)', 'Jet2 (2-bit)', 'TE (full eta)', 'XE (full eta)', 'XS', 'TE (restr. eta)', 'XE (restr. eta)']

    # add run number plot
    myGroup.defineHistogram('run',title='Run Number;run;Events',
                            path=trigPath,xbins=1000000,xmin=-0.5,xmax=999999.5)

    myGroup.defineHistogram('ctp_1d_L1CaloNeCTPSummary;h_ctp_1d_L1CaloNeCTPSummary', title='L1Calo Ne CTP',
                            type='TH1F', path=trigPath, xlabels=multiplicity_labels, xbins=len(multiplicity_labels),xmin=0,xmax=len(multiplicity_labels))

    myGroup.defineHistogram('ctp_1d_L1CaloEqCTPSummary;h_ctp_1d_L1CaloEqCTPSummary', title='L1Calo Eq CTP',
                            type='TH1F', path=trigPath, xlabels=multiplicity_labels, xbins=len(multiplicity_labels),xmin=0,xmax=len(multiplicity_labels))

    myGroup.defineHistogram('ctp_1d_TIPMatches;h_ctp_1d_TIPMatches', title='TIP Matches',
                            type='TH1F', path=trigPath, xbins=512,xmin=0.0,xmax=512.0)

    myGroup.defineHistogram('ctp_1d_HitNoTIPMismatch;h_ctp_1d_HitNoTIPMismatch', title='Hit No TIP Mismatches',
                            type='TH1F', path=trigPath, xbins=512,xmin=0.0,xmax=512.0)

    myGroup.defineHistogram('ctp_1d_TIPNoHitMismatch;h_ctp_1d_TIPNoHitMismatch', title='TIP No Hit Mismatches',
                            type='TH1F', path=trigPath, xbins=512,xmin=0.0,xmax=512.0)

    
    acc = helper.result()
    result.merge(acc)
    return result


if __name__=='__main__':
    # set debug level for whole job
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO #DEBUG
    log.setLevel(INFO)

    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob

    #inputs = glob.glob('/eos/atlas/atlastier0/rucio/data18_13TeV/physics_Main/00357750/data18_13TeV.00357750.physics_Main.recon.ESD.f1072/data18_13TeV.00357750.physics_Main.recon.ESD.f1072._lb0117._SFO-1._0201.1')
    inputs = glob.glob('/eos/atlas/atlastier0/rucio/data18_13TeV/physics_Main/00354311/data18_13TeV.00354311.physics_Main.recon.ESD.f1129/data18_13TeV.00354311.physics_Main.recon.ESD.f1129._lb0013._SFO-8._0001.1')

    flags = initConfigFlags()
    flags.Input.Files = inputs
    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1.root'

    flags.lock()
    flags.dump() # print all the configs

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    L1CaloCTPMonitorCfg = L1CaloCTPMonitoringConfig(flags)
    cfg.merge(L1CaloCTPMonitorCfg)

    # message level for algorithm
    L1CaloCTPMonitorCfg.getEventAlgo('L1CaloCTPMonAlg').OutputLevel = 2 # 1/2 INFO/DEBUG
    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=10
    cfg.run(nevents)
