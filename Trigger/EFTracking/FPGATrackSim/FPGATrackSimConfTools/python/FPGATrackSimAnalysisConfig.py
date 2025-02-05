# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import AthenaLogger
from PathResolver import PathResolver
import importlib

log = AthenaLogger(__name__)

#### Now inmport Data Prep config from other file
from FPGATrackSimConfTools import FPGATrackSimDataPrepConfig
from FPGATrackSimConfTools import FPGATrackSimSecondStageConfig

def getNSubregions(filePath):
    with open(PathResolver.FindCalibFile(filePath), 'r') as f:
        fields = f.readline()
        assert(fields.startswith('towers'))
        n = fields.split()[1]
        return int(n)
    

# Need to figure out if we have two output writers or somehow only one.
def FPGATrackSimWriteOutputCfg(flags):
    result=ComponentAccumulator()
    FPGATrackSimWriteOutput = CompFactory.FPGATrackSimOutputHeaderTool("FPGATrackSimWriteOutput")
    FPGATrackSimWriteOutput.InFileName = ["test.root"]
    # RECREATE means that that this tool opens the file.
    # HEADER would mean that something else (e.g. THistSvc) opens it and we just add the object.
    FPGATrackSimWriteOutput.RWstatus = "HEADER"
    FPGATrackSimWriteOutput.THistSvc = CompFactory.THistSvc()
    result.addPublicTool(FPGATrackSimWriteOutput, primary=True)
    return result



def FPGATrackSimBankSvcCfg(flags):
    result=ComponentAccumulator()
    FPGATrackSimBankSvc = CompFactory.FPGATrackSimBankSvc()
    pathBankSvc = flags.Trigger.FPGATrackSim.bankDir if flags.Trigger.FPGATrackSim.bankDir != '' else f'/eos/atlas/atlascerngroupdisk/det-htt/HTTsim/{flags.GeoModel.AtlasVersion}/21.9.16/'+FPGATrackSimDataPrepConfig.getBaseName(flags)+'/SectorBanks/'
    pathBankSvc=PathResolver.FindCalibDirectory(pathBankSvc)
    FPGATrackSimBankSvc.constantsNoGuess_1st = [
        f'{pathBankSvc}corrgen_raw_8L_skipPlane0.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane1.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane2.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane3.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane4.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane5.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane6.gcon', 
        f'{pathBankSvc}corrgen_raw_8L_skipPlane7.gcon']
    FPGATrackSimBankSvc.constantsNoGuess_2nd = [
        f'{pathBankSvc}corrgen_raw_13L_skipPlane0.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane1.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane2.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane3.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane4.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane5.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane6.gcon', 
        f'{pathBankSvc}corrgen_raw_13L_skipPlane7.gcon']
    layers="5L" if flags.Trigger.FPGATrackSim.ActiveConfig.genScan else "9L"
    FPGATrackSimBankSvc.constants_1st = f'{pathBankSvc}corrgen_raw_{layers}_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.gcon'
    FPGATrackSimBankSvc.constants_2nd = f'{pathBankSvc}corrgen_raw_13L_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.gcon'
    FPGATrackSimBankSvc.sectorBank_1st = f'{pathBankSvc}sectorsHW_raw_{layers}_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.patt'
    FPGATrackSimBankSvc.sectorBank_2nd = f'{pathBankSvc}sectorsHW_raw_13L_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.patt'
    FPGATrackSimBankSvc.sectorSlices = f'{pathBankSvc}slices_{layers}_reg{flags.Trigger.FPGATrackSim.region}.root'
    FPGATrackSimBankSvc.phiShift = flags.Trigger.FPGATrackSim.phiShift

    # These should be configurable. The tag system needs updating though.
    FPGATrackSimBankSvc.sectorQPtBins = [-0.001, -0.0005, 0, 0.0005, 0.001]
    FPGATrackSimBankSvc.qptAbsBinning = False

    result.addService(FPGATrackSimBankSvc, create=True, primary=True)
    return result


