# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaCommon.SystemOfUnits	import GeV
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaConfiguration.Enums import LHCPeriod
from TrigGlobalEfficiencyCorrection.TriggerLeg_DictHelpers import TriggerDict
from Campaigns.Utils import Campaign
from AthenaCommon.Logging import logging


class MuonCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the muon four-momentum correction"""

    def __init__ (self, containerName='') :
        super (MuonCalibrationConfig, self).__init__ ()
        self.setBlockName('Muons')
        self.addOption ('inputContainer', '', type=str,
            info="select muon input container, by default set to Muons")
        self.addOption ('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the output container after calibration.")
        self.addOption ('postfix', "", type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here since the calibration is common to "
            "all muons.")
        self.addOption ('minPt', 3.0*GeV, type=float,
            info="pT cut to apply to calibrated muons, in MeV. "
            "The default is 3.0 GeV.")
        self.addOption ('recalibratePhyslite', True, type=bool,
            info="whether to run the CP::EgammaCalibrationAndSmearingAlg on "
            "PHYSLITE derivations. The default is True.")
        self.addOption ('maxEta', 2.7, type=float,
            info="maximum muon |eta| (float). The default is 2.7.")
        self.addOption ('excludeNSWFromPrecisionLayers', False, type=bool,
            info="only for testing purposes, turn on to ignore NSW hits and "
            "fix a crash with older derivations (p-tag <p5834)")
        self.addOption ('calibMode', 'correctData_CB', type=str, info='calibration mode of the MuonCalibTool needed to turn on the sagitta bias corrections and to select the muon track calibration type (CB or ID+MS)')
        self.addOption ('decorateTruth', False, type=bool,
            info="decorate truth particle information on the reconstructed one")
        self.addOption ('writeTrackD0Z0', False, type = bool,
            info="save the d0 significance and z0sinTheta variables so they can be written out")

    def makeAlgs (self, config) :

        log = logging.getLogger('MuonCalibrationConfig')

        #make sure that this is sync with
        #PhysicsAnalysis/MuonID/MuonIDAnalysis/MuonMomentumCorrections/MuonMomentumCorrections/MuonCalibTool.h#L31-37
        if self.calibMode == 'correctData_CB':
            calibMode = 0
        elif self.calibMode == 'correctData_IDMS':
            calibMode = 1
        elif self.calibMode == 'notCorrectData_IDMS':
            calibMode = 2
        elif self.calibMode == 'notCorrectData_CB':
            calibMode = 3
        else :
            raise ValueError ("invalid calibMode: \"" + self.calibMode + "\". Allowed values are correctData_CB, correctData_IDMS, notCorrectData_IDMS, notCorrectData_CB")

        inputContainer = "AnalysisMuons" if config.isPhyslite() else "Muons"
        if self.inputContainer:
            inputContainer = self.inputContainer
        config.setSourceName (self.containerName, inputContainer)
        config.setContainerMeta (self.containerName, 'calibMode', calibMode)

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'MuonShallowCopyAlg' + self.postfix )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)

        # Set up the eta-cut on all muons prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                               'MuonEtaCutAlg' + self.postfix )
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.maxEta = self.maxEta
        alg.selectionDecoration = 'selectEta' + self.postfix + ',as_bits'
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration)

        # Set up the muon calibration and smearing algorithm:
        alg = config.createAlgorithm( 'CP::MuonCalibrationAndSmearingAlg',
                               'MuonCalibrationAndSmearingAlg' + self.postfix )
        config.addPrivateTool( 'calibrationAndSmearingTool',
                        'CP::MuonCalibTool' )

        alg.calibrationAndSmearingTool.IsRun3Geo = config.geometry() >= LHCPeriod.Run3
        alg.calibrationAndSmearingTool.calibMode = calibMode
        if config.geometry() is LHCPeriod.Run4:
            log.warning("Disabling NSW hits for Run4 geometry")
            alg.calibrationAndSmearingTool.ExcludeNSWFromPrecisionLayers = True
        else:
            alg.calibrationAndSmearingTool.ExcludeNSWFromPrecisionLayers = self.excludeNSWFromPrecisionLayers and (config.geometry() >= LHCPeriod.Run3)
        alg.muons = config.readName (self.containerName)
        alg.muonsOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        if config.isPhyslite() and not self.recalibratePhyslite :
            alg.skipNominal = True

        # Set up the the pt selection
        if self.minPt > 0:
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'MuonPtCutAlg' + self.postfix )
            alg.selectionDecoration = 'selectPt' + self.postfix + ',as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
            alg.particles = config.readName (self.containerName)
            alg.selectionTool.minPt = self.minPt
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                preselection = True)

        # Additional decorations
        if self.writeTrackD0Z0:
            alg = config.createAlgorithm( 'CP::AsgLeptonTrackDecorationAlg',
                                          'LeptonTrackDecorator' + self.containerName + self.postfix,
                                           reentrant=True )
            alg.particles = config.readName (self.containerName)

        alg = config.createAlgorithm( 'CP::AsgEnergyDecoratorAlg', 'EnergyDecorator' + self.containerName + self.postfix )
        alg.particles = config.readName (self.containerName)

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'e_%SYS%', 'e')
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True)

        if self.writeTrackD0Z0:
            config.addOutputVar (self.containerName, 'd0_%SYS%', 'd0', noSys=True)
            config.addOutputVar (self.containerName, 'd0sig_%SYS%', 'd0sig', noSys=True)
            config.addOutputVar (self.containerName, 'z0sintheta_%SYS%', 'z0sintheta', noSys=True)
            config.addOutputVar (self.containerName, 'z0sinthetasig_%SYS%', 'z0sinthetasig', noSys=True)

        # decorate truth information on the reconstructed object:
        if self.decorateTruth and config.dataType() is not DataType.Data:
            config.addOutputVar (self.containerName, "truthType", "truth_type", noSys=True)
            config.addOutputVar (self.containerName, "truthOrigin", "truth_origin", noSys=True)

class MuonWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the muon working point

    This may at some point be split into multiple blocks (10 Mar 22)."""

    def __init__ (self, containerName='', selectionName='') :
        super (MuonWorkingPointConfig, self).__init__ ()
        self.setBlockName('MuonsWorkingPoint')
        self.addOption ('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('selectionName', selectionName, type=str,
            noneAction='error',
            info="the name of the muon selection to define (e.g. tight or loose).")
        self.addOption ('postfix', selectionName, type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as selectionName is used internally.")
        self.addOption ('trackSelection', True, type=bool,
            info="whether or not to set up an instance of "
            "CP::AsgLeptonTrackSelectionAlg, with the recommended d_0 and "
            "z_0 sin(theta) cuts. The default is True.")
        self.addOption ('maxD0Significance', 3, type=float,
            info="maximum d0 significance used for the trackSelection"
            "The default is 3")
        self.addOption ('maxDeltaZ0SinTheta', 0.5, type=float,
            info="maximum Delta z0sinTheta in mm used for the trackSelection"
            "The default is 0.5 mm")
        self.addOption ('quality', None, type=str,
            info="the ID WP (string) to use. Supported ID WPs: Tight, Medium, "
            "Loose, LowPt, HighPt.")
        self.addOption ('isolation', None, type=str,
            info="the isolation WP (string) to use. Supported isolation WPs: "
            "PflowLoose_VarRad, PflowTight_VarRad, Loose_VarRad, "
            "Tight_VarRad, NonIso.")
        self.addOption ('addSelectionToPreselection', True, type=bool,
            info="whether to retain only muons satisfying the working point "
            "requirements. The default is True.")
        self.addOption ('isoDecSuffix', '', type=str,
            info="isoDecSuffix if using close-by-corrected isolation working points.")
        self.addOption ('systematicBreakdown', False, type=bool,
            info="enables the full breakdown of efficiency SF systematics "
            "(1 NP per uncertainty source, instead of 1 NP in total). "
            "The default is False.")
        self.addOption ('onlyRecoEffSF', False, type=bool,
            info="same as noEffSF, but retains the ID scale factor. "
            "Experimental! only useful for CI tests. The default is False.")
        self.addOption ('noEffSF', False, type=bool,
            info="disables the calculation of efficiencies and scale factors. "
            "Experimental! only useful to test a new WP for which scale "
            "factors are not available. The default is False.")
        self.addOption ('saveDetailedSF', True, type=bool,
            info="save all the independent detailed object scale factors. "
            "The default is True.")
        self.addOption ('saveCombinedSF', False, type=bool,
            info="save the combined object scale factor. "
            "The default is False.")
        self.addOption ('excludeNSWFromPrecisionLayers', False, type=bool,
            info="only for testing purposes, turn on to ignore NSW hits and "
            "fix a crash with older derivations (p-tag <p5834)")

    def makeAlgs (self, config) :
        log = logging.getLogger('MuonWorkingPointConfig')

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

        # Set up the track selection algorithm:
        if self.trackSelection:
            alg = config.createAlgorithm( 'CP::AsgLeptonTrackSelectionAlg',
                                          'MuonTrackSelectionAlg' + postfix,
                                          reentrant=True )
            alg.selectionDecoration = 'trackSelection' + postfix + ',as_bits'
            alg.maxD0Significance = self.maxD0Significance
            alg.maxDeltaZ0SinTheta = self.maxDeltaZ0SinTheta
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration, preselection=self.addSelectionToPreselection)

        # Setup the muon quality selection
        alg = config.createAlgorithm( 'CP::MuonSelectionAlgV2',
                               'MuonSelectionAlg' + postfix )
        config.addPrivateTool( 'selectionTool', 'CP::MuonSelectionTool' )
        alg.selectionTool.MuQuality = quality
        alg.selectionTool.IsRun3Geo = config.geometry() >= LHCPeriod.Run3
        if config.geometry() is LHCPeriod.Run4:
            log.warning("Disabling NSW hits for Run4 geometry")
            alg.selectionTool.ExcludeNSWFromPrecisionLayers = True
        else:
            alg.selectionTool.ExcludeNSWFromPrecisionLayers = self.excludeNSWFromPrecisionLayers and (config.geometry() >= LHCPeriod.Run3)
        alg.selectionDecoration = 'good_muon' + postfix + ',as_char'
        alg.badMuonVetoDecoration = 'is_bad' + postfix + ',as_char'
        alg.muons = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName,
                             alg.selectionDecoration,
                             preselection=self.addSelectionToPreselection)
        if self.quality == 'HighPt':
            config.addOutputVar (self.containerName, 'is_bad' + postfix, 'is_bad' + postfix)

        # Set up the isolation calculation algorithm:
        if self.isolation != 'NonIso' :
            alg = config.createAlgorithm( 'CP::MuonIsolationAlg',
                                   'MuonIsolationAlg' + postfix )
            config.addPrivateTool( 'isolationTool', 'CP::IsolationSelectionTool' )
            alg.isolationTool.MuonWP = self.isolation
            alg.isolationTool.IsoDecSuffix = self.isoDecSuffix
            alg.isolationDecoration = 'isolated_muon' + postfix + ',as_char'
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName,
                                 alg.isolationDecoration,
                                 preselection=self.addSelectionToPreselection)

        sfList = []
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
            if config.geometry() >= LHCPeriod.Run3:
                alg.efficiencyScaleFactorTool.CalibrationRelease = '240711_Preliminary_r24run3'
            alg.efficiencyScaleFactorTool.BreakDownSystematics = self.systematicBreakdown
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            if self.saveDetailedSF:
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                     'reco_effSF' + postfix)
            sfList += [alg.scaleFactorDecoration]

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
            if config.geometry() >= LHCPeriod.Run3:
                alg.efficiencyScaleFactorTool.CalibrationRelease = '220817_Preliminary_r22run3' # not available as part of '230123_Preliminary_r22run3'!
            alg.efficiencyScaleFactorTool.BreakDownSystematics = self.systematicBreakdown
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            if self.saveDetailedSF:
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                     'BadMuonVeto_effSF' + postfix)
            sfList += [alg.scaleFactorDecoration]

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
            if config.geometry() >= LHCPeriod.Run3:
                alg.efficiencyScaleFactorTool.CalibrationRelease = '240711_Preliminary_r24run3'
            alg.efficiencyScaleFactorTool.BreakDownSystematics = self.systematicBreakdown
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            if self.saveDetailedSF:
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                     'isol_effSF' + postfix)
            sfList += [alg.scaleFactorDecoration]

        # Set up the TTVA scale factor calculation algorithm:
        if config.dataType() is not DataType.Data and self.trackSelection and not self.onlyRecoEffSF and not self.noEffSF:
            alg = config.createAlgorithm( 'CP::MuonEfficiencyScaleFactorAlg',
                                   'MuonEfficiencyScaleFactorAlgTTVA' + postfix )
            config.addPrivateTool( 'efficiencyScaleFactorTool',
                            'CP::MuonEfficiencyScaleFactors' )
            alg.scaleFactorDecoration = 'muon_TTVA_effSF' + postfix + "_%SYS%"
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'muon_TTVA_bad_eff' + postfix
            alg.efficiencyScaleFactorTool.WorkingPoint = 'TTVA'
            if config.geometry() >= LHCPeriod.Run3:
                alg.efficiencyScaleFactorTool.CalibrationRelease = '240711_Preliminary_r24run3'
            alg.efficiencyScaleFactorTool.BreakDownSystematics = self.systematicBreakdown
            alg.muons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            if self.saveDetailedSF:
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                     'TTVA_effSF' + postfix)
            sfList += [alg.scaleFactorDecoration]

        if config.dataType() is not DataType.Data and not self.noEffSF and self.saveCombinedSF:
            alg = config.createAlgorithm( 'CP::AsgObjectScaleFactorAlg',
                                          'MuonCombinedEfficiencyScaleFactorAlg' + postfix )
            alg.particles = config.readName (self.containerName)
            alg.inScaleFactors = sfList
            alg.outScaleFactor = 'effSF' + postfix + '_%SYS%'
            config.addOutputVar (self.containerName, alg.outScaleFactor, 'effSF' + postfix)

