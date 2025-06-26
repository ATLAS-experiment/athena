#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# Define some handy strings for plot labels


# TO DO  - add defined cut names for selections, add them to plot names

from AthenaConfiguration.ComponentFactory import CompFactory


# selections from the digi (ADC) loop
selStr = {}
selStr["passDigiNom"] ="for unmasked SCs with ADC_max-pedestal > 10*RMS(DB) & good quality bits"
selStr["badNotMasked"] = "for unmasked SCs which have bad quality bits"
# selections from the sc (ET) loop
#selStr["passSCNom"] = "for unmasked SCs which pass #tau selection with non-zero ET < 10 GeV"
selStr["zeroET"] = "for unmasked SCs with ET == 0 GeV"

selStr["passSCNom"] = "for unmasked SCs with non-zero ET < 10 GeV"
#selStr["passSCNom0_0p325"] = "for unmasked SCs which pass #tau selection with non-zero ET < 0.2 GeV"
selStr["passSCNomInvalid"] = "for unmasked SCs with raw E -99999" # notMasked && NotSaturated && notOFCbOF
selStr["passSCNom0_0p325"] = "for unmasked SCs with 0 GeV < ET < 0.325 GeV"
#selStr["passSCNom0p325_1"] = "for unmasked SCs which pass #tau selection with 0.2 GeV < ET < 1 GeV"
selStr["passSCNom0p325_1"] = "for unmasked SCs with 0.325 GeV < ET < 1 GeV"
#selStr["passSCNom1"] = "for unmasked SCs which pass #tau selection with ET > 1 GeV"
selStr["passSCNom1"] = "for unmasked SCs with ET > 1 GeV"
#selStr["passSCNom10"] = "for unmasked SCs which pass tau selection with ET > 10 GeV"
selStr["passSCNom10"] = "for unmasked SCs with ET > 10 GeV"
#selStr["passSCNom10tauGt3"] = "for unmasked SCs which pass tau selection with ET > 10 GeV and #tau > 3"
selStr["passSCNom10tauGt3"] = "for unmasked SCs with ET > 10 GeV and #tau > 3"
selStr["saturNotMasked"] = "for unmasked SCs which are saturated"
selStr["OFCbOFNotMasked"] = "for unmasked SCs with OFCb in overflow"
selStr["onlofflEmismatch"] = "for unmasked SCs which pass #tau selection where online & offline energies are different"

selStr["notMaskedEoflNe0"] = "for unmasked SCs with non-zero ET ofl"
selStr["notMaskedEoflGt1"] = "for unmasked SCs with ET ofl > 1 GeV"



