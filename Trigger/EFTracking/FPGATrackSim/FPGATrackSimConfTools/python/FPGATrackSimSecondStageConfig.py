# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import AthenaLogger
from PathResolver import PathResolver
import importlib
import os
import AthenaCommon.Utils.unixtools as unixtools

log = AthenaLogger(__name__)

#### Now import Data Prep config from other file
from FPGATrackSimConfTools import FPGATrackSimDataPrepConfig
from FPGATrackSimConfTools import FPGATrackSimAnalysisConfig

def getNSubregions(filePath):
    with open(PathResolver.FindCalibFile(filePath), 'r') as f:
        fields = f.readline()
        assert(fields.startswith('towers'))
        n = fields.split()[1]
        return int(n)

def FPGATrackSimBinnedHitsToolCfg_2nd(flags):
    # This can probably be imported in the future from the analysis config, but for now it's here.
    result = ComponentAccumulator()

    # The second stge, like layer study alg, technically doesn't need a cuts file.
    # So for now allow the same override here I guess?
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
    else:
        if flags.Trigger.FPGATrackSim.oldRegionDefs:
            cutset = importlib.import_module(flags.Trigger.FPGATrackSim.GenScan.genScanCuts).cuts[flags.Trigger.FPGATrackSim.region]
        else:
            # this allows the cut file defined in python to be loaded from the map directory
            # Updated to use python path resolver. It seems like we have to manually pass in CALIBPATH.
            relpath = os.path.join(flags.Trigger.FPGATrackSim.mapsDir, flags.Trigger.FPGATrackSim.GenScan.genScanCuts + ".py")
            abspath = unixtools.find_datafile(relpath, pathlist=os.getenv("CALIBPATH").split(":"))
            spec=importlib.util.spec_from_file_location("FPGATrackSimGenScanCuts", abspath)
            cutmodule = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(cutmodule)
            cutset=cutmodule.cuts[flags.Trigger.FPGATrackSim.region]
        log.info("Running layer study using configured cuts file")
        log.info(cutset)

    # make the binned hits class
    BinnnedHits = CompFactory.FPGATrackSimBinnedHits("FPGATrackSimBinnedHits_2nd")
    BinnnedHits.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionCfg(flags))

    # TODO: we need a new flag for this!
    BinnnedHits.layerMapFile = flags.Trigger.FPGATrackSim.GenScan.layerMapFile

    # make the bintool class
    BinTool = CompFactory.FPGATrackSimBinTool("FPGATrackSimBinTool_2nd")

    # Inputs for the BinTool
    binsteps=[]
    BinDesc=None
    if (cutset["parSet"]=="PhiSlicedKeyLyrPars"):
        BinDesc = CompFactory.FPGATrackSimKeyLayerBinDesc("FPGATrackSimKeyLayerBinDesc2nd")
        BinDesc.rin=cutset["rin"]
        BinDesc.rout=cutset["rout"]

        # parameters for key layer bindesc are :"zR1", "zR2", "phiR1", "phiR2", "xm"
        step1 = CompFactory.FPGATrackSimBinStep("FPGATrackSimPhiBinning_2nd")
        step1.parBins = [1,1,cutset["parBins"][2],cutset["parBins"][3],cutset["parBins"][4]]
        step2 = CompFactory.FPGATrackSimBinStep("FPGATrackSimFullBinning_2nd")
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

