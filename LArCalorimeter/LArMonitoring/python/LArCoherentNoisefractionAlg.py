#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''@file LArCoherentNoisefractionAlg
@author P. Strizenec
@date 22-05-2021
@brief Adapted from LArNoiseCorrelationMonAlg by M. Spalla 
'''

def LArCoherentNoisefractionConfig(inputFlags, groupsToMonitor=[]):
    
    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArCoherentNoisefractionMonAlgCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    return LArCoherentNoisefractionConfigCore(helper, CompFactory.LArCoherentNoisefractionMonAlg,inputFlags,groupsToMonitor)

def LArCoherentNoisefractionConfigCore(helper, algoinstance, inputFlags, groupsToMonitor):
    from LArMonitoring.GlobalVariables import lArDQGlobals

    larCoherentNoisefractionMonAlg = helper.addAlgorithm(algoinstance,'larCoherentNoisefractionMonAlg')

    #set all groups to monitor, if the empty list is passed
    allGroups=["tot","top","bot","left","right","q1","q2","q3","q4","qs11","qs22","qs3","qs4"]
    groupsNChan=[128,64,64,64,64,32,32,32,32,32,32,32,32]
    allMonitor=[True,True,True,True,True,True,True,True,True,True,True,True,True]

    #from AthenaCommon.Constants import DEBUG
    #larCoherentNoisefractionMonAlg.OutputLevel = DEBUG
    try:
       larCoherentNoisefractionMonAlg.IsCalibrationRun = inputFlags.LArMon.calibRun
    except AttributeError:
       larCoherentNoisefractionMonAlg.IsCalibrationRun = False
    try:   
       larCoherentNoisefractionMonAlg.LArDigitContainerKey = inputFlags.LArMon.LArDigitKey
    except AttributeError:
       larCoherentNoisefractionMonAlg.LArDigitContainerKey = 'FREE'
    larCoherentNoisefractionMonAlg.ListOfGroupNames = allGroups
    larCoherentNoisefractionMonAlg.GroupNchan = groupsNChan
    if len(groupsToMonitor) == 0:
       customGroupstoMonitor = allMonitor
    else:
       if len(customGroupstoMonitor) != len(allMonitor):
             from AthenaCommon.Logging import logging
             logging.getLogger().warning("Wrong list of groups to monitor, setting all !")
             customGroupstoMonitor = allMonitor
       else:      
             customGroupstoMonitor = groupsToMonitor   
    larCoherentNoisefractionMonAlg.GroupsToMonitor = customGroupstoMonitor   

    try:
       customFEBStoMonitor = inputFlags.LArMon.customFEBsToMonitor
    except AttributeError:
       customFEBStoMonitor = ["endcapAft19slot12","endcapAft19slot09","endcapAft20slot09"] 

    #correct custom FEBs for upper-lower cases or single-digit ft and slot numbers (e.g. 3 instead of 03)
    from ROOT import LArStrHelper
    larStrHelp=LArStrHelper()
    customFEBStoMonitor=[larStrHelp.fixFEBname(nm) for nm in customFEBStoMonitor]


    # adding BadChan masker private tool
    from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg


    larCoherentNoisefractionMonAlg.TriggerChain = "HLT_noalg_zb_L1ZB, HLT_noalg_cosmiccalo_L1RD1_EMPTY" #turn off for calibration run 


    setCustomFEBS=set(customFEBStoMonitor)
    febsToMonitorBarrelA=list(setCustomFEBS.intersection(lArDQGlobals.febsBarrelA))
    febsToMonitorEndcapA=list(setCustomFEBS.intersection(lArDQGlobals.febsEndcapA))
    febsToMonitorBarrelC=list(setCustomFEBS.intersection(lArDQGlobals.febsBarrelC))
    febsToMonitorEndcapC=list(setCustomFEBS.intersection(lArDQGlobals.febsEndcapC))

    if len(febsToMonitorBarrelA)==0 and len(febsToMonitorEndcapA)==0 and len(febsToMonitorBarrelC)==0 and len(febsToMonitorEndcapC)==0:
        print("LArCoherentNoisefractionMonAlg:WARNING. None of the following FEBs were recognised, no plot will be produced")
        print(customFEBStoMonitor)
        larCoherentNoisefractionMonAlg.PlotsOFF=True #lets protect ourselves against poor writing
        larCoherentNoisefractionMonAlg.PlotCustomFEBSset=False
        larCoherentNoisefractionMonAlg.FEBlist=[]
    else:
        #pass to algorithm
        larCoherentNoisefractionMonAlg.PlotCustomFEBSset=True
        larCoherentNoisefractionMonAlg.FEBlist=febsToMonitorBarrelA+febsToMonitorBarrelC+febsToMonitorEndcapA+febsToMonitorEndcapC
        pass

    #prepare the monitoring groups
    for grp in range(0,len(allGroups)):
       if not customGroupstoMonitor[grp]: continue   


       cnfArray = helper.addArray([larCoherentNoisefractionMonAlg.FEBlist],larCoherentNoisefractionMonAlg,allGroups[grp]) 

       hist_path='/LAr/CNF/'
    
       tot_plot_name="cnf_tot"
       tot_var_and_name="SumDev;"+tot_plot_name
       cnfArray.defineHistogram(tot_var_and_name,
                                title=tot_plot_name,
                                type='TH1F',
                                path=hist_path+'BarrelA',
                                xbins=lArDQGlobals.CNFN_tot, xmin=lArDQGlobals.CNFXmin_tot, xmax=lArDQGlobals.CNFXmax_tot,
                                pattern=febsToMonitorBarrelA)

       cnfArray.defineHistogram(tot_var_and_name,
                                title=tot_plot_name,
                                type='TH1F',
                                path=hist_path+'BarrelC',
                                xbins=lArDQGlobals.CNFN_tot, xmin=lArDQGlobals.CNFXmin_tot, xmax=lArDQGlobals.CNFXmax_tot,
                                pattern=febsToMonitorBarrelC)

       print(lArDQGlobals.CNFN_tot)
       cnfArray.defineHistogram(tot_var_and_name,
                                title=tot_plot_name,
                                type='TH1F',
                                path=hist_path+'EndcapA',
                                xbins=lArDQGlobals.CNFN_tot, xmin=lArDQGlobals.CNFXmin_tot, xmax=lArDQGlobals.CNFXmax_tot,
                                pattern=febsToMonitorEndcapA)
                                

       cnfArray.defineHistogram(tot_var_and_name,
                                title=tot_plot_name,
                                type='TH1F',
                                path=hist_path+'EndcapC',
                                xbins=lArDQGlobals.CNFN_tot, xmin=lArDQGlobals.CNFXmin_tot, xmax=lArDQGlobals.CNFXmax_tot,
                                pattern=febsToMonitorEndcapC)


       noncoh_plot_name="cnf_noncoh"
       noncoh_var_and_name="Dev;"+noncoh_plot_name

       cnfArray.defineHistogram(noncoh_var_and_name,
                                title=noncoh_plot_name,
                                type='TH1F',
                                path=hist_path+'BarrelA',
                                xbins=lArDQGlobals.CNFN_ncoh, xmin=lArDQGlobals.CNFXmin_ncoh, xmax=lArDQGlobals.CNFXmax_ncoh,
                                pattern=febsToMonitorBarrelA)

       cnfArray.defineHistogram(noncoh_var_and_name,
                                title=noncoh_plot_name,
                                type='TH1F',
                                path=hist_path+'BarrelC',
                                xbins=lArDQGlobals.CNFN_ncoh, xmin=lArDQGlobals.CNFXmin_ncoh, xmax=lArDQGlobals.CNFXmax_ncoh,
                                pattern=febsToMonitorBarrelC)

       cnfArray.defineHistogram(noncoh_var_and_name,
                                title=noncoh_plot_name,
                                type='TH1F',
                                path=hist_path+'EndcapA',
                                xbins=lArDQGlobals.CNFN_ncoh, xmin=lArDQGlobals.CNFXmin_ncoh, xmax=lArDQGlobals.CNFXmax_ncoh,
                                pattern=febsToMonitorEndcapA)

       cnfArray.defineHistogram(noncoh_var_and_name,
                                title=noncoh_plot_name,
                                type='TH1F',
                                path=hist_path+'EndcapC',
                                xbins=lArDQGlobals.CNFN_ncoh, xmin=lArDQGlobals.CNFXmin_ncoh, xmax=lArDQGlobals.CNFXmax_ncoh,
                                pattern=febsToMonitorEndcapC)


    print(cnfArray.toolList())

    if isComponentAccumulatorCfg():
        cfg.merge(helper.result())
        return cfg
    else:    
        return helper.result()
    

if __name__=='__main__':

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import DEBUG
   log.setLevel(DEBUG)

   flags = initConfigFlags()
   from LArMonitoring.LArMonConfigFlags import addLArMonFlags
   flags.addFlagsCategory("LArMon", addLArMonFlags)
   from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
   addLArCalibFlags(flags)

   #from AthenaConfiguration.TestDefaults import defaultTestFiles

   flags.Input.Files = ['/eos/atlas/atlastier0/rucio/data21_calib/calibration_LArElec-Pedestal-5s-High-Emec-A-RawData/00393063/data21_calib.00393063.calibration_LArElec-Pedestal-5s-High-Emec-A-RawData.daq.RAW/data21_calib.00393063.calibration_LArElec-Pedestal-5s-High-Emec-A-RawData.daq.RAW._lb0000._SFO-1._0001.data','/eos/atlas/atlastier0/rucio/data21_calib/calibration_LArElec-Pedestal-5s-High-Emec-A-RawData/00393063/data21_calib.00393063.calibration_LArElec-Pedestal-5s-High-Emec-A-RawData.daq.RAW/data21_calib.00393063.calibration_LArElec-Pedestal-5s-High-Emec-A-RawData.daq.RAW._lb0000._SFO-2._0001.data','/eos/atlas/atlastier0/rucio/data21_calib/calibration_LArElec-Pedestal-5s-High-Emec-A-RawData/00393063/data21_calib.00393063.calibration_LArElec-Pedestal-5s-High-Emec-A-RawData.daq.RAW/data21_calib.00393063.calibration_LArElec-Pedestal-5s-High-Emec-A-RawData.daq.RAW._lb0000._SFO-3._0001.data','/eos/atlas/atlastier0/rucio/data21_calib/calibration_LArElec-Pedestal-5s-High-Emec-A-RawData/00393063/data21_calib.00393063.calibration_LArElec-Pedestal-5s-High-Emec-A-RawData.daq.RAW/data21_calib.00393063.calibration_LArElec-Pedestal-5s-High-Emec-A-RawData.daq.RAW._lb0000._SFO-4._0001.data']

   flags.LArMon.calibRun = True
   flags.Output.HISTFileName = 'LArCNFMonOutput.root'
   flags.DQ.enableLumiAccess = False
   flags.DQ.useTrigger = False

   from AthenaConfiguration.Enums import BeamType
   flags.Beam.Type = BeamType.Collisions
   flags.lock()

   from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
   cfg=ComponentAccumulator()
   from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
   cfg.merge(LArRawDataReadingCfg(flags,LArDigitKey="HIGH",LArRawChannelKey=""))
   # for calib digits:
   #from LArByteStream.LArRawCalibDataReadingConfig import LArRawCalibDataReadingCfg
   #cfg.merge(LArRawCalibDataReadingCfg(flags,gain="HIGH",doCalibDigit=True))
   from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
   cfg.merge(LArOnOffIdMappingCfg(flags))
   from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
   cfg.merge(LArElecCalibDBCfg(flags,["Pedestal"]))

   feblist=[]
   for ft in [11,12,23,24]:
      for slot in range(1,15):
         if slot < 10:
             feblist += ['EndcapAft'+str(ft)+'slot0'+str(slot)]
         else:
             feblist += ['EndcapAft'+str(ft)+'slot'+str(slot)]
   aff_acc = LArCoherentNoisefractionConfig(flags,feblist)

   cfg.merge(aff_acc)

   cfg.printConfig()
   log.setLevel(DEBUG)
   flags.dump()
   f=open("LArCNFMon.pkl","wb")
   cfg.store(f)
   f.close()

   #cfg.run(100)
