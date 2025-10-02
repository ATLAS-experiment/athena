#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
def EfexMonitoringConfig(inputFlags):
    '''Function to configure LVL1 Efex monitoring algorithm'''

    # get the component factory - used for merging the algorithm results
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # add algorithm to the helper
    result.addEventAlgo( CompFactory.EfexMonitorAlgorithm('EfexMonAlg',
                                                  PackageName='EfexMonitor',
                                                  LowPtCut = 0.0,
                                                  HiPtCut = 15000.0,
                                                  eFexEMTobKeyList = ['L1_eEMRoI', 'L1_eEMxRoI'],
                                                  eFexTauTobKeyList = ['L1_eTauRoI', 'L1_eTauxRoI']
                                                  ) )

    return result


def EfexMonitoringHistConfig(flags, eFexAlg):
    """
    Book the histograms for the efex monitoring. This is done in a separate method
    to the algorithm creation (above) because the histograms are based on the list of container keys
    given to the algorithm, which can be customized by the user before calling this method.
    """
    import math
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()


    # make the athena monitoring helper again so we can add groups
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,None,'EfexMonitoringCfg')
    helper.alg = eFexAlg

    # we don't add the algorithm again, use the supplied one
    EfexMonAlg = eFexAlg 
    baseGroupName = EfexMonAlg.PackageName

    # Some helpful variables for declaring the histograms
    # mainDir = 'L1Calo'
    trigPath = f'Developer/{eFexAlg.name}/' # Default Directory trigger path for output histos
    # Map from the key name to the output directory substructure.
    
    def pathFromKey(key,prefix="Nominal/"):
        path=prefix
        if "DAODSim" in key: path = "DAODSim/"
        elif "ReSim" in key: path = "ReSim/"
        elif "Sim" in key: path = "Sim/"
        if "_eEMx" in key: path += "eEMx"
        elif "_eEM" in key: path += "eEM"
        elif "_eTaux" in key: path += "eTAUx"
        elif "_eTau" in key: path += "eTAU"
        return path 
    
    cut_names = ["LowPtCut", "HiPtCut"] # List of cut names, for looping over to separate out histograms into directories
    cut_vals = [EfexMonAlg.LowPtCut, EfexMonAlg.HiPtCut] # List of values, for looping over to add to histogram titles

    # First, define the histograms with no Pt cut
    # add monitoring algorithm to group, with group name and main directory

    locIdxs = []
    for phiOct in range(0,8):
        for etaIdx in range(-25,25):
            locIdxs += [str(phiOct) + ":" + str(etaIdx)]

    for containerKey in (list(EfexMonAlg.eFexEMTobKeyList) + list(EfexMonAlg.eFexTauTobKeyList)):
        helper.defineHistogram(containerKey + '_nTOBs_nocut;h_n'+containerKey+'_nocut', title='Number of '+containerKey+';Number of '+containerKey+';Events',
                               fillGroup = baseGroupName,
                               path=trigPath+pathFromKey(containerKey)+"NoCut",
                               type='TH1I', xbins=100,xmin=-0.5,xmax=99.5)
        if "Sim" not in containerKey:
            # don't make these expensive plots for simulation
            helper.defineHistogram(f"LBN,{containerKey}_nTOBs_nocut;h_"+containerKey+"_nTOBs", title = "Average # of " + containerKey + " TOBs;LBN",
                                   fillGroup = baseGroupName + "_" + containerKey,
                                   path=trigPath+pathFromKey(containerKey)+"NoCut",
                                   type="TH2I",
                                   xbins=1,xmin=0,xmax=1,ybins=20,ymin=-0.5,ymax=19.5, opt=['kAddBinsDynamically'])


    commonAlgConfig = {"libname":"libdqm_summaries.so",
                       "name":"L1Calo_BinsDiffFromStripMedian",
                       "PublishDetail":32}
    hotCuts = {"ColdCut":-7,"WarmCut":9,"HotCut":20} # when looking at frequency of hot deposits, use these cuts

    commonThresholdConfig = {
        "NWrongKnown":[0,100], # warn of any corrections that are needed for the known anomalies lists
        "NDead":[0,2], # warn on any new dead spots, error if more than a couple
        "NHot":[0,2],  # warn on any new hot spots, error if more than a couple
        "NCold":[0,2],  # warn on any new cold spots, error if more than a couple
        "NWarm":[0,5],  # warn on any new warm spots, error if more than 5
        "NDeadStrip":[0,0], # no dead strips - exception to this will be in cold hcal, where tile cannot be negative
        "NConsecUnlikelyStrip":[2,5], # warn if more than 2 consecutive strips deemed unlikely
    }

    knownAnomalies_eEM = {
        "KnownDead":"\"49,41;49,42;48,8\"", #26,14 and 27,14 are L2 masked so cannot seed, but other layers still there so will just be cold in input (not dead). These were unmasked again in 502880 onwards
        "KnownCold":"\"48,8;2,18;3,17;4,18;5,17;20,31;21,31;23,26;49,10;11,30;11,44;11,51;15,51;40,12;40,19;40,26;10,17;10,18;10,20;2,50;24,49;14,54;12,20;14,51;16,49;21,46;23,47;31,24;34,40;35,40;36,39;37,39;42,38;46,41;47,47;47,8\"",
        "KnownWarm":"\"11,26;11,52;21,48;40,21;41,21;42,21;43,21;44,21;45,21;46,21;47,21;50,21\"", # there's a warm strip @ phi=21
        "KnownHot":"\"4,5;6,2;13,48;21,48;14,49;27,53;35,13;22,14\""
        # HI Running
        # "KnownDead":"\"49,41;49,42;26,14;27,14;42,37;42,38;46,3;14,51\"", #26,14 and 27,14 are L2 masked so cannot seed, but other layers still there so will just be cold in input (not dead)
        # "KnownCold":"\"48,8;2,18;3,17;4,18;5,17;20,31;21,31;23,26;49,10;11,30;11,44;11,51;15,51;40,12;40,19;40,26;10,17;10,18;10,20;2,50;24,49;14,54\"",
        # "KnownWarm":"\"11,26;11,52;21,48;40,21;41,21;42,21;43,21;44,21;45,21;46,21;47,21;50,21\"", # there's a warm strip @ phi=21
        # "KnownHot":"\"4,5;6,2;13,48;21,48;14,49;27,53;30,23;35,13;22,14\"" # these spots from HI running
    }

    # use KnownDead list from input hcal as an automatic cold-spot for eTAU
    from TrigT1CaloMonitoring.EfexInputMonitorAlgorithm import knownAnomalies_hotHcal

    # even though dead hcal @ 41,21 .. it's in a warm ecal area, meaning it isn't actually cold
    knownAnomalies_eTAU = {
        "KnownCold":knownAnomalies_eEM["KnownDead"][:-1]+";"+knownAnomalies_hotHcal["KnownDead"][1:-1].replace("41,21;","")+knownAnomalies_hotHcal["KnownCold"][1:-1]+";48,8;2,18;3,17;4,18;5,17;49,10;10,18;10,20;20,31;21,31;14,51;15,51;23,26;29,7;17,32;14,54;34,6;40,12;40,19;26,20;11,44;24,49;30,15;32,46;29,5;16,49;12,19;12,20;17,3;20,10;22,47;23,47;34,10;34,40;35,40;37,39;40,26;43,59;47,47;47,8\"",
        "KnownWarm":"\"24,39;41,21;43,21;44,21;47,21\"",
        "KnownHot":"\"4,5;6,2;11,27;13,48;15,49;21,48;40,24;42,35;18,4\""
        # HI Running
        # "KnownCold":knownAnomalies_eEM["KnownDead"][:-1]+";"+knownAnomalies_hotHcal["KnownDead"][1:-1].replace("41,21;","")+knownAnomalies_hotHcal["KnownCold"][1:-1]+";48,8;2,18;3,17;4,18;5,17;49,10;10,18;10,20;20,31;21,31;14,51;15,51;23,26;29,7;17,32;14,54;34,6;40,12;40,19;26,20;11,44;24,49;30,15;32,46\"",
        # "KnownWarm":"\"24,39;43,21;44,21\"",
        # "KnownHot":"\"4,5;6,2;11,27;13,48;15,49;21,48;40,24;42,35;18,4\"" # these spots from HI running
    }

    helper.defineDQAlgorithm("Efex_eEM_etaThiMapFilled",
                             hanConfig=commonAlgConfig|hotCuts|knownAnomalies_eEM,
                             thresholdConfig=commonThresholdConfig
                             )
    helper.defineDQAlgorithm("Efex_eTAU_etaThiMapFilled",
                             hanConfig=commonAlgConfig|hotCuts|knownAnomalies_eTAU,
                             thresholdConfig=commonThresholdConfig
                             )

    helper.defineDQAlgorithm("Efex_eEM_etaPhiLBMapOutliers_Shifter",
                             hanConfig=commonAlgConfig|hotCuts|knownAnomalies_eEM|{"NBinsY":64,"LiveMode":1},
                             thresholdConfig=commonThresholdConfig
                             ) # configuration for P1 monitoring
    helper.defineDQAlgorithm("Efex_eEM_etaPhiLBMapOutliers",
                             hanConfig=commonAlgConfig|hotCuts|knownAnomalies_eEM|{"NBinsY":64,"LiveMode":0},
                             thresholdConfig=commonThresholdConfig
                             )
    helper.defineDQAlgorithm("Efex_eTAU_etaPhiLBMapOutliers_Shifter",
                         hanConfig=commonAlgConfig|hotCuts|knownAnomalies_eTAU|{"NBinsY":64,"LiveMode":1},
                         thresholdConfig=commonThresholdConfig
                         ) # configuration for P1 monitoring
    helper.defineDQAlgorithm("Efex_eTAU_etaPhiLBMapOutliers",
                             hanConfig=commonAlgConfig|hotCuts|knownAnomalies_eTAU|{"NBinsY":64,"LiveMode":0},
                             thresholdConfig=commonThresholdConfig
                             )


    # Now define the histograms with low/hi Pt cut
    for cut_name, cut_val in zip(cut_names, cut_vals):
        cut_title_addition = '' if (cut_val == 0.0) else ' [Et>=' + '%.1f'%(cut_val/1000) + 'GeV]'
        # Em first
        for containerKey in EfexMonAlg.eFexEMTobKeyList:
            fillGroup = baseGroupName+'_'+containerKey+'_'+cut_name
            tobStr = containerKey
            # histograms of eEM variables
            helper.defineHistogram('nEMTOBs;h_nEmTOBs', title='Number of '+tobStr+'s'+cut_title_addition+';EM '+tobStr+'s;Number of EM '+tobStr+'s',
                                   fillGroup=fillGroup,
                                    type='TH1I', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=10,xmin=0,xmax=10)

            helper.defineHistogram('TOBTransverseEnergy;h_TOBTransverseEnergy', title=tobStr+' ET [MeV]'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=100,xmin=0,xmax=50000)

            helper.defineHistogram('TOBEta;h_TOBEta', title=tobStr+' Eta'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=50,xmin=-2.5,xmax=2.5)

            helper.defineHistogram('TOBPhi;h_TOBPhi', title=tobStr+' Phi'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=64,xmin=-math.pi,xmax=math.pi)

            helper.defineHistogram(f"TOBEta,TOBPhi;h_{containerKey}_{cut_name}_EtaPhiMap", title=tobStr+' Count'+cut_title_addition+';#eta;#phi',
                                   fillGroup=fillGroup,
                                   hanConfig={"display":"SetPalette(55)",
                                       "description":f"Any really significant hot/cold spots (see results for significances) should check over what LBs they occurred, using <a href='./detail/h_{containerKey}_{cut_name}_posVsLBN'>detail timeseries</a>. Any new (unknown) spots, if they occur multiple times in a week, please report","algorithm":"Efex_eEM_etaThiMapFilled"},
                                    type='TH2F',
                                    path=(("Expert/Outputs/"+pathFromKey(containerKey,"")) if "Sim" not in containerKey and "x" not in containerKey else trigPath+pathFromKey(containerKey)+cut_name),
                                    xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi,opt=['kAlwaysCreate'])

            if "Sim" not in containerKey and "x" not in containerKey:
                helper.defineHistogram(f"LBN,binNumber;h_{containerKey}_{cut_name}_posVsLBN", title=tobStr+' Count'+cut_title_addition+';LB;64(x-1)+y',
                                   fillGroup=fillGroup,
                                   hanConfig={"Expert/algorithm":f"Efex_{pathFromKey(containerKey,'')}_etaPhiLBMapOutliers","Shifter/algorithm":f"Efex_{pathFromKey(containerKey,'')}_etaPhiLBMapOutliers_Shifter",
                                              "description":f"Timeseries of TOB counts at each location ... y-axis relates to x and y bin numbers from <a href='../h_{containerKey}_{cut_name}_EtaPhiMap'>eta-phi map</a>. Use Projection X1 for 1D plot"},
                                   type='TH2I',
                                   paths=["Expert/Outputs/"+pathFromKey(containerKey,"")+"/detail","Shifter/Outputs/"+pathFromKey(containerKey,"")],
                                   xbins=1,xmin=0,xmax=10,
                                   ybins=64*50,ymin=0.5,ymax=64*50+0.5,opt=['kAddBinsDynamically'])

            helper.defineHistogram('TOBshelfNumber;h_TOBshelfNumber', title=tobStr+' EM Shelf Number'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=2,xmin=0,xmax=2)

            helper.defineHistogram('TOBeFEXNumberSh0;h_TOBeFEXNumberShelf0', title=tobStr+' EM Module Number Shelf 0'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=12,xmin=0,xmax=12)

            helper.defineHistogram('TOBeFEXNumberSh1;h_TOBeFEXNumberShelf1', title=tobStr+' EM Module Number Shelf 1'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=12,xmin=0,xmax=12)

            helper.defineHistogram('TOBfpga;h_TOBfpga', title=tobStr+' EM FPGA'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=4,xmin=0,xmax=4)

            helper.defineHistogram('TOBReta;h_TOBReta', title=tobStr+' EM Reta'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name,xbins=250,xmin=0,xmax=1)

            helper.defineHistogram('TOBRhad;h_TOBRhad', title=tobStr+' EM Rhad'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=250,xmin=0,xmax=1)

            helper.defineHistogram('TOBWstot;h_TOBWstot', title=tobStr+' EM Wstot'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=250,xmin=0,xmax=1)

            threshold_labels = ['fail','loose','medium','tight']
            helper.defineHistogram('TOBReta_threshold;h_TOBReta_threshold', title=tobStr+' EM Reta threshold'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name,xbins=4,xmin=0,xmax=4.0,xlabels=threshold_labels)

            helper.defineHistogram('TOBRhad_threshold;h_TOBRhad_threshold', title=tobStr+' EM Rhad threshold'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=4,xmin=0,xmax=4.0,xlabels=threshold_labels)

            helper.defineHistogram('TOBWstot_threshold;h_TOBWstot_threshold', title=tobStr+' EM Wstot threshold'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=4,xmin=0,xmax=4.0,xlabels=threshold_labels)

        # Now Tau
        for containerKey in EfexMonAlg.eFexTauTobKeyList:
            fillGroup = baseGroupName+'_'+containerKey+'_'+cut_name
            tobStr = containerKey
            # plotting of eTau variables
            helper.defineHistogram('nTauTOBs;h_nTauTOBs', title='Number of '+tobStr+'s'+cut_title_addition+';Tau '+tobStr+'s;Number of Tau '+tobStr+'s',
                                    fillGroup = fillGroup,
                                    type='TH1I', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=10,xmin=0,xmax=10)

            helper.defineHistogram('tauTOBTransverseEnergy;h_tauTOBTransverseEnergy', title=tobStr+' Tau Transverse Energy [MeV]'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=100,xmin=0,xmax=50000)

            helper.defineHistogram('tauTOBEta;h_tauTOBEta', title=tobStr+' Tau Eta'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=60,xmin=-2.5,xmax=2.5)

            helper.defineHistogram('tauTOBPhi;h_tauTOBPhi', title=tobStr+' Tau Phi'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=100,xmin=-math.pi,xmax=math.pi)

            helper.defineHistogram(f"tauTOBEta,tauTOBPhi;h_{containerKey}_{cut_name}_EtaPhiMap", title='eTAU '+tobStr+' Count'+cut_title_addition+';#eta;#phi',
                                    fillGroup = fillGroup,
                                   hanConfig={"display":"SetPalette(55)",
                                              "description":f"Any really significant hot/cold spots (see results for significances) should check over what LBs they occurred, using <a href='./detail/h_{containerKey}_{cut_name}_posVsLBN'>detail timeseries</a>. Any new (unknown) spots, if they occur multiple times in a week, please report","algorithm":"Efex_eTAU_etaThiMapFilled"},
                                   type='TH2F',
                                   path=(("Expert/Outputs/"+pathFromKey(containerKey,"")) if "Sim" not in containerKey and "x" not in containerKey else (trigPath+pathFromKey(containerKey)+cut_name)),
                                   xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi,opt=['kAlwaysCreate'])

            if "Sim" not in containerKey and "x" not in containerKey:
                helper.defineHistogram(f"LBN,binNumber;h_{containerKey}_{cut_name}_posVsLBN", title='eTAU '+tobStr+' Count'+cut_title_addition+';LB;64(x-1)+y',
                               fillGroup=fillGroup,
                               hanConfig={"Expert/algorithm":f"Efex_{pathFromKey(containerKey,'')}_etaPhiLBMapOutliers","Shifter/algorithm":f"Efex_{pathFromKey(containerKey,'')}_etaPhiLBMapOutliers_Shifter",
                                          "description":f"Timeseries of TOB counts at each location ... y-axis relates to x and y bin numbers from <a href='../h_{containerKey}_{cut_name}_EtaPhiMap'>eta-phi map</a>. Use Projection X1 for 1D plot"},
                               type='TH2I',
                               paths=["Expert/Outputs/"+pathFromKey(containerKey,"")+"/detail","Shifter/Outputs/"+pathFromKey(containerKey,"")],
                               xbins=1,xmin=0,xmax=10,
                               ybins=64*50,ymin=0.5,ymax=64*50+0.5,opt=['kAddBinsDynamically'])

            helper.defineHistogram('tauTOBshelfNumber;h_tauTOBshelfNumber', title=tobStr+' Tau Shelf Number'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=2,xmin=0,xmax=2)

            helper.defineHistogram('tauTOBeFEXNumberSh0;h_tauTOBeFEXNumberShelf0', title=tobStr+' Tau Module Number Shelf 0'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=12,xmin=0,xmax=12)

            helper.defineHistogram('tauTOBeFEXNumberSh1;h_tauTOBeFEXNumberShelf1', title=tobStr+' Tau Module Number Shelf 1'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=12,xmin=0,xmax=12)


            helper.defineHistogram('tauTOBfpga;h_tauTOBfpga', title=tobStr+' Tau FPGA'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=4,xmin=0,xmax=4)

            helper.defineHistogram('tauTOBRcore;h_tauTOBRcore', title=tobStr+' Tau rCore'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=250,xmin=0,xmax=1)

            helper.defineHistogram('tauTOBRhad;h_tauTOBRhad', title=tobStr+' Tau rHad'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=250,xmin=0,xmax=1)

            helper.defineHistogram('tauTOBRcore_threshold;h_tauTOBRcore_threshold', title=tobStr+' Tau rCore threshold'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=4,xmin=0,xmax=4.0, xlabels=threshold_labels)

            helper.defineHistogram('tauTOBRhad_threshold;h_tauTOBRhad_threshold', title=tobStr+' Tau rHad threshold'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=4,xmin=0,xmax=4.0, xlabels=threshold_labels)

            helper.defineHistogram('tauTOBthree_threshold;h_tauTOBthree_threshold', title=tobStr+' Tau 3 taus threshold'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+pathFromKey(containerKey)+cut_name, xbins=4,xmin=0,xmax=4.0, xlabels=threshold_labels)

    acc = helper.result()
    result.merge(acc)
    return result


if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob

    # MCs processed adding L1_eEMRoI
    inputs = glob.glob('/eos/user/t/thompson/ATLAS/LVL1_mon/MC_ESD/l1calo.361024.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ4W.eFex_gFex_2022-01-13T2101.root')

    flags = initConfigFlags()
    flags.Input.Files = inputs
    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1_MC.root'

    flags.lock()
    flags.dump() # print all the configs

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg  
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    EfexMonitorCfg = EfexMonitoringConfig(flags)
    cfg.merge(EfexMonitorCfg)

    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=10
    cfg.run(nevents)

