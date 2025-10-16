/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkEGamma/PhotonVertexSelectionWrapper.h"
#include "PhotonVertexSelection/PhotonVertexHelpers.h"
#include <vector>

namespace DerivationFramework {

PhotonVertexSelectionWrapper::PhotonVertexSelectionWrapper(const std::string& t,
                                                           const std::string& n,
                                                           const IInterface* p)
  : base_class(t, n, p)
  , m_decPrefix("")
{
  declareProperty(
    "DecorationPrefix", m_decPrefix, "Prefix for the decoration name");
}

StatusCode
PhotonVertexSelectionWrapper::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name() << "...");
  ATH_CHECK(m_photonPointingTool.retrieve());
  ATH_MSG_DEBUG("Retrieved tool " << m_photonPointingTool);

  ATH_CHECK(m_photonContainer.initialize());
  ATH_CHECK(m_vertexContainer.initialize());

  const std::string baseName = m_vertexContainer.key();
  std::string prefix = "";
  if(!m_decPrefix.empty()) prefix = m_decPrefix + "_";
  m_vtxPt = baseName + "." + prefix + "pt";
  m_vtxEta = baseName + "." + prefix + "eta";
  m_vtxPhi = baseName + "." + prefix + "phi";
  m_vtxSumPt = baseName + "." + prefix + "sumPt";
  m_vtxSumPt2 = baseName + "." + prefix + "sumPt2";

  ATH_CHECK(m_vtxPt.initialize());
  ATH_CHECK(m_vtxEta.initialize());
  ATH_CHECK(m_vtxPhi.initialize());
  ATH_CHECK(m_vtxSumPt.initialize());
  ATH_CHECK(m_vtxSumPt2.initialize());

  ATH_MSG_DEBUG("Initialization successful");

  return StatusCode::SUCCESS;
}

StatusCode
PhotonVertexSelectionWrapper::addBranches(const EventContext& ctx) const
{
  // retrieve the input containers
  SG::ReadHandle<xAOD::PhotonContainer> photons{ m_photonContainer, ctx };
  SG::ReadHandle<xAOD::VertexContainer> vertices{ m_vertexContainer, ctx };

  // Update calo pointing auxdata for photons
  if (m_photonPointingTool->updatePointingAuxdata(*photons).isFailure()) {
    ATH_MSG_ERROR("Couldn't update photon calo pointing auxdata");
    return StatusCode::FAILURE;
  }

  // create the decorators
  SG::WriteDecorHandle<xAOD::VertexContainer, float> vtxPt(m_vtxPt, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, float> vtxEta(m_vtxEta, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, float> vtxPhi(m_vtxPhi, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, float> vtxSumPt(m_vtxSumPt, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, float> vtxSumPt2(m_vtxSumPt2, ctx);
  bool isMomentum_available = vtxPt.isAvailable();
  bool isSumPt_available = vtxSumPt.isAvailable();
  bool isSumPt2_available = vtxSumPt2.isAvailable();
  bool found_PV = false;

  // Loop over vertices and update auxdata
  for (const auto *vertex : *vertices) {

    float pt = -999.;
    float eta = -999.;
    float phi = -999.;
    float sumPt = -999.;
    float sumPt2 = -999.;

    if(vertex->vertexType() == xAOD::VxType::VertexType::PriVtx or
       vertex->vertexType() == xAOD::VxType::VertexType::PileUp){

      found_PV = true;
      // Get momentum vector of vertex and add it as a decoration
      TLorentzVector vmom = xAOD::PVHelpers::getVertexMomentum(vertex, isMomentum_available);
      pt = sqrt(vmom.Px() * vmom.Px() + vmom.Py() * vmom.Py());
      if(pt>0.){
	eta = asinh(vmom.Pz() / pt);
	phi = acos(vmom.Px() / pt); // in [0,Pi]
      }
      // Calculate additional quantities
      sumPt = xAOD::PVHelpers::getVertexSumPt(vertex, 1, isSumPt_available);
      sumPt2 = xAOD::PVHelpers::getVertexSumPt(vertex, 2, isSumPt2_available);

    }

    // write decorations
    if(!isMomentum_available){
      vtxPt(*vertex) = pt;
      vtxEta(*vertex) = eta;
      vtxPhi(*vertex) = phi;
    }
    if(!isSumPt_available) vtxSumPt(*vertex) = sumPt;
    // For events where no PV is reconstructed, beamspot is saved and isSumPt2_available = False
    // Need to also check found_PV to avoid modifying locked store
    if(!isSumPt2_available && found_PV) vtxSumPt2(*vertex) = sumPt2;

  } // end loop o vertices

  return StatusCode::SUCCESS;
}

}
