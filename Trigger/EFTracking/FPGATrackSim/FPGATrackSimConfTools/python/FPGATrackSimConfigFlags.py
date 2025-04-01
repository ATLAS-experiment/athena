#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import AthenaCommon.Constants
from AthenaConfiguration.Enums import FlagEnum

def createFPGATrackSimConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    cf = AthConfigFlags()
    cf.addFlag('pipeline', '')
    cf.addFlag('algoTag', 'Hough')
    cf.addFlag('wrapperFileName', [])
    cf.addFlag('wrapperFileName2', [])
    cf.addFlag('secondInputToolN', 0)
    cf.addFlag('mapsDir', '__MUST_BE_SET__')
    cf.addFlag('wrapperMetaData', "Default Meta Data")
    cf.addFlag('sampleType', 'singleMuons')
    cf.addFlag('FPGATrackSimMatrixFileRegEx', [])
    cf.addFlag('FPGATrackSimMaxnMatrixInputFiles', -1)
    cf.addFlag('outputMergedFPGATrackSimMatrixFile', 'combined_matrix.root')
    cf.addFlag('FPGATrackSimNBanks', 1)
    cf.addFlag('FPGATrackSimallBanks', False)
    cf.addFlag('region', 34)
    cf.addFlag('d0min', -2.0)
    cf.addFlag('d0max', +2.0)    
    cf.addFlag('z0min', -150.0)
    cf.addFlag('z0max', +150.0)
    cf.addFlag('qOverPtmin', -0.001)
    cf.addFlag('qOverPtmax', +0.001)
    cf.addFlag('oldRegionDefs', False)
    cf.addFlag('phiShift', 0.0)
    cf.addFlag('minSpacePlusPixel', 3)
    cf.addFlag('baseName', '')    
    cf.addFlag('CheckGood2ndStage', True)
    cf.addFlag('Is2ndStage', False)
    cf.addFlag('UseHitScaleFactor', False)
    cf.addFlag('missHitsConsts', False)
    cf.addFlag('tracking', False)
    cf.addFlag('doOverlapRemoval', True)
    cf.addFlag('clustering', 1)
    cf.addFlag('bankDir', '')
    cf.addFlag('slicesFile', 'eventSelectionSlices/v1.0/slices.txt')
    cf.addFlag('spacePoints', True)
    cf.addFlag('outputMonitorFile',"monitoring.root")
    cf.addFlag('connectToToITkTracking',True)
    cf.addFlag('loglevel', AthenaCommon.Constants.INFO)
    cf.addFlag('msgLimit',-1)
    cf.addFlag('singleTrackSample',  True)
    cf.addFlag('FakeNNonnxFile', 'banks_9L/v0.20/ClassificationHT_v5.onnx')
    cf.addFlag('ExtensionNNVolonnxFile', 'banks_9L/v0.20/HT_detector_v6_3.onnx')
    cf.addFlag('ExtensionNNHitonnxFile', 'banks_9L/v0.20/Ath_Extrap_v51_6_superBig_0_outsideIN.onnx')
    cf.addFlag('ParamNNonnxFile', 'banks_9L/v0.20/ParamEstimationHT_v5.onnx')
    cf.addFlag('doNNPathFinder', False)
    cf.addFlag('windowR', [20])
    cf.addFlag('windowZ', [20])
    cf.addFlag('lowPtvalueR', -1)
    cf.addFlag('lowPtvalueZ', -1)
    cf.addFlag('lowPtWindowRScaling', 1.0)
    cf.addFlag('lowPtWindowZScaling', 1.0)
    cf.addFlag('missedHitRScaling', -1.0)
    cf.addFlag('missedHitZScaling', -1.0)    
    cf.addFlag('maxBranches', -1)
    cf.addFlag('hitThreshold', 10)
    
    def __httHough1DFlags():
        """Additional function delays import"""
        from FPGATrackSimConfTools.FPGATrackSimConfigFlags import createHough1dFPGATrackSimConfigFlags
        return createHough1dFPGATrackSimConfigFlags()
    cf.addFlagsCategory("Hough1D", __httHough1DFlags, prefix=True )

    def __httHoughFlags():
        """Additional function delays import"""
        from FPGATrackSimConfTools.FPGATrackSimConfigFlags import createHoughFPGATrackSimConfigFlags
        return createHoughFPGATrackSimConfigFlags()
    cf.addFlagsCategory("Hough", __httHoughFlags, prefix=True )

    def __httGenScanFlags():
        """Additional function delays import"""
        from FPGATrackSimConfTools.FPGATrackSimConfigFlags import createGenScanFPGATrackSimConfigFlags
        return createGenScanFPGATrackSimConfigFlags()
    cf.addFlagsCategory("GenScan", __httGenScanFlags, prefix=True )

    def __httDev21_02_15Flags():
        """Additional function delays import"""
        from FPGATrackSimConfTools.FPGATrackSimConfigFlags import createDev21_02_15_FPGATrackSimConfigFlags
        return createDev21_02_15_FPGATrackSimConfigFlags()
    cf.addFlagsCategory("Dev21_02_15", __httDev21_02_15Flags, prefix=True )

    def __GNNFlags():
        """Additional function delays import"""
        from FPGATrackSimConfTools.FPGATrackSimConfigFlags import createGNNFPGATrackSimConfigFlags
        return createGNNFPGATrackSimConfigFlags()
    cf.addFlagsCategory("GNN", __GNNFlags, prefix=True )

    # EDM conversion
    cf.addFlag('doEDMConversion', False)
    cf.addFlag('convertUnmappedHits', False)
    cf.addFlag('writeToAOD', False)
    
    # Monitoring
    cf.addFlag('writeAdditionalOutputData', True)
    
    # ACTS Tracking
    cf.addFlag('runCKF',True)
    cf.addFlag('useFPGATruthTrackMatching',False)
    return cf


def createBasicFPGATrackSimConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    cf = AthConfigFlags()

    # common
    cf.addFlag('name', '')

    # input
    cf.addFlag('firstInputToolN', 1)
    cf.addFlag('realHitsOverlay', False)
    cf.addFlag('hitFiltering', False)
    cf.addFlag('spacePointFiltering', False)
    cf.addFlag('writeTestOutput', True)

    # road finding selection
    cf.addFlag('houghRootoutput1st', False)
    cf.addFlag('houghRootoutput2nd', False)
    cf.addFlag('hough', True)
    cf.addFlag('hough1D', False)
    cf.addFlag('genScan', False)
    cf.addFlag('NumOfHitPerGrouping', 5)

    cf.addFlag('etaPatternFilter', False)
    cf.addFlag('phiRoadFilter', False)    
    cf.addFlag('GNN', False)


    # NN filtering
    cf.addFlag('trackNNAnalysis', False)

    # overlap removal
    cf.addFlag('doFastOR', False)

    #pass lowest chi2 track
    cf.addFlag('passLowestChi2TrackOnly', False)

    # hough
    cf.addFlag('xVar', 'phi')
    cf.addFlag('yVar', 'q/pt')
    cf.addFlag('qptMin', -1.0)
    cf.addFlag('qptMax', 1.0)
    cf.addFlag('d0Slices', [])
    cf.addFlag('slicing', True)
    cf.addFlag('localMaxWindowSize', 0)
    cf.addFlag('roadSliceOR', False)
    cf.addFlag('fieldCorrection', True)
    cf.addFlag('phiMin', 0.0)
    cf.addFlag('phiMax', 0.0)
    cf.addFlag('xBins', 216)
    cf.addFlag('yBins', 216)
    cf.addFlag('xBufferBins', 6)
    cf.addFlag('yBufferBins', 2)
    cf.addFlag('threshold', [8])
    cf.addFlag('IdealGeoRoads', True)
    cf.addFlag('convolution', [])
    cf.addFlag('convSizeX', 0)
    cf.addFlag('convSizeY', 0)
    cf.addFlag('hitExtendX', [])

    cf.addFlag('doMajority', 1)

    cf.addFlag('doTracking', False)
    cf.addFlag('outputHitTxt', False)

    # performance monitoring
    cf.addFlag('barcodeFracMatch', 0.5)

    # track Fitter
    cf.addFlag('chi2DoFRecoveryMin', 40)
    cf.addFlag('chi2DoFRecoveryMax', 20000)
    cf.addFlag('nHitsNoRecovery', -1)
    cf.addFlag('guessHits', True)
    cf.addFlag('doMissingHitsChecks', False)
    cf.addFlag('idealCoordFitType', 2)
    cf.addFlag('doDeltaGPhis', False)
    cf.addFlag('chi2cut', 9)
    cf.addFlag('useVaryingChi2Cut', False)
    # second stage fitting
    cf.addFlag('secondStage', False)
    cf.addFlag('secondChi2Cut', 36)

    # fast monitoring
    cf.addFlag('fastMon', False)
    cf.addFlag('canExtendHistRanges', False)
    cf.addFlag('dataClass', 2)

    # lrt settings
    cf.addFlag('lrt', False)
    cf.addFlag('lrtUseBasicHitFilter', False)
    cf.addFlag('lrtUseMlHitFilter', False)
    cf.addFlag('lrtUseStraightTrackHT', False)
    cf.addFlag('lrtUseDoubletHT', False)
    cf.addFlag('lrtDoubletD0Range', 120.0)
    cf.addFlag('lrtDoubletD0Bins', 216)
    cf.addFlag('lrtDoubletQptRange', 0.02)
    cf.addFlag('lrtDoubletQptBins', 216)
    cf.addFlag('lrtMonPhiRange', (0.2, 0.5))
    cf.addFlag('lrtMonD0Range', (-100,100))
    cf.addFlag('lrtMonZ0Range', (-300,300)) 
    cf.addFlag('sampleType', '')

    return cf


