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

def FPGATrackSimBinnedHitsToolCfg_2nd(flags,name="FPGATrackSimBinnedHitsTool_2nd"):
    result = ComponentAccumulator()

    # This can probably be imported in the future from the analysis config, but for now it's here.
    ##NameWithRegion = ComponentAccumulator(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))

    # The second stge, like layer study alg, technically doesn't need a cuts file.
    # So for now allow the same override here I guess?
    cutfile = flags.Trigger.FPGATrackSim.SecondStage.CutFile
    if cutfile.endswith(".py"):
        abspath = cutfile  # assume it's a full path to a .py file
    else:
        relpath = os.path.join(flags.Trigger.FPGATrackSim.mapsDir, cutfile + ".py")
        abspath = unixtools.find_datafile(relpath, pathlist=os.getenv("CALIBPATH").split(":"))

    spec = importlib.util.spec_from_file_location("secondStageCuts", abspath)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Failed to load cut module from: {abspath}")

    cutmodule = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(cutmodule)
    cutset = cutmodule.cuts[flags.Trigger.FPGATrackSim.region]
    log.info("Running layer study using configured cuts file")
    log.info(cutset)

    # make the binned hits class
    BinnnedHits = CompFactory.FPGATrackSimBinnedHits(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimBinnedHits_2nd"))
    BinnnedHits.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))

    # TODO: we need a new flag for this!
    if flags.Trigger.FPGATrackSim.SecondStage.LayerMapFile:
        BinnnedHits.layerMapFile = flags.Trigger.FPGATrackSim.SecondStage.LayerMapFile
    else:
        relpath = os.path.join(flags.Trigger.FPGATrackSim.mapsDir, f"region{flags.Trigger.FPGATrackSim.region}_lyrmap_2nd.json")
        abspath = unixtools.find_datafile(relpath, pathlist=os.getenv("CALIBPATH").split(":"))
        BinnnedHits.layerMapFile = abspath

    # make the bintool class
    BinTool = CompFactory.FPGATrackSimBinTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimBinTool_2nd"))

    # Inputs for the BinTool
    binsteps=[]
    BinDesc=None
    if (cutset["parSet"]=="PhiSlicedKeyLyrPars"):
        BinDesc = CompFactory.FPGATrackSimKeyLayerBinDesc(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimKeyLayerBinDesc2nd"))
        BinDesc.rin=cutset["rin"]
        BinDesc.rout=cutset["rout"]

        # parameters for key layer bindesc are :"zR1", "zR2", "phiR1", "phiR2", "xm"
        step1 = CompFactory.FPGATrackSimBinStep(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimPhiBinning_2nd"))
        step1.parBins = [1,1,cutset["parBins"][2],cutset["parBins"][3],cutset["parBins"][4]]
        step2 = CompFactory.FPGATrackSimBinStep(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimFullBinning_2nd"))
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

def getWindowCuts(region):
    binSize = 0.2
    side = (region >> 5) & 0x1
    etaBin = (region >> 6) & 0x1F
    etaRange = [round(binSize * etaBin, 1), round(binSize * (etaBin + 1), 1)] if side else [round(-binSize * (etaBin + 1), 1), round(-binSize * etaBin, 1)]
    abs_etaRange = tuple(round(abs(val), 1) for val in etaRange)

    # Default (very large windows)
    default_phi = [0]*5 + [0.75, 0.75, 1.5, 1.5, 3.24, 3.24, 4.5, 4.5]
    default_z   = [0]*5 + [2145, 2145, 3645, 3645, 4657.5, 4657.5, 8400, 8400]

    # phi windows (currently all same)
    eta_to_phi = {
        (0.0, 0.2): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (0.2, 0.4): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (0.4, 0.6): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (0.6, 0.8): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (0.8, 1.0): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (1.0, 1.2): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (1.2, 1.4): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (1.4, 1.6): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (1.6, 1.8): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (1.8, 2.0): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (2.0, 2.2): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (2.2, 2.4): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (2.4, 2.6): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (2.6, 2.8): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (2.8, 3.0): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (3.0, 3.2): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (3.2, 3.4): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (3.4, 3.6): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (3.6, 3.8): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
        (3.8, 4.0): [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045],
    }

    # z windows (also currently all same)
    eta_to_z = {
        (0.0, 0.2): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (0.2, 0.4): [0, 0, 0, 0, 0, 900, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (0.4, 0.6): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (0.6, 0.8): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (0.8, 1.0): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (1.0, 1.2): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (1.2, 1.4): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (1.4, 1.6): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (1.6, 1.8): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (1.8, 2.0): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (2.0, 2.2): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (2.2, 2.4): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (2.4, 2.6): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (2.6, 2.8): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (2.8, 3.0): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (3.0, 3.2): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (3.2, 3.4): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (3.4, 3.6): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (3.6, 3.8): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
        (3.8, 4.0): [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84.0, 84.0],
    }

    phi_window = eta_to_phi.get(abs_etaRange, default_phi)
    z_window = eta_to_z.get(abs_etaRange, default_z)

    return phi_window, z_window


def FPGATrackSimWindowExtensionToolCfg(flags,name="FPGATrackSimWindowExtensionTool"):
    result = ComponentAccumulator()
    FPGATrackSimWindowExtensionTool = CompFactory.FPGATrackSimWindowExtensionTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))

    # these are services so we use getPrimaryAndMerge; tools (configured elsewhere) should use popToolsAndMerge
    FPGATrackSimWindowExtensionTool.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimAnalysisConfig.FPGATrackSimBankSvcCfg(flags))
    FPGATrackSimWindowExtensionTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))

    # Hardcoded settings for now, hook up to flags later...
    # This threshold is the number of *missing* hits allowed
    FPGATrackSimWindowExtensionTool.threshold = flags.Trigger.FPGATrackSim.hitThreshold

    # These MUST be of size equal to the full number of layers (13), though only the "new" layers
    # in the second stage are actually used.
    #leaving these here for now although we remove the flag later when we have optimized windows
    FPGATrackSimWindowExtensionTool.zWindow =   [0, 0, 0, 0, 0, 21.45, 21.45, 36.45, 36.45, 46.575, 46.575, 84., 84.]
    FPGATrackSimWindowExtensionTool.phiWindow = [0, 0, 0, 0, 0, 0.0075, 0.0075, 0.015, 0.015, 0.0324, 0.0324, 0.045, 0.045]
    if flags.Trigger.FPGATrackSim.GenScan.useVaryingWindow:
        FPGATrackSimWindowExtensionTool.zWindow =   getWindowCuts(flags.Trigger.FPGATrackSim.region)[1]
        FPGATrackSimWindowExtensionTool.phiWindow = getWindowCuts(flags.Trigger.FPGATrackSim.region)[0]
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