def FPGATrackSimRoadUnionToolCfg(flags):
    result=ComponentAccumulator()
    RF = CompFactory.FPGATrackSimRoadUnionTool()
    
    xBins = flags.Trigger.FPGATrackSim.ActiveConfig.xBins
    xBufferBins = flags.Trigger.FPGATrackSim.ActiveConfig.xBufferBins
    yBins = flags.Trigger.FPGATrackSim.ActiveConfig.yBins
    yBufferBins = flags.Trigger.FPGATrackSim.ActiveConfig.yBufferBins
    yMin = flags.Trigger.FPGATrackSim.ActiveConfig.qptMin
    yMax = flags.Trigger.FPGATrackSim.ActiveConfig.qptMax
    xMin = flags.Trigger.FPGATrackSim.ActiveConfig.phiMin
    xMax = flags.Trigger.FPGATrackSim.ActiveConfig.phiMax
    if (not flags.Trigger.FPGATrackSim.oldRegionDefs): ### auto-configure this
        phiRange = FPGATrackSimDataPrepConfig.getPhiRange(flags)
        xMin = phiRange[0]
        xMax = phiRange[1]

    xBuffer = (xMax - xMin) / xBins * xBufferBins
    xMin = xMin - xBuffer
    xMax = xMax +  xBuffer
    yBuffer = (yMax - yMin) / yBins * yBufferBins
    yMin -= yBuffer
    yMax += yBuffer
    tools = []
    
    FPGATrackSimMapping = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    for number in range(getNSubregions(FPGATrackSimMapping.subrmap)): 
        HoughTransform = CompFactory.FPGATrackSimHoughTransformTool("HoughTransform_0_" + str(number))
        HoughTransform.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionCfg(flags))
        HoughTransform.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
        HoughTransform.FPGATrackSimMappingSvc = FPGATrackSimMapping 
        HoughTransform.combine_layers = flags.Trigger.FPGATrackSim.ActiveConfig.combineLayers 
        HoughTransform.convSize_x = flags.Trigger.FPGATrackSim.ActiveConfig.convSizeX 
        HoughTransform.convSize_y = flags.Trigger.FPGATrackSim.ActiveConfig.convSizeY 
        HoughTransform.convolution = flags.Trigger.FPGATrackSim.ActiveConfig.convolution 
        HoughTransform.d0_max = 0 
        HoughTransform.d0_min = 0 
        HoughTransform.fieldCorrection = flags.Trigger.FPGATrackSim.ActiveConfig.fieldCorrection
        HoughTransform.hitExtend_x = flags.Trigger.FPGATrackSim.ActiveConfig.hitExtendX
        HoughTransform.localMaxWindowSize = flags.Trigger.FPGATrackSim.ActiveConfig.localMaxWindowSize        
        HoughTransform.nBins_x = xBins + 2 * xBufferBins
        HoughTransform.nBins_y = yBins + 2 * yBufferBins
        HoughTransform.phi_max = xMax
        HoughTransform.phi_min = xMin
        HoughTransform.qpT_max = yMax 
        HoughTransform.qpT_min = yMin 
        HoughTransform.scale = flags.Trigger.FPGATrackSim.ActiveConfig.scale
        HoughTransform.subRegion = number
        HoughTransform.threshold = flags.Trigger.FPGATrackSim.ActiveConfig.threshold
        HoughTransform.traceHits = True
        HoughTransform.IdealGeoRoads = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
        HoughTransform.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints

        tools.append(HoughTransform)

    RF.tools = tools
    result.addPublicTool(RF, primary=True)
    return result

def FPGATrackSimRoadUnionTool1DCfg(flags):
    result=ComponentAccumulator()
    tools = []
    RF = CompFactory.FPGATrackSimRoadUnionTool()
    splitpt=flags.Trigger.FPGATrackSim.Hough1D.splitpt
    FPGATrackSimMapping = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    for ptstep in range(splitpt):
        qpt_min = flags.Trigger.FPGATrackSim.Hough1D.qptMin
        qpt_max = flags.Trigger.FPGATrackSim.Hough1D.qptMax
        lowpt = qpt_min + (qpt_max-qpt_min)/splitpt*ptstep
        highpt = qpt_min + (qpt_max-qpt_min)/splitpt*(ptstep+1)
        nSlice = getNSubregions(FPGATrackSimMapping.subrmap)
        for iSlice in range(nSlice):
            tool = CompFactory.FPGATrackSimHough1DShiftTool("Hough1DShift" + str(iSlice)+(("_pt{}".format(ptstep))  if splitpt>1 else ""))
            tool.subRegion = iSlice if nSlice > 1 else -1
            xMin = flags.Trigger.FPGATrackSim.Hough1D.phiMin
            xMax = flags.Trigger.FPGATrackSim.Hough1D.phiMax
            if (not flags.Trigger.FPGATrackSim.oldRegionDefs): ### auto-configure this
                phiRange = FPGATrackSimDataPrepConfig.getPhiRange(flags)
                xMin = phiRange[0]
                xMax = phiRange[1]
            tool.phiMin = xMin
            tool.phiMax = xMax
            tool.qptMin = lowpt
            tool.qptMax = highpt
            tool.nBins = flags.Trigger.FPGATrackSim.Hough1D.xBins
            tool.useDiff = True
            tool.variableExtend = True
            tool.drawHitMasks = False
            tool.phiRangeCut = flags.Trigger.FPGATrackSim.Hough1D.phiRangeCut
            tool.d0spread=-1.0 # mm
            tool.iterStep = 0 # auto, TODO put in tag
            tool.iterLayer = 7 # TODO put in tag
            tool.threshold = flags.Trigger.FPGATrackSim.Hough1D.threshold[0]
            tool.hitExtend = flags.Trigger.FPGATrackSim.Hough1D.hitExtendX
            tool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionCfg(flags))
            tool.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
            tool.FPGATrackSimMappingSvc = FPGATrackSimMapping
            tool.IdealGeoRoads = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
            tool.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints

            tools.append(tool)

    RF.tools = tools
    result.addPublicTool(RF, primary=True)
    return result



