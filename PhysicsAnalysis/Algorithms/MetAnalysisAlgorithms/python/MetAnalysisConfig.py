# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class MetAnalysisConfig (ConfigBlock):
    """the ConfigBlock for the MET configuration"""

    def __init__ (self, containerName='') :
        super (MetAnalysisConfig, self).__init__ ()
        self.addOption('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the input container")
        self.addOption ('useJVT', True, type=bool,
            info="whether to use the JVT decision in the calculation")
        self.addOption ('useFJVT', False, type=bool,
            info="whether to use the forward JVT decision in the calculation")
        self.addOption ('treatPUJets', False, type=bool,
            info="whether to treat pile-up jets in the MET significance calculation")
        self.addOption ('setMuonJetEMScale', True, type=bool,
            info="enables the handling of muons in jets for the MET calculation. "
            "Should be turned off for analyses where muons are not reconstructed "
            "at all.")
        self.addOption ('jets', "", type=str,
            info="the input jet container")
        self.addOption ('electrons', "", type=str,
            info="the input electron container, with a possible selection, in "
            "the format `container` or `container.selection`")
        self.addOption ('muons', "", type=str,
            info="the input muon container, with a possible selection, in the "
            "format `container` or `container.selection`")
        self.addOption ('photons', "", type=str,
            info="the input photon container, with a possible selection, in "
            "the format `container` or `container.selection`")
        self.addOption ('taus', "", type=str,
            info="the input tau-jet container, with a possible selection, in "
            "the format `container` or `container.selection`")
        self.addOption ('invisible', "", type=str,
            info="any input container to be treated as invisible particles, "
            "in the format `container` (no selection)")
        self.addOption ('metWP', "Tight", type=str,
            info="the MET working point to use: Loose, Tight, Tighter, "
            "Tenacious")
        self.addOption ('skipSystematicJetSelection', False, type=bool,
            info="EXPERIMENTAL: whether to use simplified OR based on nominal jets "
            "and for jet-related systematics only. "
            "WARNING: this option is strictly for doing physics studies of the feasibility "
            "of this OR scheme, it should not be used in a regular analysis")
        self.addOption ('saveSignificance', True, type=bool,
            info="whether to save the MET significance (default=True)")
        self.addOption ('useLRT', False, type=bool,
            info="whether to use LRT MET Core and association map")

    def makeAlgs (self, config) :

        if config.isPhyslite() :
            metSuffix = 'AnalysisMET'
        else :
            jetContainer = config.originalName (self.jets)
            metSuffix = jetContainer[:-4]
        if self.useLRT:
            metSuffix += "_LRT"

        if not self.useFJVT and self.treatPUJets:
            raise ValueError ("MET significance pile-up treatment requires fJVT")

        # Remove b-tagging calibration from the MET suffix name
        btIndex = metSuffix.find('_BTagging')
        if btIndex != -1:
            metSuffix = metSuffix[:btIndex]

        # Set up the met maker algorithm:
        alg = config.createAlgorithm( 'CP::MetMakerAlg', 'MetMakerAlg' + self.containerName )
        config.addPrivateTool( 'makerTool', 'met::METMaker' )
        alg.makerTool.skipSystematicJetSelection = self.skipSystematicJetSelection

        alg.doJetJVT = self.useJVT
        if self.useJVT:
            config.addPrivateTool( 'makerTool.JvtSelTool', 'CP::NNJvtSelectionTool' )
            alg.makerTool.JvtSelTool.JetContainer = config.readName (self.jets)
        if self.useFJVT:
            alg.makerTool.JetRejectionDec = 'passFJVT_internal'

        alg.makerTool.JetSelection = self.metWP
        alg.makerTool.DoPFlow = 'PFlow' in metSuffix or metSuffix=="AnalysisMET"
        alg.makerTool.DoSetMuonJetEMScale = self.setMuonJetEMScale

        if config.dataType() is not DataType.Data :
            config.addPrivateTool( 'systematicsTool', 'met::METSystematicsTool' )

        alg.metCore = 'MET_Core_' + metSuffix
        alg.metAssociation = 'METAssoc_' + metSuffix
        alg.jets = config.readName (self.jets)
        if self.muons != "" :
            alg.muons, alg.muonsSelection = config.readNameAndSelection (self.muons, excludeFrom={'or'})
        if self.electrons != "" :
            alg.electrons, alg.electronsSelection = config.readNameAndSelection (self.electrons, excludeFrom={'or'})
        if self.photons != "" :
            alg.photons, alg.photonsSelection = config.readNameAndSelection (self.photons, excludeFrom={'or'})
        if self.taus != "" :
            alg.taus, alg.tausSelection = config.readNameAndSelection (self.taus, excludeFrom={'or'})
        if self.invisible != "" :
            alg.invisible = config.readName (self.invisible)
        alg.met = config.writeName (self.containerName, isMet = True)


        # Set up the met builder algorithm:
        alg = config.createAlgorithm( 'CP::MetBuilderAlg', 'MetBuilderAlg' + self.containerName )
        alg.met = config.readName (self.containerName)


        # Set up the met significance algorithm:
        if self.saveSignificance:
            alg = config.createAlgorithm( 'CP::MetSignificanceAlg', 'MetSignificanceAlg' + self.containerName )
            config.addPrivateTool( 'significanceTool', 'met::METSignificance' )
            if self.muons != "" :
                config.addPrivateTool( 'significanceTool.MuonCalibTool', 'CP::MuonCalibTool' )
                # Retrieve the calibMode from the container name.selections
                alg.significanceTool.MuonCalibTool.calibMode = (
                    config.getContainerMeta(self.muons.split(".")[0], 'calibMode', failOnMiss=True))

            alg.significanceTool.SoftTermParam = 0
            alg.significanceTool.TreatPUJets = self.treatPUJets
            alg.significanceTool.IsAFII = config.dataType() is DataType.FastSim
            alg.met = config.readName (self.containerName)
            config.addOutputVar (self.containerName, 'significance', 'significance')

        config.addOutputVar (self.containerName, 'met', 'met')
        config.addOutputVar (self.containerName, 'phi', 'phi')
        config.addOutputVar (self.containerName, 'sumet', 'sumet')

