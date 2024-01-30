#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def LArCalibPedMonConfig(flags,gain="",doAccDigit=False,doCalibDigit=False,doAccCalibDigit=False):

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArCalibPedMonCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    LArCalibPedMonConfigCore(helper,CompFactory.LArCalibPedMonAlg,flags,gain,doAccDigit,doCalibDigit,doAccCalibDigit)
 
    rv = ComponentAccumulator()

    # adding LAr*Mapping algos
    from LArCabling.LArCablingConfig import LArFebRodMappingCfg, LArCalibIdMappingCfg
    rv.merge(LArFebRodMappingCfg(flags))
    rv.merge(LArCalibIdMappingCfg(flags))

    # adding LArFebErrorSummary algo
    from LArROD.LArFebErrorSummaryMakerConfig import LArFebErrorSummaryMakerCfg
    rv.merge(LArFebErrorSummaryMakerCfg(flags))
    
    rv.merge(helper.result())

    return rv

def LArCalibPedMonConfigCore(helper,algoinstance,flags,gain="",doAccDigit=False,doCalibDigit=False,doAccCalibDigit=False):

    from LArMonitoring.GlobalVariables import lArDQGlobals
    
    larPedMonAlgTest = helper.addAlgorithm(algoinstance,'larCalibPedMonAlgTest')
    if gain != "":
       if doAccDigit:
          larPedMonAlgTest.LArAccumulatedDigitContainerKey=gain
       elif doAccCalibDigit:
          larPedMonAlgTest.LArAccumulatedCalibDigitContainerKey=gain
       elif doCalibDigit:
          larPedMonAlgTest.LArCalibDigitContainerKey=gain
          
# this creates a "Group" called "PedMonGroup" which will put its histograms into the subdirectory "'/LAr/'+GroupName+'/'"
    GroupName="PedMonGroup"
       
    larPedMonAlgTest.LArPedGroupName=GroupName
    larPedMonAlgTest.PartitionNames=lArDQGlobals.Partitions
    larPedMonAlgTest.SubDetNames=lArDQGlobals.SubDet

    Group = helper.addGroup(larPedMonAlgTest,GroupName,'/LAr/'+GroupName+'/')


    #Summary histos
    summary_hist_path='Summary/'
    
# this defines a 1I histogram named "NbOfReadoutChannelsGlobal" monitoring "nbChan".
# the title will be "# of readout channels"; the x axis label is "Total channels for all FEB"; and the y axis label is "Events",
# which will be put into the subdirectory "summary_hist_path"
    Group.defineHistogram('nbChan;NbOfReadoutChannelsGlobal', 
                                  title='# of readout channels;Total channels for all FEB;Events',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.N_FEB*lArDQGlobals.FEB_N_channels+5, 
                                  xmin=-0.5, xmax=lArDQGlobals.N_FEB*lArDQGlobals.FEB_N_channels+4.5)
    Group.defineHistogram('nbFEB;NbOfReadoutFEBGlobal', 
                                  title='# of readout FEB/DSP header;N_FEB;Events',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.N_FEB+11, xmin=-0.5, xmax=lArDQGlobals.N_FEB+10+0.5)
    Group.defineHistogram('nbFEBpart,part;NbOfEvts2d', 
                                  title='# of readout FEB/DSP header;N_FEB;Partition',
                                  type='TH2I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.N_FEB_Parttions_Max, xmin=-0.5, xmax=lArDQGlobals.N_FEB_Parttions_Max-0.5,
                                  ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                                  ylabels=lArDQGlobals.Partitions)
    Group.defineHistogram('febError,part;NbOfFEBMonErrors_dE', 
                                  title='# of data corruption errors',
                                  type='TH2I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.N_FEBErrors, xmin=0.5, xmax=lArDQGlobals.N_FEBErrors+0.5,
                                  ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                                  xlabels=lArDQGlobals.FEBErrors, ylabels=lArDQGlobals.Partitions)
    Group.defineHistogram('LB0,EvtRejYield;RAW_YieldOfRejectedEventsVsLB', 
                                  title='Yield of corrupted events (DATACORRUPTED);LBs;Yield(%)',
                                  type='TProfile',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)
    Group.defineHistogram('LB0;NbOfEventsVsLB', 
                                  title='Nb of events per LB;LBs;Events',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)
    Group.defineHistogram('LB0,LArEvSize;LAreventSizeVsLB', 
                                  title='LAr event size (w/o ROS headers);LBs;Megabytes',
                                  type='TProfile',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)
    # Now per partition histograms
    for subdet in range(0,lArDQGlobals.N_SubDet):
       hist_path='/LAr/'+GroupName+'/'+lArDQGlobals.SubDet[subdet]+'/'
       slot_low = lArDQGlobals.FEB_Slot[lArDQGlobals.Partitions[subdet*2]][0] - 0.5
       slot_up  = lArDQGlobals.FEB_Slot[lArDQGlobals.Partitions[subdet*2]][1] + 0.5
       slot_n = int(slot_up - slot_low)
       ft_low = lArDQGlobals.FEB_Feedthrough[lArDQGlobals.Partitions[subdet*2]][0] - 0.5
       ft_up  = lArDQGlobals.FEB_Feedthrough[lArDQGlobals.Partitions[subdet*2]][1] + 0.5
       ft_n = int(ft_up - ft_low)
       
       darray = helper.addArray([lArDQGlobals.Partitions[2*subdet:2*subdet+2]],larPedMonAlgTest,lArDQGlobals.SubDet[subdet])
       
       darray.defineHistogram('nbFEBpart;nbOfFebBlocks',
                              title='# of readout FEBs (DSP header check only);N_FEB;Events',
                              type='TH1I',
                              path=hist_path,
                              xbins=lArDQGlobals.N_FEB_Parttions_Max, xmin=-0.5, xmax=lArDQGlobals.N_FEB_Parttions_Max-0.5)
       darray.defineHistogram('slotnb,FTnb;RAW_nbOfEvts',
                              title='Nb of events (DSP header check only);Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotnb,FTnb;RAW_nbOfFT',
                              title='Average # of cells with (qfactor+time) readout;Slot;FT',
                              type='TProfile1D',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)
             