def LArDigitalTriggMonConfig(flags,larLATOMEBuilderAlg, nsamples=32, streamTypes=[]):
    '''Function to configures some algorithms in the monitoring system.'''
    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools

    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArDigitalTriggMonAlgCfg')

    from LArMonitoring.GlobalVariables import lArDQGlobals
    
    from AthenaCommon.Logging import logging
    mlog = logging.getLogger( 'LArDigitalTriggMon' )

    if not flags.DQ.enableLumiAccess:
       from LumiBlockComps.LuminosityCondAlgConfig import  LuminosityCondAlgCfg
       helper.resobj.merge(LuminosityCondAlgCfg(flags))

    #get SC onl-offl mapping from DB    
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    helper.resobj.merge(LArOnOffIdMappingSCCfg(flags))
    
    # and elec. calib. coeffs
    from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBSCCfg
    helper.resobj.merge(LArElecCalibDBSCCfg(flags, condObjs=["Ramp","DAC2uA", "Pedestal", "uA2MeV", "MphysOverMcal", "OFC", "Shape", "HVScaleCorr"]))



    larDigitalTriggMonAlg = helper.addAlgorithm(CompFactory.LArDigitalTriggMonAlg('larDigitalTriggMonAlg'))
    larDigitalTriggMonAlg.ProblemsToMask=["maskedOSUM"] #highNoiseHG","highNoiseMG","highNoiseLG","deadReadout","deadPhys"]
    if nsamples < 4:
        larDigitalTriggMonAlg.LArRawSCEtRecoContainerKey="dummy" # this will not exists, so not used in monitoring
         
    hasEtId = False
    hasEt = False
    hasAdc = False
    hasAdcBas = False
    for i in range(0,len(streamTypes)):
        mlog.info("runinfo.streamTypes()[i]: "+str(streamTypes[i]))
        if streamTypes[i] ==  "SelectedEnergy":
            hasEtId = True
            larDigitalTriggMonAlg.LArRawSCContainerKey = "SC_ET_ID"
        if streamTypes[i] ==  "Energy":
            hasEt = True
            larDigitalTriggMonAlg.LArRawSCContainerKey = "SC_ET"
        if streamTypes[i] ==  "RawADC":
            hasAdc = True
            larLATOMEBuilderAlg.LArDigitKey = "SC"
            larLATOMEBuilderAlg.isADCBas = False
            larDigitalTriggMonAlg.LArDigitContainerKey = "SC"
        if streamTypes[i] ==  "ADC":
            hasAdcBas = True
            larDigitalTriggMonAlg.isADCBas = True
            larDigitalTriggMonAlg.LArDigitContainerKey = "SC_ADC_BAS"
            larLATOMEBuilderAlg.isADCBas = True
            larLATOMEBuilderAlg.LArDigitKey = "SC_ADC_BAS"

    if (hasEtId and hasEt): #prefer EtId if both in recipe
        hasEt = False
        #larDigitalTriggMonAlg.EtName = "SC_ET_ID"

    if (hasAdc and hasAdcBas): #prefer Raw Adc if both in recipe
        hasAdc = False
        larDigitalTriggMonAlg.isADCBas=False

    mlog.info("Mux settings from COOL:")
    mlog.info("has ET Id: "+str(hasEtId))
    mlog.info("has ET: "+str(hasEt))
    mlog.info("has ADC: "+str(hasAdc))
    mlog.info("has ADC Bas: "+str(hasAdcBas))

    SCGroupName="SC"
    larDigitalTriggMonAlg.SCMonGroup=SCGroupName
    # uncomment if needed:
    #larDigitalTriggMonAlg.FileKey="CombinedMonitoring"

    SCGroup = helper.addGroup(
        larDigitalTriggMonAlg,
        SCGroupName,
        '/LArDigitalTrigger/',
        'run'
    )

    sc_hist_path='/'

    LatomeDetBinMapping = dict([
        ("0x48",{"Subdet":"FCALC","Bin":1}),
        ("0x4c",{"Subdet":"EMEC/HECC","Bin":3}),
        ("0x44",{"Subdet":"EMECC","Bin":11}),
        ("0x4a",{"Subdet":"EMB/EMECC","Bin":27}),
        ("0x42",{"Subdet":"EMBC","Bin":43}),
        ("0x41",{"Subdet":"EMBA","Bin":59}),
        ("0x49",{"Subdet":"EMB/EMECA","Bin":75}),
        ("0x43",{"Subdet":"EMECA","Bin":91}),
        ("0x4b",{"Subdet":"EMEC/HECA","Bin":107}),
        ("0x47",{"Subdet":"FCALA","Bin":115})
    ])    
    NLatomeBins=117
    NLatomeBins_side=59

    BinLabel_LATOME=[]
    BinLabel_LATOME_A=[]
    BinLabel_LATOME_C=[]
    phi=0
    for bb in range (0,NLatomeBins):
        Label=""
        for detID in LatomeDetBinMapping:
            if bb==(LatomeDetBinMapping[detID]["Bin"]-1):
                Label=LatomeDetBinMapping[detID]["Subdet"]
                phi=1
                break
        if bb < NLatomeBins_side:
            BinLabel_LATOME_C+=[Label+str(phi)]
        else:
            BinLabel_LATOME_A+=[Label+str(phi)]

        BinLabel_LATOME+=[Label+str(phi)]
        phi+=1
        
    


    iphi_bins_dict = {"ALL": 63, "EMB": 63, "EMEC": 63, "HEC": 63, "FCAL": 15}



    #### Plots from LATOME header loop
    SCGroup.defineHistogram('lumi_block,event_size;EventSize_vs_LB',
                            title='Digital trigger event size per LB; LumiBlock; Event size [MB]',
                            type='TProfile',
                            path=sc_hist_path,
                            xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)



    #### Per-subdetector/layer plots
    partGroup_digi = helper.addArray([larDigitalTriggMonAlg.LayerNames], larDigitalTriggMonAlg, 'LArDigitalTriggerMon_digi', topPath='/LArDigitalTrigger/')
    partGroup_sc = helper.addArray([larDigitalTriggMonAlg.LayerNames], larDigitalTriggMonAlg, 'LArDigitalTriggerMon_sc', topPath='/LArDigitalTrigger/')

    for part in larDigitalTriggMonAlg.LayerNames:
        selStrPart = {}
        for sel in selStr.keys():
            selStrPart[sel] = "in "+part+" "+selStr[sel]
        iphi_bins = 63
        for key in iphi_bins_dict.keys():
            if part.startswith(key):
                iphi_bins = iphi_bins_dict[key]
                
        if part == "ALL":
            partxbins=lArDQGlobals.SuperCell_Variables["etaRange"]["All"]["All"]
            partybins=lArDQGlobals.SuperCell_Variables["phiRange"]["All"]["All"]
            topPath=""
        else:
            topPath="PerPartition/"
            Side = part[-1]
            if "HEC" in part:
                Sampling = "0"
                Part = part[:-1]
            else:
                Sampling = part[-2]
                Part = part[:-2]
            if Part == "FCAL": 
                Part = "FCal"
            if Sampling == "P": 
                Sampling = "0"
            partxbins=lArDQGlobals.SuperCell_Variables["etaRange"][Part][Side][Sampling]
            partybins=lArDQGlobals.SuperCell_Variables["phiRange"][Part][Side][Sampling]
                
        
        
        #### Plots from Digi (ADC) loop
        for thisSel in [ "passDigiNom", "badNotMasked"]:
            thisTopPath=f"/{thisSel}/{topPath}"
            # Histos that we only want for all partitions/layers combined lalala
            if part == "ALL":
                partGroup_digi.defineHistogram('Digi_part_maxpos,Digi_part_partition;Partition_vs_maxSamplePosition_'+thisSel, 
                                               title='Partition vs. position of max sample '+selStrPart[thisSel],
                                               cutmask='Digi_part_'+thisSel,
                                               type='TH2F',
                                               path=thisTopPath,
                                               xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                                               ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                                               xlabels = [str(x) for x in range(1,nsamples+1)],      
                                               ylabels=lArDQGlobals.Partitions,
                                               pattern=[(part)])

                partGroup_digi.defineHistogram('Digi_part_latomesourceidbin,Digi_part_adc;ADCFullRange_vs_LATOME_'+thisSel,
                                               title='ADC vs LATOME name '+selStrPart[thisSel]+'; ; ADC',
                                               cutmask='Digi_part_'+thisSel,
                                               type='TH2F',
                                               path=thisTopPath,
                                               xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                                               ybins=500, ymin=-2, ymax=2500, #raw ADC is 12 bit
                                               xlabels=BinLabel_LATOME,
                                               pattern=[(part)])
                
                partGroup_digi.defineHistogram('Digi_part_latomesourceidbin,Digi_part_pedestal;Pedestal_vs_LATOME_'+thisSel,
                                               title='Pedestal vs LATOME name '+selStrPart[thisSel]+'; ; Pedestal',
                                               cutmask='Digi_part_'+thisSel,
                                               type='TH2F',
                                               path=thisTopPath,
                                               xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                                               ybins=500, ymin=-2, ymax=2500, #raw ADC is 12 bit
                                               xlabels=BinLabel_LATOME,
                                               pattern=[(part)])
                
                partGroup_digi.defineHistogram('Digi_part_latomesourceidbin,Digi_part_maxpos;MaxSamplePosition_vs_LATOME_'+thisSel,
                                               title='Position of max sample vs. LATOME '+selStrPart[thisSel],
                                               type='TH2F',
                                               cutmask='Digi_part_'+thisSel,
                                               path=thisTopPath,
                                               xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                                               ybins=nsamples,ymin=0.5,ymax=nsamples+0.5,
                                               xlabels=BinLabel_LATOME,
                                               ylabels = [str(x) for x in range(1,nsamples+1)],
                                               pattern=[(part)])
                
                partGroup_digi.defineHistogram('Digi_part_latomesourceidbin,Digi_part_diff_adc0_ped;Diff_ADC0_Ped_vs_LATOME_'+thisSel,
                                               title='ADC[0] - Pedestal vs LATOME name '+selStrPart[thisSel]+'; ; ADC[0] - Pedestal',
                                               type='TH2F',
                                               cutmask='Digi_part_'+thisSel,
                                               path=thisTopPath,
                                               xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                                               ybins=64, ymin=-32, ymax=32,
                                               xlabels=BinLabel_LATOME,
                                               pattern=[(part)])
                
                partGroup_digi.defineHistogram('Digi_part_latomesourceidbin,Digi_part_diff_adc_ped_norm;Diff_ADC_Ped_Norm_vs_LATOME_'+thisSel,
                                               title='(ADC-ped)/fabs(ADC_max-ped) '+selStrPart[thisSel]+'; LATOME Name; (ADC - pedestal) / fabs(ADC_max - pedestal)',
                                               type='TH2F',
                                               cutmask='Digi_part_'+thisSel,
                                               path=thisTopPath,
                                               xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                                               ybins=64, ymin=-32, ymax=32,
                                               xlabels=BinLabel_LATOME,
                                               pattern=[(part)])

            ####  End of plots only for ALL
            
            partGroup_digi.defineHistogram('Digi_part_eta,Digi_part_phi;Coverage_Eta_Phi_'+thisSel,
                                           title='SC coverage '+selStrPart[thisSel]+': #phi vs #eta;#eta;#phi',
                                           type='TH2F', 
                                           path=thisTopPath+'/Coverage',
                                           cutmask='Digi_part_'+thisSel,
                                           xbins=partxbins,
                                           ybins=partybins,
                                           pattern=[(part)])
            

            if not flags.Common.isOnline: continue   # Skip the remaining histos if we are running offline
            #### HERE - plots which should only be booked for the nominal selection
            if thisSel != "passDigiNom": continue
            partGroup_digi.defineHistogram('Digi_part_eta,Digi_part_phi,Digi_part_diff_adc0_ped;Coverage_Diff_ADC0_Ped_'+thisSel,  
                                           title='ADC[0] - Pedestal'+selStrPart[thisSel]+': #phi vs #eta;#eta;#phi',
                                           type='TProfile2D',
                                           cutmask='Digi_part_'+thisSel,
                                           path=thisTopPath+'/Coverage',
                                           xbins=partxbins,
                                           ybins=partybins,
                                           pattern=[(part)])


            #####################
            partGroup_digi.defineHistogram('Digi_part_BCID,Digi_part_iphi,Digi_part_diff_adc_ped;Diff_ADC_Ped_Per_BCID_Per_iphi_'+thisSel,
                                           title='ADC - Pedestal (all samples) '+selStrPart[thisSel]+': iphi vs BCID;BCID;iphi',
                                           type='TProfile2D',
                                           cutmask='Digi_part_'+thisSel,
                                           path=thisTopPath,
                                           xbins=3564,xmin=-0.5,xmax=3563.5,
                                           ybins=iphi_bins+1,ymin=0,ymax=iphi_bins+1,  # Make a lardqglobals for ieta iphi?
                                           pattern=[(part)])
            


            ##################
            
            
            partGroup_digi.defineHistogram('Digi_part_sampos,Digi_part_adc;ADCZoom_vs_SamplePosition_'+thisSel,
                                           title='ADC (zoom) vs sample position '+selStrPart[thisSel],
                                           cutmask='Digi_part_'+thisSel,
                                           type='TH2F',
                                           path=thisTopPath,
                                           xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                                           xlabels = [str(x) for x in range(1,nsamples+1)],
                                           ybins=750, ymin=0, ymax=1300, #start from 0 otherwise miss endcap pedestals
                                           pattern=[(part)]) 
        
            partGroup_digi.defineHistogram('Digi_part_sampos,Digi_part_adc;ADCFullRange_vs_SamplePosition_'+thisSel, 
                                           title='ADC vs sample position '+selStrPart[thisSel],
                                           cutmask='Digi_part_'+thisSel,
                                           type='TH2F',
                                           path=thisTopPath,
                                           xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                                           xlabels = [str(x) for x in range(1,nsamples+1)],
                                           ybins=500, ymin=0, ymax=5000, #raw ADC is 12 bit
                                           pattern=[(part)]) 
            
            partGroup_digi.defineHistogram('Digi_part_sampos,Digi_part_pedestal;Pedestal_vs_SamplePosition_'+thisSel,
                                           title='Pedestal vs sample position '+selStrPart[thisSel],
                                           cutmask='Digi_part_'+thisSel,
                                           type='TH2F',
                                           path=thisTopPath,
                                           xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                                           xlabels = [str(x) for x in range(1,nsamples+1)],
                                           ybins=500, ymin=0, ymax=5000, #raw ADC is 12 bit 
                                           pattern=[(part)])
            
            partGroup_digi.defineHistogram('Digi_part_diff_adc0_ped;Diff_ADC0_Ped_'+thisSel,
                                           title='LATOME (ADC[0]-ped) '+selStrPart[thisSel]+'; (ADC - pedestal)',
                                           type='TH1F',
                                           cutmask='Digi_part_'+thisSel,
                                           path=thisTopPath,
                                           xbins=50,xmin=-25,xmax=25,
                                           pattern=[(part)])
            
            partGroup_digi.defineHistogram('Digi_part_sampos,Digi_part_diff_adc_ped_norm;Diff_ADC_Ped_Norm_vs_SamplePosition_'+thisSel,
                                           title='(ADC-ped)/fabs(ADC_max-ped) '+selStrPart[thisSel]+'; Sample position; (ADC - pedestal) / fabs(ADC_max - pedestal)',
                                           type='TH2F',
                                           cutmask='Digi_part_'+thisSel,
                                           path=thisTopPath,
                                           ybins=40,ymin=-1,ymax=1,
                                           xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                                           xlabels = [str(x) for x in range(1,nsamples+1)],
                                           pattern=[(part)])
                       
            partGroup_digi.defineHistogram('Digi_part_BCID, Digi_part_adc;ADC_vs_BCID_'+thisSel, 
                                           title='ADC value vs BCID '+selStrPart[thisSel]+'; BCID; ADC Value',
                                           type='TProfile',
                                           cutmask='Digi_part_'+thisSel,
                                           path=thisTopPath,
                                           xbins=3564,xmin=-0.5,xmax=3563.5,
                                           ybins=500, ymin=0, ymax=5000,
                                           pattern=[(part)])

            partGroup_digi.defineHistogram('Digi_part_BCID, Digi_part_diff_adc0_ped;Diff_ADC0_Ped_vs_BCID_'+thisSel, 
                                           title='ADC[0] - Ped value vs BCID '+selStrPart[thisSel]+'; BCID; ADC[0] Value',
                                           type='TProfile',
                                           cutmask='Digi_part_'+thisSel,
                                           path=thisTopPath,
                                           xbins=3564,xmin=-0.5,xmax=3563.5,
                                           ybins=500, ymin=-5, ymax=5,
                                           pattern=[(part)])

            partGroup_digi.defineHistogram('Digi_part_LB, Digi_part_adc;ADC_vs_LB_'+thisSel,
                                           title='ADC value vs LB '+selStrPart[thisSel]+'; LB; ADC Value',
                                           type='TProfile',
                                           cutmask='Digi_part_'+thisSel,
                                           path=thisTopPath,
                                           xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max,
                                           ybins=500, ymin=0, ymax=5000,
                                           pattern=[(part)])

        #### Plots from SC ET loop 

        #for thisSel in [ "passSCNom", "passSCNom1", "passSCNom10", "passSCNom10tauGt3", "saturNotMasked", "OFCbOFNotMasked", "onlofflEmismatch", "notMaskedEoflNe0", "notMaskedEoflGt1"]:
        for thisSel in [ "zeroET", "passSCNom0_0p325", "passSCNom0p325_1", "passSCNom1", "passSCNom10", "passSCNom10tauGt3", "saturNotMasked", "OFCbOFNotMasked", "onlofflEmismatch", "notMaskedEoflNe0", "notMaskedEoflGt1"]:
            thisTopPath=f"/{thisSel}/{topPath}"
            # Histos that we only want for all partitions/layers combined lalala
            if part == "ALL":
                partGroup_sc.defineHistogram('SC_part_latomesourceidbin,SC_part_et_onl;SC_ET_Onl_vs_LATOME_'+thisSel,
                                             title='SC ET [GeV] vs LATOME name '+selStrPart[thisSel]+'; ; ET SC [GeV]',
                                             type='TH2F',
                                             cutmask='SC_part_'+thisSel,
                                             path=thisTopPath,
                                             xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                                             ybins=200, ymin=-10, ymax=200,
                                             xlabels=BinLabel_LATOME,
                                             pattern=[(part)])


                partGroup_sc.defineHistogram('SC_part_LB,SC_part_latomesourceidbin;LB_vs_LATOME_'+thisSel,
                                             title='LATOME name vs LB '+selStrPart[thisSel]+';LB;LATOME',
                                             type='TH2F',
                                             cutmask='SC_part_'+thisSel,
                                             path=thisTopPath,                            
                                             xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max,
                                             ybins=NLatomeBins,ymin=1,ymax=NLatomeBins+1,
                                             ylabels=BinLabel_LATOME,
                                             pattern=[(part)])

                partGroup_sc.defineHistogram('SC_part_time,SC_part_et_ofl;time_vs_et_ofl_'+thisSel, 
                                             title='SC coverage '+selStrPart[thisSel]+': #tau vs ET ofl;#tau;ET ofl',
                                             type='TH2F',
                                             cutmask='SC_part_'+thisSel,
                                             path=thisTopPath,
                                             xbins=500,xmin=-50,xmax=50,
                                             ybins=500,ymin=-10,ymax=70,
                                             pattern=[(part)])

            
                partGroup_sc.defineHistogram('SC_part_latomesourceidbin,SC_part_time;MeanOfflineLATOMEtime_vs_LATOME_'+thisSel, 
                                             title='Average LATOME #tau from Offline computation per LATOME'+selStrPart[thisSel]+'; LATOME ; #tau [ns]',
                                             type='TH2F',
                                             cutmask='SC_part_'+thisSel,
                                             path=thisTopPath,
                                             xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                                             ybins=200, ymin=-50, ymax=50,
                                             xlabels=BinLabel_LATOME,
                                             pattern=[(part)])

                partGroup_sc.defineHistogram('SC_part_LB,SC_part_latomesourceidbin,SC_part_time;MeanOfflineLATOMEtime_perLB_perLATOME_'+thisSel,
                                             title='SC #tau '+selStrPart[thisSel]+': LATOME vs LB;LB;LATOME',
                                             type='TProfile2D',
                                             cutmask='SC_part_'+thisSel,
                                             path=thisTopPath,                            
                                             xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max,
                                             ybins=NLatomeBins,ymin=1,ymax=NLatomeBins+1,
                                             ylabels=BinLabel_LATOME,
                                             pattern=[(part)])

                partGroup_sc.defineHistogram('SC_part_LB,SC_part_time;MeanOfflineLATOMEtime_vs_LB_'+thisSel,
                                             title='Average LATOME #tau from Offline computation per LB '+selStrPart[thisSel]+'; LumiBloc; #tau [ns]',
                                             type='TProfile',
                                             cutmask='SC_part_'+thisSel,
                                             path=thisTopPath,
                                             xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max,
                                             pattern=[(part)])


            ####  End of plots only for ALL

            partGroup_sc.defineHistogram('SC_part_eta,SC_part_phi;Coverage_Eta_Phi_'+thisSel, 
                                         title='SC coverage '+selStrPart[thisSel]+': #phi vs #eta;#eta;#phi',
                                         type='TH2F',
                                         cutmask='SC_part_'+thisSel,
                                         path=thisTopPath+'/Coverage',
                                         xbins=partxbins,
                                         ybins=partybins,
                                         pattern=[(part)])


            partGroup_sc.defineHistogram('SC_part_time;OfflineLATOMEtime_'+thisSel, 
                                         title='LATOME #tau from Offline Computation '+selStrPart[thisSel]+';#tau [ns]; Evts;',
                                         type='TH1F',
                                         cutmask='SC_part_'+thisSel,
                                         path=thisTopPath,
                                         xbins=100,xmin=-25,xmax=25,
                                         pattern=[(part)])

            
            if not flags.Common.isOnline: continue   # Skip the remaining histos if we are running offline
            #### HERE - plots which should only be booked for the nominal selection
            if thisSel !=  "passSCNom1": continue

            partGroup_sc.defineHistogram('SC_part_eta,SC_part_phi,SC_part_et_onl;Coverage_Et_Onl_'+thisSel,
                                         title='SC Energy '+selStrPart[thisSel]+': #phi vs #eta;#eta;#phi',
                                         type='TProfile2D',
                                         cutmask='SC_part_'+thisSel,
                                         path=thisTopPath+'/Coverage',
                                         xbins=partxbins,
                                         ybins=partybins,
                                         pattern=[(part)])

            partGroup_sc.defineHistogram('SC_part_eta,SC_part_phi,SC_part_time;Coverage_OfflineLATOMEtime_'+thisSel,
                                         title='LATOME #tau from Offline Computation '+selStrPart[thisSel]+': #phi vs #eta;#eta;#phi',
                                         type='TProfile2D',
                                         cutmask='SC_part_'+thisSel,
                                         path=thisTopPath+'/Coverage',
                                         xbins=partxbins,
                                         ybins=partybins,
                                         pattern=[(part)])

            partGroup_sc.defineHistogram('SC_part_et_onl,SC_part_et_ofl;ET_Ofl_vs_ET_Onl_'+thisSel,
                                         title='LATOME ET vs Offline Computation '+selStrPart[thisSel]+'; ET Onl;ET Offl [GeV]',
                                         type='TH2F',
                                         cutmask='SC_part_'+thisSel,
                                         path=thisTopPath,
                                         xbins=0,xmin=0,xmax=20,
                                         ybins=0,ymin=0,ymax=20,
                                         pattern=[(part)])
            
            partGroup_sc.defineHistogram('SC_part_et_diff;ET_Diff_OnlOfl_'+thisSel,
                                         title='LATOME ET vs Offline Computation '+selStrPart[thisSel]+'; ET Onl - ET Offl [GeV]; Evts;',
                                         type='TH1F',
                                         cutmask='SC_part_'+thisSel,
                                         path=thisTopPath,
                                         xbins=200,xmin=-10,xmax=10,
                                         pattern=[(part)])

                        
            partGroup_sc.defineHistogram('SC_part_et_onl;SC_ET_Onl_'+thisSel, 
                                         title='SC eT [GeV] '+selStrPart[thisSel],
                                         type='TH1F',
                                         cutmask='SC_part_'+thisSel,
                                         path=thisTopPath,
                                         xbins=500, xmin=-100, xmax=400,
                                         pattern=[(part)])
                     
            
            partGroup_sc.defineHistogram('SC_part_BCID,SC_part_time;MeanOfflineLATOMEtime_vs_BCID_'+thisSel,
                                         title='Average LATOME #tau from Offline computation per BCID '+selStrPart[thisSel]+'; BCID; #tau [ns]',
                                         type='TProfile',
                                         cutmask='SC_part_'+thisSel,
                                         path=thisTopPath,
                                         xbins=3564,xmin=-0.5,xmax=3563.5, 
                                         pattern=[(part)])
            
            partGroup_sc.defineHistogram('SC_part_BCID,SC_part_et_onl_muscaled;AvEnergyVsBCID_'+thisSel,
                                         title='Average Energy vs BCID '+selStrPart[thisSel]+'; BCID; Energy per SC [MeV]',
                                         type='TProfile',
                                         cutmask='SC_part_'+thisSel,
                                         path=thisTopPath,
                                         xbins=3564,xmin=-0.5,xmax=3563.5,
                                         ybins=10, ymin=-20, ymax=20,
                                         pattern=[(part)])
        
        for thisSel in ["passSCNomInvalid"]:
            thisTopPath=f"/{thisSel}/{topPath}"
            partGroup_sc.defineHistogram('SC_part_eta,SC_part_phi;Coverage_Eta_Phi_'+thisSel,
                                        title='SC coverage '+selStrPart[thisSel]+': #phi vs #eta;#eta;#phi',
                                        type='TH2F',
                                        path=thisTopPath+'/Coverage',
                                        cutmask='SC_part_'+thisSel,
                                        xbins=partxbins,
                                        ybins=partybins,
                                        pattern=[(part)])

                

    return helper.result()


