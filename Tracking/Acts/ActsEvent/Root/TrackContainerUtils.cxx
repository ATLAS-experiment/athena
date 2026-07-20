/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#include "ActsEvent/TrackContainerUtils.h"

namespace ActsTrk {
const std::string TrackContainerUtils::s_fitterColumnName("fitter");
const Acts::ProxyAccessor<xAOD::TrackFitter>
  TrackContainerUtils::s_fitterAccessor(TrackContainerUtils::s_fitterColumnName);

const Acts::ConstProxyAccessor<xAOD::TrackFitter>
  TrackContainerUtils::s_constFitterAccessor(TrackContainerUtils::s_fitterColumnName);
}
