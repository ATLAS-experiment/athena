# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import AthenaLogger
log = AthenaLogger(__name__)

def getNSubregions(filePath):
    with open(filePath, 'r') as f:
        fields = f.readline()
        assert(fields.startswith('towers'))
        # towers 10 phi 16
        n = fields.split()[1] 
        return int(n)


def FPGATrackSimEventSelectionCfg(flags, name="FPGATrackSimEventSelectionSvc", sampleType='skipTruth'):
    result=ComponentAccumulator()
    eventSelector = CompFactory.FPGATrackSimEventSelectionSvc()
    eventSelector.regions = "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/HTT/TrigHTTMaps/V1/map_file/slices_v01_Jan21.txt"
    eventSelector.regionID = 0
    eventSelector.sampleType = sampleType
    eventSelector.withPU = False
    result.addService(eventSelector, create=True, primary=True)
    return result

def FPGATrackSimMappingCfg(flags):
    result=ComponentAccumulator()

    mappingSvc = CompFactory.FPGATrackSimMappingSvc()
    mappingSvc.mappingType = "FILE"
    mappingSvc.rmap = flags.Trigger.FPGATrackSim.mapsDir+"/eta0103phi0305.rmap" # we need more configurability here i.e. file choice should depend on some flag
    mappingSvc.subrmap =  flags.Trigger.FPGATrackSim.mapsDir+"/eta0103phi0305.subrmap" # presumably also here we want to be able to change the slices definition file
    mappingSvc.pmap = flags.Trigger.FPGATrackSim.mapsDir+"/pmap"
    mappingSvc.modulemap = flags.Trigger.FPGATrackSim.mapsDir+"/moduleidmap"
    mappingSvc.NNmap = ""
    mappingSvc.layerOverride = {}
    result.addService(mappingSvc, create=True, primary=True)
    return result

def FPGATrackSimBankSvcCfg(flags):
    result=ComponentAccumulator()
    FPGATrackSimBankSvc = CompFactory.FPGATrackSimBankSvc()
    pathBankSvc = f'/eos/atlas/atlascerngroupdisk/det-htt/HTTsim/{flags.GeoModel.AtlasVersion}/21.9.16/eta0103phi0305/SectorBanks/'
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
    FPGATrackSimBankSvc.constants_1st = f'{pathBankSvc}corrgen_raw_8L.gcon'
    FPGATrackSimBankSvc.constants_2nd = f'{pathBankSvc}corrgen_raw_13L_reg0_checkGood1.gcon'
    FPGATrackSimBankSvc.sectorBank_1st = f'{pathBankSvc}sectorsHW_raw_8L.patt'
    FPGATrackSimBankSvc.sectorBank_2nd = f'{pathBankSvc}sectorsHW_raw_13L_reg0_checkGood1.patt'
    FPGATrackSimBankSvc.sectorSlices = f'{pathBankSvc}slices_8L.root'
    result.addService(FPGATrackSimBankSvc, create=True, primary=True)
    return result


def FPGATrackSimRoadUnionToolCfg(flags):
    result=ComponentAccumulator()
    RF = CompFactory.FPGATrackSimRoadUnionTool()
    
    xBins = flags.Trigger.FPGATrackSim.ActiveConfig.xBins
    xBufferBins = flags.Trigger.FPGATrackSim.ActiveConfig.xBufferBins
    yBins = flags.Trigger.FPGATrackSim.ActiveConfig.yBins
    yBufferBins = flags.Trigger.FPGATrackSim.ActiveConfig.yBufferBins
    xMin = flags.Trigger.FPGATrackSim.ActiveConfig.phiMin
    xMax = flags.Trigger.FPGATrackSim.ActiveConfig.phiMax
    xBuffer = (xMax - xMin) / xBins * xBufferBins
    xMin = xMin - xBuffer
    xMax = xMax +  xBuffer
    yMin = flags.Trigger.FPGATrackSim.ActiveConfig.qptMin
    yMax = flags.Trigger.FPGATrackSim.ActiveConfig.qptMax
    yBuffer = (yMax - yMin) / yBins * yBufferBins
    yMin -= yBuffer
    yMax += yBuffer
    tools = []
    
    FPGATrackSimMapping = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    for number in range(getNSubregions(FPGATrackSimMapping.subrmap)): 
        HoughTransform = CompFactory.FPGATrackSimHoughTransformTool("HoughTransform_0_" + str(number))
        HoughTransform.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimEventSelectionCfg(flags))
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
        HoughTransform.traceHits = False
        tools.append(HoughTransform)

    RF.tools = tools
    result.addPublicTool(RF, primary=True)
    return result


