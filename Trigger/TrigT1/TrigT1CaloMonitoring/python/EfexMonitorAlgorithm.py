#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def EfexMonitoringConfig(inputFlags):
    '''Function to configure LVL1 Efex algorithm in the monitoring system.'''

    # get the component factory - used for merging the algorithm results
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # uncomment if you want to see all the flags
    #inputFlags.dump() # print all the configs

    # make the athena monitoring helper
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'EfexMonitoringCfg')

    # add algorithm to the helper
    EfexMonAlg = helper.addAlgorithm(CompFactory.EfexMonitorAlgorithm,'EfexMonAlg')

    # add any steering
    baseGroupName = 'EfexMonitor' # the monitoring group name is also used for the package name
    EfexMonAlg.PackageName = baseGroupName
    EfexMonAlg.LowPtCut = 0.0
    EfexMonAlg.HiPtCut = 15000.0
    EfexMonAlg.eFexEMTobKeyList = ['L1_eEMRoI', 'L1_eEMxRoI']
    EfexMonAlg.eFexTauTobKeyList = ['L1_eTauRoI', 'L1_eTauxRoI']

    acc = helper.result()
    result.merge(acc)
    return result


def EfexMonitoringHistConfig(flags, eFexAlg):
    """
    The histogram binning depends on the algorithm settings, configure separately
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
    trigPath = 'Developer/Efex/' # Directory trigger path for output histos
    # Map from the key name to the output directory substructure.
    keyDirPathMap = {'L1_eEMRoI' : 'TOB/eEM',
                     'L1_eEMxRoI' : 'xTOB/eEM', 'L1_eEMRoISim' : 'TOBSim/eEM', 'L1_eEMxRoISim' : 'xTOBSim/eEM',
                     'L1_eTauRoI' : 'TOB/eTau', 
                     'L1_eTauxRoI' : 'xTOB/eTau', 'L1_eTauRoISim' : 'TOBSim/eTau', 
                     'L1_eTauxRoISim' : 'xTOBSim/eTau'} 
    cut_names = ["LowPtCut", "HiPtCut"] # List of cut names, for looping over to separate out histograms into directories
    cut_vals = [EfexMonAlg.LowPtCut, EfexMonAlg.HiPtCut] # List of values, for looping over to add to histogram titles

    # First, define the histograms with no Pt cut
    # add monitoring algorithm to group, with group name and main directory

    locIdxs = []
    for phiOct in range(0,8):
        for etaIdx in range(-25,25):
            locIdxs += [str(phiOct) + ":" + str(etaIdx)]

    for containerKey in EfexMonAlg.eFexEMTobKeyList:
        tobTypeStr = "xTOB" if ('xRoI' in containerKey) else "TOB"
        tobTypeStr += "copy" if 'Copy' in containerKey else ""
        simStr = "Sim" if ('Sim' in containerKey) else ""
        tobStr = tobTypeStr + simStr
        helper.defineHistogram(containerKey + '_nEMTOBs_nocut;h_nEmTOBs_nocut', title='Number of eFex EM '+tobStr+'s;Number of EM '+tobStr+'s;Events',
                               fillGroup = baseGroupName,
                                type='TH1I', path=trigPath+tobStr, xbins=100,xmin=-0.5,xmax=99.5)
        helper.defineHistogram(containerKey + "_seedMax;h_emTob_seedMax", title='EM TOB Seedmax bit;Seedmax bit;Number of TOBs',
                               fillGroup = baseGroupName,
                                   type='TH1I',path=trigPath+tobStr, xbins=2,xmin=-0.5,xmax=1.5)

        helper.defineHistogram("LBN,locIdx,tobEt;h_"+containerKey+"_et_posLbnMap", title = "Average " + containerKey + " ET;LBN;Position (Octant:Eta)",
                            fillGroup = baseGroupName + "_" + containerKey,
                               type="TProfile2D",
                               xbins=1,xmin=0,xmax=1, ylabels=locIdxs, opt=['kAddBinsDynamically'])

    for containerKey in EfexMonAlg.eFexTauTobKeyList:
        tobTypeStr = "xTOB" if ('xRoI' in containerKey) else "TOB"
        tobTypeStr += "copy" if 'Copy' in containerKey else ""
        simStr = "Sim" if ('Sim' in containerKey) else ""
        tobStr = tobTypeStr + simStr
        helper.defineHistogram(containerKey + '_nTauTOBs_nocut;h_nTauTOBs_nocut', title='Number of eFex Tau '+tobStr+'s;Number of Tau '+tobStr+'s;Events',
                               fillGroup = baseGroupName,
                                type='TH1I', path=trigPath+tobStr, xbins=100,xmin=-0.5,xmax=99.5)
        helper.defineHistogram(containerKey + "_seedMax;h_tauTob_seedMax", title='Tau TOB SeedMax;Seedmax bit;Number of TOBs',
                               fillGroup = baseGroupName,
                                   type='TH1I',path=trigPath+tobStr, xbins=2,xmin=-0.5,xmax=1.5)

        helper.defineHistogram("LBN,locIdx,tobEt;h_"+containerKey+"_et_posLbnMap", title = "Average " + containerKey + " ET;LBN;Position (Octant:Eta)",
                               fillGroup = baseGroupName + "_" + containerKey,
                               type="TProfile2D",
                               xbins=1,xmin=0,xmax=1, ylabels=locIdxs, opt=['kAddBinsDynamically'])

    # Now define the histograms with low/hi Pt cut
    for cut_name, cut_val in zip(cut_names, cut_vals):
        cut_title_addition = '' if (cut_val == 0.0) else ' (Et>' + '%.1f'%(cut_val/1000) + 'GeV cut)'
        # Em first
        for containerKey in EfexMonAlg.eFexEMTobKeyList:
            fillGroup = baseGroupName+'_'+containerKey+'_'+cut_name
            tobTypeStr = "xTOB" if ('xRoI' in containerKey) else "TOB"
            simStr = "Sim" if ('Sim' in containerKey) else ""
            tobStr = tobTypeStr + simStr
            # histograms of eEM variables
            helper.defineHistogram('nEMTOBs;h_nEmTOBs', title='Number of eFex EM '+tobStr+'s'+cut_title_addition+';EM '+tobStr+'s;Number of EM '+tobStr+'s',
                                   fillGroup=fillGroup,
                                    type='TH1I', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=10,xmin=0,xmax=10)

            helper.defineHistogram('TOBTransverseEnergy;h_TOBTransverseEnergy', title='eFex '+tobStr+' EM Transverse Energy [MeV]'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=100,xmin=0,xmax=50000)

            helper.defineHistogram('TOBEta;h_TOBEta', title='eFex '+tobStr+' EM Eta'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=50,xmin=-2.5,xmax=2.5)

            helper.defineHistogram('TOBPhi;h_TOBPhi', title='eFex '+tobStr+' EM Phi'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=64,xmin=-math.pi,xmax=math.pi)

            helper.defineHistogram('TOBEta,TOBPhi;h_TOBEtaPhiMap', title='eFex '+tobStr+' EM Eta vs Phi'+cut_title_addition+';#eta;#phi',
                                   fillGroup=fillGroup,
                                    type='TH2F',path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)

            helper.defineHistogram('TOBshelfNumber;h_TOBshelfNumber', title='eFex '+tobStr+' EM Shelf Number'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=2,xmin=0,xmax=2)

            helper.defineHistogram('TOBeFEXNumberSh0;h_TOBeFEXNumberShelf0', title='eFex '+tobStr+' EM Module Number Shelf 0'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=12,xmin=0,xmax=12)

            helper.defineHistogram('TOBeFEXNumberSh1;h_TOBeFEXNumberShelf1', title='eFex '+tobStr+' EM Module Number Shelf 1'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=12,xmin=0,xmax=12)

            helper.defineHistogram('TOBfpga;h_TOBfpga', title='eFex '+tobStr+' EM FPGA'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=4,xmin=0,xmax=4)

            helper.defineHistogram('TOBReta;h_TOBReta', title='eFex '+tobStr+' EM Reta'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name,xbins=250,xmin=0,xmax=1)

            helper.defineHistogram('TOBRhad;h_TOBRhad', title='eFex '+tobStr+' EM Rhad'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=250,xmin=0,xmax=1)

            helper.defineHistogram('TOBWstot;h_TOBWstot', title='eFex '+tobStr+' EM Wstot'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=250,xmin=0,xmax=1)

            threshold_labels = ['fail','loose','medium','tight']
            helper.defineHistogram('TOBReta_threshold;h_TOBReta_threshold', title='eFex '+tobStr+' EM Reta threshold'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name,xbins=4,xmin=0,xmax=4.0,xlabels=threshold_labels)

            helper.defineHistogram('TOBRhad_threshold;h_TOBRhad_threshold', title='eFex '+tobStr+' EM Rhad threshold'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=4,xmin=0,xmax=4.0,xlabels=threshold_labels)

            helper.defineHistogram('TOBWstot_threshold;h_TOBWstot_threshold', title='eFex '+tobStr+' EM Wstot threshold'+cut_title_addition,
                                   fillGroup=fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=4,xmin=0,xmax=4.0,xlabels=threshold_labels)

        # Now Tau
        for containerKey in EfexMonAlg.eFexTauTobKeyList:
            fillGroup = baseGroupName+'_'+containerKey+'_'+cut_name
            tobTypeStr = "xTOB" if ('xRoI' in containerKey) else "TOB"
            simStr = "Sim" if ('Sim' in containerKey) else ""
            tobStr = tobTypeStr + simStr
            # plotting of eTau variables
            helper.defineHistogram('nTauTOBs;h_nTauTOBs', title='Number of eFex Tau '+tobStr+'s'+cut_title_addition+';Tau '+tobStr+'s;Number of Tau '+tobStr+'s',
                                    fillGroup = fillGroup,
                                    type='TH1I', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=10,xmin=0,xmax=10)

            helper.defineHistogram('tauTOBTransverseEnergy;h_tauTOBTransverseEnergy', title='eFex '+tobStr+' Tau Transverse Energy [MeV]'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=100,xmin=0,xmax=50000)

            helper.defineHistogram('tauTOBEta;h_tauTOBEta', title='eFex '+tobStr+' Tau Eta'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=60,xmin=-2.5,xmax=2.5)

            helper.defineHistogram('tauTOBPhi;h_tauTOBPhi', title='eFex '+tobStr+' Tau Phi'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=100,xmin=-math.pi,xmax=math.pi)

            helper.defineHistogram('tauTOBEta,tauTOBPhi;h_tauTOBEtaPhiMap', title='eFex '+tobStr+' Tau Eta vs Phi'+cut_title_addition+';#eta;#phi',
                                    fillGroup = fillGroup,
                                    type='TH2F',path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)

            helper.defineHistogram('tauTOBshelfNumber;h_tauTOBshelfNumber', title='eFex '+tobStr+' Tau Shelf Number'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=2,xmin=0,xmax=2)

            helper.defineHistogram('tauTOBeFEXNumberSh0;h_tauTOBeFEXNumberShelf0', title='eFex '+tobStr+' Tau Module Number Shelf 0'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=12,xmin=0,xmax=12)

            helper.defineHistogram('tauTOBeFEXNumberSh1;h_tauTOBeFEXNumberShelf1', title='eFex '+tobStr+' Tau Module Number Shelf 1'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=12,xmin=0,xmax=12)


            helper.defineHistogram('tauTOBfpga;h_tauTOBfpga', title='eFex '+tobStr+' Tau FPGA'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=4,xmin=0,xmax=4)

            helper.defineHistogram('tauTOBRcore;h_tauTOBRcore', title='eFex '+tobStr+' Tau rCore'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=250,xmin=0,xmax=1)

            helper.defineHistogram('tauTOBRhad;h_tauTOBRhad', title='eFex '+tobStr+' Tau rHad'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=250,xmin=0,xmax=1)

            helper.defineHistogram('tauTOBRcore_threshold;h_tauTOBRcore_threshold', title='eFex '+tobStr+' Tau rCore threshold'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=4,xmin=0,xmax=4.0, xlabels=threshold_labels)

            helper.defineHistogram('tauTOBRhad_threshold;h_tauTOBRhad_threshold', title='eFex '+tobStr+' Tau rHad threshold'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=4,xmin=0,xmax=4.0, xlabels=threshold_labels)

            helper.defineHistogram('tauTOBthree_threshold;h_tauTOBthree_threshold', title='eFex '+tobStr+' Tau 3 taus threshold'+cut_title_addition,
                                    fillGroup = fillGroup,
                                    type='TH1F', path=trigPath+keyDirPathMap[containerKey]+cut_name, xbins=4,xmin=0,xmax=4.0, xlabels=threshold_labels)

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

