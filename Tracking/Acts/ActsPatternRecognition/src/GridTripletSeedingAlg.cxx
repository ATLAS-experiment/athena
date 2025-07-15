/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

  // Apply selection on which SPs you want to use from the input container
  Acts::Experimental::SpacePointContainer2 selectedSpacePoints;
  selectedSpacePoints.createExtraColumns(
      Acts::Experimental::SpacePointKnownExtraColumn::R |
      Acts::Experimental::SpacePointKnownExtraColumn::Phi |
      Acts::Experimental::SpacePointKnownExtraColumn::VarianceR |
      Acts::Experimental::SpacePointKnownExtraColumn::VarianceZ |
      Acts::Experimental::SpacePointKnownExtraColumn::Strip);

  for (const auto* collection : allInputCollections) {
    for (const xAOD::SpacePoint* inputSp : *collection) {
      auto newSp = selectedSpacePoints.createSpacePoint(
          std::array<Acts::SourceLink, 1>{Acts::SourceLink(inputSp)},
          inputSp->x(), inputSp->y(), inputSp->z());
      newSp.phi() = std::atan2(inputSp->y(), inputSp->x());
      newSp.r() = std::hypot(inputSp->x(), inputSp->y());
      newSp.varianceR() = inputSp->varianceR();
      newSp.varianceZ() = inputSp->varianceZ();
      if (!m_usePixel.value()) {
        newSp.topStripVector() =
            inputSp->topHalfStripLength() * inputSp->topStripDirection();
        newSp.bottomStripVector() =
            inputSp->bottomHalfStripLength() * inputSp->bottomStripDirection();
        newSp.stripCenterDistance() = inputSp->stripCenterDistance();
        newSp.topStripCenter() = inputSp->topStripCenter();
      }
    }
  }

  ATH_MSG_DEBUG(
      "    \\__ Total input space points: " << selectedSpacePoints.size());
  m_stat[kNSpacepoints] += selectedSpacePoints.size();

  // Early Exit in case no space points at this stage
  if (selectedSpacePoints.empty()) {
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

  Acts::Experimental::SeedContainer2 seedContainer;

  ATH_MSG_DEBUG("Running Grid Triplet Seed Finding ...");
  time_seedCreation.start();
  try {
    ATH_CHECK(m_seedsTool->createSeeds2(ctx, selectedSpacePoints,
                                        beamPos.cast<float>(), bField.z(),
                                        seedContainer));
  } catch (const std::exception& e) {
    ATH_MSG_ERROR("Exception caught during seed creation: " << e.what());
    return StatusCode::FAILURE;
  }
  time_seedCreation.stop();

  for (const auto& seed : seedContainer) {
    const auto* bottom = selectedSpacePoints.at(seed.spacePointIndices()[0])
                             .sourceLinks()[0]
                             .get<const xAOD::SpacePoint*>();
    const auto* middle = selectedSpacePoints.at(seed.spacePointIndices()[1])
                             .sourceLinks()[0]
                             .get<const xAOD::SpacePoint*>();
    const auto* top = selectedSpacePoints.at(seed.spacePointIndices()[2])
                          .sourceLinks()[0]
                          .get<const xAOD::SpacePoint*>();

    auto outputSeed = std::make_unique<ActsTrk::Seed>(*bottom, *middle, *top);
    outputSeed->setVertexZ(seed.vertexZ());
    outputSeed->setQuality(seed.quality());
    seedPtrs->push_back(std::move(outputSeed));
  }

  ATH_MSG_DEBUG("    \\__ Created " << seedPtrs->size() << " seeds");
  m_stat[kNSeeds] += seedPtrs->size();

  mon_nSeeds = seedPtrs->size();

  return StatusCode::SUCCESS;
}

}  // namespace ActsTrk