def FPGATrackSimWindowExtensionToolCfg(flags):
    result = ComponentAccumulator()
    FPGATrackSimWindowExtensionTool = CompFactory.FPGATrackSimWindowExtensionTool()

    # these are services so we use getPrimaryAndMerge; tools (configured elsewhere) should use popToolsAndMerge
    FPGATrackSimWindowExtensionTool.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimAnalysisConfig.FPGATrackSimBankSvcCfg(flags))
    FPGATrackSimWindowExtensionTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))

    # Hardcoded settings for now, hook up to flags later...
    FPGATrackSimWindowExtensionTool.threshold = 11

    # These MUST be of size equal to the full number of layers (13), though only the "new" layers
    # in the second stage are actually used.
    FPGATrackSimWindowExtensionTool.zWindow =   [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84., 84.]
    FPGATrackSimWindowExtensionTool.phiWindow = [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045]

    # If we're doing binning, i.e. genscan.
    if flags.Trigger.FPGATrackSim.ActiveConfig.genScan:
        FPGATrackSimWindowExtensionTool.doBinning = True
        FPGATrackSimWindowExtensionTool.BinningTool = result.getPrimaryAndMerge(FPGATrackSimBinnedHitsToolCfg_2nd(flags))

    # Other settings, shared with the first stage mostly. disable 2nd stage tracking for now.
    FPGATrackSimWindowExtensionTool.fieldCorrection =flags.Trigger.FPGATrackSim.ActiveConfig.fieldCorrection
    FPGATrackSimWindowExtensionTool.IdealGeoRoads = False # (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
    FPGATrackSimWindowExtensionTool.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints
    FPGATrackSimWindowExtensionTool.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
    result.setPrivateTools(FPGATrackSimWindowExtensionTool)
    return result

def FPGATrackSimNNPathfinderExtensionToolCfg(flags):
    result = ComponentAccumulator()
    FPGATrackSimNNPathfinderExtensionTool = CompFactory.FPGATrackSimNNPathfinderExtensionTool()

    # these are services so we use getPrimaryAndMerge; tools (configured elsewhere) should use popToolsAndMerge
    FPGATrackSimNNPathfinderExtensionTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))

    # Hardcoded settings for now, hook up to flags later...
    FPGATrackSimNNPathfinderExtensionTool.threshold = flags.Trigger.FPGATrackSim.hitThreshold
    FPGATrackSimNNPathfinderExtensionTool.windowR = flags.Trigger.FPGATrackSim.windowR
    FPGATrackSimNNPathfinderExtensionTool.windowZ = flags.Trigger.FPGATrackSim.windowZ
    FPGATrackSimNNPathfinderExtensionTool.lowPtValueWindowR = flags.Trigger.FPGATrackSim.lowPtvalueR
    FPGATrackSimNNPathfinderExtensionTool.lowPtRScaling = flags.Trigger.FPGATrackSim.lowPtWindowRScaling
    FPGATrackSimNNPathfinderExtensionTool.lowPtValueWindowZ = flags.Trigger.FPGATrackSim.lowPtvalueZ
    FPGATrackSimNNPathfinderExtensionTool.lowPtZScaling = flags.Trigger.FPGATrackSim.lowPtWindowZScaling
    FPGATrackSimNNPathfinderExtensionTool.missedHitRScaling = flags.Trigger.FPGATrackSim.missedHitRScaling
    FPGATrackSimNNPathfinderExtensionTool.missedHitZScaling = flags.Trigger.FPGATrackSim.missedHitZScaling    
    FPGATrackSimNNPathfinderExtensionTool.maxBranches = flags.Trigger.FPGATrackSim.maxBranches
    FPGATrackSimNNPathfinderExtensionTool.doOutsideIn = True
    if (flags.Trigger.FPGATrackSim.ActiveConfig.genScan):
        FPGATrackSimNNPathfinderExtensionTool.doOutsideIn = False
        FPGATrackSimNNPathfinderExtensionTool.predictionWindowLength = 4
    
    # Other settings
    FPGATrackSimNNPathfinderExtensionTool.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
    result.setPrivateTools(FPGATrackSimNNPathfinderExtensionTool)
    return result


# Need to figure out if we have two output writers or somehow only one.
def FPGATrackSimSecondStageOutputCfg(flags):
    result=ComponentAccumulator()
    FPGATrackSimWriteOutput = CompFactory.FPGATrackSimOutputHeaderTool("FPGATrackSimWriteOutputSecondStage")
    FPGATrackSimWriteOutput.InFileName = ["test.root"]
    FPGATrackSimWriteOutput.OutputTreeName = "FPGATrackSimSecondStageTree"
    # RECREATE means that that this tool opens the file.
    # HEADER would mean that something else (e.g. THistSvc) opens it and we just add the object.
    FPGATrackSimWriteOutput.RWstatus = "HEADER"
    FPGATrackSimWriteOutput.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(FPGATrackSimWriteOutput)
    return result