def FPGATrackSimRoadUnionToolGenScanCfg(flags):
    result=ComponentAccumulator()

    # read the cuts from a seperate python file specified by FPGATrackSim.GenScan.genScanCuts
    cutset = importlib.import_module(flags.Trigger.FPGATrackSim.GenScan.genScanCuts).cuts[flags.Trigger.FPGATrackSim.region]
    
    # make the binning class
    Binning = None
    if (cutset["parSet"]=="PhiSlicedKeyLyrPars") :
        Binning = CompFactory.FPGATrackSimGenScanPhiSlicedKeyLyrBinning("GenScanBinning")
        Binning.approxMath = False
    else:
        log.error("Unknown Binning") 
    Binning.rin=cutset["rin"]
    Binning.rout=cutset["rout"]
    Binning.OutputLevel=flags.Trigger.FPGATrackSim.loglevel

    # make the monitoring class
    Monitor = CompFactory.FPGATrackSimGenScanMonitoring("GenScanMonitoring")
    Monitor.THistSvc = CompFactory.THistSvc()
    Monitor.OutputLevel=flags.Trigger.FPGATrackSim.loglevel

    # make the main tool
    tool = CompFactory.FPGATrackSimGenScanTool("GenScanTool")
    tool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionCfg(flags))
    tool.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
    tool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    tool.Monitoring = Monitor
    tool.Binning = Binning
    tool.OutputLevel=flags.Trigger.FPGATrackSim.loglevel

    # configure which filers and thresholds to apply
    tool.binFilter=flags.Trigger.FPGATrackSim.GenScan.binFilter
    tool.reversePairDir=flags.Trigger.FPGATrackSim.GenScan.reverse
    tool.applyPairFilter=True
    tool.applyPairSetFilter=True
    tool.threshold = 4

    # configure the padding around the nominal region
    tool.d0FractionalPadding =0.05
    tool.z0FractionalPadding =0.05
    tool.etaFractionalPadding =0.05
    tool.phiFractionalPadding =0.05
    tool.qOverPtFractionalPadding =0.05                

    # set cuts
    for (cut,val) in cutset.items():
        setattr(tool,cut,val)

    # even though we are not actually doing a Union, we need the 
    # RoadUnionTool because mapping is now there
    RoadUnion = CompFactory.FPGATrackSimRoadUnionTool()
    RoadUnion.tools = [tool,]
    result.addPublicTool(RoadUnion, primary=True)

    # special configuration for studing layer definitions
    # pass through all hits, but turn off pairing because
    # it won't be able to run
    if flags.Trigger.FPGATrackSim.GenScan.layerStudy:
        RoadUnion.noHitFilter=True
        tool.binningOnly=True
    return result

def FPGATrackSimRoadUnionToolGNNCfg(flags):
    result = ComponentAccumulator()
    RF = CompFactory.FPGATrackSimRoadUnionTool()

    GNNGraphHitSelectorTool = CompFactory.FPGATrackSimGNNGraphHitSelectorTool()

    patternRecoTool = CompFactory.FPGATrackSimGNNPatternRecoTool()
    patternRecoTool.GNNGraphHitSelector = GNNGraphHitSelectorTool
    
    RF.tools = [patternRecoTool]
    result.addPublicTool(RF, primary=True)

    return result

def FPGATrackSimDataFlowToolCfg(flags):
    result=ComponentAccumulator()
    DataFlowTool = CompFactory.FPGATrackSimDataFlowTool()
    DataFlowTool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionCfg(flags))
    DataFlowTool.FPGATrackSimMappingSvc =  result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    DataFlowTool.Chi2ndofCut = flags.Trigger.FPGATrackSim.ActiveConfig.chi2cut
    DataFlowTool.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(DataFlowTool)
    return result

