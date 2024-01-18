# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaConfiguration.Enums import LHCPeriod


class MuonCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the muon four-momentum correction"""

    def __init__ (self, containerName) :
        super (MuonCalibrationConfig, self).__init__ (containerName)
        self.containerName = containerName
        self.addOption ('postfix', "", type=str)
        self.addOption ('ptSelectionOutput', False, type=bool)
        self.addOption ('trackSelection', True, type=bool)
        self.addOption ('recalibratePhyslite', True, type=bool)

    def makeAlgs (self, config) :

        if config.isPhyslite() :
            config.setSourceName (self.containerName, "AnalysisMuons")
        else :
            config.setSourceName (self.containerName, "Muons")

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'MuonShallowCopyAlg' + self.postfix )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)

        # Set up the eta-cut on all muons prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                               'MuonEtaCutAlg' + self.postfix )
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.maxEta = 2.7
        alg.selectionDecoration = 'selectEta' + self.postfix + ',as_bits'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration)

        # Set up the track selection algorithm:
        if self.trackSelection :
            alg = config.createAlgorithm( 'CP::AsgLeptonTrackSelectionAlg',
                                'MuonTrackSelectionAlg' + self.postfix )
            alg.selectionDecoration = 'trackSelection' + self.postfix + ',as_bits'
            alg.maxD0Significance = 3
            alg.maxDeltaZ0SinTheta = 0.5
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration)

        # Set up the muon calibration and smearing algorithm:
        alg = config.createAlgorithm( 'CP::MuonCalibrationAndSmearingAlg',
                               'MuonCalibrationAndSmearingAlg' + self.postfix )
        config.addPrivateTool( 'calibrationAndSmearingTool',
                        'CP::MuonCalibTool' )
        alg.calibrationAndSmearingTool.calibMode = 2 # choose ID+MS with no sagitta bias
        alg.muons = config.readName (self.containerName)
        alg.muonsOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        if config.isPhyslite() and not self.recalibratePhyslite :
            alg.skipNominal = True

        # Set up the the pt selection
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'MuonPtCutAlg' + self.postfix )
        alg.selectionDecoration = 'selectPt' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.particles = config.readName (self.containerName)
        alg.selectionTool.minPt = 3e3
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             preselection = self.ptSelectionOutput)

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True)


class MuonWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the muon working point

    This may at some point be split into multiple blocks (10 Mar 22)."""

    def __init__ (self, containerName, selectionName) :
        super (MuonWorkingPointConfig, self).__init__ (containerName + '.' + selectionName)
        self.containerName = containerName
        self.selectionName = selectionName
        self.addOption ('postfix', selectionName, type=str)
        self.addOption ('quality', None, type=str)
        self.addOption ('isolation', None, type=str)
        self.addOption ('qualitySelectionOutput', True, type=bool)
        self.addOption ('systematicBreakdown', False, type=bool)
        self.addOption ('onlyRecoEffSF', False, type=bool)
        self.addOption ('noEffSF', False, type=bool)

    def makeAlgs (self, config) :
        from xAODMuon.xAODMuonEnums import xAODMuonEnums
        if self.quality == 'Tight' :
            quality = xAODMuonEnums.Quality.Tight
        elif self.quality == 'Medium' :
            quality = xAODMuonEnums.Quality.Medium
        elif self.quality == 'Loose' :
            quality = xAODMuonEnums.Quality.Loose
        elif self.quality == 'VeryLoose' :
            quality = xAODMuonEnums.Quality.VeryLoose
        elif self.quality == 'HighPt' :
            quality = 4
        elif self.quality == 'LowPtEfficiency' :
            quality = 5
        else :
            raise ValueError ("invalid muon quality: \"" + self.quality +
                              "\", allowed values are Tight, Medium, Loose, " +
                              "VeryLoose, HighPt, LowPtEfficiency")

        # The setup below is inappropriate for Run 1
        if config.geometry() is LHCPeriod.Run1:
            raise ValueError ("Can't set up the MuonWorkingPointConfig with %s, there must be something wrong!" % config.geometry().value)

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        # Setup the muon quality selection
        alg = config.createAlgorithm( 'CP::MuonSelectionAlgV2',
                               'MuonSelectionAlg' + postfix )
        config.addPrivateTool( 'selectionTool', 'CP::MuonSelectionTool' )
        alg.selectionTool.MuQuality = quality
        alg.selectionTool.IsRun3Geo = config.geometry() == LHCPeriod.Run3
        alg.selectionDecoration = 'good_muon' + postfix + ',as_bits'
        alg.badMuonVetoDecoration = 'is_bad' + postfix + ',as_char'
        alg.muons = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName,
                             alg.selectionDecoration,
                             preselection=self.qualitySelectionOutput)

        # Set up the isolation calculation algorithm:
        if self.isolation != 'NonIso' :
            alg = config.createAlgorithm( 'CP::MuonIsolationAlg',
                                   'MuonIsolationAlg' + postfix )
            config.addPrivateTool( 'isolationTool', 'CP::IsolationSelectionTool' )
            alg.isolationTool.MuonWP = self.isolation
            alg.isolationDecoration = 'isolated_muon' + postfix + ',as_bits'
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName,
                                 alg.isolationDecoration,
                                 preselection=self.qualitySelectionOutput)

        # Set up the reco/ID efficiency scale factor calculation algorithm:
        if config.dataType() is not DataType.Data and (not self.noEffSF or self.onlyRecoEffSF):
            alg = config.createAlgorithm( 'CP::MuonEfficiencyScaleFactorAlg',
                                   'MuonEfficiencyScaleFactorAlgReco' + postfix )
            config.addPrivateTool( 'efficiencyScaleFactorTool',
                            'CP::MuonEfficiencyScaleFactors' )
            alg.scaleFactorDecoration = 'muon_reco_effSF' + postfix + "_%SYS%"
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'muon_reco_bad_eff' + postfix
            alg.efficiencyScaleFactorTool.WorkingPoint = self.quality
            if config.geometry() == LHCPeriod.Run3:
                alg.efficiencyScaleFactorTool.CalibrationRelease = '230309_Preliminary_r22run3'
            alg.efficiencyScaleFactorTool.BreakDownSystematics = self.systematicBreakdown
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'reco_effSF' + postfix)

        # Set up the HighPt-specific BadMuonVeto efficiency scale factor calculation algorithm:
        if config.dataType() is not DataType.Data and self.quality == 'HighPt' and not self.onlyRecoEffSF and not self.noEffSF:
            alg = config.createAlgorithm( 'CP::MuonEfficiencyScaleFactorAlg',
                                   'MuonEfficiencyScaleFactorAlgBMVHighPt' + postfix )
            config.addPrivateTool( 'efficiencyScaleFactorTool',
                            'CP::MuonEfficiencyScaleFactors' )
            alg.scaleFactorDecoration = 'muon_BadMuonVeto_effSF' + postfix + "_%SYS%"
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'muon_BadMuonVeto_bad_eff' + postfix
            alg.efficiencyScaleFactorTool.WorkingPoint = 'BadMuonVeto_HighPt'
            if config.geometry() == LHCPeriod.Run3:
                alg.efficiencyScaleFactorTool.CalibrationRelease = '220817_Preliminary_r22run3' # not available as part of '230123_Preliminary_r22run3'!
            alg.efficiencyScaleFactorTool.BreakDownSystematics = self.systematicBreakdown
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'BadMuonVeto_effSF' + postfix)

        # Set up the isolation efficiency scale factor calculation algorithm:
        if config.dataType() is not DataType.Data and self.isolation != 'NonIso' and not self.onlyRecoEffSF and not self.noEffSF:
            alg = config.createAlgorithm( 'CP::MuonEfficiencyScaleFactorAlg',
                                   'MuonEfficiencyScaleFactorAlgIsol' + postfix )
            config.addPrivateTool( 'efficiencyScaleFactorTool',
                            'CP::MuonEfficiencyScaleFactors' )
            alg.scaleFactorDecoration = 'muon_isol_effSF' + postfix + "_%SYS%"
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'muon_isol_bad_eff' + postfix
            alg.efficiencyScaleFactorTool.WorkingPoint = self.isolation + 'Iso'
            if config.geometry() == LHCPeriod.Run3:
                alg.efficiencyScaleFactorTool.CalibrationRelease = '230123_Preliminary_r22run3'
            alg.efficiencyScaleFactorTool.BreakDownSystematics = self.systematicBreakdown
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'isol_effSF' + postfix)

        # Set up the TTVA scale factor calculation algorithm:
        if config.dataType() is not DataType.Data and not self.onlyRecoEffSF and not self.noEffSF:
            alg = config.createAlgorithm( 'CP::MuonEfficiencyScaleFactorAlg',
                                   'MuonEfficiencyScaleFactorAlgTTVA' + postfix )
            config.addPrivateTool( 'efficiencyScaleFactorTool',
                            'CP::MuonEfficiencyScaleFactors' )
            alg.scaleFactorDecoration = 'muon_TTVA_effSF' + postfix + "_%SYS%"
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'muon_TTVA_bad_eff' + postfix
            alg.efficiencyScaleFactorTool.WorkingPoint = 'TTVA'
            if config.geometry() == LHCPeriod.Run3:
                alg.efficiencyScaleFactorTool.CalibrationRelease = '230123_Preliminary_r22run3'
            alg.efficiencyScaleFactorTool.BreakDownSystematics = self.systematicBreakdown
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'TTVA_effSF' + postfix)

        # Set up an algorithm used for decorating baseline muon selection:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                                      'MuonSelectionSummary' + postfix )
        alg.selectionDecoration = 'baselineSelection' + postfix + ',as_char'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getFullSelection (self.containerName, self.selectionName)
        config.addOutputVar (self.containerName, 'baselineSelection' + postfix, 'select' + postfix)



