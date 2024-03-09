/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PhotonVertexSelection/BuildVertexPointingAlg.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AthContainers/ConstDataVector.h>
#include <EventPrimitives/EventPrimitives.h>
#include <TLorentzVector.h>
#include <xAODEgamma/EgammaContainer.h>
#include <xAODTracking/VertexAuxContainer.h>
#include <xAODTracking/VertexContainer.h>

#include <numeric>
#include <utility>

#include "PhotonVertexSelection/PhotonVertexHelpers.h"

BuildVertexPointingAlg::BuildVertexPointingAlg(const std::string& name,
                                               ISvcLocator* svcLoc)
    : EL::AnaAlgorithm(name, svcLoc) {}

StatusCode BuildVertexPointingAlg::initialize() {
  // read
  ATH_CHECK(m_photonContainerKey.initialize());
  ATH_CHECK(m_eventInfoKey.initialize());

  ATH_CHECK(m_photon_zvertex_key.initialize());
  ATH_CHECK(m_photon_errz_key.initialize());
  ATH_CHECK(m_photon_HPV_zvertex_key.initialize());
  ATH_CHECK(m_photon_HPV_errz_key.initialize());

  // write
  ATH_CHECK(m_pointingVertexContainerKey.initialize());

  ATH_CHECK(m_nphotons_good_key.initialize());
  ATH_CHECK(m_z_common_nphotons_key.initialize());
  ATH_CHECK(m_photons_px_key.initialize());
  ATH_CHECK(m_photons_py_key.initialize());
  ATH_CHECK(m_photons_pz_key.initialize());
  ATH_CHECK(m_photon_links_key.initialize());

  // tool
  ATH_CHECK(m_selectionTool.retrieve());

  ATH_MSG_INFO("Use at maximum " << m_nphotons_to_use
                                 << " photons for pointing");
  ATH_MSG_INFO("Selecting photons for pointing with "
               << m_selectionTool << " with "
               << m_selectionTool->getAcceptInfo().getNCuts() << "cuts:");
  for (unsigned int icut = 0;
       icut < m_selectionTool->getAcceptInfo().getNCuts(); ++icut) {
    ATH_MSG_INFO(
        "cut " << icut << ": "
               << m_selectionTool->getAcceptInfo().getCutDescription(icut));
  }
  return StatusCode::SUCCESS;
}

