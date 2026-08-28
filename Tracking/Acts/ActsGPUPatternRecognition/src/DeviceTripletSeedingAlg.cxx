/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceTripletSeedingAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

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
  ATH_CHECK(m_deviceMR.retrieve());
  ATH_CHECK(m_inputPixelSPKey.initialize());
  ATH_CHECK(m_outputPixelSeedsKey.initialize());

  ATH_CHECK(configureTripletSeeder());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode DeviceTripletSeedingAlg::configureTripletSeeder()
{

  ATH_MSG_INFO("Setting up configs");
  m_seedfinder.zMin = -3000.f * unit<traccc::scalar>::mm;
  m_seedfinder.zMax = 3000.f * unit<traccc::scalar>::mm;
  m_seedfinder.rMax = 320.f * unit<traccc::scalar>::mm;
  m_seedfinder.rMin = 33.f * unit<traccc::scalar>::mm;
  // 3 sigmas of max beam spot delta z for Run 3
  // https://twiki.cern.ch/twiki/pub/AtlasPublic/BeamSpotPublicResults/BeamspotRun2vLumi_sig_z.png
  m_seedfinder.collisionRegionMax = 3 * 38 * unit<traccc::scalar>::mm;
  m_seedfinder.collisionRegionMin = -m_seedfinder.collisionRegionMax;
  // Based on Pixel barrel layers layout:
  // https://cds.cern.ch/record/2850865/files/ITK_schematic.png
  // Barrel layers position: 33, 96, 126, 225, 286
  // Minimum R distance between 2 layers: 30
  // Max distance between N, N+2 layer (allowing one "hole"): 160
  // Plus some margin (2 mm)
  m_seedfinder.deltaRMin = 20 * unit<traccc::scalar>::mm;
  m_seedfinder.deltaRMax = 100 * unit<traccc::scalar>::mm;
  m_seedfinder.deltaZMax = 800 * unit<traccc::scalar>::mm;

  m_seedfinder.minPt = 900.f * unit<traccc::scalar>::MeV;
  m_seedfinder.cotThetaMax = 27.2899f;
  m_seedfinder.impactMax = 2.f * unit<traccc::scalar>::mm;
  m_seedfinder.sigmaScattering = 3.0f;
  m_seedfinder.maxPtScattering = 10.f * unit<traccc::scalar>::GeV;
  m_seedfinder.radLengthPerSeed = 0.05f;
  m_seedfinder.maxSeedsPerSpM = 2;
  m_seedfinder.setup();

  m_seedfilter.good_spB_min_radius = 150.f * unit<traccc::scalar>::mm;
  m_seedfilter.good_spB_weight_increase = 400.f;
  m_seedfilter.good_spT_max_radius = 150.f * unit<traccc::scalar>::mm;
  m_seedfilter.good_spT_weight_increase = 200.f;
  m_seedfilter.good_spB_min_weight = 380.f;
  m_seedfilter.seed_min_weight = 200.f;
  m_seedfilter.spB_min_radius = 43.f * unit<traccc::scalar>::mm;
  m_seedfilter.compatSeedLimit = 1;
  // m_seedfilter.deltaInvHelixDiameter = 0.00003f / unit<traccc::scalar>::mm;

  return StatusCode::SUCCESS;

}

StatusCode DeviceTripletSeedingAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Executing device pixel seeding.");

  // ---- 1. Read input traccc measurements from StoreGate --------------------------------
  auto inputTracccPixelSpacepoints = SG::makeHandle(m_inputPixelSPKey, ctx);
  ATH_CHECK(inputTracccPixelSpacepoints.isValid());
  ATH_MSG_DEBUG("Read traccc spacepoints from '"
                         << inputTracccPixelSpacepoints.key() << "'");

  // ---- 2. Get traccc seeding alg ---------------------------------------------
  auto seeding_alg = m_seedingAlgProviderTool->getTripletSeedingAlgorithm(ctx, m_seedfinder, m_seedfilter);

  // ---- 3. Run traccc pixel seed formation ---------------------------------------------
  traccc::edm::seed_collection::buffer pixel_seeds_gpu_buffer = (*seeding_alg)(*inputTracccPixelSpacepoints);

  ATH_MSG_DEBUG("Reconstructed " << seeding_alg.copy().get_size(pixel_seeds_gpu_buffer) << " pixel seeds.");

  // ---- 4. Write output traccc seeds to StoreGate -------------------------
  auto outputTracccPixelSeeds = SG::makeHandle(m_outputPixelSeedsKey, ctx);
  ATH_CHECK(outputTracccPixelSeeds.record(
    std::make_unique<traccc::edm::seed_collection::buffer>(
        std::move(pixel_seeds_gpu_buffer))));
  ATH_MSG_DEBUG("Wrote seeds buffer to '" << m_outputPixelSeedsKey.key() << "'");

  return StatusCode::SUCCESS;
}



} // namespace ActsTrk
