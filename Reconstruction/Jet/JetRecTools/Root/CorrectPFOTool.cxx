/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// CorrectPFOTool.cxx

#include "JetRecTools/CorrectPFOTool.h"
#include "PFlowUtils/FEHelpers.h"
#include "AsgDataHandles/ReadHandle.h"

#include <cmath>

CorrectPFOTool::CorrectPFOTool(const std::string &name):
  JetConstituentModifierBase(name), 
  m_weightPFOTool("",this) {

  // Configuration
  declareProperty("WeightPFOTool",   m_weightPFOTool,     "Name of tool that extracts the cPFO weights.");
  declareProperty("InputIsEM",       m_inputIsEM = true,  "True if neutral PFOs are EM scale clusters.");
  declareProperty("CalibratePFO",    m_calibrate = false, "True if LC calibration should be applied to EM PFOs.");
  declareProperty("CorrectNeutral",      m_correctneutral = true, "True to use the neutral component of PFlow.");
  declareProperty("CorrectCharged",      m_correctcharged = true, "True if use the charged component of PFlow.");
  declareProperty("UseChargedWeights",m_useChargedWeights = true, "True if we make use of weighting scheme for charged PFO");
  declareProperty("DoByVertex", m_doByVertex = false, "True to add vertex-by-vertex corrections for neutral PFOs");

  // Input properties
  declareProperty("VertexContainerKey", 
                  m_vertexContainer_key="PrimaryVertices",
                  "Datahandle key for the primary vertex container");
}

