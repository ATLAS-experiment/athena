/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
#include "AsgDataHandles/ReadHandle.h"
#include "JetRec/JetClusterer.h"
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/config.h"
#include "fastjet/contrib/VariableRPlugin.hh"

#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"

#include "JetEDM/FastJetUtils.h"

#include "JetRec/PseudoJetTranslator.h"

#include "JetRec/JetClustererByVertex.h"
#include "JetEDM/LabelIndex.h"
#include "JetEDM/VertexIndexedConstituentUserInfo.h"

JetClustererByVertex::JetClustererByVertex(const std::string &myname) : JetClusterer(myname) {}

StatusCode JetClustererByVertex::initialize()
{
  ATH_CHECK( JetClusterer::initialize() ); 
  ATH_CHECK(m_vertexContainer_key.initialize());
  m_jetRankAccessor = SG::AuxElement::Accessor<int>(m_jetRank);

  return StatusCode::SUCCESS;
}

std::pair<std::unique_ptr<xAOD::JetContainer>, std::unique_ptr<SG::IAuxStore>> JetClustererByVertex::getJets() const
{
  // Return this in case of any problems
  auto nullreturn = std::make_pair(std::unique_ptr<xAOD::JetContainer>(nullptr), std::unique_ptr<SG::IAuxStore>(nullptr));

  // -----------------------
  // retrieve input
  SG::ReadHandle<PseudoJetContainer> pjContHandle(m_inputPseudoJets);
  if (!pjContHandle.isValid())
  {
    ATH_MSG_ERROR("No valid PseudoJetContainer with key " << m_inputPseudoJets.key());
    return nullreturn;
  }

  // Get reco vertices
  SG::ReadHandle<xAOD::VertexContainer> vertexHandle = SG::makeHandle(m_vertexContainer_key);
  const xAOD::VertexContainer *vertices = vertexHandle.cptr();
  unsigned numVertices = vertices->size();
  ATH_MSG_DEBUG("Retrieved vertex container for by-vertex clustering");

  // Build the container to be returned
  // Avoid memory leaks with unique_ptr
  auto jets = std::make_unique<xAOD::JetContainer>();
  auto auxCont = std::make_unique<xAOD::JetAuxContainer>();
  jets->setStore(auxCont.get());

  // Create a copy of the PseudoJet container that we modify for each vertex
  // There is no valid copy constructor, but can use the `append` method instead
  PseudoJetContainer pjContCopy;
  pjContCopy.append(pjContHandle.cptr());

  // The pointer to the PseudoJetVector (non-const) that we can modify for each iteration
  PseudoJetVector *inputPseudoJetVector = pjContCopy.casVectorPseudoJet();
  // We need to keep a copy of the original vector, otherwise we will lose information
  const PseudoJetVector cachedPseudoJetVector(*inputPseudoJetVector);

  ATH_MSG_DEBUG("Pseudojet input container has size " << inputPseudoJetVector->size());

  std::unique_ptr<fastjet::ClusterSequence> clSequence = std::make_unique<fastjet::ClusterSequence>();

  std::unique_ptr<PseudoJetVector> outputPseudoJetVector = std::make_unique<PseudoJetVector>();

  for (unsigned int iVertex{0}; iVertex < numVertices; iVertex++)
  {
    const xAOD::Vertex *vertex = vertices->at(iVertex);

    for (unsigned int iJet{0}; iJet < inputPseudoJetVector->size(); iJet++)
    {
      fastjet::PseudoJet &pseudoJet = inputPseudoJetVector->at(iJet);
      // Check if pseudoJet has VertexIndexedConstituentUserInfo
      if (pseudoJet.has_user_info<jet::VertexIndexedConstituentUserInfo>())
      {
        const jet::VertexIndexedConstituentUserInfo &userInfo = pseudoJet.user_info<jet::VertexIndexedConstituentUserInfo>();
        const xAOD::Vertex *originVertex = userInfo.vertex();
        // Reset state
        pseudoJet = cachedPseudoJetVector.at(iJet);
        if (originVertex != vertex)
        {
          // pseudoJet should not affect this vertex iteration
          // nullify its four-momentum
          pseudoJet *= 1e-12;
        }
        else
        {
          // Do nothing -- this is the correct vertex
          ATH_MSG_VERBOSE("Constituent found with pT = " << pseudoJet.pt() << " belonging to vertex index: " << userInfo.vertex()->index());
        }
      }
      // Else we expect this to be a ghost, so do nothing
    }
    std::unique_ptr<fastjet::ClusterSequence> clSequenceByVertex = buildClusterSequence(inputPseudoJetVector);
    if (!clSequenceByVertex)
      return nullreturn;

    // -----------------------
    // Build a new pointer to a PseudoJetVector containing the final PseudoJet
    // This allows us to own the vector of PseudoJet which we will put in the evt store.
    // Thus the contained PseudoJet will be kept frozen there and we can safely use pointer to them from the xAOD::Jet objects

    auto outputPseudoJetVectorByVertex = std::make_unique<PseudoJetVector>(fastjet::sorted_by_pt(clSequenceByVertex->inclusive_jets(m_ptmin)));
    if (msgLvl(MSG::VERBOSE))
    {
      for (const auto &pj : *outputPseudoJetVectorByVertex)
      {
        msg() << "  Pseudojet with pt " << std::setprecision(4) << pj.Et() * 1e-3 << " has " << pj.constituents().size() << " constituents" << endmsg;
      }
    }

    // No PseudoJets, so there's nothing else to do
    // Delete the cluster sequence before we go
    if (!outputPseudoJetVectorByVertex->empty())
    {
      for (const fastjet::PseudoJet &pj : *outputPseudoJetVectorByVertex)
      {
        processPseudoJet(pj, pjContCopy, jets.get(), vertex);
        //Mario we need to add jetRank for btagging
        // we want the rank to start with zero, but this is after the
        // jet has been added, thus the "size() - 1" here.
        m_jetRankAccessor(*jets.get()->back()) = jets->size() - 1;
      }
    }
    ATH_MSG_DEBUG("For vertex index " << iVertex << ", total reconstructed jet count so far: " << jets->size() << "  clusterseq=" << clSequenceByVertex.get());

    // Copy the contents of pjVectorByVertex into pjVector
    for (const fastjet::PseudoJet &pj : *outputPseudoJetVectorByVertex)
    {
      outputPseudoJetVector->emplace_back(pj);
    }

    // We want to add the clSequence from this vertex to the total one
    // TODO: how to best achieve this?
    // Looks like this will simply delete the old sequence..
    // clSequence->transfer_from_sequence(*clSequenceByVertex);
  }

  if (!outputPseudoJetVector->empty())
  {
    // -------------------------------------
    // record final PseudoJetVector
    SG::WriteHandle<PseudoJetVector> pjVectorHandle(m_finalPseudoJets);
    if (!pjVectorHandle.record(std::move(outputPseudoJetVector)))
    {
      ATH_MSG_ERROR("Can't record PseudoJetVector under key " << m_finalPseudoJets);
      return nullreturn;
    }
    // -------------------------------------
    // record ClusterSequences vector
    SG::WriteHandle<jet::ClusterSequence> clusterSeqHandle(m_clusterSequence);
    if (!clusterSeqHandle.record(std::move(clSequence)))
    {
      ATH_MSG_ERROR("Can't record ClusterSequence under key " << m_clusterSequence);
      return nullreturn;
    }
  }

  // Return the jet container and aux, use move to transfer
  // ownership of pointers to caller
  return std::make_pair(std::move(jets), std::move(auxCont));
}