/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBRATION_DETAIL_TRKPRDSURFACEACCESSOR_H
#define ACTSCALIBRATION_DETAIL_TRKPRDSURFACEACCESSOR_H


#include "Acts/EventData/SourceLink.hpp"
#include "ActsToolInterfaces/IActsToTrkConverterTool.h"

namespace ActsTrk::detail {
    /** @brief Helper class to access the Acts::surface associated with a Trk::PrepRawData measurement.
     *         The link between Trk <-> Acts world is established via the IActsToTrkConverterTool. */
    class TrkPrepRawDataSurfaceAcc{
        public:
            /** @brief Empty default constructor -> conversion will crash */
            TrkPrepRawDataSurfaceAcc() = default;
            /** @brief Standard constructor taking the pointer to a configured surface
             *         conversion tool. */
            TrkPrepRawDataSurfaceAcc(const IActsToTrkConverterTool* trkConvTool);
            /** @brief Operator called by the Acts API to fetch the surface. */
            const Acts::Surface* operator()(const Acts::SourceLink& sourceLink) const;
        private:
            /** @brief Pointer to the converter tool caching the Acts surfaces */
            const IActsToTrkConverterTool* m_trkConvTool{nullptr};

    };
}

#endif