def FPGATrackSimHoughRootOutputToolCfg(flags):
    result=ComponentAccumulator()
    HoughRootOutputTool = CompFactory.FPGATrackSimHoughRootOutputTool()
    HoughRootOutputTool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionCfg(flags))
    HoughRootOutputTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    HoughRootOutputTool.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(HoughRootOutputTool)
    return result

def NNTrackToolCfg(flags):
    result=ComponentAccumulator()
    NNTrackTool = CompFactory.FPGATrackSimNNTrackTool()
    NNTrackTool.THistSvc = CompFactory.THistSvc()
    NNTrackTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    NNTrackTool.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimAnalysisConfig.FPGATrackSimBankSvcCfg(flags))
    NNTrackTool.IdealGeoRoads = False
    NNTrackTool.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints and not flags.Trigger.FPGATrackSim.ActiveConfig.genScan
    NNTrackTool.SPRoadFilterTool = FPGATrackSimAnalysisConfig.getSPRoadFilterTool(flags,secondStage=True)
    NNTrackTool.Do2ndStageTrackFit = True
    NNTrackTool.useSectors = False
    result.setPrivateTools(NNTrackTool)
    return result


def FPGATrackSimTrackFitterToolCfg(flags):
    result=ComponentAccumulator()
    TF = CompFactory.FPGATrackSimTrackFitterTool("FPGATrackSimTrackFitterTool_2nd")
    TF.GuessHits = flags.Trigger.FPGATrackSim.ActiveConfig.guessHits
    TF.IdealCoordFitType = flags.Trigger.FPGATrackSim.ActiveConfig.idealCoordFitType
    TF.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimAnalysisConfig.FPGATrackSimBankSvcCfg(flags))
    TF.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    TF.chi2DofRecoveryMax = flags.Trigger.FPGATrackSim.ActiveConfig.chi2DoFRecoveryMax
    TF.chi2DofRecoveryMin = flags.Trigger.FPGATrackSim.ActiveConfig.chi2DoFRecoveryMin
    TF.doMajority = flags.Trigger.FPGATrackSim.ActiveConfig.doMajority
    TF.nHits_noRecovery = flags.Trigger.FPGATrackSim.ActiveConfig.nHitsNoRecovery
    TF.DoDeltaGPhis = flags.Trigger.FPGATrackSim.ActiveConfig.doDeltaGPhis
    TF.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    TF.IdealGeoRoads = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
    TF.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints
    TF.SPRoadFilterTool = FPGATrackSimAnalysisConfig.getSPRoadFilterTool(flags,secondStage=True)
    TF.Do2ndStageTrackFit = True
    result.setPrivateTools(TF)
    return result

def FPGATrackSimOverlapRemovalToolCfg(flags):
    result=ComponentAccumulator()
    OR = CompFactory.FPGATrackSimOverlapRemovalTool("FPGATrackSimOverlapRemovalTool_2nd")
    OR.ORAlgo = "Normal"
    OR.doFastOR =flags.Trigger.FPGATrackSim.ActiveConfig.doFastOR
    OR.NumOfHitPerGrouping = 5
    OR.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    OR.MinChi2 = flags.Trigger.FPGATrackSim.ActiveConfig.secondChi2Cut
    if flags.Trigger.FPGATrackSim.ActiveConfig.hough:
        OR.nBins_x = flags.Trigger.FPGATrackSim.ActiveConfig.xBins + 2 * flags.Trigger.FPGATrackSim.ActiveConfig.xBufferBins
        OR.nBins_y = flags.Trigger.FPGATrackSim.ActiveConfig.yBins + 2 * flags.Trigger.FPGATrackSim.ActiveConfig.yBufferBins
        OR.localMaxWindowSize = flags.Trigger.FPGATrackSim.ActiveConfig.localMaxWindowSize
        OR.roadSliceOR = flags.Trigger.FPGATrackSim.ActiveConfig.roadSliceOR
    
    from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmConfig import FPGATrackSimOverlapRemovalToolMonitoringCfg
    OR.MonTool = result.getPrimaryAndMerge(FPGATrackSimOverlapRemovalToolMonitoringCfg(flags))

    result.setPrivateTools(OR)
    return result