def createHough1dFPGATrackSimConfigFlags():
    cf = createBasicFPGATrackSimConfigFlags()

    cf.name = 'hough_1d'

    cf.hough1D =  True

    cf.phiMin = 0.0
    cf.phiMax = 0.8
    cf.xBins = 200
    cf.threshold = [7]
    cf.hitExtendX = [1] * 9

    cf.addFlag('phiRangeCut', True)
    cf.addFlag('splitpt', 1)
    cf.addFlag('phifilterwindow', 0.005)

    cf.outputHitTxt = ""

    return cf


def createHoughFPGATrackSimConfigFlags():
    cf = createBasicFPGATrackSimConfigFlags()

    cf.name = 'hough'
    cf.hough = True


    cf.phiMin = 0.3
    cf.phiMax = 0.5
    cf.xBins = 216
    cf.yBins = 216
    cf.xBufferBins = 6
    cf.yBufferBins = 2
    cf.addFlag('combineLayers', [])
    cf.addFlag('scale', [])
    cf.hitExtendX = [2,1,0,0,0,0,0,0,0]

    cf.addFlag('lrtSkipHitFiltering', False)
    cf.addFlag('lrtPtmin', 5)
    cf.addFlag('allowHighTruthBarcode', False)
    cf.addFlag('mLRTpdgID', 0)
    cf.addFlag('lrtStraighttrackXVar', 'phi')
    cf.addFlag('lrtStraighttrackYVar', 'd0')
    cf.addFlag('lrtStraighttrackPhiMin', 0.3)
    cf.addFlag('lrtStraighttrackPhiMax', 0.5)
    cf.addFlag('lrtStraighttrackD0Min', -300.0)
    cf.addFlag('lrtStraighttrackD0Max', 300.0)
    cf.addFlag('lrtStraighttrackXBins', 216)
    cf.addFlag('lrtStraighttrackYBins', 216)
    cf.addFlag('lrtStraighttrackXBufferBins', 6)
    cf.addFlag('lrtStraighttrackYBufferBins', 2)
    cf.addFlag('lrtStraighttrackSlicing', True)
    cf.addFlag('lrtStraighttrackThreshold', [1])
    cf.addFlag('lrtStraighttrackConvolution', [])
    cf.addFlag('lrtStraighttrackCombineLayers', [0,1,2,3,4,5,6,7])
    cf.addFlag('lrtStraighttrackScale', [1])
    cf.addFlag('lrtStraighttrackConvSizeX', 0)
    cf.addFlag('lrtStraighttrackConvSizeY', 0)
    cf.addFlag('lrtStraighttrackHitExtendX', [])
    cf.addFlag('lrtStraighttrackStereo', False)
    cf.addFlag('lrtStraighttrackLocalMaxWindowSize', 0) 

    return cf 


