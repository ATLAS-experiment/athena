# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging


import AthenaCommon.Utils.unixtools as unixtools
import importlib
import os
from FPGATrackSimConfTools.FPGATrackSimAnalysisConfig import ConfigureMultiRegionFlags
from FPGATrackSimConfTools.FPGATrackSimSecondStageConfig import getPadding

log = logging.getLogger ('FPGATrackSim')

#### Now inmport Data Prep config from other file
from FPGATrackSimConfTools import FPGATrackSimDataPrepConfig
from FPGATrackSimConfTools import FPGATrackSimAnalysisConfig

def FPGATrackSimBinnedHitsToolCfg(flags):
    # This can probably be imported in the future from the analysis config, but for now it's here.
    result = ComponentAccumulator()
    log.info("Setting binning parameters")

    # Allow the initial set of cuts to be read in via config flags, instead of the cuts file.
    # This effectively eliminates the need to make a "step 0" cut file.
    # If not set then default to loading the cuts file.
    if flags.Trigger.FPGATrackSim.GenScan.initialLayerStudy:
        cutset = {"rin": flags.Trigger.FPGATrackSim.GenScan.rin,
                  "rout": flags.Trigger.FPGATrackSim.GenScan.rout,
                  "parBins": flags.Trigger.FPGATrackSim.GenScan.parBins,
                  "parMin": flags.Trigger.FPGATrackSim.GenScan.parMin,
                  "parMax": flags.Trigger.FPGATrackSim.GenScan.parMax,
                  "parSet": flags.Trigger.FPGATrackSim.GenScan.parSet
                  }
        log.info("Running initial layer study, taking FPGATrackSimBinning cuts from flags")
        log.info(cutset)
    elif flags.Trigger.FPGATrackSim.GenScan.layerStudyCutFile:
        if flags.Trigger.FPGATrackSim.oldRegionDefs:
            cutset = importlib.import_module(flags.Trigger.FPGATrackSim.GenScan.layerStudyCutFile).cuts[flags.Trigger.FPGATrackSim.region]
        else:
            # this allows the cut file defined in python to be loaded from the map directory
            # Updated to use python path resolver. It seems like we have to manually pass in CALIBPATH.
            relpath = os.path.join(flags.Trigger.FPGATrackSim.mapsDir, flags.Trigger.FPGATrackSim.GenScan.layerStudyCutFile + ".py")
            abspath = unixtools.find_datafile(relpath, pathlist=os.getenv("CALIBPATH").split(":"))
            spec=importlib.util.spec_from_file_location("FPGATrackSimGenScanCuts", abspath)
            if spec is None:
                log.fatal("Failed to load cuts file")
            cutmodule = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(cutmodule)
            cutset=cutmodule.cuts[flags.Trigger.FPGATrackSim.region]
        log.info("Running layer study using configured cuts file")
        log.info(cutset)
    else:
        log.fatal("Must either set initialLayerStudy=True or set layerStudyCutFile to the name of a cut file!")

    # make the binned hits class
    BinnnedHits = CompFactory.FPGATrackSimBinnedHits("BinnedHits_LayerStudy")
    BinnnedHits.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
    BinnnedHits.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))

    # make the bintool class
    BinTool = CompFactory.FPGATrackSimBinTool("BinTool_LayerStudy")
    BinTool.OutputLevel=flags.Trigger.FPGATrackSim.loglevel

    # Inputs for the BinTool
    binsteps=[]
    BinDesc=None
    if (cutset["parSet"]=="PhiSlicedKeyLyrPars"):
        BinDesc = CompFactory.FPGATrackSimKeyLayerBinDesc("KeyLayerBinDescLayerStudy")
        BinDesc.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
        BinDesc.rin=cutset["rin"]
        BinDesc.rout=cutset["rout"]


        #resolution padding
        BinDesc.D0Pad=getPadding(flags.Trigger.FPGATrackSim.region)["d0"]
        BinDesc.EtaPad=getPadding(flags.Trigger.FPGATrackSim.region)["eta"]
        BinDesc.QPtPad=getPadding(flags.Trigger.FPGATrackSim.region)["qpt"]
        BinDesc.PhiPad=getPadding(flags.Trigger.FPGATrackSim.region)["phi"]
        BinDesc.Z0Pad=getPadding(flags.Trigger.FPGATrackSim.region)["z0"]
        BinDesc.fieldCorrection=True
        BinDesc.fieldCorRegion=flags.Trigger.FPGATrackSim.region

        # parameters for key layer bindesc are :"zR1", "zR2", "phiR1", "phiR2", "xm"
        step1 = CompFactory.FPGATrackSimBinStep("PhiBinning")
        step1.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
        step1.parBins = [1,1,cutset["parBins"][2],cutset["parBins"][3],cutset["parBins"][4]]
        step2 = CompFactory.FPGATrackSimBinStep("FullBinning")
        step2.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
        step2.parBins = cutset["parBins"]
        binsteps = [step1,step2]
    else:
        log.fatal("Unknown Binning Setup: ",cutset["parSet"])

    BinTool.BinDesc = BinDesc
    BinTool.Steps = binsteps


    # configure the padding around the nominal region
    BinTool.d0FractionalPadding =0.05
    BinTool.z0FractionalPadding =0.05
    BinTool.etaFractionalPadding =0.05
    BinTool.phiFractionalPadding =0.05
    BinTool.qOverPtFractionalPadding =0.05
    BinTool.parMin = cutset["parMin"]
    BinTool.parMax = cutset["parMax"]
    BinnnedHits.BinTool = BinTool

    result.setPrivateTools(BinnnedHits)

    return result