def FPGATrackSimNNPathfinderExtensionToolCfg(flags,name="FPGATrackSimNNPathfinderExtensionTool"):
    result = ComponentAccumulator()
    FPGATrackSimNNPathfinderExtensionTool = CompFactory.FPGATrackSimNNPathfinderExtensionTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))

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
def FPGATrackSimSecondStageOutputCfg(flags,name="FPGATrackSimWriteOutputSecondStage"):
    result=ComponentAccumulator()
    FPGATrackSimWriteOutput = CompFactory.FPGATrackSimOutputHeaderTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    FPGATrackSimWriteOutput.InFileName = ["test.root"]
    FPGATrackSimWriteOutput.OutputTreeName = FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimSecondStageTree")
    # RECREATE means that that this tool opens the file.
    # HEADER would mean that something else (e.g. THistSvc) opens it and we just add the object.
    FPGATrackSimWriteOutput.RWstatus = "HEADER"
    FPGATrackSimWriteOutput.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(FPGATrackSimWriteOutput)
    return result

def FPGATrackSimHoughRootOutputToolCfg(flags,name="FPGATrackSimHoughRootOutputTool"):
    result=ComponentAccumulator()
    HoughRootOutputTool = CompFactory.FPGATrackSimHoughRootOutputTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    HoughRootOutputTool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))
    HoughRootOutputTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    HoughRootOutputTool.THistSvc = CompFactory.THistSvc()
    HoughRootOutputTool.OutputRegion = str(flags.Trigger.FPGATrackSim.region)
    result.setPrivateTools(HoughRootOutputTool)
    return result