def FPGATrackSimHoughRootOutputToolCfg(flags):
    result=ComponentAccumulator()
    HoughRootOutputTool = CompFactory.FPGATrackSimHoughRootOutputTool()
    HoughRootOutputTool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionCfg(flags))
    HoughRootOutputTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    HoughRootOutputTool.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(HoughRootOutputTool)
    return result

def LRTRoadFinderCfg(flags):
    result=ComponentAccumulator()
    LRTRoadFinder =CompFactory.FPGATrackSimHoughTransform_d0phi0_Tool()
    LRTRoadFinder.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
    LRTRoadFinder.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    LRTRoadFinder.combine_layers = flags.Trigger.FPGATrackSim.ActiveConfig.lrtStraighttrackCombineLayers
    LRTRoadFinder.convolution = flags.Trigger.FPGATrackSim.ActiveConfig.lrtStraighttrackConvolution
    LRTRoadFinder.hitExtend_x = flags.Trigger.FPGATrackSim.ActiveConfig.lrtStraighttrackHitExtendX
    LRTRoadFinder.scale = flags.Trigger.FPGATrackSim.ActiveConfig.scale
    LRTRoadFinder.threshold = flags.Trigger.FPGATrackSim.ActiveConfig.lrtStraighttrackThreshold
    result.setPrivateTools(LRTRoadFinder)
    return result

def NNTrackToolCfg(flags):
    result=ComponentAccumulator()
    NNTrackTool = CompFactory.FPGATrackSimNNTrackTool()
    NNTrackTool.THistSvc = CompFactory.THistSvc()
    NNTrackTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    NNTrackTool.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
    NNTrackTool.IdealGeoRoads = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
    NNTrackTool.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints and not flags.Trigger.FPGATrackSim.ActiveConfig.genScan
    NNTrackTool.SPRoadFilterTool = getSPRoadFilterTool(flags)
    NNTrackTool.MinNumberOfRealHitsInATrack = 5 if flags.Trigger.FPGATrackSim.ActiveConfig.genScan else 9
    
    result.setPrivateTools(NNTrackTool)
    return result

def FPGATrackSimTrackFitterToolCfg(flags):
    result=ComponentAccumulator()
    TF_1st = CompFactory.FPGATrackSimTrackFitterTool("FPGATrackSimTrackFitterTool_1st")
    TF_1st.GuessHits = flags.Trigger.FPGATrackSim.ActiveConfig.guessHits
    TF_1st.IdealCoordFitType = flags.Trigger.FPGATrackSim.ActiveConfig.idealCoordFitType
    TF_1st.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
    TF_1st.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    TF_1st.chi2DofRecoveryMax = flags.Trigger.FPGATrackSim.ActiveConfig.chi2DoFRecoveryMax
    TF_1st.chi2DofRecoveryMin = flags.Trigger.FPGATrackSim.ActiveConfig.chi2DoFRecoveryMin
    TF_1st.doMajority = flags.Trigger.FPGATrackSim.ActiveConfig.doMajority
    TF_1st.nHits_noRecovery = flags.Trigger.FPGATrackSim.ActiveConfig.nHitsNoRecovery
    TF_1st.DoDeltaGPhis = flags.Trigger.FPGATrackSim.ActiveConfig.doDeltaGPhis
    TF_1st.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    TF_1st.IdealGeoRoads = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
    TF_1st.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints and not flags.Trigger.FPGATrackSim.ActiveConfig.genScan
    TF_1st.SPRoadFilterTool = getSPRoadFilterTool(flags)
    result.addPublicTool(TF_1st, primary=True)
    return result

def FPGATrackSimOverlapRemovalToolCfg(flags):
    result=ComponentAccumulator()
    OR_1st = CompFactory.FPGATrackSimOverlapRemovalTool("FPGATrackSimOverlapRemovalTool_1st")
    OR_1st.ORAlgo = "Normal"
    OR_1st.doFastOR =flags.Trigger.FPGATrackSim.ActiveConfig.doFastOR
    OR_1st.NumOfHitPerGrouping = 3
    OR_1st.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    OR_1st.MinChi2 = flags.Trigger.FPGATrackSim.ActiveConfig.chi2cut    
    if flags.Trigger.FPGATrackSim.ActiveConfig.hough or flags.Trigger.FPGATrackSim.ActiveConfig.hough1D:
        OR_1st.nBins_x = flags.Trigger.FPGATrackSim.ActiveConfig.xBins + 2 * flags.Trigger.FPGATrackSim.ActiveConfig.xBufferBins
        OR_1st.nBins_y = flags.Trigger.FPGATrackSim.ActiveConfig.yBins + 2 * flags.Trigger.FPGATrackSim.ActiveConfig.yBufferBins
        OR_1st.localMaxWindowSize = flags.Trigger.FPGATrackSim.ActiveConfig.localMaxWindowSize
        OR_1st.roadSliceOR = flags.Trigger.FPGATrackSim.ActiveConfig.roadSliceOR
    
    result.addPublicTool(OR_1st, primary=True)
    return result

