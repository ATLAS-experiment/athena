#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
import sys 


def PPMSimBSMonitoringConfig(flags):
    '''Function to configure LVL1 PPM Sim algorithm in the monitoring system.'''

    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # use L1Calo's special MonitoringCfgHelper
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.PPMSimBSMonitorAlgorithm,'PPMSimBSMonAlg')

    # get any algorithms
    PPMSimBSMonAlg = helper.alg
    
    # import tools
    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg # Needed for L1TTIDTools (ATR-21865)
    helper.result().merge(DetDescrCnvSvcCfg(flags))

    from TrigT1CaloCondSvc.L1CaloCondConfig import L1CaloCondAlgCfg 
    helper.result().merge(L1CaloCondAlgCfg(flags,Physics=True, Calib1=False, Calib2=False)) 

    from TrigConfxAOD.TrigConfxAODConfig import getxAODConfigSvc
    helper.result().getPrimaryAndMerge(getxAODConfigSvc(flags))

    # add any steering
    groupName = 'PPMSimBSMonitor' # the monitoring group name is also used for the package name
    PPMSimBSMonAlg.PackageName = groupName
    
    # Histogram paths
    histPath_dev = 'Developer/PPMSimBSMonAlg/Simulation'
    histPath_exp = 'Expert/PpmTrex/Simulation'

    # Trigger tower plots: eta-phi granularity
    etabins = [-4.9,-4.475,-4.050,-3.625,-3.2,-3.1,-2.9,
               -2.7,-2.5,-2.4,-2.3,-2.2,-2.1,-2.0,-1.9,
               -1.8,-1.7,-1.6,-1.5,-1.4,-1.3,-1.2,-1.1,
               -1.0,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,
               -0.2,-0.1,0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,
               0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5,1.6,1.7,
               1.8,1.9,2.0,2.1,2.2,2.3,2.4,2.5,2.7,2.9,
               3.1,3.2,3.625,4.050,4.475,4.9]

    phibins = 64
    phimin = 0

    ####################
    #      BCID        #
    ####################

    histPath = histPath_exp+'/BcidSim'
    group_names = {"peakf": "Peakfinder", "satBcid": "SaturatedBCID"}
    mismatch_map= {"NonZeroMatches": "Data/Simulation Non-Zero Matches", "ZeroMatches":  "Data/Simulation Zero Matches", "DataNoSim":"Data but no Simulation", "SimNoData":"Simulation but no Data"}

    # Eta-phi maps
    for names in group_names:
        for elem in mismatch_map:
            helper.defineHistogram('etaTT_2D,phiTT_2D;ppm_2d_etaPhi_tt_'+names+'_'+elem,
                                   fillGroup='group_Mismatch_{0}_{1}'.format(names, elem),
                                   title='PPM '+group_names[names]+' ' + mismatch_map[elem],
                                   type='TH2D', path=histPath,
                                   hanConfig={
                                       "description": 'PPM '+group_names[names]+' ' + mismatch_map[elem]
                                   },
                                   xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins,
                                   opt='kAlwaysCreate')
    


    ####################
    #      LUT         #
    ####################

    histPath = histPath_exp+'/LutSim'
    lut_map =  { "SimEqData": "Data/Simulation Non-zero Matches", "SimNeData": "Data/Simulation Non-zero Mismatches", "SimNoData":  "Simulation but no Data", "DataNoSim": "Data but no Simulation" }
    group_names_lut = {"lutCp": ["CP", histPath], "lutJep": ["JEP", histPath] }
    
    # EM layer
    # MW 20/02/2025: removed kAlwaysCreate for EM which is powered off as of 2025
    histPath = histPath_dev+'/LutSim'
    for names in group_names_lut:
        for elem in lut_map:
            helper.defineHistogram('etaTT_2D,phiTT_2D;ppm_em_2d_etaPhi_tt_'+names+'_'+elem,
                                   fillGroup='groupLUT{0}_EM_{1}'.format(group_names_lut[names][0],elem),
                                   title='PPM LUT-'+group_names_lut[names][0]+' EM ' + lut_map[elem],
                                   type='TH2D', path=histPath,
                                   xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins,
                                   opt='')
    
    # HAD layer 
    histPath = histPath_exp+'/LutSim'
    for names in group_names_lut:
        for elem in lut_map:
            helper.defineHistogram('etaTT_2D,phiTT_2D;ppm_had_2d_etaPhi_tt_'+names+'_'+elem,
                                   fillGroup='groupLUT{0}_HAD_{1}'.format(group_names_lut[names][0],elem),
                                   title='PPM LUT-'+group_names_lut[names][0]+' HAD ' + lut_map[elem],
                                   type='TH2D', path=histPath,
                                   hanConfig={
                                       "description": 'PPM LUT-'+group_names_lut[names][0]+' HAD ' + lut_map[elem]
                                   },
                                   xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins,
                                   opt='kAlwaysCreate')

    ####################
    #      Errors      #
    ####################
  
    # Mismatch errors 
    nCrates          = 8
    nModulesPerCrate = 16

    y = 2 * (nModulesPerCrate + 16 * (nCrates % 2) )
    yErrorLabels = []
    crate_map= [ "cr0cr1", "cr2cr3", "cr4cr5", "cr6cr7"]
    
    for elem in crate_map:
        # kAlwaysCreate only for crates 6 and 7 (others switched off as of 2025)
        yErrorLabels =  BinErrors(y, elem)
        helper.defineHistogram('eventMonitor,y_2D;ppm_2d_LUT_MismatchEvents_'+ elem,
                               fillGroup='group_Error_{}'.format(elem),
                               title='PPM LUT Mismatch Event Numbers;Events with Error/Mismatch;Crate/Module',
                               type='TH2I', path=histPath_exp,
                               hanConfig={
                                   "display" : "SetGridx,SetGridy",
                                   "algorithm" : "Histogram_Empty",
                                   "description" : "PPM LUT Mismatch Event Numbers."
                               },
                               xbins=10,xmin=0, xmax=10,
                               ybins=y, ymin=0, ymax=y,ylabels=yErrorLabels,
                               opt='kAlwaysCreate' if elem == 'cr6cr7' else '')
        
    # Finish up
    acc = helper.result()
    result.merge(acc)
    return result

def BinErrors(y, crs):

    cr0 = crs.split("cr")[1]
    cr1 = crs.split("cr")[2]


    yErrorLabels = []
    for i in range(0, int(y/2)):
        yErrorLabels.append(str(cr0)+'/'+str(i))
    for i in range(0, int(y/2)):
        yErrorLabels.append(str(cr1)+'/'+str(i))
        
    return yErrorLabels
        




if __name__=='__main__':
   
  
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Exec.MaxEvents = 10
    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2022-02'
    flags.Input.Files =  ["/eos/atlas/atlastier0/rucio/data22_13p6TeV/express_express/00423433/data22_13p6TeV.00423433.express_express.recon.ESD.x653/data22_13p6TeV.00423433.express_express.recon.ESD.x653._lb0015._SFO-ALL._0001.2"]

    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1.root'

    
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc = MainServicesCfg(flags)
    acc.merge(PoolReadCfg(flags))
    

    PPMSimBSMonitorCfg = PPMSimBSMonitoringConfig(flags)
    acc.merge(PPMSimBSMonitorCfg)


    acc.printConfig(withDetails=True, summariseProps = True)

    sys.exit(acc.run().isFailure())