def NNTrackToolCfg(flags,name="FPGATrackSimNNTrackTool_2nd"):
    result=ComponentAccumulator()
    NNTrackTool = CompFactory.FPGATrackSimNNTrackTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    NNTrackTool.THistSvc = CompFactory.THistSvc()
    NNTrackTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    NNTrackTool.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimAnalysisConfig.FPGATrackSimBankSvcCfg(flags))
    NNTrackTool.IdealGeoRoads = False
    NNTrackTool.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints and not flags.Trigger.FPGATrackSim.ActiveConfig.genScan
    NNTrackTool.SPRoadFilterTool = result.popToolsAndMerge(FPGATrackSimAnalysisConfig.SPRoadFilterToolCfg(flags,secondStage=True))
    NNTrackTool.Do2ndStageTrackFit = True
    NNTrackTool.useSectors = False
    result.setPrivateTools(NNTrackTool)
    return result


def FPGATrackSimTrackFitterToolCfg(flags,name="FPGATrackSimTrackFitterTool_2nd"):
    result=ComponentAccumulator()
    TF = CompFactory.FPGATrackSimTrackFitterTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
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
    TF.SPRoadFilterTool = result.popToolsAndMerge(FPGATrackSimAnalysisConfig.SPRoadFilterToolCfg(flags,secondStage=True))
    TF.Do2ndStageTrackFit = True
    result.setPrivateTools(TF)
    return result

def FPGATrackSimOverlapRemovalToolCfg(flags,name="FPGATrackSimOverlapRemovalTool_2nd"):
    result=ComponentAccumulator()
    OR = CompFactory.FPGATrackSimOverlapRemovalTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
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
    newFlags = flags.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag,keepOriginal=True)
    return newFlags

def FPGATrackSimSecondStageAlgCfg(inputFlags,name="FPGATrackSimSecondStageAlg",suffix="",**kwargs):

    flags = prepareFlagsForFPGATrackSimSecondStageAlg(inputFlags)

    result=ComponentAccumulator()

    theFPGATrackSimSecondStageAlg=CompFactory.FPGATrackSimSecondStageAlg(name=FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name),**kwargs)
    theFPGATrackSimSecondStageAlg.writeOutputData = flags.Trigger.FPGATrackSim.writeAdditionalOutputData
    theFPGATrackSimSecondStageAlg.tracking = flags.Trigger.FPGATrackSim.secondTracking
    theFPGATrackSimSecondStageAlg.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    theFPGATrackSimSecondStageAlg.DoHoughRootOutput2nd = flags.Trigger.FPGATrackSim.ActiveConfig.houghRootoutput2nd
    theFPGATrackSimSecondStageAlg.DoNNTrack_2nd = flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis2nd
    theFPGATrackSimSecondStageAlg.eventSelector = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))
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

    theFPGATrackSimSecondStageAlg.Spacepoints = flags.Trigger.FPGATrackSim.spacePoints

    from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmConfig import FPGATrackSimSecondStageAlgMonitoringCfg
    theFPGATrackSimSecondStageAlg.MonTool = result.popToolsAndMerge(FPGATrackSimSecondStageAlgMonitoringCfg(flags))

    result.addEventAlgo(theFPGATrackSimSecondStageAlg)

    return result

if __name__ == "__main__":
    print("Running second stage separately is currently unsupported")