def FPGATrackSimRawLogicCfg(flags):
    result=ComponentAccumulator()
    FPGATrackSimRawLogic = CompFactory.FPGATrackSimRawToLogicalHitsTool()
    FPGATrackSimRawLogic.SaveOptional = 2
    if (flags.Trigger.FPGATrackSim.ActiveConfig.sampleType == 'skipTruth'): 
        FPGATrackSimRawLogic.SaveOptional = 1
    FPGATrackSimRawLogic.TowersToMap = [0] # TODO TODO why is this hardcoded?
    FPGATrackSimRawLogic.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimEventSelectionCfg(flags))
    FPGATrackSimRawLogic.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    result.addPublicTool(FPGATrackSimRawLogic, primary=True)
    return result

def FPGATrackSimDataFlowToolCfg(flags):
    result=ComponentAccumulator()
    DataFlowTool = CompFactory.FPGATrackSimDataFlowTool()
    DataFlowTool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimEventSelectionCfg(flags))
    DataFlowTool.FPGATrackSimMappingSvc =  result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    DataFlowTool.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(DataFlowTool)
    return result

def FPGATrackSimSpacePointsToolCfg(flags):
    result=ComponentAccumulator()
    SpacePointTool = CompFactory.FPGATrackSimSpacePointsTool_v2()
    SpacePointTool.Filtering = flags.Trigger.FPGATrackSim.ActiveConfig.spacePointFiltering
    SpacePointTool.FilteringClosePoints = False
    SpacePointTool.PhiWindow = 0.008
    SpacePointTool.Duplication = True
    result.addPublicTool(SpacePointTool, primary=True)
    return result


def FPGATrackSimHitFilteringToolCfg(flags):
    result=ComponentAccumulator()
    HitFilteringTool = CompFactory.FPGATrackSimHitFilteringTool()
    HitFilteringTool.barrelStubDphiCut = 3.0
    HitFilteringTool.doRandomRemoval = False
    HitFilteringTool.doStubs = False
    HitFilteringTool.endcapStubDphiCut = 1.5
    HitFilteringTool.pixelClusRmFrac = 0
    HitFilteringTool.pixelHitRmFrac = 0
    HitFilteringTool.stripClusRmFrac = 0
    HitFilteringTool.stripHitRmFrac = 0
    HitFilteringTool.useNstrips = False
    result.addPublicTool(HitFilteringTool, primary=True)
    return result


def FPGATrackSimHoughRootOutputToolCfg(flags):
    result=ComponentAccumulator()
    HoughRootOutputTool = CompFactory.FPGATrackSimHoughRootOutputTool()
    HoughRootOutputTool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimEventSelectionCfg(flags))
    HoughRootOutputTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    HoughRootOutputTool.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(HoughRootOutputTool)
    return result

def LRTRoadFinderCfg(flags):
    result=ComponentAccumulator()
    LRTRoadFinder =CompFactory.FPGATrackSimHoughTransform_d0phi0_Tool()
    LRTRoadFinder.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
    LRTRoadFinder.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
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
    NNTrackTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    result.setPrivateTools(NNTrackTool)
    return result

def FPGATrackSimWriteOutputCfg(flags):
    result=ComponentAccumulator()
    FPGATrackSimWriteOutput = CompFactory.FPGATrackSimOutputHeaderTool("FPGATrackSimWriteOutput")
    FPGATrackSimWriteOutput.InFileName = ["test"]
    FPGATrackSimWriteOutput.RWstatus = "HEADER" # do not open file, use THistSvc
    FPGATrackSimWriteOutput.RunSecondStage = flags.Trigger.FPGATrackSim.ActiveConfig.secondStage
    result.addPublicTool(FPGATrackSimWriteOutput, primary=True)
    return result