if __name__=='__main__':

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags = initConfigFlags()
   
   from AthenaConfiguration.Enums import LHCPeriod
   flags.GeoModel.Run = LHCPeriod.Run3

   flags.Input.Files = ["/eos/atlas/atlascerngroupdisk/proj-spot/spot-job-inputs/data23_13p6TeV/data23_13p6TeV.00451569.physics_Main.daq.RAW._lb0260._SFO-14._0001.data"]

   flags.Output.HISTFileName = 'LArDigitalTriggMonOutput.root'

   flags.DQ.useTrigger = False
   flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
   flags.PerfMon.doFullMonMT=True
   flags.Exec.MaxEvents=100
   flags.fillFromArgs()
   flags.lock()

   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(flags)
   
   # in case of tier0 workflow:
   from CaloRec.CaloRecoConfig import CaloRecoCfg
   cfg.merge(CaloRecoCfg(flags))

   from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
   cfg.merge(LArBadChannelCfg(flags, isSC=True))

   from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
   SCData_acc =  LArRawSCDataReadingCfg(flags)
 
   cfg.merge(SCData_acc)

   larLATOMEBuilderAlg=CompFactory.LArLATOMEBuilderAlg("LArLATOMEBuilderAlg",LArDigitKey="SC", isADCBas=False)
   cfg.addEventAlgo(larLATOMEBuilderAlg)

   streamTypes = ["SelectedEnergy", "ADC"]
   aff_acc = LArDigitalTriggMonConfig(flags, larLATOMEBuilderAlg, streamTypes=streamTypes)
   cfg.merge(aff_acc)

   from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
   cfg.merge(PerfMonMTSvcCfg(flags))
   
   cfg.run()


