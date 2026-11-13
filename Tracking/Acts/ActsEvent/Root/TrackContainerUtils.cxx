/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#include "ActsEvent/TrackContainerUtils.h"

namespace ActsTrk {
const Acts::ProxyAccessor<UnderlyingOrSelf_t<xAOD::TrackFitter>>
  TrackContainerUtils::s_fitterAccessor(std::string{s_fitterColumnName});

const Acts::ConstProxyAccessor<UnderlyingOrSelf_t<xAOD::TrackFitter>>
  TrackContainerUtils::s_constFitterAccessor(std::string{s_fitterColumnName});
}
