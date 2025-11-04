/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

// Local include(s).
#include <AsgAnalysisAlgorithms/AsgClassificationDecorationAlg.h>
#include <AsgAnalysisAlgorithms/AsgCutBookkeeperAlg.h>
#include <AsgAnalysisAlgorithms/AsgEnergyDecoratorAlg.h>
#include <AsgAnalysisAlgorithms/AsgEventScaleFactorAlg.h>
#include <AsgAnalysisAlgorithms/AsgObjectScaleFactorAlg.h>
#include <AsgAnalysisAlgorithms/AsgFlagSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgLeptonTrackDecorationAlg.h>
#include <AsgAnalysisAlgorithms/AsgLeptonTrackSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgMaskSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgOriginalObjectLinkAlg.h>
#include <AsgAnalysisAlgorithms/AsgPriorityDecorationAlg.h>
#include <AsgAnalysisAlgorithms/AsgPtEtaSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgMassSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgNumDecorationSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgShallowCopyAlg.h>
#include <AsgAnalysisAlgorithms/AsgUnionPreselectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgUnionSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgViewFromSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgxAODMetNTupleMakerAlg.h>
#include <AsgAnalysisAlgorithms/AsgxAODNTupleMakerAlg.h>
#include <AsgAnalysisAlgorithms/BootstrapGeneratorAlg.h>
#include <AsgAnalysisAlgorithms/CopyNominalSelectionAlg.h>
#include <AsgAnalysisAlgorithms/EventCutFlowHistAlg.h>
#include <AsgAnalysisAlgorithms/EventDecoratorAlg.h>
#include <AsgAnalysisAlgorithms/NJetDecoratorAlg.h>
#include <AsgAnalysisAlgorithms/EventFlagSelectionAlg.h>
#include <AsgAnalysisAlgorithms/EventSelectionByObjectFlagAlg.h>
#include <AsgAnalysisAlgorithms/EventStatusSelectionAlg.h>
#include <AsgAnalysisAlgorithms/FakeBkgCalculatorAlg.h>
#include <AsgAnalysisAlgorithms/IOStatsAlg.h>
#include <AsgAnalysisAlgorithms/KinematicHistAlg.h>
#include <AsgAnalysisAlgorithms/LeptonSFCalculatorAlg.h>
#include <AsgAnalysisAlgorithms/MCTCDecorationAlg.h>
#include <AsgAnalysisAlgorithms/ObjectCutFlowHistAlg.h>
#include <AsgAnalysisAlgorithms/OverlapRemovalAlg.h>
#include <AsgAnalysisAlgorithms/PileupReweightingAlg.h>
#include <AsgAnalysisAlgorithms/PDFinfoAlg.h>
#include <AsgAnalysisAlgorithms/PMGTruthWeightAlg.h>
#include <AsgAnalysisAlgorithms/SysTruthWeightAlg.h>
#include <AsgAnalysisAlgorithms/SysListDumperAlg.h>
#include <AsgAnalysisAlgorithms/SystObjectLinkerAlg.h>
#include <AsgAnalysisAlgorithms/SystObjectUnioniserAlg.h>
#include <AsgAnalysisAlgorithms/TreeFillerAlg.h>
#include <AsgAnalysisAlgorithms/TreeMakerAlg.h>
#include <AsgAnalysisAlgorithms/VGammaORAlg.h>

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT (CP::AsgClassificationDecorationAlg)
DECLARE_COMPONENT (CP::AsgCutBookkeeperAlg)
DECLARE_COMPONENT (CP::AsgEnergyDecoratorAlg)
DECLARE_COMPONENT (CP::AsgEventScaleFactorAlg)
DECLARE_COMPONENT (CP::AsgObjectScaleFactorAlg)
DECLARE_COMPONENT (CP::AsgFlagSelectionTool)
DECLARE_COMPONENT (CP::AsgLeptonTrackDecorationAlg)
DECLARE_COMPONENT (CP::AsgLeptonTrackSelectionAlg)
DECLARE_COMPONENT (CP::AsgMaskSelectionTool)
DECLARE_COMPONENT (CP::AsgOriginalObjectLinkAlg)
DECLARE_COMPONENT (CP::AsgPriorityDecorationAlg)
DECLARE_COMPONENT (CP::AsgPtEtaSelectionTool)
DECLARE_COMPONENT (CP::AsgMassSelectionTool)
DECLARE_COMPONENT (CP::AsgNumDecorationSelectionToolInt)
DECLARE_COMPONENT (CP::AsgNumDecorationSelectionToolUInt8)
DECLARE_COMPONENT (CP::AsgSelectionAlg)
DECLARE_COMPONENT (CP::AsgShallowCopyAlg)
DECLARE_COMPONENT (CP::AsgUnionPreselectionAlg)
DECLARE_COMPONENT (CP::AsgUnionSelectionAlg)
DECLARE_COMPONENT (CP::AsgViewFromSelectionAlg)
DECLARE_COMPONENT (CP::AsgxAODMetNTupleMakerAlg)
DECLARE_COMPONENT (CP::AsgxAODNTupleMakerAlg)
DECLARE_COMPONENT (CP::BootstrapGeneratorAlg)
DECLARE_COMPONENT (CP::CopyNominalSelectionAlg)
DECLARE_COMPONENT (CP::EventCutFlowHistAlg)
DECLARE_COMPONENT (CP::EventDecoratorAlg)
DECLARE_COMPONENT (CP::NJetDecoratorAlg)
DECLARE_COMPONENT (CP::EventFlagSelectionAlg)
DECLARE_COMPONENT (CP::EventSelectionByObjectFlagAlg)
DECLARE_COMPONENT (CP::EventStatusSelectionAlg)
DECLARE_COMPONENT (CP::FakeBkgCalculatorAlg)
DECLARE_COMPONENT (CP::IOStatsAlg)
DECLARE_COMPONENT (CP::KinematicHistAlg)
DECLARE_COMPONENT (CP::LeptonSFCalculatorAlg)
DECLARE_COMPONENT (CP::MCTCDecorationAlg)
DECLARE_COMPONENT (CP::ObjectCutFlowHistAlg)
DECLARE_COMPONENT (CP::OverlapRemovalAlg)
DECLARE_COMPONENT (CP::PileupReweightingAlg)
DECLARE_COMPONENT (CP::PDFinfoAlg)
DECLARE_COMPONENT (CP::PMGTruthWeightAlg)
DECLARE_COMPONENT (CP::SysTruthWeightAlg)
DECLARE_COMPONENT (CP::SysListDumperAlg)
DECLARE_COMPONENT (CP::SystObjectLinkerAlg)
DECLARE_COMPONENT (CP::TreeFillerAlg)
DECLARE_COMPONENT (CP::TreeMakerAlg)
DECLARE_COMPONENT (CP::VGammaORAlg)
// Concrete classes of SystObjectUnioniserAlg
DECLARE_COMPONENT (CP::SystJetUnioniserAlg)
DECLARE_COMPONENT (CP::SystElectronUnioniserAlg)
DECLARE_COMPONENT (CP::SystPhotonUnioniserAlg)
DECLARE_COMPONENT (CP::SystMuonUnioniserAlg)
DECLARE_COMPONENT (CP::SystTauUnioniserAlg)
DECLARE_COMPONENT (CP::SystDiTauUnioniserAlg)