def prepareFlagsForFPGATrackSimSecondStageAlg(flags):
    newFlags = flags.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag)
    return newFlags

def FPGATrackSimSecondStageAlgCfg(inputFlags):

    flags = prepareFlagsForFPGATrackSimSecondStageAlg(inputFlags)

    result=ComponentAccumulator()

    theFPGATrackSimSecondStageAlg=CompFactory.FPGATrackSimSecondStageAlg()
    theFPGATrackSimSecondStageAlg.writeOutputData = flags.Trigger.FPGATrackSim.writeAdditionalOutputData
    theFPGATrackSimSecondStageAlg.tracking = flags.Trigger.FPGATrackSim.secondTracking
    theFPGATrackSimSecondStageAlg.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    theFPGATrackSimSecondStageAlg.DoHoughRootOutput2nd = flags.Trigger.FPGATrackSim.ActiveConfig.houghRootoutput2nd
    theFPGATrackSimSecondStageAlg.DoNNTrack_2nd = flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis2nd
    theFPGATrackSimSecondStageAlg.eventSelector = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionCfg(flags))
    theFPGATrackSimSecondStageAlg.TrackScoreCut = flags.Trigger.FPGATrackSim.ActiveConfig.secondChi2Cut
    theFPGATrackSimSecondStageAlg.doNNPathFinder = flags.Trigger.FPGATrackSim.doNNPathFinder

    FPGATrackSimMapping = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    theFPGATrackSimSecondStageAlg.FPGATrackSimMapping = FPGATrackSimMapping
    theFPGATrackSimSecondStageAlg.passLowestChi2TrackOnly = flags.Trigger.FPGATrackSim.ActiveConfig.passLowestChi2TrackOnly
    # If tracking is set to False, don't configure the bank service
    if theFPGATrackSimSecondStageAlg.tracking and not flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis2nd:
        result.getPrimaryAndMerge(FPGATrackSimAnalysisConfig.FPGATrackSimBankSvcCfg(flags))

    # Here, configure the window tool.
    if(flags.Trigger.FPGATrackSim.doNNPathFinder ):
        theFPGATrackSimSecondStageAlg.TrackExtensionTool = result.popToolsAndMerge(FPGATrackSimNNPathfinderExtensionToolCfg(flags))
    else:
        theFPGATrackSimSecondStageAlg.TrackExtensionTool = result.popToolsAndMerge(FPGATrackSimWindowExtensionToolCfg(flags))

    theFPGATrackSimSecondStageAlg.HoughRootOutputTool = result.popToolsAndMerge(FPGATrackSimHoughRootOutputToolCfg(flags))

    theFPGATrackSimSecondStageAlg.NNTrackTool = result.popToolsAndMerge(NNTrackToolCfg(flags))

    theFPGATrackSimSecondStageAlg.OutputTool = result.popToolsAndMerge(FPGATrackSimSecondStageOutputCfg(flags))
    theFPGATrackSimSecondStageAlg.TrackFitter_2nd = result.popToolsAndMerge(FPGATrackSimTrackFitterToolCfg(flags))
    theFPGATrackSimSecondStageAlg.OverlapRemoval_2nd = result.popToolsAndMerge(FPGATrackSimOverlapRemovalToolCfg(flags))

    # Create SPRoadFilterTool if spacepoints are turned on. TODO: make things configurable?
    if flags.Trigger.FPGATrackSim.spacePoints and theFPGATrackSimSecondStageAlg.tracking:
        theFPGATrackSimSecondStageAlg.SPRoadFilterTool = FPGATrackSimAnalysisConfig.getSPRoadFilterTool(flags,secondStage=True)
        theFPGATrackSimSecondStageAlg.Spacepoints = True

    from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmConfig import FPGATrackSimSecondStageAlgMonitoringCfg
    theFPGATrackSimSecondStageAlg.MonTool = result.popToolsAndMerge(FPGATrackSimSecondStageAlgMonitoringCfg(flags))

    result.addEventAlgo(theFPGATrackSimSecondStageAlg)

    return result

if __name__ == "__main__":
    print("Running second stage separately is currently unsupported")
