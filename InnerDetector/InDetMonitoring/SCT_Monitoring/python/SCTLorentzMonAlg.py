#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file SCTLorentzMonAlg.py
@author Susumu Oda
@date 2019-04-02
@brief Based on AthenaMonitoring/ExampleMonitorAlgorithm.py
'''

def SCTLorentzMonAlgConfig(flags):
    
    ### STEP 1 ###
    # Define one top-level monitoring algorithm. The new configuration 
    # framework uses a component accumulator.
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'SCTLorentzMonCfg')


    ### STEP 2 ###
    # Adding an algorithm to the helper. Here, we will use the example 
    # algorithm in the AthenaMonitoring package. Just pass the type to the 
    # helper. Then, the helper will instantiate an instance and set up the 
    # base class configuration following the flags. The returned object 
    # is the algorithm.
    from AthenaConfiguration.ComponentFactory import CompFactory
    from InDetConfig.InDetAssociationToolsConfig import InDetPRDtoTrackMapToolGangedPixelsCfg
    from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg

    myMonAlg = helper.addAlgorithm(
        CompFactory.SCTLorentzMonAlg,
        'SCTLorentzMonAlg',
        AssociationTool = result.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)),
        TrackSummaryTool = result.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags)))

    # # If for some really obscure reason you need to instantiate an algorithm
    # # yourself, the AddAlgorithm method will still configure the base 
    # # properties and add the algorithm to the monitoring sequence.
    # helper.AddAlgorithm(myExistingAlg)


    ### STEP 3 ###
    # Edit properties of a algorithm
    myMonAlg.TriggerChain = ''
    # myMonAlg.RandomHist = True

    ### STEP 4 ###
    # Add some tools. N.B. Do not use your own trigger decion tool. Use the
    # standard one that is included with AthMonitorAlgorithm.


    # set up conditions
    from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig import SCT_DetectorElementCondAlgCfg
    result.merge(SCT_DetectorElementCondAlgCfg(flags))

    # # Then, add a tool that doesn't have its own configuration function. In
    # # this example, no accumulator is returned, so no merge is necessary.
    # from MyDomainPackage.MyDomainPackageConf import MyDomainTool
    # myMonAlg.MyDomainTool = MyDomainTool()

    # Add a generic monitoring tool (a "group" in old language). The returned 
    # object here is the standard GenericMonitoringTool.
    myMonGroup = helper.addGroup(
        myMonAlg,
        "SCTLorentzMonitor",
        "SCT/GENERAL/"
    )

    ### STEP 5 ###
    # Configure histograms
    N_BARRELS = 4
    nSides = 2 # 0: Side 0, 1: Side 1
    nSurfaces = 2 # 0: 100, 1: 111
    surfaceNames = ["_100",   "_111"]
    surfaceNames2 = ["_100_",   "_111_"]
    surfaceTitles = ["100 - ", "111 - "]
    sideNames = ["_0", "_1"]
    for l in range(N_BARRELS):
        for iSurface in range(nSurfaces):
            for side in range(nSides):
                xVar = "phiToWafer_"+str(l)+surfaceNames[iSurface]+sideNames[side]
                yVar = "nStrip_"+str(l)+surfaceNames[iSurface]+sideNames[side]
                histTitle = surfaceTitles[iSurface]+"Inc. Angle vs nStrips for Layer Side"+str(l)+str(side)
                histName = "h_phiVsNstrips"+surfaceNames2[iSurface]+str(l)+"Side"+str(side)
                myMonGroup.defineHistogram(varname=xVar+","+yVar+";"+histName, # ; means alias
                                           type="TProfile",
                                           title=histTitle+";#phi to Wafer;Num of Strips",
                                           path="lorentz", # path cannot be "".
                                           xbins=360, xmin=-90., xmax=90.)

                myMonGroup.defineHistogram(varname=xVar+","+yVar+";"+histName + "_e0p5", # ; means alias
                                           type="TProfile",
                                           title=histTitle+"(|#eta| < 0.5);#phi to Wafer;Num of Strips",
                                           path="lorentz", # path cannot be "".
                                           xbins=360, xmin=-90., xmax=90.,
                                           cutmask="isCentral")
    nEndcaps = 2
    nDisks = 9
    nSides = 2 # 0: Side 0, 1: Side 1
    nEta = 3 # outer, middle, inner
    endcapNames = ["ECA", "ECC"]
    etaNames = ["outer", "middle", "inner"]
    sideNames = ["_0", "_1"]
    for e in range(nEndcaps):
        for d in range(nDisks):
            for iEta in range(nEta):
                sensorType = "HPK"
                if ((d == 1 and iEta == 1) or 
                    (d == 3 and iEta == 1) or
                    (d == 4 and iEta >= 1) or
                    (d == 5 and iEta == 2) or
                    (d == 7 and iEta == 1)):
                    sensorType = "CiS"
                elif (d == 2 and iEta == 1):
                    sensorType = "Both"
                    
                for side in range(nSides):
                    xVar = "phiToWafer_"+endcapNames[e]+str(d)+"_"+etaNames[iEta]+sideNames[side]
                    yVar = "nStrip_"+endcapNames[e]+str(d)+"_"+etaNames[iEta]+sideNames[side]
                    histTitle = "111 (" + sensorType + ") - Inc. Angle vs nStrips for " + endcapNames[e] + " Disk Eta Side"+str(d)+str(iEta)+str(side)
                    histName = "h_phiVsNstrips_"+endcapNames[e]+str(d)+"_"+etaNames[iEta]+"_Side"+str(side)
                    myMonGroup.defineHistogram(varname=xVar+","+yVar+";"+histName, # ; means alias
                                               type="TProfile",
                                               title=histTitle+";#phi to Wafer;Num of Strips",
                                               path="lorentz", # path cannot be "".
                                               xbins=360, xmin=-90., xmax=90.)

    ### STEP 6 ###
    # Finalize. The return value should be a tuple of the ComponentAccumulator
    # and the sequence containing the created algorithms. If we haven't called
    # any configuration other than the AthMonitorCfgHelper here, then we can 
    # just return directly (and not create "result" above)
    #return helper.result()
    
    # # Otherwise, merge with result object and return
    result.merge(helper.result())
    return result
    
if __name__ == "__main__": 
    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc16_13TeV.361022.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ2W.recon.ESD.e3668_s3170_r10572_homeMade.pool.root"]
    flags.Input.isMC = True
    flags.Output.HISTFileName = 'SCTLorentzMonOutput.root'
    flags.GeoModel.Align.Dynamic = False
    flags.Detector.GeometryPixel = True
    flags.Detector.GeometrySCT = True
    flags.Detector.GeometryTRT = True
    flags.Detector.GeometryCalo = False
    flags.Detector.GeometryMuon = True
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(SCTLorentzMonAlgConfig(flags))

    cfg.run()
