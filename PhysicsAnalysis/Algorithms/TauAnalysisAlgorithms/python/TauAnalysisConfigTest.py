# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import createService
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator

def makeSequence (dataType, EXPERIMENTAL_CombineMuonRemovalTaus = False) :

    algSeq = AlgSequence()

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    sysService.sigmaRecommended = 1
    createService( 'CP::SelectionNameSvc', 'SelectionNameSvc', sequence = algSeq )


    configSeq = ConfigSequence ()


    # Include, and then set up the tau analysis algorithm sequence:
    from TauAnalysisAlgorithms.TauAnalysisConfig import makeTauCalibrationConfig, makeTauWorkingPointConfig

    if EXPERIMENTAL_CombineMuonRemovalTaus:
        from TauAnalysisAlgorithms.TauAnalysisConfig import EXPERIMENTAL_makeTauCombineMuonRemovalConfig
        EXPERIMENTAL_makeTauCombineMuonRemovalConfig (
            configSeq,
            inputTaus = 'TauJets',
            inputTausMuRM = 'TauJets_MuonRM',
            outputTaus = 'TauJets_MuonRmCombined',
        )
        tau_container = 'TauJets_MuonRmCombined'
    else:
        tau_container = 'AnalysisTauJets'

    makeTauCalibrationConfig (configSeq, tau_container, tau_container)
    makeTauWorkingPointConfig (configSeq, tau_container, workingPoint='Tight', selectionName='tight')


    # temporarily disabled until di-taus are supported in R22
    # Include, and then set up the tau analysis algorithm sequence:
    # from TauAnalysisAlgorithms.DiTauAnalysisConfig import makeDiTauCalibrationConfig, makeDiTauWorkingPointConfig

    # disabling this, the standard test files don't have DiTauJets
    # makeDiTauCalibrationConfig (configSeq, 'AnalysisDiTauJets')
    # makeDiTauWorkingPointConfig (configSeq, 'AnalysisDiTauJets', workingPoint='Tight', postfix='tight')


    configAccumulator = ConfigAccumulator (algSeq, dataType, geometry="RUN2")
    configSeq.fullConfigure (configAccumulator)

    return algSeq