class MuonTriggerAnalysisSFBlock (ConfigBlock):

    def __init__ (self, configName='') :
        super (MuonTriggerAnalysisSFBlock, self).__init__ ()

        self.addOption ('triggerChainsPerYear', {}, type=None,
                        info="a dictionary with key (string) the year and value (list of "
                        "strings) the trigger chains. The default is {} (empty dictionary).")
        self.addOption ('muonID', '', type=str,
                        info="the muon quality WP (string) to use.")
        self.addOption ('saveSF', True, type=bool,
                        info="define whether we decorate the trigger scale factor "
                        "The default is True.")
        self.addOption ('saveEff', False, type=bool,
                        info="define whether we decorate the trigger MC efficiencies "
                        "The default is False.")
        self.addOption ('saveEffData', False, type=bool,
                        info="define whether we decorate the trigger data efficiencies "
                        "The default is False.")
        self.addOption ('prefixSF', 'trigEffSF', type=str,
                        info="the decoration prefix for trigger scale factors, "
                        "the default is 'trigEffSF'")
        self.addOption ('prefixEff', 'trigEff', type=str,
                        info="the decoration prefix for MC trigger efficiencies, "
                        "the default is 'trigEff'")
        self.addOption ('prefixEffData', 'trigEffData', type=str,
                        info="the decoration prefix for data trigger efficiencies, "
                        "the default is 'trigEffData'")
        self.addOption ('includeAllYears', False, type=bool,
                        info="if True, all configured years will be included in all jobs. "
                        "The default is False.")
        self.addOption ('removeHLTPrefix', True, type=bool,
                        info="remove the HLT prefix from trigger chain names, "
                        "The default is True.")
        self.addOption ('containerName', '', type=str,
                        info="the input muon container, with a possible selection, in "
                        "the format container or container.selection.")

    def makeAlgs (self, config) :

        if config.dataType() is not DataType.Data:

            # Dictionary from TrigGlobalEfficiencyCorrection/Triggers.cfg
            # Key is trigger chain (w/o HLT prefix)
            # Value is empty for single leg trigger or list of legs
            triggerDict = TriggerDict()

            if self.includeAllYears:
                years = [int(year) for year in self.triggerChainsPerYear.keys()]
            elif config.campaign() is Campaign.MC20a:
                years = [2015, 2016]
            elif config.campaign() is Campaign.MC20d:
                years = [2017]
            elif config.campaign() is Campaign.MC20e:
                years = [2018]
            elif config.campaign() in [Campaign.MC21a, Campaign.MC23a]:
                years = [2022]
            elif config.campaign() in [Campaign.MC23c, Campaign.MC23d]:
                years = [2023]

            triggerYearStartBoundaries = {
                2015: 260000,
                2016: 290000,
                2017: 324000,
                2018: 348000,
                2022: 410000,
                2023: 450000,
                2024: 470000,
            }

            triggerConfigs = {}
            triggerConfigYears = {}
            for year in years:
                triggerChains = self.triggerChainsPerYear.get(int(year), self.triggerChainsPerYear.get(str(year), []))
                for chain in triggerChains:
                    chain = chain.replace(" || ", "_OR_")
                    chain_noHLT = chain.replace("HLT_", "")
                    chain_out = chain_noHLT if self.removeHLTPrefix else chain
                    legs = triggerDict[chain_noHLT]
                    if not legs:
                        if chain_noHLT.startswith('mu') and chain_noHLT[2].isdigit:
                            # Need to support HLT_mu26_ivarmedium_OR_HLT_mu50
                            triggerConfigs[chain_out] = chain
                            if chain_out in triggerConfigYears.keys():
                                triggerConfigYears[chain_out].append(year)
                            else:
                                triggerConfigYears[chain_out] = [year]
                    else:
                        for leg in legs:
                            if leg.startswith('mu') and leg[2].isdigit:
                                # Need to support HLT_mu14_ivarloose
                                leg_out = leg if self.removeHLTPrefix else f"HLT_{leg}"
                                triggerConfigs[leg_out] = f"HLT_{leg}"
                                if leg_out in triggerConfigYears.keys():
                                    triggerConfigYears[leg_out].append(year)
                                else:
                                    triggerConfigYears[leg_out] = [year]

            for trig_short, trig in triggerConfigs.items():
                alg = config.createAlgorithm('CP::MuonTriggerEfficiencyScaleFactorAlg',
                                             'MuonTrigEfficiencyCorrectionsAlg_' + self.muonID + '_' + trig_short)
                config.addPrivateTool( 'efficiencyScaleFactorTool',
                                       'CP::MuonTriggerScaleFactors' )

                # Reproduce config from TrigGlobalEfficiencyAlg
                alg.efficiencyScaleFactorTool.MuonQuality = self.muonID
                alg.efficiencyScaleFactorTool.AllowZeroSF = True

                # Avoid warnings for missing triggers
                if self.includeAllYears:
                    alg.minRunNumber = 0
                    alg.maxRunNumber = 999999

                    if triggerConfigYears[trig_short][0] != years[0]:
                        alg.minRunNumber = triggerYearStartBoundaries.get(triggerConfigYears[trig_short][0], 999999)
                    if triggerConfigYears[trig_short][-1] != years[-1]:
                        alg.maxRunNumber = triggerYearStartBoundaries.get(triggerConfigYears[trig_short][-1] + 1, 999999)
                elif config.campaign() is Campaign.MC20a:  # to avoid potential corner-cases keep the default config unchanged
                    if triggerConfigYears[trig_short] == [2015]:
                        alg.maxRunNumber = 290000
                    elif triggerConfigYears[trig_short] == [2016]:
                        alg.minRunNumber = 290000

                alg.trigger = trig
                if self.saveSF:
                    alg.scaleFactorDecoration = f"muon_{self.prefixSF}_{trig_short}_%SYS%"
                if self.saveEff:
                    alg.mcEfficiencyDecoration = f"muon_{self.prefixEff}_{trig_short}_%SYS%"
                if self.saveEffData:
                    alg.dataEfficiencyDecoration = f"muon_{self.prefixEffData}_{trig_short}_%SYS%"
                alg.outOfValidity = 2 #silent
                alg.outOfValidityDeco = f"bad_eff_muontrig_{trig_short}"
                alg.muons = config.readName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, '')
                if self.saveSF:
                    config.addOutputVar (self.containerName, alg.scaleFactorDecoration, f"{self.prefixSF}_{trig_short}")
                if self.saveEff:
                    config.addOutputVar (self.containerName, alg.mcEfficiencyDecoration, f"{self.prefixEff}_{trig_short}")
                if self.saveEffData:
                    config.addOutputVar (self.containerName, alg.dataEfficiencyDecoration, f"{self.prefixEffData}_{trig_short}")


