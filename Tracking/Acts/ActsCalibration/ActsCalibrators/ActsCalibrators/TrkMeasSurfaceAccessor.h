/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBRATION_DETAIL_TRKMEASSURFACEACCESSOR_H
#define ACTSCALIBRATION_DETAIL_TRKMEASSURFACEACCESSOR_H

#include "Acts/EventData/SourceLink.hpp"
#include "ActsToolInterfaces/IActsToTrkConverterTool.h"

namespace ActsTrk::detail {
    /** @brief  Helper class to access the Acts::Surface for a given Acts::SourceLink which is
     *          poiniting to a Trk::MeasurementBase. The measurement must be be associated with a
     *          surface hold by a Trk::DetElementBase. Temporary ad-hoc surfaces cannot be converted. */
    class TrkMeasSurfaceAccessor {
        public:
            /** @brief Empty default constructor -> conversion will crash */
            TrkMeasSurfaceAccessor() = default;
            /** @brief Standard constructor taking the pointer to a configured surface
             *         conversion tool. */
            TrkMeasSurfaceAccessor(const IActsToTrkConverterTool* trkConvTool);
            /** @brief Operator called by the Acts API to fetch the surface. */
            const Acts::Surface* operator()(const Acts::SourceLink& sourceLink) const;
        private:
            const IActsToTrkConverterTool* m_trkConvTool{nullptr};
    };
}
#endif