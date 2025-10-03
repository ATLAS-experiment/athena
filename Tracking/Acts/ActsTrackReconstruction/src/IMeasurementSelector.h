/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTRK_MEASUREMENTSELECTOR_H
#define ACTSTRK_MEASUREMENTSELECTOR_H

#include "src/detail/AtlasUncalibSourceLinkAccessor.h"
#include <any>

namespace ActsTrk {
   class IMeasurementSelector {
   public:
      virtual ~IMeasurementSelector() {}
      virtual void connect(std::any delegate_wrap) const = 0;
      virtual void setMeasurementRangesForced(const ActsTrk::detail::MeasurementRangeListFlat *measurementRangesForced) = 0;
   };
}
#endif
