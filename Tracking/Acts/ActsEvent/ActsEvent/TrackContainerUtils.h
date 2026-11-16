/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTRK_TRACKCONTAINERUTILS_H
#define ACTSTRK_TRACKCONTAINERUTILS_H

// fitter definition:
#include "xAODTracking/TrackingPrimitives.h"
#include "Acts/EventData/ProxyAccessor.hpp"
#include "ActsEvent/Decoration.h"

#include <cstdint>
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
      return static_cast<xAOD::TrackFitter>(s_constFitterAccessor(trackProxy));
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
   static void addFitterTypeProperty(track_container_t &tracksContainer) { tracksContainer.template addColumn<UnderlyingOrSelf_t<xAOD::TrackFitter>>(s_fitterColumnName); }

   /// flag a track as found with the brem recovery of the track finding
   /// @tparam trackproxy_t should be of type Acts::TrackProxy<...>
   template <Acts::detail::ProxyType trackproxy_t>
   static void setBremTrack(trackproxy_t &trackProxy, bool bremTrack) {
      s_bremTrackAccessor(trackProxy) = bremTrack;
   }

   /// test whether a track was found with the brem recovery of the track finding.
   /// Returns false if the track container has no brem track column.
   /// @tparam consttrackproxy_t should be of type Acts::ConstTrackProxy<...>
   template <Acts::detail::ProxyType consttrackproxy_t>
   static bool isBremTrack(const consttrackproxy_t &trackProxy) {
      return s_constBremTrackAccessor.hasColumn(trackProxy) && s_constBremTrackAccessor(trackProxy) != 0;
   }

   /// add brem track column to the track container
   /// @tparam track_container_t should be of type Acts::TackContainer<...>
   template <typename track_container_t>
   static void addBremTrackProperty(track_container_t &tracksContainer) { tracksContainer.template addColumn<std::uint8_t>(s_bremTrackColumnName); }

private:
   static constexpr std::string_view s_fitterColumnName = "fitter";
   static const Acts::ProxyAccessor<UnderlyingOrSelf_t<xAOD::TrackFitter>> s_fitterAccessor;
   static const Acts::ConstProxyAccessor<UnderlyingOrSelf_t<xAOD::TrackFitter>> s_constFitterAccessor;

   static constexpr std::string_view s_bremTrackColumnName = "bremTrack";
   static const Acts::ProxyAccessor<std::uint8_t> s_bremTrackAccessor;
   static const Acts::ConstProxyAccessor<std::uint8_t> s_constBremTrackAccessor;
};
}
#endif
