# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import AthenaLogger
from PathResolver import PathResolver

log = AthenaLogger(__name__)

#### Now inmport Data Prep config from other file
from FPGATrackSimConfTools import FPGATrackSimDataPrepConfig


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
    FPGATrackSimWriteOutput.RWstatus = "RECREATE"
    result.addPublicTool(FPGATrackSimWriteOutput, primary=True)
    return result



def FPGATrackSimBankSvcCfg(flags):
    result=ComponentAccumulator()
    FPGATrackSimBankSvc = CompFactory.FPGATrackSimBankSvc()
    pathBankSvc = flags.Trigger.FPGATrackSim.bankDir if flags.Trigger.FPGATrackSim.bankDir != '' else f'/eos/atlas/atlascerngroupdisk/det-htt/HTTsim/{flags.GeoModel.AtlasVersion}/21.9.16/'+FPGATrackSimDataPrepConfig.getBaseName(flags)+'/SectorBanks/'
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
    FPGATrackSimBankSvc.constants_1st = f'{pathBankSvc}corrgen_raw_9L_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.gcon'
    FPGATrackSimBankSvc.constants_2nd = f'{pathBankSvc}corrgen_raw_13L_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.gcon'
    FPGATrackSimBankSvc.sectorBank_1st = f'{pathBankSvc}sectorsHW_raw_9L_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.patt'
    FPGATrackSimBankSvc.sectorBank_2nd = f'{pathBankSvc}sectorsHW_raw_13L_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.patt'
    FPGATrackSimBankSvc.sectorSlices = f'{pathBankSvc}slices_9L_reg{flags.Trigger.FPGATrackSim.region}.root'
    
    # These should be configurable. The tag system needs updating though.
    import FPGATrackSimConfTools.FPGATrackSimTagConfig as FPGATrackSimTagConfig
    bank_tag = FPGATrackSimTagConfig.getTags(stage='bank')['bank']
    FPGATrackSimBankSvc.sectorQPtBins = bank_tag['sectorQPtBins']
    FPGATrackSimBankSvc.qptAbsBinning = bank_tag['qptAbsBinning']

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
            tool.phiMin = flags.Trigger.FPGATrackSim.Hough1D.phiMin
            tool.phiMax = flags.Trigger.FPGATrackSim.Hough1D.phiMax
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
    result.addPublicTool(TF_1st, primary=True)
    return result

def FPGATrackSimOverlapRemovalToolCfg(flags):
    result=ComponentAccumulator()
    OR_1st = CompFactory.FPGATrackSimOverlapRemovalTool("FPGATrackSimOverlapRemovalTool_1st")
    OR_1st.ORAlgo = "Normal"
    OR_1st.doFastOR =flags.Trigger.FPGATrackSim.ActiveConfig.doFastOR
    OR_1st.NumOfHitPerGrouping = 5
    OR_1st.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    OR_1st.MinChi2 = flags.Trigger.FPGATrackSim.ActiveConfig.chi2cut    
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
    OR_2nd.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
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
    TF_2nd.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    if flags.Trigger.FPGATrackSim.ActiveConfig.secondStage:
        TF_2nd.Do2ndStageTrackFit = True 
    result.setPrivateTools(TF_2nd)
    return result

def prepareFlagsForFPGATrackSimLogicalHitsProcessAlg(flags):
    newFlags = flags.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag)
    return newFlags

