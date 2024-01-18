#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def createFPGATrackSimConfigFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    cf = AthConfigFlags()
    cf.addFlag('algoTag', 'Hough')
    cf.addFlag('wrapperFileName', [])
    cf.addFlag('mapsDir', '__MUST_BE_SET__')
    cf.addFlag('wrapperMetaData', "Default Meta Data")

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

    def __httDev21_02_15Flags():
        """Additional function delays import"""
        from FPGATrackSimConfTools.FPGATrackSimConfigFlags import createDev21_02_15_FPGATrackSimConfigFlags
        return createDev21_02_15_FPGATrackSimConfigFlags()
    cf.addFlagsCategory("Dev21_02_15", __httDev21_02_15Flags, prefix=True )

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
    cf.addFlag('spacePoints', False)
    cf.addFlag('spacePointFiltering', False)
    cf.addFlag('writeOutputData', True)

    # road finding selection
    cf.addFlag('houghRootoutput', False)
    cf.addFlag('hough', True)
    cf.addFlag('hough1D', False)

    # NN filtering
    cf.addFlag('trackNNAnalysis', False)

    # overlap removal
    cf.addFlag('doFastOR', False)

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
    cf.addFlag('threshold', [7])
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
    cf.addFlag('idealCoordFitType', 0)
    cf.addFlag('doDeltaGPhis', False)

    # second stage fitting
    cf.addFlag('secondStage', False)

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

    # FPGATrackSimLogicalHistProcessAlg
    # cf.addFlag('etaPatternRoadFilter', None) #TO DO (will be uncomment when the value is known)
    # cf.addFlag('phiRoadFilter', None) #TO DO (will be uncomment when the value is known)
    cf.addFlag('sampleType', '')

    return cf


def createHough1dFPGATrackSimConfigFlags():
    cf = createBasicFPGATrackSimConfigFlags()

    cf.name = 'hough_1d'

    cf.hough1D =  True

    cf.phiMax = 0.8
    cf.xBins = 200
    cf.hitExtendX = [1] * 8

    cf.addFlag('phiRangeCut', True)
    cf.addFlag('splitpt', 1)

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
    cf.hitExtendX = [2,1,0,0,0,0,0,0]

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