if __name__=='__main__':

   from AthenaConfiguration.AllConfigFlags import initConfigFlags

   flags = initConfigFlags()

   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import DEBUG
   log.setLevel(DEBUG)

   from AthenaMonitoring.DQConfigFlags import DQDataType
   from LArMonitoring.LArMonConfigFlags import addLArMonFlags

   flags.addFlagsCategory("LArMon", addLArMonFlags)

   run="00404637"

   flags.Input.Files = [
       "/eos/atlas/atlastier0/rucio/data21_calib/calibration_LArElec-Pedestal-32s-High-All/00404637/data21_calib.00404637.calibration_LArElec-Pedestal-32s-High-All.daq.RAW/data21_calib.00404637.calibration_LArElec-Pedestal-32s-High-All.daq.RAW._lb0000._SFO-1._0001.data",
       "/eos/atlas/atlastier0/rucio/data21_calib/calibration_LArElec-Pedestal-32s-High-All/00404637/data21_calib.00404637.calibration_LArElec-Pedestal-32s-High-All.daq.RAW/data21_calib.00404637.calibration_LArElec-Pedestal-32s-High-All.daq.RAW._lb0000._SFO-2._0001.data",
       "/eos/atlas/atlastier0/rucio/data21_calib/calibration_LArElec-Pedestal-32s-High-All/00404637/data21_calib.00404637.calibration_LArElec-Pedestal-32s-High-All.daq.RAW/data21_calib.00404637.calibration_LArElec-Pedestal-32s-High-All.daq.RAW._lb0000._SFO-3._0001.data",
       "/eos/atlas/atlastier0/rucio/data21_calib/calibration_LArElec-Pedestal-32s-High-All/00404637/data21_calib.00404637.calibration_LArElec-Pedestal-32s-High-All.daq.RAW/data21_calib.00404637.calibration_LArElec-Pedestal-32s-High-All.daq.RAW._lb0000._SFO-4._0001.data"]
   flags.Output.HISTFileName = 'LArCalibPedMonOutput_'+run+'.root'
      
   flags.DQ.enableLumiAccess = False
   flags.Input.isMC = False
   flags.DQ.useTrigger = False
   flags.LAr.doAlign=False
   from AthenaConfiguration.Enums import BeamType
   flags.Beam.Type = BeamType.Collisions
   flags.DQ.DataType = DQDataType.Collisions
   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN2
   flags.Detector.GeometryCSC=False
   flags.Detector.GeometrysTGC=False
   flags.Detector.GeometryMM=False
   flags.Exec.OutputLevel=DEBUG
   flags.lock()

# Initialize configuration object, add accumulator, merge, and run.
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(flags)

   from LArByteStream.LArRawCalibDataReadingConfig import LArRawCalibDataReadingCfg
   cfg.merge(LArRawCalibDataReadingCfg(flags,gain="HIGH",doAccDigit=True))

   cfg.merge(LArCalibPedMonConfig(flags, gain="HIGH",doAccDigit=True))
   
   cfg.printConfig(withDetails=False) #set True for exhaustive info

   flags.dump()
   f=open("LArCalibPedMon_"+run+".pkl","wb")
   cfg.store(f)
   f.close()

   cfg.run(500) #to only run on first 500 events
