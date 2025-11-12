/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ClassifyAndCalculateHFAugmentation.h                                 //
// Header file for class ClassifyAndCalculateHFAugmentation             //
// Author: Adrian Berrocal Guardia <adrian.berrocal.guardia@cern.ch>    //
//                                                                      //
// Algorithm to add a variable called HFClassification which classifies //
// ttbar+jets events according to the number of additional HF jets.     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_ClassifyAndCalculateHFAugmentation_H
#define DERIVATIONFRAMEWORK_ClassifyAndCalculateHFAugmentation_H

// Basic C++ headers.

#include <string>
#include <vector>

// Athena tools headers.

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "DerivationFrameworkMCTruth/JetMatchingTool.h"
#include "DerivationFrameworkMCTruth/ClassifyAndCalculateHFTool.h"
#include "DerivationFrameworkMCTruth/HadronOriginClassifier.h"

#include "xAODEventInfo/EventInfo.h"

#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "xAODCore/AuxContainerBase.h"
#include "xAODCore/AuxStoreAccessorMacros.h"

namespace DerivationFramework {

  // Declare a set of classes:
  //  -JetMatchingTool:            It matches the hadrons with the jets.
  //  -HadronOriginClassifier:     It determines the origin of the HF hadrons.
  //  -ClassifyAndCalculateHFTool: It computes the the HF classifiers.

  // Declare the class that adds the HF classifier in the output derivation file.

  class ClassifyAndCalculateHFAugmentation : public extends<AthAlgTool, IAugmentationTool> {

    /*
      -------------------------------------------------------------------------------------------------------------------------------------
      --------------------------------------------------- Public Variables and Functions --------------------------------------------------
      -------------------------------------------------------------------------------------------------------------------------------------
    */

  public:

    using base_class::base_class;

    // Declare the functions initialize and finalize which are called before and after processing an event respectively.

    virtual StatusCode initialize() override final;

    // Declare the function addBranches that adds the HF classifier in the output derivation file.

    virtual StatusCode addBranches(const EventContext& ctx) const override final;

    /*
      -------------------------------------------------------------------------------------------------------------------------------------
      -------------------------------------------------- Private Variables and Functions --------------------------------------------------
      -------------------------------------------------------------------------------------------------------------------------------------
    */

  private:

    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticlesKey {this, "TruthParticleContainerName", "TruthParticles", "Name of the truth particles collection that is used to compute the HF Classification"};
    SG::ReadHandleKey<xAOD::JetContainer> m_jetCollectionKey {this, "jetCollectionName", "AntiKt4TruthDressedWZJets", "Name of the jet collection that is used to compute the HF Classification"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "EventInfo", "EventInfo", ""};

    // Declare a set of strings variables:
    //  -m_hfDecorKey:           It contains the name used to save the HF classifier.
    //  -m_SimplehfDecorKey:     It contains the name used to save the simple HF classifier.

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_hfDecorKey {this, "hfDecorationName", m_eventInfoKey , "HF_Classification", "Name that is used to store the HF Classification."};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_SimplehfDecorKey {this, "SimplehfDecorationName", m_eventInfoKey ,"SimpleHFClassification", "Name that is used to store the simple HF Classification."};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_jetIDDecorationKey{this, "HadronOriginIDDecorationName", m_jetCollectionKey, "HFHadronOriginID", "jet origin ID decoration key"};
    std::string m_hfDecorationName{""};
    // Add the necessary tools:
    //  -m_JetMatchingTool_Tool:        It matches the hadrons to jets.
    //  -m_HFClassification_tool:       It computes the HF classifier.
    //  -m_HadronOriginClassifier_Tool: It determines the origin of the HF hadrons.

    PublicToolHandle<DerivationFramework::JetMatchingTool> m_JetMatchingTool_Tool{this, "JetMatchingTool", ""};
    PublicToolHandle<DerivationFramework::ClassifyAndCalculateHFTool> m_HFClassification_tool{this, "ClassifyAndComputeHFtool", ""};
    PublicToolHandle<DerivationFramework::HadronOriginClassifier> m_HadronOriginClassifier_Tool{this, "HadronOriginClassifierTool", ""};

  };
}

#endif // DERIVATIONFRAMEWORK_ClassifyAndCalculateHFAugmentation_H
