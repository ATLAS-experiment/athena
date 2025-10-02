/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

  class JetMatchingTool;
  class HadronOriginClassifier;
  class ClassifyAndCalculateHFTool;

  // Declare the class that adds the HF classifier in the output derivation file.

  class ClassifyAndCalculateHFAugmentation : public extends<AthAlgTool, IAugmentationTool> {

    /*
    -------------------------------------------------------------------------------------------------------------------------------------
    --------------------------------------------------- Public Variables and Functions --------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------
    */
  
    public: 

      // Declare the constructor and destructor functions.

      ClassifyAndCalculateHFAugmentation(const std::string& t, const std::string& n, const IInterface* p);
      ~ClassifyAndCalculateHFAugmentation();

      // Declare the functions initialize and finalize which are called before and after processing an event respectively.

      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;

      // Declare the function addBranches that adds the HF classifier in the output derivation file.

      virtual StatusCode addBranches() const override;

    /*
    -------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------- Private Variables and Functions --------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------
    */

    private:

      SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticlesKey {this, "TruthParticleContainerName", "TruthParticles", "Name of the truth particles collection that is used to compute the HF Classification"};
      SG::ReadHandleKey<xAOD::JetContainer> m_jetCollectionKey {this, "jetCollectionName", "AntiKt4TruthDressedWZJets", "Name of the jet collection that is used to compute the HF Classification"};
      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "EventInfo", "EventInfo", ""};

      SG::WriteDecorHandleKey<xAOD::EventInfo> m_hfDecorKey {this, "EventInfoHFDecorName", "", "Name that is used to store the HF Classification."};
      SG::WriteDecorHandleKey<xAOD::EventInfo> m_SimplehfDecorKey {this, "EventInfoSimpleHFDecorName", "", "Name that is used to store the simple HF Classification."};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_jetIDDecorationKey{this, "JetOriginIDDecorationKey", "", "jet origin ID decoration key"};

      // Declare a set of strings variables:
      //  -m_hfDecorationName:           It contains the name used to save the HF classifier.
      //  -m_SimplehfDecorationName:     It contains the name used to save the simple HF classifier.

      Gaudi::Property<std::string> m_hfDecorationName{this, "hfDecorationName", "HF_Classification", "Name that is used to store the HF Classification."};
      Gaudi::Property<std::string> m_SimplehfDecorationName{this, "SimplehfDecorationName", "SimpleHFClassification", "Name that is used to store the simple HF Classification."};
      Gaudi::Property<std::string> m_jetIDDecorationName{this, "HadronOriginIDDecorationName", "HFHadronOriginID", "Name that is used to store the jet origin ID."};
      
      // Add the necessary tools:
      //  -m_JetMatchingTool_Tool:        It matches the hadrons to jets. 
      //  -m_HFClassification_tool:       It computes the HF classifier. 
      //  -m_HadronOriginClassifier_Tool: It determines the origin of the HF hadrons. 

      ToolHandle<DerivationFramework::JetMatchingTool> m_JetMatchingTool_Tool;
      ToolHandle<DerivationFramework::ClassifyAndCalculateHFTool> m_HFClassification_tool;
      ToolHandle<DerivationFramework::HadronOriginClassifier> m_HadronOriginClassifier_Tool;

  }; 
}

#endif // DERIVATIONFRAMEWORK_ClassifyAndCalculateHFAugmentation_H
