/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "../eflowTrackCaloExtensionTool.h"
#include "../PFTrackClusterMatchingTool.h"
#include "../eflowCellEOverPTool_Run2_mc20_JetETMiss.h"
#include "../PFCellEOverPTool.h"
#include "../eflowCellEOverPTool_mc12_HLLHC.h"
#include "../PFLeptonSelector.h"
#include "../PFTrackSelector.h"
#include "../PFClusterSelectorTool.h"
#include "../PFAlgorithm.h"
#include "../PFChargedFlowElementCreatorAlgorithm.h"
#include "../PFNeutralFlowElementCreatorAlgorithm.h"
#include "../PFLCNeutralFlowElementCreatorAlgorithm.h"
#include "../PFOClusterMLCorrectionAlgorithm.h"
#include "../NeutralPFOClusterMLCorrectionTool.h"
#include "../PFSubtractionTool.h"
#include "../PFMomentCalculatorTool.h"
#include "../PFClusterCollectionTool.h"
#include "../PFLCCalibTool.h"
#include "../PFMuonFlowElementAssoc.h"
#include "../PFEGamFlowElementAssoc.h"
#include "../PFTauFlowElementAssoc.h"
#include "../PFTrackPreselAlg.h"
#include "../PFTrackMuonCaloTaggingAlg.h"
#include "../PFTrackMuonIsoTaggingAlg.h"
#include "../PFEnergyPredictorTool.h"
#include "../PFClusterWidthDecorator.h"
#include "../PFSimulateTruthShowerTool.h"
#include "../PFRadialEnergyCalculatorTool.h"

#include "../PFUnifiedMatchingTool.h"
#include "../PFUnifiedSubtractionOnlyTool.h"

DECLARE_COMPONENT( PFLeptonSelector )
DECLARE_COMPONENT( PFClusterSelectorTool )
DECLARE_COMPONENT( PFTrackSelector )
DECLARE_COMPONENT( PFAlgorithm )
DECLARE_COMPONENT( PFChargedFlowElementCreatorAlgorithm)
DECLARE_COMPONENT( PFNeutralFlowElementCreatorAlgorithm)
DECLARE_COMPONENT( PFLCNeutralFlowElementCreatorAlgorithm)
DECLARE_COMPONENT( PFOClusterMLCorrectionAlgorithm)
DECLARE_COMPONENT( NeutralPFOClusterMLCorrectionTool )
DECLARE_COMPONENT( PFSubtractionTool )
DECLARE_COMPONENT( PFMomentCalculatorTool )
DECLARE_COMPONENT( PFClusterCollectionTool )
DECLARE_COMPONENT( PFLCCalibTool )
DECLARE_COMPONENT( eflowTrackCaloExtensionTool )
DECLARE_COMPONENT( PFTrackClusterMatchingTool )
DECLARE_COMPONENT( PFCellEOverPTool)
DECLARE_COMPONENT( eflowCellEOverPTool_Run2_mc20_JetETMiss)
DECLARE_COMPONENT(  eflowCellEOverPTool_mc12_HLLHC)
DECLARE_COMPONENT( PFMuonFlowElementAssoc )
DECLARE_COMPONENT( PFEGamFlowElementAssoc )
DECLARE_COMPONENT( PFTauFlowElementAssoc )
DECLARE_COMPONENT( PFTrackPreselAlg )
DECLARE_COMPONENT( PFTrackMuonCaloTaggingAlg )
DECLARE_COMPONENT( PFTrackMuonIsoTaggingAlg )
DECLARE_COMPONENT( PFEnergyPredictorTool )
DECLARE_COMPONENT( PFSimulateTruthShowerTool)
DECLARE_COMPONENT( PFClusterWidthDecorator )
DECLARE_COMPONENT( PFRadialEnergyCalculatorTool )

DECLARE_COMPONENT( PFUnifiedMatchingTool )
DECLARE_COMPONENT( PFUnifiedSubtractionOnlyTool )
