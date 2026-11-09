/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "JetRecTools/ChargedHadronSubtractionTool.h"
#include "AsgDataHandles/ReadHandle.h"

using namespace std;

ChargedHadronSubtractionTool::ChargedHadronSubtractionTool(const std::string &name) : JetConstituentModifierBase(name)
{
}

StatusCode ChargedHadronSubtractionTool::initialize()
{
  if (m_inputType != xAOD::Type::ParticleFlow && m_inputType != xAOD::Type::FlowElement)
  {
    ATH_MSG_ERROR("ChargedHadronSubtractionTool requires PFO inputs. It cannot operate on objects of type "
                  << m_inputType);
    return StatusCode::FAILURE;
  }

  if(m_byVertex){
    ATH_MSG_INFO("Running ChargedHadronSubtractionTool by-vertex: " << m_byVertex.value());
  }
  if(m_useTrackToVertexTool){
    ATH_MSG_INFO("Running ChargedHadronSubtractionTool using TTVA: " << m_useTrackToVertexTool.value());
  }

  ATH_CHECK(m_trkVtxAssoc_key.initialize(m_useTrackToVertexTool && !m_ignoreVertex));
  ATH_CHECK(m_vertexContainer_key.initialize(!m_ignoreVertex));

  ATH_CHECK(m_matchedToPVKeys.initialize());
  ATH_CHECK(m_matchedToPUsidebandKeys.initialize());
  ATH_CHECK(m_matchingPVsKeys.initialize());
  ATH_CHECK(m_matchingPUsidebandKeys.initialize());


  return StatusCode::SUCCESS;
}

StatusCode ChargedHadronSubtractionTool::configure_decor_handle_keys()
{
  if (!m_byVertex) {

    ATH_CHECK(m_matchedToPVKeys.assign(
        m_outputContainer,
        m_outputChargedContainer,
        m_outputNeutralContainer));

    ATH_CHECK(m_matchedToPUsidebandKeys.assign(
        m_outputContainer,
        m_outputChargedContainer,
        m_outputNeutralContainer));

  } else {

    ATH_CHECK(m_matchingPVsKeys.assign(
        m_outputContainer,
        m_outputChargedContainer,
        m_outputNeutralContainer));

    ATH_CHECK(m_matchingPUsidebandKeys.assign(
        m_outputContainer,
        m_outputChargedContainer,
        m_outputNeutralContainer));
  }

  return StatusCode::SUCCESS;
}

StatusCode ChargedHadronSubtractionTool::process_impl(xAOD::IParticleContainer *cont, const EventContext& ctx) const
{

  // Type-checking happens in the JetConstituentModifierBase class
  // so it is safe just to static_cast
  if (m_inputType == xAOD::Type::FlowElement)
  {
    xAOD::FlowElementContainer *feCont = static_cast<xAOD::FlowElementContainer *>(cont);

    // Check that these FlowElements are actually PFOs; nothing else is supported
    if (!feCont->empty() && !(feCont->front()->signalType() & xAOD::FlowElement::PFlow))
    {
      ATH_MSG_ERROR("This tool correctly recieved a FlowElement, but it wasn't a PFO!"
                    << " Signal type was " << feCont->front()->signalType());
      return StatusCode::FAILURE;
    }

    return !m_byVertex ? matchToPrimaryVertex<xAOD::FlowElementContainer, xAOD::FlowElement>(*feCont, ctx) : matchByPrimaryVertex<xAOD::FlowElementContainer, xAOD::FlowElement>(*feCont, ctx);
  }
  xAOD::PFOContainer *pfoCont = static_cast<xAOD::PFOContainer *>(cont);
  return !m_byVertex ? matchToPrimaryVertex<xAOD::PFOContainer, xAOD::PFO>(*pfoCont, ctx) : matchByPrimaryVertex<xAOD::PFOContainer, xAOD::PFO>(*pfoCont, ctx);
}


const xAOD::Vertex *ChargedHadronSubtractionTool::getPrimaryVertex() const
{
  // Retrieve Primary Vertices
  auto handle = SG::makeHandle(m_vertexContainer_key);
  if (!handle.isValid())
  {
    ATH_MSG_WARNING(" This event has no primary vertex container");
    return nullptr;
  }

  const xAOD::VertexContainer *pvtxs = handle.cptr();
  if (pvtxs->empty())
  {
    ATH_MSG_WARNING(" Failed to retrieve valid primary vertex container");
    return nullptr;
  }

  // Usually the 0th vertex is the primary one, but this is not always
  //  the case. So we will choose the first vertex of type PriVtx
  for (const auto *theVertex : *pvtxs)
  {
    if (theVertex->vertexType() == xAOD::VxType::PriVtx)
    {
      return theVertex;
    } // If we have a vertex of type primary vertex
  }   // iterate over the vertices and check their type

  // If we failed to find an appropriate vertex, return the dummy vertex
  ATH_MSG_DEBUG("Could not find a primary vertex in this event");
  for (const auto *theVertex : *pvtxs)
  {
    if (theVertex->vertexType() == xAOD::VxType::NoVtx)
    {
      return theVertex;
    }
  }

  // If there is no primary vertex, then we cannot do PV matching.
  ATH_MSG_WARNING("Primary vertex container is empty");
  return nullptr;
}

double ChargedHadronSubtractionTool::calcAbsZ0SinTheta(const xAOD::TrackParticle &trk, const xAOD::Vertex &vtx)
{
  // vtz.z() provides z of that vertex w.r.t the center of the beamspot (z = 0).
  // Thus we correct the track z0 to be w.r.t z = 0
  const float z0 = trk.z0() + trk.vz() - vtx.z();
  const float theta = trk.theta();
  return std::abs(z0 * std::sin(theta));
}