def makeMuonCalibrationConfig( seq, containerName,
                               postfix = None, ptSelectionOutput = None):
    """Create muon calibration analysis algorithms

    This makes all the algorithms that need to be run first befor
    all working point specific algorithms and that can be shared
    between the working points.

    Keyword arguments:
      containerName -- name of the output container
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      ptSelectionOutput -- Whether or not to apply pt selection when creating
                           output containers.
    """

    config = MuonCalibrationConfig (containerName)
    config.setOptionValue ('postfix', postfix, noneAction='ignore')
    config.setOptionValue ('ptSelectionOutput', ptSelectionOutput, noneAction='ignore')
    seq.append (config)





def makeMuonWorkingPointConfig( seq, containerName, workingPoint, selectionName,
                                qualitySelectionOutput = None,
                                systematicBreakdown = None,
                                noEffSF = None,
                                onlyRecoEffSF = None ):
    """Create muon analysis algorithms for a single working point

    Keyword arguments:
      workingPoint -- The working point to use
      selectionName -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      qualitySelectionOutput -- Whether or not to apply muon quality selection
                                when creating output containers.
      systematicBreakdown -- enables the full breakdown of eff SF systematics
      noEffSF -- Disables the calculation of efficiencies and scale factors
      onlyRecoEffSF -- Only enables the reconstruction scale factor
    """


    config = MuonWorkingPointConfig (containerName, selectionName)
    if workingPoint is not None :
        splitWP = workingPoint.split ('.')
        if len (splitWP) != 2 :
            raise ValueError ('working point should be of format "quality.isolation", not ' + workingPoint)
        config.setOptionValue ('quality', splitWP[0])
        config.setOptionValue ('isolation', splitWP[1])
    config.setOptionValue ('qualitySelectionOutput', qualitySelectionOutput, noneAction='ignore')
    config.setOptionValue ('systematicBreakdown', systematicBreakdown, noneAction='ignore')
    config.setOptionValue ('noEffSF', noEffSF, noneAction='ignore')
    config.setOptionValue ('onlyRecoEffSF', onlyRecoEffSF, noneAction='ignore')
    seq.append (config)
