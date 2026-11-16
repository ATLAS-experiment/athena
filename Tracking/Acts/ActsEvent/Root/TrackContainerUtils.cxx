/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#include "ActsEvent/TrackContainerUtils.h"

namespace ActsTrk {
const Acts::ProxyAccessor<UnderlyingOrSelf_t<xAOD::TrackFitter>>
  TrackContainerUtils::s_fitterAccessor(std::string{s_fitterColumnName});

const Acts::ConstProxyAccessor<UnderlyingOrSelf_t<xAOD::TrackFitter>>
  TrackContainerUtils::s_constFitterAccessor(std::string{s_fitterColumnName});

const Acts::ProxyAccessor<std::uint8_t>
  TrackContainerUtils::s_bremTrackAccessor(std::string{s_bremTrackColumnName});

const Acts::ConstProxyAccessor<std::uint8_t>
  TrackContainerUtils::s_constBremTrackAccessor(std::string{s_bremTrackColumnName});
}