def FPGATrackSimLogicalHitsProcessAlgCfg(inputFlags):

    flags = prepareFlagsForFPGATrackSimLogicalHitsProcessAlg(inputFlags)
   
    result=ComponentAccumulator()
    if not flags.Trigger.FPGATrackSim.wrapperFileName:
        from InDetConfig.InDetPrepRawDataFormationConfig import AthenaTrkClusterizationCfg
        result.merge(AthenaTrkClusterizationCfg(flags))

    theFPGATrackSimLogicalHitsProcessAlg=CompFactory.FPGATrackSimLogicalHitsProcessAlg()
    theFPGATrackSimLogicalHitsProcessAlg.writeOutputData = flags.Trigger.FPGATrackSim.ActiveConfig.writeOutputData
    theFPGATrackSimLogicalHitsProcessAlg.tracking = flags.Trigger.FPGATrackSim.tracking
    theFPGATrackSimLogicalHitsProcessAlg.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    theFPGATrackSimLogicalHitsProcessAlg.DoHoughRootOutput = flags.Trigger.FPGATrackSim.ActiveConfig.houghRootoutput
    theFPGATrackSimLogicalHitsProcessAlg.DoNNTrack = False
    theFPGATrackSimLogicalHitsProcessAlg.runOnRDO = not flags.Trigger.FPGATrackSim.wrapperFileName

    FPGATrackSimMaping = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    theFPGATrackSimLogicalHitsProcessAlg.FPGATrackSimMapping = FPGATrackSimMaping

    # If tracking is set to False, don't configure the bank service
    if flags.Trigger.FPGATrackSim.tracking:
        result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))

    if (flags.Trigger.FPGATrackSim.ActiveConfig.hough1D):
      theFPGATrackSimLogicalHitsProcessAlg.RoadFinder = result.getPrimaryAndMerge(FPGATrackSimRoadUnionTool1DCfg(flags))
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
    if flags.Trigger.FPGATrackSim.spacePoints:
        SPRoadFilter = CompFactory.FPGATrackSimSpacepointRoadFilterTool()
        SPRoadFilter.filtering = flags.Trigger.FPGATrackSim.ActiveConfig.spacePointFiltering
        SPRoadFilter.minSpacePlusPixel = flags.Trigger.FPGATrackSim.minSpacePlusPixel
        # TODO guard here against threshold being more than one value?
        if (flags.Trigger.FPGATrackSim.ActiveConfig.hough1D):
          SPRoadFilter.threshold = flags.Trigger.FPGATrackSim.Hough1D.threshold[0]
        else:
          SPRoadFilter.threshold = flags.Trigger.FPGATrackSim.ActiveConfig.threshold[0]
        SPRoadFilter.setSectors = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)

        theFPGATrackSimLogicalHitsProcessAlg.SPRoadFilterTool = SPRoadFilter
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

    # IDTPM flags
    from InDetTrackPerfMon.InDetTrackPerfMonFlags import initializeIDTPMConfigFlags, initializeIDTPMTrkAnaConfigFlags
    flags = initializeIDTPMConfigFlags(flags)
    
    flags.PhysVal.IDTPM.outputFilePrefix = "myIDTPM_CA"
    flags.PhysVal.IDTPM.plotsDefFileList = "InDetTrackPerfMon/PlotsDefFileList_default.txt" # default value - not needed
    flags.PhysVal.IDTPM.plotsCommonValuesFile = "InDetTrackPerfMon/PlotsDefCommonValues.json" # default value - not needed
    flags.PhysVal.OutputFileName = flags.PhysVal.IDTPM.outputFilePrefix + '.HIST.root' # automatically set in IDTPM config - not needed
    flags.Output.doWriteAOD_IDTPM = True
    flags.Output.AOD_IDTPMFileName = flags.PhysVal.IDTPM.outputFilePrefix + '.AOD_IDTPM.pool.root' # automatically set in IDTPM config - not needed
    flags.PhysVal.IDTPM.trkAnaCfgFile = "InDetTrackPerfMon/EFTrkAnaConfig_example.json"
    
    flags = initializeIDTPMTrkAnaConfigFlags(flags)
    ## override respective configurations from trkAnaCfgFile (in case something changes in the config file)
    flags.PhysVal.IDTPM.TrkAnaEF.TrigTrkKey = f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"
    flags.PhysVal.IDTPM.TrkAnaDoubleRatio.TrigTrkKey = f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"
    
    ############################################
    flags.Concurrency.NumThreads=1
    #flags.Concurrency.NumProcs=0
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True

    # flags.Exec.DebugStage="exec" # useful option to debug the execution of the job - we want it commented out for production
    flags.fillFromArgs()
    if isinstance(flags.Trigger.FPGATrackSim.wrapperFileName, str):
        log.info("wrapperFile is string, converting to list")
        flags.Trigger.FPGATrackSim.wrapperFileName = [flags.Trigger.FPGATrackSim.wrapperFileName]
        flags.Input.Files = lambda f: [f.Trigger.FPGATrackSim.wrapperFileName]

    flags.lock()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig","Tracking.MainPass")
    acc=MainServicesCfg(flags)

    acc.addService(CompFactory.THistSvc(Output = ["EXPERT DATAFILE='monitoring.root', OPT='RECREATE'"]))
    acc.addService(CompFactory.THistSvc(Output = ["TRIGFPGATrackSimHOUGHOUTPUT DATAFILE='HoughRootOutput.root', OPT='RECREATE'"]))

    
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

    if flags.Trigger.FPGATrackSim.doEDMConversion:
        acc.merge(FPGATrackSimDataPrepConfig.FPGAConversionAlgCfg(flags, name = 'FPGAConversionAlg_1st', stage = '_1st', doActsTrk=True))
        if flags.Trigger.FPGATrackSim.spacePoints : acc.merge(FPGATrackSimDataPrepConfig.FPGAConversionAlgCfg(flags, name = 'FPGAConversionAlgSpacePoints_1st', stage = '_1st', doSP = True, doClusters = False, doHits = False)) 
        
        from FPGATrackSimReporting.FPGATrackSimReportingConfig import FPGATrackSimReportingCfg
        acc.merge(FPGATrackSimReportingCfg(flags,perEventReports=False))        
        
        from FPGATrackSimPrototrackFitter.FPGATrackSimPrototrackFitterConfig import FPGATruthDecorationCfg, FPGAProtoTrackFitCfg
        acc.merge(FPGAProtoTrackFitCfg(flags,stage='_1st')) # Run ACTS KF for 1st stage
        acc.merge(FPGATruthDecorationCfg(flags,FinalProtoTrackChainxAODTracksKey=FinalProtoTrackChainxAODTracksKey,stage='_1st')) # Run ACTS KF for 1st stage
        if not flags.Trigger.FPGATrackSim.wrapperFileName:
            from FPGATrackSimConfTools.FPGATrackExtensionConfig import FPGATrackExtensionAlgCfg
            acc.merge(FPGATrackExtensionAlgCfg(flags, name="FPGATrackExtension", ProtoTracksLocation="ActsProtoTracks_1stFromFPGATrack")) # run CKF track extension on FPGA tracks

        if flags.Trigger.FPGATrackSim.writeToAOD: acc.merge(FPGATrackSimDataPrepConfig.WriteToAOD(flags, stage = '_1st'))
        if flags.Trigger.FPGATrackSim.Hough.secondStage : acc.merge(FPGATrackSimDataPrepConfig.FPGAConversionAlgCfg(flags, name = 'FPGAConversionAlg_2nd', stage = '_2nd')) # Default disabled, doesn't work if enabled
        if flags.Trigger.FPGATrackSim.convertUnmappedHits: acc.merge(FPGATrackSimDataPrepConfig.FPGAConversionAlgCfg(flags, name = 'FPGAConversionAlgUnmapped_1st', stage = 'Unmapped_1st', doClusters = False))
        if flags.Trigger.FPGATrackSim.Hough.hitFiltering : acc.merge(FPGATrackSimDataPrepConfig.FPGAConversionAlgCfg(flags, name = 'FPGAConversionAlgFiltered_1st', stage = 'Filtered_1st', doHits = False)) # Default disabled, works if enabled

        # Add the truth decorators
        from InDetPhysValMonitoring.InDetPhysValDecorationConfig import AddDecoratorCfg
        acc.merge(AddDecoratorCfg(flags))

        # # IDPVM running
        from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetPhysValMonitoringCfg
        acc.merge(InDetPhysValMonitoringCfg(flags))
    
    acc.store(open('AnalysisConfig.pkl','wb'))

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess() is True, "Application execution did not succeed"
