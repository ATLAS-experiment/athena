/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTRK_TRACKCONTAINERUTILS_H
#define ACTSTRK_TRACKCONTAINERUTILS_H

// fitter definition:
#include "xAODTracking/TrackingPrimitives.h"
#include "Acts/EventData/ProxyAccessor.hpp"

#include <string>

namespace ActsTrk {
/// Convenience struct which defines track augmentations.
struct TrackContainerUtils
{
   /// set fitter type of a track
   /// @tparam trackproxy_t should be of type Acts::TrackProxy<...>
   template <Acts::detail::ProxyType trackproxy_t>
   static void setFitterType(trackproxy_t &trackProxy, xAOD::TrackFitter fitterType) {
      s_fitterAccessor(trackProxy) = fitterType;
   }

   /// get fitter type of a track
   /// @tparam consttrackproxy_t should be of type Acts::ConstTrackProxy<...>
   template <Acts::detail::ProxyType consttrackproxy_t>
   static xAOD::TrackFitter fitterType(const consttrackproxy_t &trackProxy) {
      return s_constFitterAccessor(trackProxy);
   }

   /// test whether a track has a fitter type
   /// @tparam consttrackproxy_t should be of type Acts::ConstTrackProxy<...>
   template <Acts::detail::ProxyType consttrackproxy_t>
   static bool hasFitterType(const consttrackproxy_t &trackProxy) {
      return s_constFitterAccessor.hasColumn(trackProxy);
   }

   /// add fitter column to the track container
   /// @tparam track_container_t should be of type Acts::TackContainer<...>
   template <typename track_container_t>
   static void addFitterTypeProperty(track_container_t &tracksContainer) { tracksContainer.template addColumn<xAOD::TrackFitter>(s_fitterColumnName); }

private:
   static const std::string s_fitterColumnName;
   static const Acts::ProxyAccessor<xAOD::TrackFitter> s_fitterAccessor;
   static const Acts::ConstProxyAccessor<xAOD::TrackFitter> s_constFitterAccessor;
};
}
#endif
