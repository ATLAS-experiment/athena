# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AsgAnalysisAlgorithms.AnalysisObjectSharedSequence import makeSharedObjectSequence
from AnaAlgorithm.AnaAlgSequence import AnaAlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, addPrivateTool

def makeMuonAnalysisSequence( dataType, workingPoint,
                              deepCopyOutput = False,
                              shallowViewOutput = True,
                              postfix = '',
                              ptSelectionOutput = False,
                              trackSelection = True,
                              qualitySelectionOutput = True,
                              enableCutflow = False,
                              enableKinematicHistograms = False,
                              isRun3Geo = False,
                              defineSystObjectLinks = False ):
    """Create a muon analysis algorithm sequence

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
      workingPoint -- The working point to use
      deepCopyOutput -- If set to 'True', the output containers will be
                        standalone, deep copies (slower, but needed for xAOD
                        output writing)
      shallowViewOutput -- Create a view container if required
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      ptSelectionOutput -- Whether or not to apply pt selection when creating
                           output containers.
      trackSelection -- apply selection on tracks (d0, z0, siHits, etc.)
      qualitySelectionOutput -- Whether or not to apply muon quality selection
                                when creating output containers.
      enableCutflow -- Whether or not to dump the cutflow
      enableKinematicHistograms -- Whether or not to dump the kinematic histograms
    """

    if dataType not in ["data", "mc", "afii"] :
        raise ValueError ("invalid data type: " + dataType)

    if postfix != '' :
        postfix = '_' + postfix
        pass

    # Create the analysis algorithm sequence object:
    seq = AnaAlgSequence( "MuonAnalysisSequence" + postfix )

    seq.addMetaConfigDefault ("selectionDecorNames", [])
    seq.addMetaConfigDefault ("selectionDecorNamesOutput", [])
    seq.addMetaConfigDefault ("selectionDecorCount", [])

    makeMuonCalibrationSequence (seq, dataType, postfix=postfix,
                                 ptSelectionOutput = ptSelectionOutput, 
                                 trackSelection = trackSelection)
    makeMuonWorkingPointSequence (seq, dataType, workingPoint, postfix=postfix,
                                  qualitySelectionOutput = qualitySelectionOutput, isRun3Geo = isRun3Geo)
    makeSharedObjectSequence (seq, deepCopyOutput = deepCopyOutput,
                              shallowViewOutput = shallowViewOutput,
                              postfix = '_Muon' + postfix,
                              enableCutflow = enableCutflow,
                              enableKinematicHistograms = enableKinematicHistograms,
                              defineSystObjectLinks = defineSystObjectLinks )

    # Return the sequence:
    return seq





