/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETRECTOOLS_CHARGEDHADRONSUBTRACTIONTOOL_H
#define JETRECTOOLS_CHARGEDHADRONSUBTRACTIONTOOL_H

////////////////////////////////////////////
/// \class ChargedHadronSubtractionTool
///
/// Removes charged PFO not associated to the PV
///
/// \author John Stupak, Jennifer Roloff, and Steven Schramm
//////////////////////////////////////////////////


#include "JetRecTools/JetConstituentModifierBase.h"
#include "xAODBase/IParticleContainer.h"

#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "JetEDM/TrackVertexAssociation.h"
#include "xAODTracking/VertexContainer.h" 
#include "xAODPFlow/PFOContainer.h"
#include "xAODPFlow/FlowElementContainer.h"

#include "AsgDataHandles/ReadHandleKey.h"
#include <string>


class ChargedHadronSubtractionTool : public JetConstituentModifierBase{
  ASG_TOOL_CLASS(ChargedHadronSubtractionTool, IJetConstituentModifier)

  public:
  
  ChargedHadronSubtractionTool(const std::string& name);

  // Check that the configuration is reasonable
  StatusCode initialize();

  private:
  // Implement the correction
  StatusCode process_impl(xAOD::IParticleContainer* cont) const; 
  template <class T, class U> StatusCode matchToPrimaryVertex(T &) const;
  template <class T, class U> StatusCode matchByPrimaryVertex(T &) const;

  const xAOD::Vertex* getPrimaryVertex() const;

  // properties -----------------
  
  Gaudi::Property<bool> m_useTrackToVertexTool = {this, "UseTrackToVertexTool", false, "True if we will use the track to vertex tool"};
  Gaudi::Property<bool> m_ignoreVertex = {this, "IgnoreVertex", false, "Dummy option for cosmics - accept everything"};
  Gaudi::Property<float> m_z0sinThetaCutValue = {this, "Z0sinThetaCutValue", 2.0, "True if we will use the track to vertex tool"};
  Gaudi::Property<bool> m_byVertex = {this, "DoByVertex", false, "True if we should match to each primary vertex, not just PV0"};

  static double calcAbsZ0SinTheta(const xAOD::TrackParticle& trk, const xAOD::Vertex& vtx);
	
  SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainer_key = {this, "VertexContainerKey", "PrimaryVertices", "key for the primary vertex container"};
  SG::ReadHandleKey<jet::TrackVertexAssociation> m_trkVtxAssoc_key = {this, "TrackVertexAssociation", "JetTrackVtxAssoc", "key for the TrackVertexAssociation object"};
  


};


