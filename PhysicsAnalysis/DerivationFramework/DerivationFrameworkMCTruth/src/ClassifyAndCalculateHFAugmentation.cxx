/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ClassifyAndCalculateHFAugmentation.cxx                               //
// Implementation file for class ClassifyAndCalculateHFAugmentation     //
// Author: Adrian Berrocal Guardia <adrian.berrocal.guardia@cern.ch>    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

// Header of the class ClassifyAndCalculateHFAugmentation.

#include "DerivationFrameworkMCTruth/ClassifyAndCalculateHFAugmentation.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"

namespace DerivationFramework {

  /*
    ---------------------------------------------------------------------------------------------------------------------------------------
    ------------------------------------------------------------- Initialize -------------------------------------------------------------
    ---------------------------------------------------------------------------------------------------------------------------------------
  */

  StatusCode ClassifyAndCalculateHFAugmentation::initialize(){

    ATH_MSG_INFO("Initialize HF computation");

    ATH_MSG_INFO("Jets Container Name "            << m_jetCollectionKey.key());
    ATH_MSG_INFO("Truth Particles Container Name " << m_truthParticlesKey.key());
    ATH_MSG_INFO("HF Classifier Name "             << m_hfDecorKey.key());
    ATH_MSG_INFO("Simple HF Classifier Name "      << m_SimplehfDecorKey.key());
    ATH_MSG_INFO("Jet Origin ID Decoration Name "  << m_jetIDDecorationKey.key());
    std::size_t pos = m_hfDecorKey.key().find(".");
    if (pos != std::string::npos) { m_hfDecorationName = m_hfDecorKey.key().substr (pos+1); }
    ATH_CHECK( m_truthParticlesKey.initialize() );
    ATH_CHECK( m_jetCollectionKey.initialize() );
    ATH_CHECK( m_eventInfoKey.initialize() );
    ATH_CHECK( m_hfDecorKey.initialize() );
    ATH_CHECK( m_SimplehfDecorKey.initialize() );
    ATH_CHECK( m_jetIDDecorationKey.initialize() );

    // Retrieve the necessary tools
    if(m_HFClassification_tool.retrieve().isFailure()){
      ATH_MSG_ERROR("Unable to retrieve the tool " << m_HFClassification_tool);
      return StatusCode::FAILURE;
    }

    if(m_HadronOriginClassifier_Tool.retrieve().isFailure()){
      ATH_MSG_ERROR("Unable to retrieve the tool " << m_HadronOriginClassifier_Tool);
      return StatusCode::FAILURE;
    }

    if(m_JetMatchingTool_Tool.retrieve().isFailure()){
      ATH_MSG_ERROR("Unable to retrieve the tool " << m_JetMatchingTool_Tool);
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  /*
    ---------------------------------------------------------------------------------------------------------------------------------------
    ------------------------------------------------------------- AddBranches -------------------------------------------------------------
    ---------------------------------------------------------------------------------------------------------------------------------------
  */

  StatusCode ClassifyAndCalculateHFAugmentation::addBranches(const EventContext& ctx) const
  {


    // Retrieve the truth particle container
    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticlesHandle(m_truthParticlesKey, ctx);
    if (!truthParticlesHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve TruthParticleContainer " << truthParticlesHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::TruthParticleContainer* xTruthParticleContainer = truthParticlesHandle.cptr();

    // Retrieve the jets container
    SG::ReadHandle<xAOD::JetContainer> jetInputHandle(m_jetCollectionKey, ctx);
    if (!jetInputHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve JetContainer " << jetInputHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::JetContainer* JetCollection = jetInputHandle.cptr();

    // Compute a map that associates the HF hadrons with their origin using the tool m_HadronOriginClassifier_Tool.
    std::map<const xAOD::TruthParticle*, DerivationFramework::HadronOriginClassifier::HF_id> hadronMap = m_HadronOriginClassifier_Tool->GetOriginMap();

    // Create a map with a list of matched hadrons for each jet.
    std::map<const xAOD::Jet*, std::vector<xAOD::TruthParticleContainer::const_iterator>> particleMatch = m_JetMatchingTool_Tool->matchHadronsToJets(xTruthParticleContainer, JetCollection);

    // Calculate the necessary information from the jets to compute the HF classifier.
    m_HFClassification_tool->flagJets(JetCollection, particleMatch, hadronMap, m_hfDecorationName);

    // Compute the HF classifier and the simple HF classifier.
    int hfclassif     = m_HFClassification_tool->computeHFClassification(JetCollection, m_hfDecorationName);
    int simpleclassif = m_HFClassification_tool->getSimpleClassification(hfclassif);

    // Retrieve EventInfo
    SG::ReadHandle<xAOD::EventInfo> eventInfoHandle(m_eventInfoKey, ctx);
    if (!eventInfoHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve EventInfo " << eventInfoHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::EventInfo* EventInfo = eventInfoHandle.cptr();

    // Decorate EventInfo with the HF Classification and the simple version
    SG::WriteDecorHandle<xAOD::EventInfo, int> decorator_HFClassification(m_hfDecorKey, ctx);
    decorator_HFClassification(*EventInfo) = hfclassif;

    SG::WriteDecorHandle<xAOD::EventInfo, int> decorator_SimpleHFClassification(m_SimplehfDecorKey, ctx);
    decorator_SimpleHFClassification(*EventInfo) = simpleclassif;

    // Decorate truth jets with origin ID
    SG::WriteDecorHandle<xAOD::JetContainer, int> jetIdDecorator(m_jetIDDecorationKey, ctx);
    for (const auto jet : *JetCollection) {
      int id = -999;
      SG::ConstAccessor<int> hfidAcc(m_hfDecorationName + "_id");
      if(hfidAcc.isAvailable(*jet)){
        id = hfidAcc(*jet);
      }
      jetIdDecorator(*jet) = id;
    }

    return StatusCode::SUCCESS;
  }
}