def prepareFlagsForFPGATrackSimLogicalHitsProcessAlg(flags):
    newFlags = flags.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag)
    return newFlags

def getSPRoadFilterTool(flags,secondStage=False):
    name="FPGATrackSimSpacepointRoadFilterTool_1st"
    if secondStage:
        name="FPGATrackSimSpacepointRoadFilterTool_2st"
    SPRoadFilter = CompFactory.FPGATrackSimSpacepointRoadFilterTool(name)
    SPRoadFilter.filtering = flags.Trigger.FPGATrackSim.ActiveConfig.spacePointFiltering
    SPRoadFilter.minSpacePlusPixel = flags.Trigger.FPGATrackSim.minSpacePlusPixel
    SPRoadFilter.isSecondStage = secondStage
    # TODO guard here against threshold being more than one value?
    if (flags.Trigger.FPGATrackSim.ActiveConfig.hough1D):
        SPRoadFilter.threshold = flags.Trigger.FPGATrackSim.Hough1D.threshold[0]
    else:
        SPRoadFilter.threshold = flags.Trigger.FPGATrackSim.ActiveConfig.threshold[0]
    SPRoadFilter.setSectors = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
    return SPRoadFilter

def FPGATrackSimLogicalHitsProcessAlgCfg(inputFlags):

    flags = prepareFlagsForFPGATrackSimLogicalHitsProcessAlg(inputFlags)
   
    result=ComponentAccumulator()
    if not flags.Trigger.FPGATrackSim.wrapperFileName:
        from InDetConfig.InDetPrepRawDataFormationConfig import AthenaTrkClusterizationCfg
        result.merge(AthenaTrkClusterizationCfg(flags))

    theFPGATrackSimLogicalHitsProcessAlg=CompFactory.FPGATrackSimLogicalHitsProcessAlg()
    theFPGATrackSimLogicalHitsProcessAlg.writeOutputData = flags.Trigger.FPGATrackSim.ActiveConfig.writeOutputData
    theFPGATrackSimLogicalHitsProcessAlg.tracking = flags.Trigger.FPGATrackSim.tracking
    theFPGATrackSimLogicalHitsProcessAlg.doOverlapRemoval = flags.Trigger.FPGATrackSim.doOverlapRemoval
    theFPGATrackSimLogicalHitsProcessAlg.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    theFPGATrackSimLogicalHitsProcessAlg.DoHoughRootOutput = flags.Trigger.FPGATrackSim.ActiveConfig.houghRootoutput
    theFPGATrackSimLogicalHitsProcessAlg.NumOfHitPerGrouping = flags.Trigger.FPGATrackSim.ActiveConfig.NumOfHitPerGrouping
    theFPGATrackSimLogicalHitsProcessAlg.DoNNTrack = flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis
    theFPGATrackSimLogicalHitsProcessAlg.runOnRDO = not flags.Trigger.FPGATrackSim.wrapperFileName
    theFPGATrackSimLogicalHitsProcessAlg.eventSelector = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionCfg(flags))
    theFPGATrackSimLogicalHitsProcessAlg.TrackScoreCut = flags.Trigger.FPGATrackSim.ActiveConfig.chi2cut

    FPGATrackSimMaping = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    theFPGATrackSimLogicalHitsProcessAlg.FPGATrackSimMapping = FPGATrackSimMaping

    # If tracking is set to False, don't configure the bank service
    if flags.Trigger.FPGATrackSim.tracking:
        result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))

    if (flags.Trigger.FPGATrackSim.ActiveConfig.hough1D):
        theFPGATrackSimLogicalHitsProcessAlg.RoadFinder = result.getPrimaryAndMerge(FPGATrackSimRoadUnionTool1DCfg(flags))
    elif (flags.Trigger.FPGATrackSim.ActiveConfig.genScan):
        theFPGATrackSimLogicalHitsProcessAlg.RoadFinder = result.getPrimaryAndMerge(FPGATrackSimRoadUnionToolGenScanCfg(flags))
    elif (flags.Trigger.FPGATrackSim.ActiveConfig.GNN):
        theFPGATrackSimLogicalHitsProcessAlg.RoadFinder = result.getPrimaryAndMerge(FPGATrackSimRoadUnionToolGNNCfg(flags))
    else:
        theFPGATrackSimLogicalHitsProcessAlg.RoadFinder = result.getPrimaryAndMerge(FPGATrackSimRoadUnionToolCfg(flags))

    if (flags.Trigger.FPGATrackSim.ActiveConfig.etaPatternFilter):
        EtaPatternFilter = CompFactory.FPGATrackSimEtaPatternFilterTool()
        EtaPatternFilter.FPGATrackSimMappingSvc = FPGATrackSimMaping
        EtaPatternFilter.threshold = flags.Trigger.FPGATrackSim.Hough1D.threshold[0]
        EtaPatternFilter.EtaPatterns = flags.Trigger.FPGATrackSim.mapsDir+"/"+FPGATrackSimDataPrepConfig.getBaseName(flags)+".patt"
        theFPGATrackSimLogicalHitsProcessAlg.RoadFilter = EtaPatternFilter
        theFPGATrackSimLogicalHitsProcessAlg.FilterRoads = True

    if (flags.Trigger.FPGATrackSim.ActiveConfig.phiRoadFilter):
        RoadFilter2 = CompFactory.FPGATrackSimPhiRoadFilterTool()
        RoadFilter2.FPGATrackSimMappingSvc = FPGATrackSimMaping
        RoadFilter2.threshold = flags.Trigger.FPGATrackSim.Hough1D.threshold[0]
        RoadFilter2.fieldCorrection = flags.Trigger.FPGATrackSim.ActiveConfig.fieldCorrection
        ### set the window to be a constant value (could be changed), array should be length of the threshold
        windows = [flags.Trigger.FPGATrackSim.Hough1D.phifilterwindow for i in range(len(flags.Trigger.FPGATrackSim.ActiveConfig.hitExtendX))]
        RoadFilter2.window = windows
        
        theFPGATrackSimLogicalHitsProcessAlg.RoadFilter2 = RoadFilter2
        theFPGATrackSimLogicalHitsProcessAlg.FilterRoads2 = True


    theFPGATrackSimLogicalHitsProcessAlg.HoughRootOutputTool = result.getPrimaryAndMerge(FPGATrackSimHoughRootOutputToolCfg(flags))

    LRTRoadFilter = CompFactory.FPGATrackSimLLPRoadFilterTool()
    result.addPublicTool(LRTRoadFilter)
    theFPGATrackSimLogicalHitsProcessAlg.LRTRoadFilter = LRTRoadFilter

    theFPGATrackSimLogicalHitsProcessAlg.LRTRoadFinder = result.getPrimaryAndMerge(LRTRoadFinderCfg(flags))
    theFPGATrackSimLogicalHitsProcessAlg.NNTrackTool = result.getPrimaryAndMerge(NNTrackToolCfg(flags))

    theFPGATrackSimLogicalHitsProcessAlg.OutputTool = result.getPrimaryAndMerge(FPGATrackSimWriteOutputCfg(flags))
    theFPGATrackSimLogicalHitsProcessAlg.TrackFitter_1st = result.getPrimaryAndMerge(FPGATrackSimTrackFitterToolCfg(flags))
    theFPGATrackSimLogicalHitsProcessAlg.OverlapRemoval_1st = result.getPrimaryAndMerge(FPGATrackSimOverlapRemovalToolCfg(flags))

    # Create SPRoadFilterTool if spacepoints are turned on. TODO: make things configurable?
    if flags.Trigger.FPGATrackSim.spacePoints and not flags.Trigger.FPGATrackSim.ActiveConfig.genScan:
        theFPGATrackSimLogicalHitsProcessAlg.SPRoadFilterTool = getSPRoadFilterTool(flags)
        theFPGATrackSimLogicalHitsProcessAlg.Spacepoints = True


    if flags.Trigger.FPGATrackSim.ActiveConfig.lrt:
        assert flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseBasicHitFilter != flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseMlHitFilter, 'Inconsistent LRT hit filtering setup, need either ML of Basic filtering enabled'
        assert flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseStraightTrackHT != flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseDoubletHT, 'Inconsistent LRT HT setup, need either double or strightTrack enabled'
        theFPGATrackSimLogicalHitsProcessAlg.doLRT = True
        theFPGATrackSimLogicalHitsProcessAlg.LRTHitFiltering = (not flags.Trigger.FPGATrackSim.ActiveConfig.lrtSkipHitFiltering)

    from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmConfig import FPGATrackSimLogicalHitsProcessAlgMonitoringCfg
    theFPGATrackSimLogicalHitsProcessAlg.MonTool = result.getPrimaryAndMerge(FPGATrackSimLogicalHitsProcessAlgMonitoringCfg(flags))

    result.addEventAlgo(theFPGATrackSimLogicalHitsProcessAlg)

    return result