def createDev21_02_15_FPGATrackSimConfigFlags():
    cf = createBasicFPGATrackSimConfigFlags()

    cf.name = 'dev_21-02-15'
    cf.phiMin = 0.3
    cf.phiMax = 0.5
    cf.xBins = 64
    cf.yBins = 216
    cf.xBufferBins = 2
    cf.yBufferBins = 2
    cf.threshold = [70]
    cf.convolution = [1, 10, 1]
    cf.addFlag('combineLayers', [])
    cf.addFlag('scale', [])
    cf.convSizeX = 3
    cf.convSizeY = 1

    cf.doMajority = 0
    cf.doTracking = True

    return cf

def createGenScanFPGATrackSimConfigFlags():
    cf = createBasicFPGATrackSimConfigFlags()

    cf.name = 'genScan'
    cf.addFlag('genScanCuts','FPGATrackSimGenScanCuts')    
    cf.addFlag('reverse','True')
    cf.addFlag('binFilter','IncrementalBuild')
    cf.addFlag('layerStudy',False)
    cf.addFlag('layerMapFile','')
    cf.addFlag('noCuts',False)
    
    return cf

class graphTool(FlagEnum):
    ModuleMap = 'ModuleMap'

class moduleMapType(FlagEnum):
    doublet = 'doublet'

class moduleMapFunc(FlagEnum):
    minmax = 'minmax'

class roadMakerTool(FlagEnum):
    ConnectedComponents = 'ConnectedComponents'

def createGNNFPGATrackSimConfigFlags():
    cf = createBasicFPGATrackSimConfigFlags()

    cf.name = 'GNN'
    cf.addFlag("graphTool", graphTool.ModuleMap, type=graphTool)
    cf.addFlag("moduleMapType", moduleMapType.doublet, type=moduleMapType)
    cf.addFlag("moduleMapFunc", moduleMapFunc.minmax, type=moduleMapFunc)
    cf.addFlag("moduleMapTol",0.0000000001) # 1e-10
    cf.addFlag("moduleMapPath",'')
    cf.addFlag("GNNModelPath",'') 
    cf.addFlag("roadMakerTool", roadMakerTool.ConnectedComponents, type=roadMakerTool)
    cf.addFlag("edgeScoreCut",0.8)
    cf.addFlag("doGNNRootOutput",False)
    
    return cf

#####################################################################
#####################################################################
#####################################################################

if __name__ == "__main__":

  flags = createFPGATrackSimConfigFlags()
  flags.loadAllDynamicFlags()
  flags.initAll()
  flags.dump()
  assert flags.Hough.firstInputToolN == 1 , "default firstInputToolN  is wrong"
  assert flags.Hough.barcodeFracMatch == 0.5, "default barcodeFracMatch  is wrong"
  assert flags.Hough.fastMon is False , "default fastMon is wrong"
  assert flags.Hough.lrtMonZ0Range ==  (-300,300), "default lrtMonZ0Rang is wrong"

  print( "allok" )   







    
