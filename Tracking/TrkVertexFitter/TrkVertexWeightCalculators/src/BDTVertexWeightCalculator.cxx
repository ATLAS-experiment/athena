/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "TrkVertexWeightCalculators/BDTVertexWeightCalculator.h"

#include "MVAUtils/BDT.h"
#include "PathResolver/PathResolver.h"
#include "PhotonVertexSelection/PhotonVertexHelpers.h"
#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadDecorHandle.h>

#include <cmath>
#include <limits>

BDTVertexWeightCalculator::BDTVertexWeightCalculator(const std::string& name)
    : asg::AsgTool(name) { }

BDTVertexWeightCalculator::~BDTVertexWeightCalculator() = default;

StatusCode BDTVertexWeightCalculator::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(m_pointingVertexContainerKey.initialize());
  ATH_CHECK(m_nphotons_good_key.initialize());
  ATH_CHECK(m_photons_px_key.initialize());
  ATH_CHECK(m_photons_py_key.initialize());
  ATH_CHECK(m_photons_pz_key.initialize());

  ATH_CHECK(initialize_BDT());

  return StatusCode::SUCCESS;
}

StatusCode BDTVertexWeightCalculator::initialize_BDT() {
  const std::string fullPathToFile = PathResolverFindCalibFile(m_bdt_file);
  if (fullPathToFile.empty()) {
    ATH_MSG_ERROR("Can not find BDT file: '" << m_bdt_file
                                             << "'. Please check if the "
                                                "file exists.");
    return StatusCode::FAILURE;
  }

  std::unique_ptr<TFile> rootFile(TFile::Open(fullPathToFile.c_str(), "READ"));
  if (!rootFile) {
    ATH_MSG_ERROR("Can not open BDT file root file: '"
                  << fullPathToFile
                  << "'. Please check if the file "
                     "exists and is not corrupted.");

    return StatusCode::FAILURE;
  }
  std::unique_ptr<TTree> tree((TTree*)rootFile->Get(m_bdt_name.value().c_str()));
  if (!tree) {
    ATH_MSG_ERROR("Can not find BDT tree '" << m_bdt_name << "' in file: '"
                                            << fullPathToFile << "'.");
  }

  ATH_MSG_INFO("Loading BDT '" << m_bdt_name << "' from file: '"
                               << fullPathToFile << "'.");
  m_bdt = std::make_unique<MVAUtils::BDT>(tree.get());

  return StatusCode::SUCCESS;
}

std::vector<float> BDTVertexWeightCalculator::get_input_values(
    const xAOD::Vertex& vertex) const {
  SG::ReadHandle<xAOD::VertexContainer> pointingVertexContainer{
      m_pointingVertexContainerKey};
  if (pointingVertexContainer->size() != 1) {
    ATH_MSG_ERROR("Pointing vertex container has size "
                  << pointingVertexContainer->size()
                  << " instead of 1");
    return {};
  }
  const xAOD::Vertex* pointingVertex = pointingVertexContainer->front();

  const float zcommon = pointingVertex->z();
  const float zcommon_error =
      std::sqrt(pointingVertex->covariancePosition()(2, 2));

  auto acc_nph_good = SG::makeHandle<unsigned int>(m_nphotons_good_key);
  const float nph_good = acc_nph_good(*pointingVertex);
  const float vtx_sumpt = xAOD::PVHelpers::getVertexSumPt(&vertex, 1);
  const float vtx_sumpt2 =xAOD::PVHelpers::getVertexSumPt(&vertex, 2);
  const float vtx_z = vertex.z();

  const float deltaz_sig_photons = std::abs(vtx_z - zcommon) / zcommon_error;

  auto acc_photons_px = SG::makeHandle<float>(m_photons_px_key);
  auto acc_photons_py = SG::makeHandle<float>(m_photons_py_key);
  auto acc_photons_pz = SG::makeHandle<float>(m_photons_pz_key);
  TVector3 photon_sump(acc_photons_px(*pointingVertex),
                       acc_photons_py(*pointingVertex),
                       acc_photons_pz(*pointingVertex));
  TLorentzVector vertex_p4 = xAOD::PVHelpers::getVertexMomentum(&vertex, true);
  const float deltaphi_sig_photons = vertex_p4.Vect().DeltaPhi(photon_sump);

  ATH_MSG_DEBUG("Input variables: nph_good: "
                << nph_good << ", vtx_sumpt: " << vtx_sumpt
                << ", vtx_sumpt2: " << vtx_sumpt2
                << ", deltaz_sig_photons: " << deltaz_sig_photons
                << ", deltaphi_sig_photons: " << deltaphi_sig_photons);

  return {nph_good, vtx_sumpt, vtx_sumpt2, deltaz_sig_photons,
          deltaphi_sig_photons};
}

double BDTVertexWeightCalculator::estimateSignalCompatibility(
    const xAOD::Vertex& vertex) const {
  if (vertex.vertexType() != xAOD::VxType::VertexType::PriVtx and
      vertex.vertexType() != xAOD::VxType::VertexType::PileUp) {
    return 0.;
  }
  const std::vector<float> input_values = get_input_values(vertex);
  if (input_values.empty()) { return 0.; }
  return m_bdt->GetClassification(input_values);
}

const xAOD::Vertex* BDTVertexWeightCalculator::getVertex(
    const xAOD::VertexContainer& vertices) const {
  float best_score = std::numeric_limits<float>::lowest();
  const xAOD::Vertex* vertex = nullptr;
  for (const auto& v : vertices) {
    if (v == nullptr) {
      ATH_MSG_WARNING("Null vertex in container");
      continue;
    }
    if (v->vertexType() == xAOD::VxType::PriVtx) {
      const float score = v->isAvailable<float>("score")
                              ? v->auxdata<float>("score")
                              : estimateSignalCompatibility(*v);
      if (score > best_score) {
        best_score = score;
        vertex = v;
      }
    }
  }
  if (vertex == nullptr) {
     ATH_MSG_WARNING("No vertex found");
  }
  return vertex;
}