if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    

    flags = initConfigFlags()

    
    
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    
    ############################################
    # Flags used in the prototrack chain
    FinalProtoTrackChainxAODTracksKey="xAODFPGAProtoTracks"
    flags.Detector.EnableCalo = False

    # ensure that the xAOD SP and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True
    from TrkConfig.TrkConfigFlags import TrackingComponent
    flags.Tracking.recoChain = [TrackingComponent.ActsChain] # another viable option is TrackingComponent.AthenaChain
    flags.Acts.doRotCorrection = False

    ############################################
    flags.Concurrency.NumThreads=1
    #flags.Concurrency.NumProcs=0
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True
    flags.Debug.DumpEvtStore=False

    # flags.Exec.DebugStage="exec" # useful option to debug the execution of the job - we want it commented out for production
    flags.fillFromArgs()

    assert not flags.Trigger.FPGATrackSim.pipeline.startswith('F-5'),"ERROR You are trying to run an F-5* pipeline! This is not yet supported!"

    if (flags.Trigger.FPGATrackSim.pipeline.startswith('F-1')):
        print("You are trying to run an F-!* pipeline! I am going to run the Data Prep chain for you and nothing else!")
        FPGATrackSimDataPrepConfig.runDataPrepChain()
    elif (flags.Trigger.FPGATrackSim.pipeline.startswith('F-2')):
        print("You are trying to run an F-2* pipeline! I am auto-configuring the 1D bitshift for you, including eta pattern filters and phi road filters")
        flags.Trigger.FPGATrackSim.Hough.etaPatternFilter = True
        flags.Trigger.FPGATrackSim.Hough.phiRoadFilter = True
        flags.Trigger.FPGATrackSim.Hough.hough1D = True
        flags.Trigger.FPGATrackSim.Hough.hough = False
    elif (flags.Trigger.FPGATrackSim.pipeline.startswith('F-3')):
        print("You are trying to run an F-3* pipeline! I am auto-configuring the 2D HT for you, and disabling the eta pattern filter and phi road filter. Whether you wanted to or not")
        flags.Trigger.FPGATrackSim.Hough.etaPatternFilter = False
        flags.Trigger.FPGATrackSim.Hough.phiRoadFilter = False
        flags.Trigger.FPGATrackSim.Hough.hough1D = False
        flags.Trigger.FPGATrackSim.Hough.hough = True
    elif (flags.Trigger.FPGATrackSim.pipeline.startswith('F-4')):
        print("You are trying to run an F-4* pipeline! I am auto-configuring the GNN pattern recognition for you. Whether you wanted to or not")
        flags.Trigger.FPGATrackSim.Hough.GNN = True
        flags.Trigger.FPGATrackSim.Hough.chi2cut = 25 # All of the track candidates have chi2 values around 20 for some reason. Needs further investigation. For now move the default cut value to 25
    elif (flags.Trigger.FPGATrackSim.pipeline.startswith('F-6')):
        print("You are trying to run an F-6* pipeline! I am auto-configuring the Inside-Out for you. Whether you wanted to or not")
        flags.Trigger.FPGATrackSim.Hough.genScan=True
        flags.Trigger.FPGATrackSim.spacePoints= flags.Trigger.FPGATrackSim.Hough.secondStage
    elif (flags.Trigger.FPGATrackSim.pipeline != ""):
        raise AssertionError("ERROR You are trying to run the pipeline " + flags.Trigger.FPGATrackSim.pipeline + " which is not yet supported!")

    if (not flags.Trigger.FPGATrackSim.pipeline.startswith('F-1')): ### if DP pipeline skip everything else!
       
       splitPipeline=flags.Trigger.FPGATrackSim.pipeline.split('-')
       trackingOption=9999999
       if (len(splitPipeline) > 1): trackingOption=int(splitPipeline[1])
       if (trackingOption < 9999999):           
           trackingOptionMod = (trackingOption % 100)
           if (trackingOptionMod == 0):
               print("You are trying to run the linearized chi2 fit as part of a pipeline! I am going to enable this for you whether you want to or not")
               flags.Trigger.FPGATrackSim.tracking = True
               flags.Trigger.FPGATrackSim.Hough.trackNNAnalysis = False
           elif (trackingOptionMod == 10):
               print("You are trying to run the NN fake rejection as part of a pipeline! I am going to enable this for you whether you want to or not")               
               flags.Trigger.FPGATrackSim.tracking = True
               flags.Trigger.FPGATrackSim.Hough.trackNNAnalysis = True
           else:
               raise AssertionError("ERROR Your tracking option for the pipeline = " + str(trackingOption) + " is not yet supported!")
   
       if isinstance(flags.Trigger.FPGATrackSim.wrapperFileName, str):
           log.info("wrapperFile is string, converting to list")
           flags.Trigger.FPGATrackSim.wrapperFileName = [flags.Trigger.FPGATrackSim.wrapperFileName]
           flags.Input.Files = lambda f: [f.Trigger.FPGATrackSim.wrapperFileName]
   
       flags.lock()
       flags.dump()
       flags = flags.cloneAndReplace("Tracking.ActiveConfig","Tracking.MainPass")
       acc=MainServicesCfg(flags)
   
       acc.addService(CompFactory.THistSvc(Output = ["EXPERT DATAFILE='monitoring.root', OPT='RECREATE'"]))
   
       if (flags.Trigger.FPGATrackSim.Hough.houghRootoutput):
           acc.addService(CompFactory.THistSvc(Output = ["TRIGFPGATrackSimHOUGHOUTPUT DATAFILE='HoughRootOutput.root', OPT='RECREATE'"]))
   
       acc.addService(CompFactory.THistSvc(Output = ["FPGATRACKSIMOUTPUT DATAFILE='test.root', OPT='RECREATE'"]))

       if (flags.Trigger.FPGATrackSim.Hough.genScan):
           acc.addService(CompFactory.THistSvc(Output = ["GENSCAN DATAFILE='genscan.root', OPT='RECREATE'"]))

       if (flags.Trigger.FPGATrackSim.Hough.GNN):
           acc.addService(CompFactory.THistSvc(Output = ["TRIGFPGATrackSimGNNOUTPUT DATAFILE='GNNRootOutput.root', OPT='RECREATE'"]))
       
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
       acc.merge(FPGATrackSimLogicalHitsProcessAlgCfg(flags))

       # If second stage is turned on, turn that algorithm on too.
       if flags.Trigger.FPGATrackSim.Hough.secondStage:
           acc.merge(FPGATrackSimSecondStageConfig.FPGATrackSimSecondStageAlgCfg(flags))
   
       if flags.Trigger.FPGATrackSim.doEDMConversion:
           stage = "_2nd" if flags.Trigger.FPGATrackSim.Hough.secondStage else "_1st"
           acc.merge(FPGATrackSimDataPrepConfig.FPGAConversionAlgCfg(flags, name = f"FPGAConversionAlg{stage}", stage = f"{stage}", doActsTrk=True, doSP=flags.Trigger.FPGATrackSim.spacePoints))
           from FPGATrackSimPrototrackFitter.FPGATrackSimPrototrackFitterConfig import FPGATruthDecorationCfg, FPGAProtoTrackFitCfg
           acc.merge(FPGAProtoTrackFitCfg(flags,stage=f"{stage}")) # Run ACTS KF
           acc.merge(FPGATruthDecorationCfg(flags,FinalProtoTrackChainxAODTracksKey=FinalProtoTrackChainxAODTracksKey,stage=f"{stage}")) # Run Truth Matching/Decoration chain
           if not flags.Trigger.FPGATrackSim.wrapperFileName and flags.Trigger.FPGATrackSim.runCKF:
               from FPGATrackSimConfTools.FPGATrackExtensionConfig import FPGATrackExtensionAlgCfg
               acc.merge(FPGATrackExtensionAlgCfg(flags, enableTrackStatePrinter=False, name="FPGATrackExtension",
                                                  ProtoTracksLocation=f"ActsProtoTracks{stage}FromFPGATrack")) # run CKF track extension on FPGA tracks
   
           if flags.Trigger.FPGATrackSim.writeToAOD: acc.merge(FPGATrackSimDataPrepConfig.WriteToAOD(flags,
                                                                                                     stage = f"{stage}",
                                                                                                     finalTrackParticles=f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"))
   
           # Reporting algorithm (used for debugging - can be disabled)
           from FPGATrackSimReporting.FPGATrackSimReportingConfig import FPGATrackSimReportingCfg
           acc.merge(FPGATrackSimReportingCfg(flags, stage=f"{stage}",
                                              perEventReports = ((flags.Trigger.FPGATrackSim.sampleType != 'skipTruth') and flags.Exec.MaxEvents<=10 ) )) # disable perEventReports for pileup samples or many events
       
       acc.store(open('AnalysisConfig.pkl','wb'))
   
       acc.foreach_component("FPGATrackSim*").OutputLevel=flags.Trigger.FPGATrackSim.loglevel
       if flags.Trigger.FPGATrackSim.msgLimit!=-1:
        acc.getService("MessageSvc").debugLimit = flags.Trigger.FPGATrackSim.msgLimit
        acc.getService("MessageSvc").infoLimit = flags.Trigger.FPGATrackSim.msgLimit

       statusCode = acc.run(flags.Exec.MaxEvents)
       assert statusCode.isSuccess() is True, "Application execution did not succeed"
