# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class MetAnalysisConfig (ConfigBlock):
    """the ConfigBlock for the MET configuration"""

    def __init__ (self, containerName) :
        super (MetAnalysisConfig, self).__init__ (containerName)
        self.containerName = containerName
        self.addOption ('postfix', '', type=str)
        self.addOption ('useFJVT', False, type=bool)
        self.addOption ('treatPUJets', False, type=bool)
        self.addOption ('setMuonJetEMScale', True, type=bool)
        self.addOption ('jets', "", type=str)
        self.addOption ('electrons', "", type=str)
        self.addOption ('muons', "", type=str)
        self.addOption ('photons', "", type=str)
        self.addOption ('taus', "", type=str)
        self.addOption ('invisible', "", type=str)

    def makeAlgs (self, config) :

        postfix = self.postfix

        if config.isPhyslite() :
            metSuffix = 'AnalysisMET'
        else :
            jetContainer = config.originalName (self.jets)
            metSuffix = jetContainer[:-4]

        if not self.useFJVT and self.treatPUJets:
            raise ValueError ("MET significance pile-up treatment requires fJVT")

        # Remove b-tagging calibration from the MET suffix name
        btIndex = metSuffix.find('_BTagging')
        if btIndex != -1:
            metSuffix = metSuffix[:btIndex]

        # Set up the met maker algorithm:
        alg = config.createAlgorithm( 'CP::MetMakerAlg', 'MetMakerAlg' + postfix)
        config.addPrivateTool( 'makerTool', 'met::METMaker' )
        config.addPrivateTool( 'makerTool.JvtSelTool', 'CP::NNJvtSelectionTool' )
        alg.makerTool.JvtSelTool.JetContainer = config.readName (self.jets)

        alg.makerTool.DoPFlow = 'PFlow' in metSuffix or metSuffix=="AnalysisMET"
        alg.makerTool.DoSetMuonJetEMScale = self.setMuonJetEMScale

        if self.useFJVT:
            alg.makerTool.JetRejectionDec = 'passFJVT'
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
        alg = config.createAlgorithm( 'CP::MetBuilderAlg', 'MetBuilderAlg' + postfix )
        alg.met = config.readName (self.containerName)


        # Set up the met significance algorithm:
        alg = config.createAlgorithm( 'CP::MetSignificanceAlg', 'MetSignificanceAlg' + postfix )
        config.addPrivateTool( 'significanceTool', 'met::METSignificance' )
        alg.significanceTool.SoftTermParam = 0
        alg.significanceTool.TreatPUJets = self.treatPUJets
        alg.significanceTool.IsAFII = config.dataType() is DataType.FastSim
        alg.met = config.readName (self.containerName)

        config.addOutputVar (self.containerName, 'met', 'met')
        config.addOutputVar (self.containerName, 'phi', 'phi')
        config.addOutputVar (self.containerName, 'sumet', 'sumet')



def makeMetAnalysisConfig( seq, containerName,
                             postfix = None,
                             useFJVT = None,
                             treatPUJets = None,
                             setMuonJetEMScale = None,
                             jets = None,
                             electrons = None,
                             muons = None,
                             photons = None,
                             taus = None):
    """Create a met analysis algorithm config

    Note that defining a jet container is mandatory, but all other input
    containers are optional.

    Selections on each container can also be defined

    Keyword arguments:
      useFJVT -- Use FJVT decision for the calculation
      treatPUJets -- Treat pile-up jets in the MET significance calculation
      setMuonJetEMScale -- Use consituent scale and subtract muon eloss for jets overlapping muons
    """

    config = MetAnalysisConfig (containerName)
    if postfix is not None :
        config.setOptionValue ('postfix', postfix)
    if useFJVT is not None :
        config.setOptionValue ('useFJVT', useFJVT)
    if treatPUJets is not None :
        config.setOptionValue ('treatPUJets', treatPUJets)
    if setMuonJetEMScale is not None :
        config.setOptionValue ('setMuonJetEMScale', setMuonJetEMScale)
    if jets is not None :
        config.setOptionValue ('jets', jets)
    if electrons is not None :
        config.setOptionValue ('electrons', electrons)
    if muons is not None :
        config.setOptionValue ('muons', muons)
    if photons is not None :
        config.setOptionValue ('photons', photons)
    if taus is not None :
        config.setOptionValue ('taus', taus)
    seq.append (config)
