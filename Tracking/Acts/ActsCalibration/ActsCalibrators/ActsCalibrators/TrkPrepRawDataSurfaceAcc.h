/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBRATION_DETAIL_TRKPRDSURFACEACCESSOR_H
#define ACTSCALIBRATION_DETAIL_TRKPRDSURFACEACCESSOR_H


#include "Acts/EventData/SourceLink.hpp"
#include "ActsGeometryInterfaces/IGeometryRealmConvTool.h"

namespace ActsTrk::detail {
    /** @brief Helper class to access the Acts::surface associated with a Trk::PrepRawData measurement.
     *         The link between Trk <-> Acts world is established via the IGeometryRealmConvTool. */
    class TrkPrepRawDataSurfaceAcc{
        public:
            /** @brief Empty default constructor -> conversion will crash */
            TrkPrepRawDataSurfaceAcc() = default;
            /** @brief Standard constructor taking the pointer to a configured surface
             *         conversion tool. */
            TrkPrepRawDataSurfaceAcc(const IGeometryRealmConvTool* trkConvTool);
            /** @brief Operator called by the Acts API to fetch the surface. */
            const Acts::Surface* operator()(const Acts::SourceLink& sourceLink) const;
        private:
            /** @brief Pointer to the converter tool caching the Acts surfaces */
            const IGeometryRealmConvTool* m_trkConvTool{nullptr};

    };
}

#endif