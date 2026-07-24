/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FastCaloSimParametrizationTool.h"

#include "G4AtlasTools/FastCaloSimGeoLoader.h"
#include "FastCaloSim/Geometry/CaloGeo.h"
#include "FastCaloSim/Definitions/ParticleData.h"

#include "PathResolver/PathResolver.h"
#include "TruthUtils/HepMCHelpers.h"

#include "G4GDMLParser.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Threading.hh"

#include "TFile.h"

#include <memory>

namespace {

/// Use the ATLAS truth database for FastCaloSim particle charges.
class AtlasParticleDataProvider final : public ParticleData::Provider {
public:
  auto charge(int pdgID) const -> double override {
    return MC::charge(pdgID);
  }
};

// Logical world name in the simplified-geometry GDML file.
constexpr const char* s_worldLogName = "WorldLog";

}  // namespace

FastCaloSimParametrizationTool::FastCaloSimParametrizationTool(
    const std::string &type, const std::string &name, const IInterface *parent)
    : base_class(type, name, parent) {}

FastCaloSimParametrizationTool::~FastCaloSimParametrizationTool() = default;

StatusCode FastCaloSimParametrizationTool::initialize() {

  if (m_geoTag.value().empty()) {
    ATH_MSG_ERROR(
        "CaloGeoTag is not set (expected the job geometry tag, e.g. "
        "ATLAS-R3S-2021-03-02-00)");
    return StatusCode::FAILURE;
  }
  if (m_paramsFilename.value().empty()) {
    ATH_MSG_ERROR(
        "ParamsInputFilename is not set (expected a calib-area path, e.g. "
        "FastCaloSim/MC23/TFCSparam_AF3_MC23_Sep23.root)");
    return StatusCode::FAILURE;
  }

  // Locate the calorimeter geometry folder in the calibration area
  const std::string geoFolder =
      PathResolverFindCalibDirectory(m_geoInputFolder.value());
  if (geoFolder.empty()) {
    ATH_MSG_ERROR("Could not locate FastCaloSim geometry folder = "
                  << m_geoInputFolder.value());
    return StatusCode::FAILURE;
  }

  // Extend the external library's charge lookup to all ATLAS particle IDs.
  ParticleData::setProvider(std::make_shared<const AtlasParticleDataProvider>());

  // Build the external FastCaloSim calorimeter geometry (incl. FCal handler)
  try {
    m_caloGeo = FastCaloSimGeo::loadCaloGeo(geoFolder, m_geoTag.value());
  } catch (const std::exception &e) {
    ATH_MSG_ERROR("Failed to build the FastCaloSim calorimeter geometry: "
                  << e.what());
    return StatusCode::FAILURE;
  }
  CaloGeo *geo = m_caloGeo.get();

  // Locate and open the parametrization input file from the calibration area
  const std::string paramFile =
      PathResolverFindCalibFile(m_paramsFilename.value());
  if (paramFile.empty()) {
    ATH_MSG_ERROR("Could not locate FastCaloSim parametrization file = "
                  << m_paramsFilename.value());
    return StatusCode::FAILURE;
  }
  std::unique_ptr<TFile> param_file(TFile::Open(paramFile.c_str(), "READ"));
  if (!param_file || param_file->IsZombie()) {
    ATH_MSG_ERROR("Could not open FastCaloSim parametrization file = "
                  << paramFile);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Opened FastCaloSim parametrization file = " << paramFile);

  m_param.reset(static_cast<TFCSParametrizationBase *>(
      param_file->Get(m_paramsObject.value().c_str())));
  if (!m_param) {
    ATH_MSG_ERROR("Object " << m_paramsObject.value()
                            << " not found in parametrization file = "
                            << paramFile);
    return StatusCode::FAILURE;
  }

  m_param->set_geometry(geo);
  m_param->setLevel(msg().level());

  m_caloExtrapolationTool.set_geometry(geo);

  return StatusCode::SUCCESS;
}

StatusCode FastCaloSimParametrizationTool::finalize() {

  return StatusCode::SUCCESS;
}

StatusCode FastCaloSimParametrizationTool::initializeTransportGeometry() {
  // AtlasG4 loads this geometry earlier; ISF relies on this fallback.
  if (G4Threading::IsMasterThread() && !m_simplifiedGeoPath.value().empty() &&
      !G4LogicalVolumeStore::GetInstance()->GetVolume(s_worldLogName, false)) {
    const std::string geoFile =
        PathResolverFindCalibFile(m_simplifiedGeoPath.value());
    if (geoFile.empty()) {
      ATH_MSG_ERROR("Could not find simplified transport geometry file: "
                    << m_simplifiedGeoPath.value());
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Reading simplified transport geometry from " << geoFile);
    G4GDMLParser parser;
    parser.Read(geoFile, false);
  }

  // Workers can use, but cannot create, the shared transport world.
  if (!m_caloTransportTool.initializeGeometry()) {
    ATH_MSG_FATAL(
        "Failed to initialize the transport world volume. Ensure the "
        "simplified transport geometry is loaded before this call.");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode FastCaloSimParametrizationTool::initializeTransportPropagator() {
  if (!m_caloTransportTool.initializePropagator()) {
    ATH_MSG_FATAL(
        "Failed to initialize the transport propagator: the shared world "
        "volume is not available. initializeTransportGeometry() must run "
        "before initializeTransportPropagator().");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

std::vector<G4FieldTrack> FastCaloSimParametrizationTool::transport(
    const G4Track &G4InputTrack) {
  // These calls are idempotent; workers lazily create only their propagator.
  if (initializeTransportGeometry().isFailure() ||
      initializeTransportPropagator().isFailure()) {
    ATH_MSG_ERROR(
        "Particle transport is not initialized (missing world volume or "
        "propagator); returning no transport steps.");
    return {};
  }
  return m_caloTransportTool.transport(G4InputTrack);
}

void FastCaloSimParametrizationTool::extrapolate(
    TFCSExtrapolationState &result, const TFCSTruthState *truth,
    const std::vector<G4FieldTrack> &caloSteps) {
  return m_caloExtrapolationTool.extrapolate(result, truth, caloSteps);
}

FCSReturnCode FastCaloSimParametrizationTool::simulate(
    TFCSSimulationState &simulstate, const TFCSTruthState *truth,
    const TFCSExtrapolationState *extrapol) {
  return m_param->simulate(simulstate, truth, extrapol);
}