StatusCode BuildVertexPointingAlg::execute() {
  // the code has been written with photons in mind, but it is important
  // the code to work also with electrons since electrons are used as a
  // proxy of photons in performance studies
  SG::ReadHandle<xAOD::EgammaContainer> original_photons{m_photonContainerKey};
  ATH_CHECK(original_photons.isValid());

  ATH_MSG_DEBUG("number of photon in the container ("
                << m_photonContainerKey << "): " << original_photons->size());

  // these photons will be used to compute pointing variables
  ConstDataVector<DataVector<xAOD::Egamma>> photons_selected(SG::VIEW_ELEMENTS);
  selectPhotons(*original_photons, photons_selected);

  const xAOD::EventInfo* ei = SG::get(m_eventInfoKey);
  const std::pair<float, float> z_common_and_error =
      xAOD::PVHelpers::getZCommonAndError(ei, photons_selected.asDataVector(),
                                          m_convPtCut);

  // create new vertex container
  auto pointingVertices = std::make_unique<xAOD::VertexContainer>();
  auto pointingVerticesAux = std::make_unique<xAOD::VertexAuxContainer>();
  pointingVertices->setStore(pointingVerticesAux.get());

  // create new vertex
  xAOD::Vertex* vtx_pointing =
      (*pointingVertices).push_back(std::make_unique<xAOD::Vertex>());

  vtx_pointing->setVertexType(xAOD::VxType::PriVtx);

  vtx_pointing->setZ(z_common_and_error.first);
  vtx_pointing->setX(0.0);
  vtx_pointing->setY(0.0);
  AmgSymMatrix(3) cov;
  cov.setZero();
  cov(2, 2) = z_common_and_error.second * z_common_and_error.second;
  vtx_pointing->setCovariancePosition(cov);

  auto writeHandle = SG::makeHandle(m_pointingVertexContainerKey);
  ANA_CHECK(writeHandle.record(std::move(pointingVertices),
                               std::move(pointingVerticesAux)));

  // below only decorations to the vertex

  const unsigned int n_good_photons =
      std::count_if(photons_selected.begin(), photons_selected.end(),
                    [this](const xAOD::Egamma* photon) {
                      return m_goodPhotonSelectionTool->accept(photon);
                    });

  std::vector<ElementLink<xAOD::EgammaContainer>> ph_links;
  ph_links.reserve(photons_selected.size());
  for (const xAOD::Egamma* photon : photons_selected) {
    ph_links.emplace_back(*original_photons, photon->index());
  }

  auto dec_nphotons = SG::makeHandle<unsigned int>(m_z_common_nphotons_key);
  auto dec_ph_links = SG::makeHandle<decltype(ph_links)>(m_photon_links_key);
  auto dec_nphotons_good = SG::makeHandle<unsigned int>(m_nphotons_good_key);

  dec_nphotons(*vtx_pointing) = photons_selected.size();
  dec_ph_links(*vtx_pointing) = ph_links;
  dec_nphotons_good(*vtx_pointing) = n_good_photons;

  // compute common vertex from the pointing
  const TLorentzVector p4_photons = std::accumulate(
      photons_selected.begin(), photons_selected.end(),
      TLorentzVector(0, 0, 0, 0),
      [](TLorentzVector sum, const xAOD::Egamma* photon) {
        TLorentzVector v_ph;
        const xAOD::CaloCluster* cluster = photon->caloCluster();
        v_ph.SetPtEtaPhiM(photon->e() / cosh(cluster->etaBE(2)),
                          cluster->etaBE(2), cluster->phiBE(2), 0.0);
        sum += v_ph;
        return sum;
      });

  auto dec_photons_px = SG::makeHandle<float>(m_photons_px_key);
  auto dec_photons_py = SG::makeHandle<float>(m_photons_py_key);
  auto dec_photons_pz = SG::makeHandle<float>(m_photons_pz_key);

  dec_photons_px(*vtx_pointing) = p4_photons.X();
  dec_photons_py(*vtx_pointing) = p4_photons.Y();
  dec_photons_pz(*vtx_pointing) = p4_photons.Z();

  return StatusCode::SUCCESS;
}

void BuildVertexPointingAlg::selectPhotons(
    const xAOD::EgammaContainer& original_photons,
    ConstDataVector<DataVector<xAOD::Egamma>>& photons_selected) const {
  for (const auto& photon : original_photons) {
    if (m_selectionTool->accept(photon)) {
      photons_selected.push_back(photon);
      ATH_MSG_DEBUG("photon selected for pointing: pT = " << photon->pt()
                                                          << " GeV");
    }
  }
  ATH_MSG_DEBUG(
      "number of photons selected for pointing: " << photons_selected.size());

  // limit the number of photons to use
  if (m_nphotons_to_use >= 0 and
      photons_selected.size() > static_cast<std::size_t>(m_nphotons_to_use)) {
    std::sort(photons_selected.begin(), photons_selected.end(),
              [](const xAOD::IParticle* a, const xAOD::IParticle* b) {
                return a->pt() > b->pt();  // sort from the highest pT
              });

    photons_selected.resize(m_nphotons_to_use);
  }
  ATH_MSG_DEBUG("number of photons selected for pointing after limiting: "
                << photons_selected.size());
  if (ATH_UNLIKELY(this->msgLvl(MSG::DEBUG))) {
    for (const xAOD::Egamma* photon : photons_selected) {
      ATH_MSG_DEBUG("photon selected for pointing: pT = " << photon->pt()
                                                          << " GeV");
    }
  }
}