def FPGATrackSimTrackFitterToolCfg(flags):
    result=ComponentAccumulator()
    TF_1st = CompFactory.FPGATrackSimTrackFitterTool("FPGATrackSimTrackFitterTool_1st")
    TF_1st.GuessHits = flags.Trigger.FPGATrackSim.ActiveConfig.guessHits
    TF_1st.IdealCoordFitType = flags.Trigger.FPGATrackSim.ActiveConfig.idealCoordFitType
    TF_1st.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
    TF_1st.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    TF_1st.chi2DofRecoveryMax = flags.Trigger.FPGATrackSim.ActiveConfig.chi2DoFRecoveryMax
    TF_1st.chi2DofRecoveryMin = flags.Trigger.FPGATrackSim.ActiveConfig.chi2DoFRecoveryMin
    TF_1st.doMajority = flags.Trigger.FPGATrackSim.ActiveConfig.doMajority
    TF_1st.nHits_noRecovery = flags.Trigger.FPGATrackSim.ActiveConfig.nHitsNoRecovery
    TF_1st.DoDeltaGPhis = flags.Trigger.FPGATrackSim.ActiveConfig.doDeltaGPhis
    TF_1st.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    result.addPublicTool(TF_1st, primary=True)
    return result

def FPGATrackSimOverlapRemovalToolCfg(flags):
    result=ComponentAccumulator()
    OR_1st = CompFactory.FPGATrackSimOverlapRemovalTool("FPGATrackSimOverlapRemovalTool_1st")
    OR_1st.ORAlgo = "Normal"
    OR_1st.doFastOR =flags.Trigger.FPGATrackSim.ActiveConfig.doFastOR
    OR_1st.NumOfHitPerGrouping = 5
    OR_1st.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    if flags.Trigger.FPGATrackSim.ActiveConfig.hough:
        OR_1st.nBins_x = flags.Trigger.FPGATrackSim.ActiveConfig.xBins + 2 * flags.Trigger.FPGATrackSim.ActiveConfig.xBufferBins
        OR_1st.nBins_y = flags.Trigger.FPGATrackSim.ActiveConfig.yBins + 2 * flags.Trigger.FPGATrackSim.ActiveConfig.yBufferBins
        OR_1st.localMaxWindowSize = flags.Trigger.FPGATrackSim.ActiveConfig.localMaxWindowSize
        OR_1st.roadSliceOR = flags.Trigger.FPGATrackSim.ActiveConfig.roadSliceOR
    
    result.addPublicTool(OR_1st, primary=True)
    return result


def FPGATrackSimOverlapRemovalTool_2ndCfg(flags):
    result=ComponentAccumulator()
    OR_2nd = CompFactory.FPGATrackSimOverlapRemovalTool("FPGATrackSimOverlapRemovalTool_2nd")
    OR_2nd.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    if flags.Trigger.FPGATrackSim.ActiveConfig.secondStage:
        OR_2nd.DoSecondStage = True
        OR_2nd.ORAlgo = "Normal"
        OR_2nd.doFastOR = flags.Trigger.FPGATrackSim.ActiveConfig.doFastOR
        OR_2nd.NumOfHitPerGrouping = 5
    result.setPrivateTools(OR_2nd)
    return result


def FPGATrackSimTrackFitterTool_2ndCfg(flags):
    result=ComponentAccumulator()
    TF_2nd = CompFactory.FPGATrackSimTrackFitterTool(name="FPGATrackSimTrackFitterTool_2nd")
    TF_2nd.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
    TF_2nd.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    if flags.Trigger.FPGATrackSim.ActiveConfig.secondStage:
        TF_2nd.Do2ndStageTrackFit = True 
    result.setPrivateTools(TF_2nd)
    return result



def FPGATrackSimReadInputCfg(flags):
    result=ComponentAccumulator()
    InputTool = CompFactory.FPGATrackSimInputHeaderTool(name="FPGATrackSimReadInput",
                                               InFileName = flags.Trigger.FPGATrackSim.wrapperFileName)
    result.addPublicTool(InputTool, primary=True)
    return result

def FPGATrackSimSGInputToolCfg(flags):
    """In this cotext the tool is configured to only read RDO hits"""
    result = ComponentAccumulator()
    tool = CompFactory.FPGATrackSimSGToRawHitsTool(                       
        TruthToTrackTool=None,
        Extrapolator=None,
        pixelClustersName="",
        SCT_ClustersName="",
        OfflineTracks="",
        McTruth="",
        dumpHitsOnTracks=False,
        dumpTruthIntersections=False,
        ReadOfflineClusters=False,
        ReadTruthTracks=False,
        ReadOfflineTracks=False
    )

    result.addPublicTool(tool, primary=True)
    return result