def FPGATrackSimLayerStudyToolCfg(flags):
    result=ComponentAccumulator()

    Monitor = CompFactory.FPGATrackSimLayerStudyTool("BinMonitoring")
    Monitor.THistSvc = CompFactory.THistSvc()
    Monitor.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
    Monitor.phiScale = 10.0
    Monitor.etaScale = 100.0
    Monitor.drScale = 20.0
    Monitor.plotAllBins = False

    Monitor.D0Pad=getPadding(flags.Trigger.FPGATrackSim.region)["d0"]
    Monitor.EtaPad=getPadding(flags.Trigger.FPGATrackSim.region)["eta"]
    Monitor.QPtPad=getPadding(flags.Trigger.FPGATrackSim.region)["qpt"]
    Monitor.PhiPad=getPadding(flags.Trigger.FPGATrackSim.region)["phi"]
    Monitor.Z0Pad=getPadding(flags.Trigger.FPGATrackSim.region)["z0"]

    result.setPrivateTools(Monitor)
    return result

def FPGATrackSimLayerStudyCfg(inputFlags):

    flags = FPGATrackSimAnalysisConfig.prepareFlagsForFPGATrackSimLogicalHitsProcessAlg(inputFlags)

    result=ComponentAccumulator()
    if not flags.Trigger.FPGATrackSim.wrapperFileName:
        from InDetConfig.InDetPrepRawDataFormationConfig import AthenaTrkClusterizationCfg
        result.merge(AthenaTrkClusterizationCfg(flags))

    theFPGATrackSimLayerStudyAlg = CompFactory.FPGATrackSimLayerStudyAlg()

    theFPGATrackSimLayerStudyAlg.threshold = flags.Trigger.FPGATrackSim.ActiveConfig.threshold[0]
    theFPGATrackSimLayerStudyAlg.stage = flags.Trigger.FPGATrackSim.layerStudyStage

    theFPGATrackSimLayerStudyAlg.eventSelector = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))
    theFPGATrackSimLayerStudyAlg.FPGATrackSimMapping = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))

    theFPGATrackSimLayerStudyAlg.BinningTool = result.getPrimaryAndMerge(FPGATrackSimBinnedHitsToolCfg(flags))
    theFPGATrackSimLayerStudyAlg.BinMonitoringTool = result.getPrimaryAndMerge(FPGATrackSimLayerStudyToolCfg(flags))

    result.addEventAlgo(theFPGATrackSimLayerStudyAlg)

    return result

