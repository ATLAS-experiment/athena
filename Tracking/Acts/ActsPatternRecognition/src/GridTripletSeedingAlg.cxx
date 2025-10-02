/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/GridTripletSeedingAlg.h"

// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/SeedContainer2.hpp"
#include "Acts/EventData/SpacePointContainer2.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Seeding/BinnedGroup.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/SeedFinder.hpp"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"

// Other
#include "ActsInterop/TableUtils.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SiSPSeededTrackFinderData/ITkSiSpacePointForSeed.h"
#include "SiSPSeededTrackFinderData/SiSpacePointForSeed.h"
#include "TrkSpacePoint/SpacePointCollection.h"

namespace ActsTrk {

GridTripletSeedingAlg::GridTripletSeedingAlg(const std::string& name,
                                             ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode GridTripletSeedingAlg::initialize() {
  ATH_MSG_INFO("Initializing " << name() << " ... ");
  if (m_fastTracking)
    ATH_MSG_INFO("   using fast tracking configuration.");

  // Retrieve seed tool
  ATH_CHECK(m_seedsTool.retrieve());

  // Cond
  ATH_CHECK(m_beamSpotKey.initialize());
  ATH_CHECK(m_fieldCondObjInputKey.initialize());

  // Read and Write handles
  ATH_CHECK(m_spacePointKey.initialize());
  ATH_CHECK(m_seedKey.initialize());

  ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));

  return StatusCode::SUCCESS;
}

StatusCode GridTripletSeedingAlg::finalize() {
  ATH_MSG_INFO("Seed statistics" << std::endl
                                 << makeTable(m_stat,
                                              std::array<std::string, kNStat>{
                                                  "Spacepoints", "Seeds"})
                                        .columnWidth(10));
  return StatusCode::SUCCESS;
}

StatusCode GridTripletSeedingAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("Executing " << name() << " ... ");

  auto timer = Monitored::Timer<std::chrono::milliseconds>("TIME_execute");
  auto time_seedCreation =
      Monitored::Timer<std::chrono::milliseconds>("TIME_seedCreation");
  auto mon_nSeeds = Monitored::Scalar<int>("nSeeds");
  auto mon = Monitored::Group(m_monTool, timer, time_seedCreation, mon_nSeeds);

  // ================================================== //
  // ===================== OUTPUTS ==================== //
  // ================================================== //

  SG::WriteHandle<ActsTrk::SeedContainer> seedHandle =
      SG::makeHandle(m_seedKey, ctx);
  ATH_MSG_DEBUG("    \\__ Seed Container `" << m_seedKey.key()
                                            << "` created ...");
  ATH_CHECK(seedHandle.record(std::make_unique<ActsTrk::SeedContainer>()));
  ActsTrk::SeedContainer* seedPtrs = seedHandle.ptr();

  // ================================================== //
  // ===================== INPUTS ===================== //
  // ================================================== //

  // Read the Beam Spot information
  const InDet::BeamSpotData* beamSpotData{nullptr};
  ATH_CHECK(SG::get(beamSpotData, m_beamSpotKey, ctx));
  // Beam Spot Position
  Acts::Vector3 beamPos(beamSpotData->beamPos().x() * Acts::UnitConstants::mm,
                        beamSpotData->beamPos().y() * Acts::UnitConstants::mm,
                        beamSpotData->beamPos().z() * Acts::UnitConstants::mm);

  ATH_MSG_DEBUG("Retrieving elements from " << m_spacePointKey.size()
                                            << " input collections...");
  std::vector<const xAOD::SpacePointContainer*> allInputCollections;
  allInputCollections.reserve(m_spacePointKey.size());

  for (const auto& spacePointKey : m_spacePointKey) {
    ATH_MSG_DEBUG("Retrieving from Input Collection '" << spacePointKey.key()
                                                       << "' ...");
    const xAOD::SpacePointContainer* spCont{nullptr};
    ATH_CHECK(SG::get(spCont, spacePointKey, ctx));
    allInputCollections.push_back(spCont);
    ATH_MSG_DEBUG("    \\__ " << spCont->size() << " elements!");
  }

  std::size_t totalSpacePoints = 0;
  for (const xAOD::SpacePointContainer* collection : allInputCollections) {
    totalSpacePoints += collection->size();
  }

  ATH_MSG_DEBUG("    \\__ Total input space points: " << totalSpacePoints);
  m_stat[kNSpacepoints] += totalSpacePoints;

  // Early Exit in case no space points at this stage
  if (totalSpacePoints == 0) {
    ATH_MSG_DEBUG("No input space points found, we stop seeding");
    return StatusCode::SUCCESS;
  }

  // ================================================== //
  // ===================== CONDS ====================== //
  // ================================================== //

  // Read the b-field information
  const AtlasFieldCacheCondObj* fieldCondObj{nullptr};
  ATH_CHECK(SG::get(fieldCondObj, m_fieldCondObjInputKey, ctx));

  // Get the magnetic field
  // Using ACTS classes in order to be sure we are consistent
  Acts::MagneticFieldContext magFieldContext(fieldCondObj);
  ATLASMagneticFieldWrapper magneticField;
  Acts::MagneticFieldProvider::Cache magFieldCache =
      magneticField.makeCache(magFieldContext);
  Acts::Vector3 bField = *magneticField.getField(
      Acts::Vector3(beamPos.x(), beamPos.y(), 0), magFieldCache);

  // ================================================== //
  // ===================== COMPUTATION ================ //
  // ================================================== //

  ATH_MSG_DEBUG("Running Grid Triplet Seed Finding ...");
  time_seedCreation.start();
  try {
    ATH_CHECK(m_seedsTool->createSeeds2(ctx, allInputCollections,
                                        beamPos.cast<float>(), bField.z(),
                                        *seedPtrs));
  } catch (const std::exception& e) {
    ATH_MSG_ERROR("Exception caught during seed creation: " << e.what());
    return StatusCode::FAILURE;
  }
  time_seedCreation.stop();

  ATH_MSG_DEBUG("    \\__ Created " << seedPtrs->size() << " seeds");
  m_stat[kNSeeds] += seedPtrs->size();

  mon_nSeeds = seedPtrs->size();

  return StatusCode::SUCCESS;
}

}  // namespace ActsTrk