def makeMuonCalibrationSequence( seq, dataType,
                                 postfix = '', ptSelectionOutput = False, trackSelection = False):
    """Create muon calibration analysis algorithms

    This makes all the algorithms that need to be run first befor
    all working point specific algorithms and that can be shared
    between the working points.

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      ptSelectionOutput -- Whether or not to apply pt selection when creating
                           output containers.
    """

    if dataType not in ["data", "mc", "afii"] :
        raise ValueError ("invalid data type: " + dataType)

    # Set up a shallow copy to decorate
    alg = createAlgorithm( 'CP::AsgShallowCopyAlg', 'MuonShallowCopyAlg' + postfix )
    seq.append( alg, inputPropName = 'input',
                outputPropName = 'output',
                stageName = 'prepare')

    # Set up the eta-cut on all muons prior to everything else
    alg = createAlgorithm( 'CP::AsgSelectionAlg',
                           'MuonEtaCutAlg' + postfix )
    addPrivateTool( alg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    alg.selectionTool.maxEta = 2.7
    alg.selectionDecoration = 'selectEta' + postfix + ',as_bits'
    seq.append( alg, inputPropName = 'particles',
                stageName = 'selection',
                metaConfig = {'selectionDecorNames' : [alg.selectionDecoration],
                              'selectionDecorNamesOutput' : [alg.selectionDecoration],
                              'selectionDecorCount' : [2]},
                dynConfig = {'preselection' : lambda meta : "&&".join (meta["selectionDecorNamesOutput"])})

    # Set up the track selection algorithm:
    if trackSelection:
        alg = createAlgorithm( 'CP::AsgLeptonTrackSelectionAlg',
                               'MuonTrackSelectionAlg' + postfix )
        alg.selectionDecoration = 'trackSelection' + postfix + ',as_bits'
        alg.maxD0Significance  = 3
        alg.maxDeltaZ0SinTheta = 0.5
        seq.append( alg, inputPropName = 'particles',
                    stageName = 'selection',
                    metaConfig = {'selectionDecorNames' : [alg.selectionDecoration],
                                'selectionDecorNamesOutput' : [alg.selectionDecoration],
                                'selectionDecorCount' : [3]},
                    dynConfig = {'preselection' : lambda meta : "&&".join (meta["selectionDecorNamesOutput"])})

    # Set up the muon calibration and smearing algorithm:
    alg = createAlgorithm( 'CP::MuonCalibrationAndSmearingAlg',
                           'MuonCalibrationAndSmearingAlg' + postfix )
    addPrivateTool( alg, 'calibrationAndSmearingTool',
                    'CP::MuonCalibTool' )
    alg.calibrationAndSmearingTool.calibMode = 2 # choose ID+MS with no sagitta bias
    seq.append( alg, inputPropName = 'muons', outputPropName = 'muonsOut',
                stageName = 'calibration',
                dynConfig = {'preselection' : lambda meta : "&&".join (meta["selectionDecorNamesOutput"])})

    # Set up the the pt selection
    alg = createAlgorithm( 'CP::AsgSelectionAlg', 'MuonPtCutAlg' + postfix )
    alg.selectionDecoration = 'selectPt' + postfix + ',as_bits'
    addPrivateTool( alg, 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
    alg.selectionTool.minPt = 3e3
    seq.append( alg, inputPropName = 'particles',
                stageName = 'selection',
                metaConfig = {'selectionDecorNames' : [alg.selectionDecoration],
                              'selectionDecorNamesOutput' : [alg.selectionDecoration] if ptSelectionOutput else [],
                              'selectionDecorCount' : [2]},
                dynConfig = {'preselection' : lambda meta : "&&".join (meta["selectionDecorNamesOutput"])})
    pass





def makeMuonWorkingPointSequence( seq, dataType, workingPoint, postfix = '',
                                  qualitySelectionOutput = True, isRun3Geo = False):
    """Create muon analysis algorithms for a single working point

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
      workingPoint -- The working point to use
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      qualitySelectionOutput -- Whether or not to apply muon quality selection
                                when creating output containers.
      isRun3Geo -- switches the muon selection tool to run 3 geometry
    """

    if dataType not in ["data", "mc", "afii"] :
        raise ValueError ("invalid data type: " + dataType)

    splitWP = workingPoint.split ('.')
    if len (splitWP) != 2 :
        raise ValueError ('working point should be of format "quality.isolation", not ' + workingPoint)

    sfWorkingPoint = splitWP[0]
    from xAODMuon.xAODMuonEnums import xAODMuonEnums
    if splitWP[0] == 'Tight' :
        quality = xAODMuonEnums.Quality.Tight
        pass
    elif splitWP[0] == 'Medium' :
        quality = xAODMuonEnums.Quality.Medium
        pass
    elif splitWP[0] == 'Loose' :
        quality = xAODMuonEnums.Quality.Loose
        pass
    elif splitWP[0] == 'VeryLoose' :
        quality = xAODMuonEnums.Quality.VeryLoose
        pass
    elif splitWP[0] == 'HighPt' :
        quality = 4
        pass
    elif splitWP[0] == 'LowPtEfficiency' :
        quality = 5
        pass
    else :
        raise ValueError ("invalid muon quality: \"" + splitWP[0] +
                          "\", allowed values are Tight, Medium, Loose, " +
                          "VeryLoose, HighPt, LowPtEfficiency")

    # Setup the muon quality selection
    alg = createAlgorithm( 'CP::MuonSelectionAlgV2',
                           'MuonSelectionAlg' + postfix )
    addPrivateTool( alg, 'selectionTool', 'CP::MuonSelectionTool' )
    alg.selectionTool.MuQuality = quality
    alg.selectionTool.IsRun3Geo = isRun3Geo
    alg.selectionDecoration = 'good_muon' + postfix + ',as_bits'
    alg.badMuonVetoDecoration = 'is_bad' + postfix + ',as_char'
    seq.append( alg, inputPropName = 'muons',
                stageName = 'selection',
                metaConfig = {'selectionDecorNames' : [alg.selectionDecoration],
                              'selectionDecorNamesOutput' : [alg.selectionDecoration] if qualitySelectionOutput else [],
                              'selectionDecorCount' : [4]},
                dynConfig = {'preselection' : lambda meta : "&&".join (meta["selectionDecorNamesOutput"])})

    # Set up the isolation calculation algorithm:
    if splitWP[1] != 'NonIso' :
        alg = createAlgorithm( 'CP::MuonIsolationAlg',
                               'MuonIsolationAlg' + postfix )
        addPrivateTool( alg, 'isolationTool', 'CP::IsolationSelectionTool' )
        alg.isolationTool.MuonWP = splitWP[1]
        alg.isolationDecoration = 'isolated_muon' + postfix + ',as_bits'
        seq.append( alg, inputPropName = 'muons',
                    stageName = 'selection',
                    metaConfig = {'selectionDecorNames' : [alg.isolationDecoration],
                                  'selectionDecorNamesOutput' : [alg.isolationDecoration],
                                  'selectionDecorCount' : [1]},
                    dynConfig = {'preselection' : lambda meta : "&&".join (meta["selectionDecorNamesOutput"])})
        pass

    # Set up the efficiency scale factor calculation algorithm:
    alg = createAlgorithm( 'CP::MuonEfficiencyScaleFactorAlg',
                           'MuonEfficiencyScaleFactorAlg' + postfix )
    addPrivateTool( alg, 'efficiencyScaleFactorTool',
                    'CP::MuonEfficiencyScaleFactors' )
    alg.scaleFactorDecoration = 'muon_effSF' + postfix + "_%SYS%"
    alg.outOfValidity = 2 #silent
    alg.outOfValidityDeco = 'bad_eff' + postfix
    alg.efficiencyScaleFactorTool.WorkingPoint = sfWorkingPoint
    if isRun3Geo:
        alg.efficiencyScaleFactorTool.CalibrationRelease = '230309_Preliminary_r22run3'
    if dataType != 'data':
        seq.append( alg, inputPropName = 'muons',
                    stageName = 'efficiency',
                    dynConfig = {'preselection' : lambda meta : "&&".join (meta["selectionDecorNamesOutput"])})

    # Set up an algorithm used for decorating baseline muon selection:
    alg = createAlgorithm( 'CP::AsgSelectionAlg',
                           'MuonSelectionSummary' + postfix )
    alg.selectionDecoration = 'baselineSelection' + postfix + ',as_char'
    seq.append( alg, inputPropName = 'particles',
                stageName = 'selection',
                dynConfig = {'preselection' : lambda meta : "&&".join (meta["selectionDecorNames"])})
    pass