template <class T, class U> StatusCode ChargedHadronSubtractionTool::matchToPrimaryVertex(T& cont) const
{
  const static SG::AuxElement::Accessor<char> PVMatchedAcc("matchedToPV");
  const static SG::AuxElement::Accessor<char> PUsidebandMatchedAcc("matchedToPUsideband");

  // Use only one of TVA or PV
  const jet::TrackVertexAssociation *trkVtxAssoc = nullptr;
  const xAOD::Vertex *vtx = nullptr;
  if (!m_ignoreVertex)
  {
    // In cosmics, there's no PV container so we need to avoid attempting
    // to retrieve anything related to it
    if (m_useTrackToVertexTool)
    {
      auto handle = SG::makeHandle(m_trkVtxAssoc_key);
      if (!handle.isValid())
      {
        ATH_MSG_ERROR("Can't retrieve TrackVertexAssociation : " << m_trkVtxAssoc_key.key());
        return StatusCode::FAILURE;
      }
      trkVtxAssoc = handle.cptr();
    }
    else
    {
      vtx = getPrimaryVertex();
      if (vtx == nullptr)
      {
        ATH_MSG_ERROR("Primary vertex container was empty or no valid vertex found!");
        return StatusCode::FAILURE;
      }
      else if (vtx->vertexType() == xAOD::VxType::NoVtx)
      {
        ATH_MSG_VERBOSE("No genuine primary vertex found. Will consider all PFOs matched.");
      }
    }
  }

  for (U *ppfo : cont)
  {
    // Ignore neutral PFOs
    if constexpr (std::is_same_v<U, xAOD::PFO>) {
      if (std::abs(ppfo->charge()) < FLT_MIN)
      continue;
    } else if constexpr (std::is_same_v<U, xAOD::FlowElement>) {
      if (!ppfo->isCharged())
      continue;
    }

    bool matchedToPrimaryVertex = false;
    bool matchedToPileupSideband = false;
    if (m_ignoreVertex)
    {
      // If we don't use vertex information, don't bother computing the decision
      // Just pass every cPFO -- there shouldn't be many in cosmics!
      matchedToPrimaryVertex = true;
    }
    else
    {
      const xAOD::TrackParticle* ptrk = nullptr;

      // Use different methods to get TrackParticle based on U
      if constexpr (std::is_same_v<U, xAOD::PFO>) {
        ptrk = ppfo->track(0);
      } else if constexpr (std::is_same_v<U, xAOD::FlowElement>) {
        ptrk = dynamic_cast<const xAOD::TrackParticle*>(ppfo->chargedObject(0));
      }
      if (ptrk == nullptr)
      {
        ATH_MSG_WARNING("Charged PFO with index " << ppfo->index() << " has no ID track!");
        continue;
      }
      if (trkVtxAssoc)
      { // Use TrackVertexAssociation
        const xAOD::Vertex *thisTracksVertex = trkVtxAssoc->associatedVertex(ptrk);
        if (thisTracksVertex == nullptr)
        {
          ATH_MSG_DEBUG("No vertex associated to track " << ptrk->index() << "! So it cannot be associated to the primary vertex");
          matchedToPrimaryVertex = false;
        }
        else
        {
          matchedToPrimaryVertex = (xAOD::VxType::PriVtx == thisTracksVertex->vertexType());

          // r21 PU sideband definition (see below)
          // needed for comparisons with new r22 definition (neutral only)
          vtx = getPrimaryVertex();
          if (vtx != nullptr && vtx->vertexType() != xAOD::VxType::NoVtx)
          {
            const double absZ0sinTheta = calcAbsZ0SinTheta(*ptrk, *vtx);
            if (absZ0sinTheta < 2.0 * m_z0sinThetaCutValue && absZ0sinTheta >= m_z0sinThetaCutValue)
              matchedToPileupSideband = true;
          }
        }
      }
      else if (vtx) { // Use Primary Vertex
        if (vtx->vertexType() == xAOD::VxType::NoVtx)
        {                                // No reconstructed vertices
          matchedToPrimaryVertex = true; // simply match all cPFOs in this case
        }
        else
        { // Had a good reconstructed vertex.
          // vtz.z() provides z of that vertex w.r.t the center of the beamspot (z = 0).
          // Thus we correct the track z0 to be w.r.t z = 0
          float z0 = ptrk->z0() + ptrk->vz() - vtx->z();
          float theta = ptrk->theta();
          matchedToPrimaryVertex = (std::abs(z0 * sin(theta)) < m_z0sinThetaCutValue);
          if (std::abs(z0 * sin(theta)) < 2.0 * m_z0sinThetaCutValue && std::abs(z0 * sin(theta)) >= m_z0sinThetaCutValue)
            matchedToPileupSideband = true;
        }
      } // TVA vs PV decision
    }
    PVMatchedAcc(*ppfo) = matchedToPrimaryVertex;
    PUsidebandMatchedAcc(*ppfo) = matchedToPileupSideband;
  }

  return StatusCode::SUCCESS;
}

