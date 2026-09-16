/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceTripletSeedingAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteHandle.h"
#include "MagFieldElements/AtlasFieldCache.h"

// traccc EDM
#include "traccc/edm/silicon_cell_collection.hpp"
#include "traccc/edm/measurement_collection.hpp"

// vecmem
#include "vecmem/memory/memory_resource.hpp"


namespace ActsTrk {

// -----------------------------------------------------------------------
StatusCode DeviceTripletSeedingAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name());

  ATH_CHECK(m_seedingAlgProviderTool.retrieve());
  ATH_CHECK(m_inputPixelSPKey.initialize());
  ATH_CHECK(m_outputPixelSeedsKey.initialize());
  ATH_CHECK(m_beamSpotKey.initialize());
  ATH_CHECK(m_fieldCondObjInputKey.initialize());

  ATH_CHECK(configureTripletSeeder());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode DeviceTripletSeedingAlg::configureTripletSeeder()
{

  ATH_MSG_INFO("Setting up configs");
  m_seedfinder.zMin = m_zMin * unit<traccc::scalar>::mm;
  m_seedfinder.zMax = m_zMax * unit<traccc::scalar>::mm;
  m_seedfinder.rMin = m_rMin * unit<traccc::scalar>::mm;
  m_seedfinder.rMax = m_rMax * unit<traccc::scalar>::mm;
  m_seedfinder.collisionRegionMin = m_collisionRegionMin * unit<traccc::scalar>::mm;
  m_seedfinder.collisionRegionMax = m_collisionRegionMax * unit<traccc::scalar>::mm;
  m_seedfinder.minPt = m_minPt * unit<traccc::scalar>::MeV;
  m_seedfinder.cotThetaMax = m_cotThetaMax;
  m_seedfinder.deltaRMin = m_deltaRMin * unit<traccc::scalar>::mm;
  m_seedfinder.deltaRMax = m_deltaRMax * unit<traccc::scalar>::mm;
  m_seedfinder.deltaZMax = m_deltaZMax * unit<traccc::scalar>::mm;
  m_seedfinder.impactMax = m_impactMax * unit<traccc::scalar>::mm;
  m_seedfinder.sigmaScattering = m_sigmaScattering;
  m_seedfinder.maxPtScattering = m_maxPtScattering * unit<traccc::scalar>::MeV;
  m_seedfinder.radLengthPerSeed = m_radLengthPerSeed;
  m_seedfinder.maxSeedsPerSpM = m_maxSeedsPerSpM;
  m_seedfinder.phiBinDeflectionCoverage = m_phiBinDeflectionCoverage;
  m_seedfinder.setup();

  m_seedfilter.deltaInvHelixDiameter = m_deltaInvHelixDiameter / unit<traccc::scalar>::mm;
  m_seedfilter.impactWeightFactor = m_impactWeightFactor;
  m_seedfilter.compatSeedWeight = m_compatSeedWeight;
  m_seedfilter.deltaRMin = m_filterDeltaRMin * unit<traccc::scalar>::mm;
  m_seedfilter.compatSeedLimit = m_compatSeedLimit;
  m_seedfilter.good_spB_min_radius = m_goodSpBMinRadius * unit<traccc::scalar>::mm;
  m_seedfilter.good_spB_weight_increase = m_goodSpBWeightIncrease;
  m_seedfilter.good_spT_max_radius = m_goodSpTMaxRadius * unit<traccc::scalar>::mm;
  m_seedfilter.good_spT_weight_increase = m_goodSpTWeightIncrease;
  m_seedfilter.good_spB_min_weight = m_goodSpBMinWeight;
  m_seedfilter.seed_min_weight = m_seedMinWeight;
  m_seedfilter.spB_min_radius = m_spBMinRadius * unit<traccc::scalar>::mm;

  return StatusCode::SUCCESS;

}

StatusCode DeviceTripletSeedingAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Executing device triplet seeding.");

  // ---- 1. Read input traccc spacepoints from StoreGate --------------------------------
  auto inputTracccPixelSpacepoints = SG::makeHandle(m_inputPixelSPKey, ctx);
  ATH_CHECK(inputTracccPixelSpacepoints.isValid());
  ATH_MSG_DEBUG("Read traccc spacepoints from '"
                         << inputTracccPixelSpacepoints.key() << "'");

  // ---- 2. Read the beam spot and the magnetic field at the beam spot -------------------
  const InDet::BeamSpotData* beamSpotData{nullptr};
  ATH_CHECK(SG::get(beamSpotData, m_beamSpotKey, ctx));
  const Amg::Vector3D beamPos = beamSpotData->beamPos();

  const AtlasFieldCacheCondObj* fieldCondObj{nullptr};
  ATH_CHECK(SG::get(fieldCondObj, m_fieldCondObjInputKey, ctx));
  MagField::AtlasFieldCache fieldCache;
  fieldCondObj->getInitializedCache(fieldCache);
  double bField[3]{0., 0., 0.};
  const double position[3]{beamPos.x(), beamPos.y(), 0.};
  fieldCache.getField(position, bField);

  // The field cache returns the field in kT
  traccc::seedfinder_config seedfinder = m_seedfinder;
  seedfinder.bFieldInZ = static_cast<float>(bField[2] * 1000. * unit<traccc::scalar>::T);
  seedfinder.beamPos = {static_cast<float>(beamPos.x() * unit<traccc::scalar>::mm),
                        static_cast<float>(beamPos.y() * unit<traccc::scalar>::mm)};
  seedfinder.setup();

  traccc::spacepoint_grid_config grid{seedfinder};
  if (m_gridDeltaRMax > 0.f) {
    grid.deltaRMax = m_gridDeltaRMax * unit<traccc::scalar>::mm;
  }

  // ---- 3. Get traccc seeding alg ---------------------------------------------
  auto seeding_alg = m_seedingAlgProviderTool->getTripletSeedingAlgorithm(ctx, seedfinder, grid, m_seedfilter);

  // ---- 4. Run traccc seed formation ---------------------------------------------
  traccc::edm::seed_collection::buffer pixel_seeds_gpu_buffer = (*seeding_alg)(*inputTracccPixelSpacepoints);

  ATH_MSG_DEBUG("Reconstructed " << seeding_alg.copy().get_size(pixel_seeds_gpu_buffer) << " seeds.");

  // ---- 5. Write output traccc seeds to StoreGate -------------------------
  auto outputTracccPixelSeeds = SG::makeHandle(m_outputPixelSeedsKey, ctx);
  ATH_CHECK(outputTracccPixelSeeds.record(
    std::make_unique<traccc::edm::seed_collection::buffer>(
        std::move(pixel_seeds_gpu_buffer))));
  ATH_MSG_DEBUG("Wrote seeds buffer to '" << m_outputPixelSeedsKey.key() << "'");

  return StatusCode::SUCCESS;
}



} // namespace ActsTrk