def prepareFlagsForFPGATrackSimLogicalHistProcessAlg(flags):
    newFlags = flags.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag)
    return newFlags


def FPGATrackSimLogicalHistProcessAlgCfg(inputFlags):
    flags = prepareFlagsForFPGATrackSimLogicalHistProcessAlg(inputFlags)
   
    result=ComponentAccumulator()

   
    theFPGATrackSimLogicalHistProcessAlg=CompFactory.FPGATrackSimLogicalHitsProcessAlg()
    theFPGATrackSimLogicalHistProcessAlg.HitFiltering = flags.Trigger.FPGATrackSim.ActiveConfig.hitFiltering
    theFPGATrackSimLogicalHistProcessAlg.writeOutputData = flags.Trigger.FPGATrackSim.ActiveConfig.writeOutputData
    theFPGATrackSimLogicalHistProcessAlg.Clustering = True
    theFPGATrackSimLogicalHistProcessAlg.tracking = flags.Trigger.FPGATrackSim.ActiveConfig.doTracking
    theFPGATrackSimLogicalHistProcessAlg.outputHitTxt = flags.Trigger.FPGATrackSim.ActiveConfig.outputHitTxt
    theFPGATrackSimLogicalHistProcessAlg.RunSecondStage = flags.Trigger.FPGATrackSim.ActiveConfig.secondStage
    theFPGATrackSimLogicalHistProcessAlg.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    theFPGATrackSimLogicalHistProcessAlg.DoHoughRootOutput = False
    theFPGATrackSimLogicalHistProcessAlg.DoNNTrack = False
    theFPGATrackSimLogicalHistProcessAlg.eventSelector = result.getPrimaryAndMerge(FPGATrackSimEventSelectionCfg(flags))

    FPGATrackSimMaping = result.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    theFPGATrackSimLogicalHistProcessAlg.FPGATrackSimMapping = FPGATrackSimMaping

    result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))

    theFPGATrackSimLogicalHistProcessAlg.RoadFinder = result.getPrimaryAndMerge(FPGATrackSimRoadUnionToolCfg(flags))
    theFPGATrackSimLogicalHistProcessAlg.RawToLogicalHitsTool = result.getPrimaryAndMerge(FPGATrackSimRawLogicCfg(flags))

    if flags.Trigger.FPGATrackSim.wrapperFileName != []:
        theFPGATrackSimLogicalHistProcessAlg.InputTool = result.getPrimaryAndMerge(FPGATrackSimReadInputCfg(flags))
    else:
        theFPGATrackSimLogicalHistProcessAlg.InputTool = ""
        theFPGATrackSimLogicalHistProcessAlg.InputTool2 = ""
        theFPGATrackSimLogicalHistProcessAlg.SGInputTool = result.getPrimaryAndMerge(FPGATrackSimSGInputToolCfg(flags))

    # TODO, will need also the alternative TOOL implementation
    # InputTool2 = CompFactory.FPGATrackSimReadRawRandomHitsTool("FPGATrackSimReadRawRandomHitsTool")
    # InputTool2.InFileName = flags.Input.Files[0]
    # result.addPublicTool(InputTool2)
    # theFPGATrackSimLogicalHistProcessAlg.InputTool2 = InputTool2

    theFPGATrackSimLogicalHistProcessAlg.DataFlowTool = result.getPrimaryAndMerge(FPGATrackSimDataFlowToolCfg(flags))
    theFPGATrackSimLogicalHistProcessAlg.SpacePointTool = result.getPrimaryAndMerge(FPGATrackSimSpacePointsToolCfg(flags))

    RoadFilter = CompFactory.FPGATrackSimEtaPatternFilterTool()
    RoadFilter.FPGATrackSimMappingSvc = FPGATrackSimMaping
    theFPGATrackSimLogicalHistProcessAlg.RoadFilter = RoadFilter

    theFPGATrackSimLogicalHistProcessAlg.HitFilteringTool = result.getPrimaryAndMerge(FPGATrackSimHitFilteringToolCfg(flags))
    theFPGATrackSimLogicalHistProcessAlg.HoughRootOutputTool = result.getPrimaryAndMerge(FPGATrackSimHoughRootOutputToolCfg(flags))

    LRTRoadFilter = CompFactory.FPGATrackSimLLPRoadFilterTool()
    result.addPublicTool(LRTRoadFilter)
    theFPGATrackSimLogicalHistProcessAlg.LRTRoadFilter = LRTRoadFilter

    theFPGATrackSimLogicalHistProcessAlg.LRTRoadFinder = result.getPrimaryAndMerge(LRTRoadFinderCfg(flags))
    theFPGATrackSimLogicalHistProcessAlg.NNTrackTool = result.getPrimaryAndMerge(NNTrackToolCfg(flags))

    RoadFilter2 = CompFactory.FPGATrackSimPhiRoadFilterTool()
    RoadFilter2.FPGATrackSimMappingSvc = FPGATrackSimMaping
    RoadFilter2.window = []
    theFPGATrackSimLogicalHistProcessAlg.RoadFilter2 = RoadFilter2

    theFPGATrackSimLogicalHistProcessAlg.ClusteringTool = CompFactory.FPGATrackSimClusteringTool()
    theFPGATrackSimLogicalHistProcessAlg.OutputTool = result.getPrimaryAndMerge(FPGATrackSimWriteOutputCfg(flags))
    theFPGATrackSimLogicalHistProcessAlg.TrackFitter_1st = result.getPrimaryAndMerge(FPGATrackSimTrackFitterToolCfg(flags))
    theFPGATrackSimLogicalHistProcessAlg.OverlapRemoval_1st = result.getPrimaryAndMerge(FPGATrackSimOverlapRemovalToolCfg(flags))
    theFPGATrackSimLogicalHistProcessAlg.OverlapRemoval_2nd = result.getPrimaryAndMerge(FPGATrackSimOverlapRemovalTool_2ndCfg(flags))
    theFPGATrackSimLogicalHistProcessAlg.TrackFitter_2nd = result.getPrimaryAndMerge(FPGATrackSimTrackFitterTool_2ndCfg(flags))


    if flags.Trigger.FPGATrackSim.ActiveConfig.secondStage:
        FPGATrackSimExtrapolatorTool = CompFactory.FPGATrackSimExtrapolator()
        FPGATrackSimExtrapolatorTool.Ncombinations = 16
        theFPGATrackSimLogicalHistProcessAlg.Extrapolator = FPGATrackSimExtrapolatorTool


    if flags.Trigger.FPGATrackSim.ActiveConfig.lrt:
        assert flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseBasicHitFilter != flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseMlHitFilter, 'Inconsistent LRT hit filtering setup, need either ML of Basic filtering enabled'
        assert flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseStraightTrackHT != flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseDoubletHT, 'Inconsistent LRT HT setup, need either double or strightTrack enabled'
        theFPGATrackSimLogicalHistProcessAlg.doLRT = True
        theFPGATrackSimLogicalHistProcessAlg.LRTHitFiltering = (not flags.Trigger.FPGATrackSim.ActiveConfig.lrtSkipHitFiltering)

    from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmConfig import FPGATrackSimLogicalHitsProcessAlgMonitoringCfg
    theFPGATrackSimLogicalHistProcessAlg.MonTool = result.getPrimaryAndMerge(FPGATrackSimLogicalHitsProcessAlgMonitoringCfg(flags))

    result.addEventAlgo(theFPGATrackSimLogicalHistProcessAlg)
    return result




if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.fillFromArgs()
    flags.GeoModel.AtlasVersion = "ATLAS-P2-RUN4-03-00-00"
    if isinstance(flags.Trigger.FPGATrackSim.wrapperFileName, str):
        log.info("wrapperFile is string, converting to list")
        flags.Trigger.FPGATrackSim.wrapperFileName = [flags.Trigger.FPGATrackSim.wrapperFileName]

    flags.Input.Files = lambda f: [f.Trigger.FPGATrackSim.wrapperFileName]
    flags.lock()
    acc=MainServicesCfg(flags)

    acc.addService(CompFactory.THistSvc(Output = ["EXPERT DATAFILE='monitoring.root', OPT='RECREATE'"]))
    acc.merge(FPGATrackSimLogicalHistProcessAlgCfg(flags)) 
    acc.store(open('AnalysisConfig.pkl','wb'))
    
    statusCode = acc.run()
    assert statusCode.isSuccess() is True, "Application execution did not succeed"

