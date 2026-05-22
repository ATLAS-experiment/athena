/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_MUONMLEVENT_H
#define MUONINFERENCE_MUONMLEVENT_H

/**
 * @file MuonMLEvent.h
 * @brief Generic feature definitions and utilities shared across ML inference tools.
 *
 * This file defines the standard node feature enumeration and related constants
 * used in training and inference pipelines. Tools can reference these definitions
 * to ensure consistency and enable code reuse.
 */

#include <cstdint>

namespace MuonML {

/**
 * @enum SegmentNodeFeatureId
 * @brief Identifier for each node feature in segment-based GNNs.
 *
 * The order of these enum values matches the training data export order.
 * They are used to look up feature indices by name from model metadata
 * and to compute feature vectors during inference.
 */
enum class SegmentNodeFeatureId : uint8_t {
  SegmentPositionX,      ///< Segment position (x-coordinate, in meters)
  SegmentPositionY,      ///< Segment position (y-coordinate, in meters)
  SegmentPositionZ,      ///< Segment position (z-coordinate, in meters)
  SegmentDirectionX,     ///< Segment direction (x-component, unit vector)
  SegmentDirectionY,     ///< Segment direction (y-component, unit vector)
  SegmentDirectionZ,     ///< Segment direction (z-component, unit vector)
  BucketChamberIndex,    ///< Muon chamber index (integer ID)
  BucketLayers,          ///< Number of precision/phi/trigger layers in segment
  BucketSector,          ///< Sector number (0 to SectorModulo-1, typically 16)
  BucketSegments         ///< Count of segments in the same chamber/layer group
};

} // namespace MuonML

#endif
