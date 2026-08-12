/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CLEAN_HIT_DECORATOR_ALG_HH
#define CLEAN_HIT_DECORATOR_ALG_HH

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Containers
#include "xAODTracking/TrackMeasurementValidationContainer.h"

// Read and write handle keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace FlavorTagDiscriminants {

  class CleanHitDecoratorAlg : public AthReentrantAlgorithm {
    /** @name CleanHitDecoratorAlg
     *  @brief Flag each measurement in a TrackMeasurementValidationContainer as
     *         "clean" (isCleanHit). A hit is clean if it passes the per-hit
     *         goodness checks (not fake, no bytestream error, good DCS state)
     *         and, optionally, if it is not tagged as a module overlap.
     */

    public:
      CleanHitDecoratorAlg(const std::string& name, ISvcLocator* svcLoc);

      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;

    private:
      // Input hits
      SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer> m_hitContainer{
        this, "hitContainer", "PixelClusters", "Input measurement container"};

      // Per-hit decorations read to decide the clean flag
      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_becKey{
        this, "BECKey", m_hitContainer, "bec", "bec decoration"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_isFakeKey{
        this, "IsFakeKey", m_hitContainer, "isFake", "isFake decoration"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_hasBSErrKey{
        this, "HasBSErr", m_hitContainer, "hasBSError", "has BS error decoration"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_DCSStateKey{
        this, "DCSState", m_hitContainer, "DCSState", "DCS state decoration"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_deidKey{
        this, "deidKey", m_hitContainer, "detectorElementID", "detector element ID decoration"};

      SG::ReadDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_layerKey{
        this, "layerKey", m_hitContainer, "layer", "layer decoration key"};

      // Output decoration
      SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_cleanHitKey{
        this, "CleanHitKey", m_hitContainer, "isCleanHit", "clean hit decoration"};

      // Configurable behaviour
      Gaudi::Property<bool> m_isSCT{
        this, "isSCT", false, "hits are from the SCT"};

      Gaudi::Property<bool> m_doOverlapRemoval{
        this, "DoOverlapRemoval", true, "Apply module-overlap tagging"};
  };

}

#endif
