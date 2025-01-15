#
#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory

def NSWMonitoringCfg(inputFlags):

    import math

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

    NSWMonGroup=helper.addGroup(nswMonAlg, "NSWMonGroup", "Muon/MuonRawDataMonitoring/NSW")



    NSWMonGroup.defineHistogram('sector,phi_mu_sideA;s_phi_track_sideA', type='TH2F', title='NSWheel; sector; phi', path='Test', xbins=20, xmin=0, xmax=20, ybins=63, ymin=-math.pi, ymax=math.pi, opt='kAlwaysCreate')
    NSWMonGroup.defineHistogram('sector,phi_mu_sideC;s_phi_track_sideC', type='TH2F', title='NSWheel; sector; phi', path='Test', xbins=20, xmin=0, xmax=20, ybins=63, ymin=-math.pi, ymax=math.pi, opt='kAlwaysCreate')


    NSWMonGroup.defineHistogram('hitcut,lb,phi_mu_sideA;eff_lb_phi_track_sideA', type='TEfficiency', title='Efficiency SideA; lb; phi', path='Efficiency', xbins=600, xmin=-0.5, xmax=3000.5, ybins=32, ymin=-math.pi, ymax=math.pi, opt='kAlwaysCreate') 
    NSWMonGroup.defineHistogram('hitcut,lb,phi_mu_sideC;eff_lb_phi_track_sideC', type='TEfficiency', title='Efficiency SideC; lb; phi', path='Efficiency', xbins=600, xmin=-0.5, xmax=3000.5, ybins=32, ymin=-math.pi, ymax=math.pi, opt='kAlwaysCreate') 

    NSWMonGroup.defineHistogram('hitcut,lb,sector_phi;eff_lb_sector', type='TEfficiency', title='Efficiency; lb; sector', path='Efficiency', xbins=600, xmin=-0.5, xmax=3000.5, ybins=33, ymin=-16, ymax=17, opt='kAlwaysCreate') #bin 0.1

    NSWMonGroup.defineHistogram('hitcut,lb_sideA;eff_lb_sideA', type='TEfficiency', title='Efficiency SideA; lb; efficiency', path='Efficiency', xbins=600, xmin=-0.5, xmax=3000.5, opt='kAlwaysCreate') #bin 0.1
    NSWMonGroup.defineHistogram('hitcut,lb_sideC;eff_lb_sideC', type='TEfficiency', title='Efficiency SideC; lb; efficiency', path='Efficiency', xbins=600, xmin=-0.5, xmax=3000.5, opt='kAlwaysCreate') #bin 0.1
    

    
    NSWMonGroup.defineHistogram('x_trk_sideA,y_trk_sideA;x_y_track_sideA', type='TH2F', title='NSWheel; x_trk; y_trk', path='Test', xbins=100, xmin=-5000, xmax=5000, ybins=100, ymin=-5000, ymax=5000, opt='kAlwaysCreate')
    NSWMonGroup.defineHistogram('x_trk_sideC,y_trk_sideC;x_y_track_sideC', type='TH2F', title='NSWheel; x_trk; y_trk', path='Test', xbins=100, xmin=-5000, xmax=5000, ybins=100, ymin=-5000, ymax=5000, opt='kAlwaysCreate')

    NSWMonGroup.defineHistogram('hitcut,x_trk_sideA,y_trk_sideA;eff_x_y_track_sideA', type='TEfficiency', title='efficiency; x_trk; y_trk', path='Efficiency', xbins=100, xmin=-5000, xmax=5000, ybins=100, ymin=-5000, ymax=5000, opt='kAlwaysCreate')
    NSWMonGroup.defineHistogram('hitcut,x_trk_sideC,y_trk_sideC;eff_x_y_track_sideC', type='TEfficiency', title='efficiency; x_trk; y_trk', path='Efficiency', xbins=100, xmin=-5000, xmax=5000, ybins=100, ymin=-5000, ymax=5000, opt='kAlwaysCreate')


    acc = helper.result()
    result.merge(acc)
    return result

if __name__=='__main__':
    from AthenaCommon.Constants import DEBUG
    
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Input.Files =[
        '/eos/atlas/atlastier0/rucio/data24_13p6TeV/express_express/00481182/data24_13p6TeV.00481182.express_express.recon.ESD.x854/data24_13p6TeV.00481182.express_express.recon.ESD.x854._lb0800._SFO-ALL._0001.1',
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
    #nswMonitorAcc  =  NSWMonitoringConfig(flags)
    nswMonitorAcc.OutputLevel=DEBUG
    cfg.merge(nswMonitorAcc)
    #cfg.printConfig(withDetails=True, summariseProps = True)
    # number of events selected in the ESD
    cfg.run(-1)