if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg


    flags = initConfigFlags()

    ############################################
    # Flags used in the prototrack chain
    FinalProtoTrackChainxAODTracksKey="FPGA"
    flags.Detector.EnableCalo = False

    # ensure that the xAOD SP and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True
    from ActsConfig.ActsCIFlags import actsLegacyWorkflowFlags
    actsLegacyWorkflowFlags(flags)
    flags.Acts.doRotCorrection = False

    ############################################
    flags.Concurrency.NumThreads=1
    #flags.Concurrency.NumProcs=0
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True
    flags.Debug.DumpEvtStore=False

    # flags.Exec.DebugStage="exec" # useful option to debug the execution of the job - we want it commented out for production
    flags.fillFromArgs()
    ConfigureMultiRegionFlags(flags)


    if isinstance(flags.Trigger.FPGATrackSim.wrapperFileName, str):
        log.info("wrapperFile is string, converting to list")
        flags.Trigger.FPGATrackSim.wrapperFileName = [flags.Trigger.FPGATrackSim.wrapperFileName]
        flags.Input.Files = lambda f: [f.Trigger.FPGATrackSim.wrapperFileName]

    from FPGATrackSimConfTools.FPGATrackSimAnalysisConfig import ConfigureMultiRegionFlags
    ConfigureMultiRegionFlags(flags)

    # The region map needs to not be loaded when running layer study; we set this here to
    # guarantee it propagates consistently to all code that tries to set up the mapping service.
    flags.Trigger.FPGATrackSim.loadRegionMap = False
    flags.Trigger.FPGATrackSim.loadRadii = False

    # We also don't want to load any of the ONNX files, so set them to the empty string.
    # Again, override the user.
    flags.Trigger.FPGATrackSim.FakeNNonnxFile1st = ""
    flags.Trigger.FPGATrackSim.FakeNNonnxFile2nd = ""
    flags.Trigger.FPGATrackSim.ParamNNonnxFile1st = ""
    flags.Trigger.FPGATrackSim.ParamNNonnxFile2nd = ""
    flags.Trigger.FPGATrackSim.ExtensionNNVolonnxFile = ""
    flags.Trigger.FPGATrackSim.ExtensionNNHitonnxFile = ""

    flags.lock()
    flags.dump()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig","Tracking.MainPass")
    acc=MainServicesCfg(flags)


    if flags.Trigger.FPGATrackSim.writeAdditionalOutputData:
        acc.addService(CompFactory.THistSvc(Output = ["EXPERT DATAFILE='monitoring.root', OPT='RECREATE'"]))


    # Always produce the "genscan" layer study tree.
    if (flags.Trigger.FPGATrackSim.Hough.genScan):
        acc.addService(CompFactory.THistSvc(Output = ["GENSCAN DATAFILE='genscan.root', OPT='RECREATE'"]))

    if not flags.Trigger.FPGATrackSim.wrapperFileName:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        acc.merge(PoolReadCfg(flags))

        if flags.Input.isMC:
            from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
            acc.merge(GEN_AOD2xAODCfg(flags))

            from JetRecConfig.JetRecoSteering import addTruthPileupJetsToOutputCfg # TO DO: check if this is indeed necessary for pileup samples
            acc.merge(addTruthPileupJetsToOutputCfg(flags))

        if flags.Detector.EnableCalo:
            from CaloRec.CaloRecoConfig import CaloRecoCfg
            acc.merge(CaloRecoCfg(flags))

        if flags.Tracking.recoChain:
            from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
            acc.merge(InDetTrackRecoCfg(flags))

    # Configure both the dataprep and logical hits algorithms.
    acc.merge(FPGATrackSimDataPrepConfig.FPGATrackSimDataPrepAlgCfg(flags))
    acc.merge(FPGATrackSimLayerStudyCfg(flags))


    acc.store(open('AnalysisConfig.pkl','wb'))
    acc.foreach_component("*FPGATrackSim*").OutputLevel=flags.Trigger.FPGATrackSim.loglevel
    if flags.Trigger.FPGATrackSim.msgLimit!=-1:
        acc.getService("MessageSvc").debugLimit = flags.Trigger.FPGATrackSim.msgLimit
        acc.getService("MessageSvc").infoLimit = flags.Trigger.FPGATrackSim.msgLimit

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess() is True, "Application execution did not succeed"