class MuonLRTMergedConfig (ConfigBlock) :
    def __init__ (self) :
        super (MuonLRTMergedConfig, self).__init__ ()
        self.addOption (
            'inputMuons', 'Muons', type=str,
            noneAction='error',
            info="the name of the input muon container."
        )
        self.addOption (
            'inputLRTMuons', 'MuonsLRT', type=str,
            noneAction='error',
            info="the name of the input LRT muon container."
        )
        self.addOption (
            'containerName', 'Muons_LRTMerged', type=str,
            noneAction='error',
            info="the name of the output container after LRT merging."
        )

    def makeAlgs (self, config) :

        if config.isPhyslite() :
            raise(RuntimeError("Muon LRT merging is not available in Physlite mode"))

        alg = config.createAlgorithm( "CP::MuonLRTMergingAlg", "MuonLRTMergingAlg" + self.containerName )
        alg.PromptMuonLocation = self.inputMuons
        alg.LRTMuonLocation = self.inputLRTMuons
        alg.OutputMuonLocation = self.containerName
        alg.UseRun3WP = config.geometry() >= LHCPeriod.Run3 
        alg.CreateViewCollection = False

class MuonContainerMergingConfig (ConfigBlock) :
    def __init__ (self) :
        super (MuonContainerMergingConfig, self).__init__ ()
        self.addOption (
            'inputMuonContainers', [], type=list,
            noneAction='error',
            info="List of container names to be merged of type xAOD::MuonContainer."
        )
        self.addOption (
            'outputMuonLocation', 'MuonsMerged', type=str,
            noneAction='error',
            info="The name of the output container."
        )
        self.addOption (
            'createViewCollection', True, type=bool,
            info="Decided if output container is a view (default) or a deep copy."
        )


    def makeAlgs (self, config) :
        algname = "MuonContainerMergingAlg"
        algname = algname + self.outputMuonLocation
        alg = config.createAlgorithm( "CP::MuonContainerMergingAlg", algname )
        alg.InputMuonContainers = self.inputMuonContainers
        alg.OutputMuonLocation = self.outputMuonLocation
        alg.CreateViewCollection = self.createViewCollection
