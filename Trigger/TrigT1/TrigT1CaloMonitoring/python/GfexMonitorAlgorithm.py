#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def GfexMonitoringConfig(flags):
    '''Function to configure LVL1 Gfex algorithm in the monitoring system.'''
    import math
    # get the component factory - used for merging the algorithm results
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    #inputFlags.dump() # print all the configs

    # use L1Calo's special MonitoringCfgHelper
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.GfexMonitorAlgorithm,'GfexMonAlg')
    GfexMonAlg = helper.alg

    # add any steering
    groupName = 'GfexMonitor' # the monitoring group name is also used for the package name
    GfexMonAlg.PackageName = groupName

    GfexMonAlg.gFexJetTobKeyList    = ["L1_gFexLRJetRoI", "L1_gFexSRJetRoI"]
    GfexMonAlg.gFexRhoTobKeyList    = ["L1_gFexRhoRoI"]
    GfexMonAlg.gFexGlobalTobKeyList = ["L1_gScalarEJwoj", "L1_gMETComponentsJwoj", "L1_gMHTComponentsJwoj", "L1_gMSTComponentsJwoj", "L1_gMETComponentsNoiseCut", "L1_gMETComponentsRms", "L1_gScalarENoiseCut", "L1_gScalarERms"]

    # Define various quantities
    trigPath = 'Developer/Gfex/'
    globTobVarDict = {"gScalarEJwoj":["gFexMet", "gFexSumEt"], "gMETComponentsJwoj":["METx", "METy"], "gMHTComponentsJwoj":["MHTx", "MHTy"], "gMSTComponentsJwoj":["MSTx", "MSTy"], "gMETComponentsNoiseCut":["METx_NoiseCut", "METy_NoiseCut"], "gMETComponentsRms":["METx_Rms", "METy_Rms"], "gScalarENoiseCut":["gFexMet_NoiseCut", "gFexSumEt_NoiseCut"], "gScalarERms":["gFexMet_Rms", "gFexSumEt_Rms"]}
    keyDirPathMap  = {"gFexMet":"gXE/", "gFexSumEt":"gTE/", "METx":"gXE/", "METy":"gXE/", "MHTx":"gMHT/", "MHTy":"gMHT/", "MSTx":"gMST/", "MSTy":"gMST/", "METx_NoiseCut":"gXE_NoiseCut/", "METy_NoiseCut":"gXE_NoiseCut/", "METx_Rms":"gXE_RMS/", "METy_Rms":"gXE_RMS/", "gFexMet_NoiseCut":"gXE_NoiseCut/", "gFexSumEt_NoiseCut":"gTE_NoiseCut/", "gFexMet_Rms":"gTE_RMS/", "gFexSumEt_Rms":"gTE_RMS/"}
    globTobRangeDict = {"gFexMet":[0, 6e5], "gFexSumEt":[0, 5e6], "METx":[-3e5, 3e5], "METy":[-3e5, 3e5], "MHTx":[-3e5, 3e5], "MHTy":[-3e5, 3e5], "MSTx":[-2e5, 2e5], "MSTy":[-2e5, 2e5], "METx_NoiseCut":[-6e5, 6e5], "METy_NoiseCut":[-6e5, 6e5], "METx_Rms":[-6e5, 6e5], "METy_Rms":[-6e5, 6e5], "gFexMet_NoiseCut":[-1e4, 1e6], "gFexSumEt_NoiseCut":[0, 2.5e6], "gFexMet_Rms":[-1e5, 1e6], "gFexSumEt_Rms":[0, 2e6]}

    # Define gfex histograms
    ptCutValues = GfexMonAlg.ptCutValues
    gFexJetTobKeyList = GfexMonAlg.gFexJetTobKeyList
    gFexRhoTobKeyList = GfexMonAlg.gFexRhoTobKeyList
    gFexGlobalTobKeyList = GfexMonAlg.gFexGlobalTobKeyList

    # Jet TOB list
    for containerKey in gFexJetTobKeyList:
        for ptCut in ptCutValues:
            for pFPGA in ["", "FPGAa", "FPGAb", "FPGAc"]:# Empty str for inclusive

                ptCutString = "_CutPt{:.0f}".format(ptCut) if (ptCut != -1) else ""
                containerKey = containerKey.split("+")[-1] # Needed to remove storeGate prefix if gFexJetTobKeyList is not set above

                # 1D histograms
                histKey = containerKey + "{}" + ptCutString
                histKey += ";h_" + containerKey + "{}" + ptCutString
                tobTypeStr = "gFex SRJet" if "SRJet" in containerKey else "gFex LRJet"
                ptStrTitle = " - tobEt [200 MeV Scale]>{:.0f}".format(ptCut) if ptCut != -1 else ""
                jPath = "gFexSRJets" if "SRJet" in containerKey else "gFexLRJets"
                jPath += ptCutString

                helper.defineHistogram(histKey.format("Eta{}".format(pFPGA), "Eta{}".format(pFPGA)), title="{} {} #eta{}; #eta; counts".format(tobTypeStr, pFPGA, ptStrTitle), type='TH1F', fillGroup=groupName, path=trigPath+"{}/".format(jPath), xbins=32,xmin=-3.3,xmax=3.3)
                helper.defineHistogram(histKey.format("Phi{}".format(pFPGA), "Phi{}".format(pFPGA)), title="{} {} #phi{}; #phi; counts".format(tobTypeStr, pFPGA, ptStrTitle), type='TH1F', fillGroup=groupName, path=trigPath+"{}/".format(jPath), xbins=32,xmin=-math.pi,xmax=math.pi)
                helper.defineHistogram(histKey.format("Pt{}".format(pFPGA),  "Pt{}".format(pFPGA)) , title="{} {} #Pt{} ; #Pt ; counts".format(tobTypeStr, pFPGA, ptStrTitle), type='TH1F', fillGroup=groupName, path=trigPath+"{}/".format(jPath), xbins=100,xmin=-1,xmax=4096)

                # 2D histograms
                histKey = containerKey + "{}" + ptCutString
                etaKey = histKey.format("Eta{}".format(pFPGA))
                phiKey = histKey.format("Phi{}".format(pFPGA))
                helper.defineHistogram('{},{};h_{}{}'.format(etaKey, phiKey, etaKey, phiKey), title="{} {} #eta vs #phi {}; #eta; #phi".format(tobTypeStr, pFPGA, ptStrTitle), type='TH2F',fillGroup=groupName, path=trigPath+"{}/".format(jPath), xbins=32,xmin=-3.3,xmax=3.3,ybins=32,ymin=-math.pi,ymax=math.pi)

    # Rho TOB list
    for containerKey in gFexRhoTobKeyList:
        helper.defineHistogram("{};h_{}".format(containerKey, containerKey), title="{}; gFexRho Et [MeV]; counts".format(containerKey), fillGroup=groupName, type="TH1F", path=trigPath+"gRHO", xbins=100,xmin=-300000,xmax=300000)

    # Global TOB list
    for containerKey in gFexGlobalTobKeyList:
        for key, dictVal in globTobVarDict.items():
            if key in containerKey:
                varOne, varTwo = dictVal
                break

        xminOne, xmaxOne = globTobRangeDict.get(varOne, [0,1e6])
        xminTwo, xmaxTwo = globTobRangeDict.get(varTwo, [0,1e6])

        helper.defineHistogram("{};h_{}".format(varOne, varOne), title="{}; {} [MeV]; counts".format(varOne, varOne), type="TH1F", fillGroup=groupName, path=trigPath+keyDirPathMap.get(varOne, "gFexGlob/"), xbins=100,xmin=xminOne,xmax=xmaxOne)
        helper.defineHistogram("{};h_{}".format(varTwo, varTwo), title="{}; {} [MeV]; counts".format(varTwo, varTwo), type="TH1F", fillGroup=groupName, path=trigPath+keyDirPathMap.get(varTwo, "gFexGlob/"), xbins=100,xmin=xminTwo,xmax=xmaxTwo)

    acc = helper.result()
    result.merge(acc)
    return result

if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob

    # Above MCs processed adding L1_eEMRoI
    inputs = glob.glob('/afs/cern.ch/user/t/thompson/work/public/LVL1_monbatch/run_sim/l1calo.361106.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zee.ESD.eFex_2021-05-16T2101.root')
    flags = initConfigFlags()
    flags.Input.Files = inputs
    flags.Output.HISTFileName = 'ExampleMonitorOutput.root'
    flags.lock()
    flags.dump() # print all the configs

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg  
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    GfexMonitorCfg = GfexMonitoringConfig(flags)
    cfg.merge(GfexMonitorCfg)
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=100
    cfg.run(nevents)