template <class T, class U> StatusCode ChargedHadronSubtractionTool::matchByPrimaryVertex(T& cont) const
{
  const static SG::AuxElement::Accessor<std::vector<unsigned>> matchingPVs("MatchingPVs");
  const static SG::AuxElement::Accessor<std::vector<unsigned>> matchingPUSBs("MatchingPUsidebands");

  // Retrieve Primary Vertices
  auto handle = SG::makeHandle(m_vertexContainer_key);
  if (!handle.isValid())
  {
    ATH_MSG_WARNING(" This event has no primary vertex container");
    return StatusCode::FAILURE;
  }

  const xAOD::VertexContainer *pvtxs = handle.cptr();
  if (pvtxs->empty())
  {
    ATH_MSG_WARNING(" Failed to retrieve valid primary vertex container");
    return StatusCode::FAILURE;
  }

  // Use only one of TVA or PV
  const jet::TrackVertexAssociation *trkVtxAssoc = nullptr;
  if (m_useTrackToVertexTool)
  {
    auto handle = SG::makeHandle(m_trkVtxAssoc_key);
    if (!handle.isValid())
    {
      ATH_MSG_ERROR("Can't retrieve TrackVertexAssociation : " << m_trkVtxAssoc_key.key());
      return StatusCode::FAILURE;
    }
    trkVtxAssoc = handle.cptr();
  }

  for (U *ppfo : cont)
  {
    // Ignore neutral PFOs
    if constexpr (std::is_same_v<U, xAOD::PFO>) {
      if (std::abs(ppfo->charge()) < FLT_MIN)
      continue;
    } else if constexpr (std::is_same_v<U, xAOD::FlowElement>) {
      if (!ppfo->isCharged())
      continue;
    }

    // Get the track for this charged PFO
    const xAOD::TrackParticle* ptrk = nullptr;

    // Use different methods to get TrackParticle based on U
    if constexpr (std::is_same_v<U, xAOD::PFO>) {
      ptrk = ppfo->track(0);
    } else if constexpr (std::is_same_v<U, xAOD::FlowElement>) {
      ptrk = dynamic_cast<const xAOD::TrackParticle*>(ppfo->chargedObject(0));
    }
    if (ptrk == nullptr)
    {
      ATH_MSG_WARNING("Charged PFO with index " << ppfo->index() << " has no ID track!");
      continue;
    }

    std::vector<unsigned> matchingVertexList;
    std::vector<unsigned> matchingPUSBList;

    if (trkVtxAssoc)
    { 
      // Use TrackVertexAssociation
      const xAOD::Vertex *thisTracksVertex = trkVtxAssoc->associatedVertex(ptrk);
      if (thisTracksVertex == nullptr)
      {
        ATH_MSG_DEBUG("No vertex associated to track " << ptrk->index() << "! So it cannot be associated to the primary vertex");
      }
      else
      {
        matchingVertexList.push_back(thisTracksVertex->index());
        for (const xAOD::Vertex *vtx : *pvtxs){
          if (vtx != nullptr && vtx->vertexType() != xAOD::VxType::NoVtx)
          {
            float z0 = ptrk->z0() + ptrk->vz() - vtx->z();
            float theta = ptrk->theta();
          
            if (std::abs(z0 * sin(theta)) < 2.0 * m_z0sinThetaCutValue && std::abs(z0 * sin(theta)) >= m_z0sinThetaCutValue)
              matchingPUSBList.push_back(vtx->index());
          }
        }
      }
    }
    else{
      // Use z0sinThetaCutValue
      // Loop over the primary vertices to determine which ones potentially match
      for (const xAOD::Vertex *vtx : *pvtxs)
      {
        bool matchedToVertex = false;
        bool matchedToPileupSideband = false;

        if (vtx == nullptr)
        {
          ATH_MSG_WARNING("Encountered a nullptr vertex when trying to match charged PFOs to vertices");
          continue;
        }
        else if (vtx->vertexType() == xAOD::VxType::NoVtx)
        {                         // No reconstructed vertices
          matchedToVertex = true; // simply match all cPFOs in this case
        }
        else
        { // Had a good reconstructed vertex
          const double absZ0sinTheta = calcAbsZ0SinTheta(*ptrk, *vtx);
          matchedToVertex = (absZ0sinTheta < m_z0sinThetaCutValue);
          if (absZ0sinTheta < 2.0 * m_z0sinThetaCutValue && absZ0sinTheta >= m_z0sinThetaCutValue)
            matchedToPileupSideband = true;
        }

        if (matchedToVertex)
          matchingVertexList.push_back(vtx->index());
        if (matchedToPileupSideband)
          matchingPUSBList.push_back(vtx->index());
      }
    }

    matchingPVs(*ppfo) = std::move(matchingVertexList);
    matchingPUSBs(*ppfo) = std::move(matchingPUSBList);
  }

  return StatusCode::SUCCESS;
}

#endif