StatusCode CorrectPFOTool::initialize() {
  if(!m_correctneutral && !m_correctcharged) {
    ATH_MSG_ERROR("This tool is configured to do nothing!");
    return StatusCode::FAILURE;
  }
  if(m_inputType!=xAOD::Type::ParticleFlow && m_inputType!=xAOD::Type::FlowElement) {
    ATH_MSG_ERROR("CorrectPFOTool requires PFO inputs. It cannot operate on objects of type "
                  << m_inputType);
    return StatusCode::FAILURE;
  }
  if(m_useChargedWeights) {
    ATH_CHECK( m_weightPFOTool.retrieve() );
  }
  ATH_CHECK( m_vertexContainer_key.initialize() );

  if(m_doByVertex){
    ATH_MSG_INFO("Running CorrectPFOTool by vertex:" << m_doByVertex);
  }
  
  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::process_impl(xAOD::IParticleContainer* cont) const {
  // Type-checking happens in the JetConstituentModifierBase class
  // so it is safe just to static_cast
  if(m_inputType == xAOD::Type::FlowElement){
    xAOD::FlowElementContainer* feCont = static_cast<xAOD::FlowElementContainer*>(cont);
    if(!feCont->empty() && !(feCont->front()->signalType() & xAOD::FlowElement::PFlow)){
      ATH_MSG_ERROR("CorrectPFOTool received FlowElements that aren't PFOs");
      return StatusCode::FAILURE;
    }
    return m_doByVertex ? correctPFOByVertex(*feCont) : correctPFO(*feCont);
  }
  xAOD::PFOContainer* pfoCont = static_cast<xAOD::PFOContainer*> (cont);
  return m_doByVertex ? correctPFOByVertex(*pfoCont) : correctPFO(*pfoCont);
}

const xAOD::Vertex* CorrectPFOTool::getPrimaryVertex() const {
  // Retrieve Primary Vertices
  auto handle = SG::makeHandle(m_vertexContainer_key);
  if (!handle.isValid()){
      ATH_MSG_WARNING(" This event has no primary vertex container" );
      return nullptr;
  }
    
  const xAOD::VertexContainer* pvtxs = handle.cptr();
  if(pvtxs->empty()){
      ATH_MSG_WARNING(" Failed to retrieve valid primary vertex container" );
      return nullptr;
  } 

  //Usually the 0th vertex is the primary one, but this is not always 
  // the case. So we will choose the first vertex of type PriVtx
  for (const auto *theVertex : *pvtxs) {
    if (theVertex->vertexType()==xAOD::VxType::PriVtx) {
      return theVertex;
    }//If we have a vertex of type primary vertex
  }//iterate over the vertices and check their type

  // If we failed to find an appropriate vertex, return the dummy vertex
  ATH_MSG_DEBUG("Could not find a primary vertex in this event " );
  for (const auto *theVertex : *pvtxs) {
    if (theVertex->vertexType()==xAOD::VxType::NoVtx) {
      return theVertex;
    }
  }

  // If there is no primary vertex, then we cannot do PV matching.
  ATH_MSG_WARNING("Primary vertex container was empty");
  return nullptr;
}

StatusCode CorrectPFOTool::correctPFO(xAOD::PFOContainer& cont) const { 

  const xAOD::Vertex* vtx = nullptr;
  if(m_correctneutral) {
    vtx = getPrimaryVertex();
    if(vtx==nullptr) {
      ATH_MSG_ERROR("Primary vertex container was empty or no valid vertex found!");
      return StatusCode::FAILURE;
    } else if (vtx->vertexType()==xAOD::VxType::NoVtx) {
      ATH_MSG_VERBOSE("No genuine primary vertex found. Will not apply origin correction");
    }
  }

  for ( xAOD::PFO* ppfo : cont ) {

    if ( std::abs(ppfo->charge())<FLT_MIN) { // Neutral PFOs
      if(m_correctneutral) {
        ATH_CHECK( applyNeutralCorrection(*ppfo, *vtx) );
      }
    } else { // Charged PFOs
      if(m_correctcharged) {
        ATH_CHECK( applyChargedCorrection(*ppfo) );
      }      
    }
  } // PFO loop

  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::correctPFO(xAOD::FlowElementContainer& cont) const { 
  ATH_MSG_DEBUG("Using correctPFO");
  const xAOD::Vertex* vtx = nullptr;
  if(m_correctneutral) {
    vtx = getPrimaryVertex();
    if(vtx==nullptr) {
      ATH_MSG_ERROR("Primary vertex container was empty or no valid vertex found!");
      return StatusCode::FAILURE;
    } else if (vtx->vertexType()==xAOD::VxType::NoVtx) {
      ATH_MSG_VERBOSE("No genuine primary vertex found. Will not apply origin correction");
    }
  }

  for ( xAOD::FlowElement* ppfo : cont ) {

    if ( !ppfo->isCharged()) { // Neutral PFOs
      if(m_correctneutral) {
        ATH_CHECK( applyNeutralCorrection(*ppfo, *vtx) );
      }
    } else { // Charged PFOs
      if(m_correctcharged) {
        ATH_CHECK( applyChargedCorrection(*ppfo) );
      }      
    }
  } // PFO loop

  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::correctPFOByVertex(xAOD::PFOContainer& cont) const {
  static const SG::Accessor<unsigned> copyIndex("ConstituentCopyIndex");
  // Retrieve Primary Vertices
  auto handle = SG::makeHandle(m_vertexContainer_key);
  if (!handle.isValid()){
      ATH_MSG_WARNING(" This event has no primary vertex container" );
      return StatusCode::FAILURE;
  }
    
  const xAOD::VertexContainer* pvtxs = handle.cptr();
  if(pvtxs->empty()){
      ATH_MSG_WARNING(" Failed to retrieve valid primary vertex container" );
      return StatusCode::FAILURE;
  } 
  
  for ( xAOD::PFO* ppfo : cont ) {

    if ( std::abs(ppfo->charge())<FLT_MIN) { // Neutral PFOs
      if(m_correctneutral) {
        // Neutral PFOs - there are copies, one per vertex, already created
        // We need to now need to correct each copy to point to the corresponding vertex
        if (!copyIndex.isAvailable(*ppfo))
        {
          ATH_MSG_WARNING("Encountered a neutral per-vertex PFO object without the corresponding vertex index attribute");
          continue;
        }
        
        const unsigned iVtx = copyIndex(*ppfo);
        if (iVtx >= pvtxs->size())
        {
          ATH_MSG_WARNING("Encountered a neutral per-vertex PFO object with an index beyond the size of the vertex container");
          continue;
        }
        const xAOD::Vertex* vtx = pvtxs->at(iVtx);
        // Determine the correct four-vector interpretation for a neutral PFO associated with this vertex
        // Cannot use applyNeutralCorrection as that only used VxType::PriVtx, but we want to apply to all primary vertices
        // As such, extract the relevant parts and modify appropriately here
        // This is necessary to avoid changing sign of pT for pT<0 PFO
        if (ppfo->e() < FLT_MIN) {
          ppfo->setP4(0, 0, 0, 0);
        }
        if (!m_inputIsEM || m_calibrate) { // Use LC four-vector
          ppfo->setP4(ppfo->GetVertexCorrectedFourVec(*vtx));
        } else { // Use EM four-vector
          ppfo->setP4(ppfo->GetVertexCorrectedEMFourVec(*vtx));
        }
      }
    } else { // Charged PFOs
      if(m_correctcharged) {
        ATH_CHECK( applyChargedCorrection(*ppfo) );
      }      
    }
  } // PFO loop


  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::correctPFOByVertex(xAOD::FlowElementContainer& cont) const {
  ATH_MSG_DEBUG("Using correctPFOByVertex");
  static const SG::Accessor<unsigned> copyIndex("ConstituentCopyIndex");

  // Retrieve Primary Vertices
  auto handle = SG::makeHandle(m_vertexContainer_key);
  if (!handle.isValid()){
      ATH_MSG_WARNING(" This event has no primary vertex container" );
      return StatusCode::FAILURE;
  }
    
  const xAOD::VertexContainer* pvtxs = handle.cptr();
  if(pvtxs->empty()){
      ATH_MSG_WARNING(" Failed to retrieve valid primary vertex container" );
      return StatusCode::FAILURE;
  } 
  
  for ( xAOD::FlowElement* ppfo : cont ) {

    if ( !ppfo->isCharged()) { // Neutral PFOs
      if(m_correctneutral) {
        // Neutral PFOs - there are copies, one per vertex, already created
        // We need to now need to correct each copy to point to the corresponding vertex
        if (!copyIndex.isAvailable(*ppfo))
        {
          ATH_MSG_WARNING("Encountered a neutral per-vertex PFO object without the corresponding vertex index attribute");
          continue;
        }
        
        const unsigned iVtx = copyIndex(*ppfo);
        if (iVtx >= pvtxs->size())
        {
          ATH_MSG_WARNING("Encountered a neutral per-vertex PFO object with an index beyond the size of the vertex container");
          continue;
        }
        const xAOD::Vertex* vtx = pvtxs->at(iVtx);
        // Determine the correct four-vector interpretation for a neutral PFO associated with this vertex
        // Cannot use applyNeutralCorrection as that only used VxType::PriVtx, but we want to apply to all primary vertices
        // As such, extract the relevant parts and modify appropriately here
        // This is necessary to avoid changing sign of pT for pT<0 PFO
        if (ppfo->e() < FLT_MIN) {
          ppfo->setP4(0, 0, 0, 0);
        }
        ppfo->setP4(FEHelpers::getVertexCorrectedFourVec(*ppfo, *vtx));
      }
    } else { // Charged PFOs
      if(m_correctcharged) {
        ATH_CHECK( applyChargedCorrection(*ppfo) );
      }      
    }
  } // PFO loop


  return StatusCode::SUCCESS;
}


StatusCode CorrectPFOTool::applyNeutralCorrection(xAOD::PFO& pfo, const xAOD::Vertex& vtx) const {
  if (pfo.e() < FLT_MIN) { //This is necessary to avoid changing sign of pT for pT<0 PFO
    pfo.setP4(0,0,0,0);
  } else {
    if ( !m_inputIsEM || m_calibrate ) { // Use LC four-vector
      // Only correct if we really reconstructed a vertex
      if(vtx.vertexType()==xAOD::VxType::PriVtx) {
        pfo.setP4(pfo.GetVertexCorrectedFourVec(vtx));
      }
    } else { // Use EM four-vector
      // Only apply origin correction if we really reconstructed a vertex
      if(vtx.vertexType()==xAOD::VxType::PriVtx) {
        pfo.setP4(pfo.GetVertexCorrectedEMFourVec(vtx));
      } else {pfo.setP4(pfo.p4EM());} // Just set EM 4-vec
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::applyNeutralCorrection(xAOD::FlowElement& pfo, const xAOD::Vertex& vtx) const {
  if (pfo.e() < FLT_MIN) { //This is necessary to avoid changing sign of pT for pT<0 PFO
    pfo.setP4(0,0,0,0);
  }
  // Only apply origin correction if we really reconstructed a vertex
  else if(vtx.vertexType() == xAOD::VxType::PriVtx) {
    pfo.setP4(FEHelpers::getVertexCorrectedFourVec(pfo, vtx));
  }
  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::applyChargedCorrection(xAOD::PFO& pfo) const {
  if (m_useChargedWeights) {
    float weight = 0.0;
    ATH_CHECK( m_weightPFOTool->fillWeight( pfo, weight ) );
    ATH_MSG_VERBOSE("Fill pseudojet for CPFO with weighted pt: " << pfo.pt()*weight);
    pfo.setP4(pfo.p4()*weight);
  }//if should use charged PFO weighting scheme
  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::applyChargedCorrection(xAOD::FlowElement& pfo) const {
  if (m_useChargedWeights) {
    float weight = 0.0;
    ATH_CHECK( m_weightPFOTool->fillWeight( pfo, weight ) );
    ATH_MSG_VERBOSE("Fill pseudojet for CPFO with weighted pt: " << pfo.pt()*weight);
    pfo.setP4(pfo.p4()*weight);
  }//if should use charged PFO weighting scheme
  return StatusCode::SUCCESS;
}
