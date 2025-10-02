/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_EXPECTEDHITUTILS_H
#define ACTSTRK_EXPECTEDHITUTILS_H

#include "ActsGeometry/ActsDetectorElement.h"
#include "GaudiKernel/EventContext.h"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include <Acts/EventData/ProxyAccessor.hpp>
#include <array>

class ActsDetectorElement;

namespace ActsTrk::detail {

  /**
   * Helper functions to encode the expected layer patterns in separate columns.
   * The reason for this is that the xAOD backend does not currently support the native
   * std::array that is used to store these.
   */
  struct ExpectedLayerPatternHelper {
    inline static const std::string_view kExpectedLayerPatternColumnName = "expectedLayerPattern";

    inline static const std::string kPixelBarrel = std::string{kExpectedLayerPatternColumnName}+"_PixelBarrel";
    inline static const std::string kPixelEndcap = std::string{kExpectedLayerPatternColumnName}+"_PixelEndcap";
    inline static const std::string kStripBarrel = std::string{kExpectedLayerPatternColumnName}+"_StripBarrel";
    inline static const std::string kStripEndcap = std::string{kExpectedLayerPatternColumnName}+"_StripEndcap";


    template <typename track_container_t>
    static void add(track_container_t& trackContainer) {
      trackContainer.template addColumn<unsigned int>(kPixelBarrel);
      trackContainer.template addColumn<unsigned int>(kPixelEndcap);
      trackContainer.template addColumn<unsigned int>(kStripBarrel);
      trackContainer.template addColumn<unsigned int>(kStripEndcap);
    }

    template <typename track_container_t>
    static bool exists(track_container_t& trackContainer) {
      return trackContainer.hasColumn(kPixelBarrel) 
        && trackContainer.hasColumn(kPixelEndcap)
        && trackContainer.hasColumn(kStripBarrel)
        && trackContainer.hasColumn(kStripEndcap);
    }
    
    template <typename track_proxy_t>
    static void set(track_proxy_t& track, std::array<unsigned int, 4> values) {
      static const Acts::ProxyAccessor<unsigned int> pixelBarrel{kPixelBarrel};
      static const Acts::ProxyAccessor<unsigned int> pixelEndcap{kPixelEndcap};
      static const Acts::ProxyAccessor<unsigned int> stripBarrel{kStripBarrel};
      static const Acts::ProxyAccessor<unsigned int> stripEndcap{kStripEndcap};
      pixelBarrel(track) = values.at(0);
      pixelEndcap(track) = values.at(1);
      stripBarrel(track) = values.at(2);
      stripEndcap(track) = values.at(3);
    }

    template <typename track_proxy_t>
    static std::array<unsigned int, 4> get(const track_proxy_t& track) {
      static const Acts::ConstProxyAccessor<unsigned int> pixelBarrel{kPixelBarrel};
      static const Acts::ConstProxyAccessor<unsigned int> pixelEndcap{kPixelEndcap};
      static const Acts::ConstProxyAccessor<unsigned int> stripBarrel{kStripBarrel};
      static const Acts::ConstProxyAccessor<unsigned int> stripEndcap{kStripEndcap};

      return {
        pixelBarrel(track),
        pixelEndcap(track),
        stripBarrel(track),
        stripEndcap(track)
      };
    }

  };


  /** Extrapolate from the perigee outwards and gather information which detector layers should have hits.
   * @param ctx the current athena EvetContext
   * @paran extrapolator referece to the Acts extrapolation tool
   * @param perigee_parameters the Acts defining track parameters (parameters at the perigee).
   * @param pathLimit the maximum path length to extrapolate to.
   * @return array of layer pattern words: two pairs for pixel and strips containing the barrel and end caps bit pattern with one bit per detector layer.
   * Will extrapolate from the perigee to the given cylinder volume surface and gather the layers which are crossed and on
   * which hits are expected.
   */
  std::array<unsigned int,4> expectedLayerPattern(const EventContext& ctx,
                                                  const IActsExtrapolationTool &extrapolator,
                                                  const Acts::BoundTrackParameters& perigee_parameters,
                                                  double pathLimit);

  /** From a pre-computed set of detector elements, determine what the hit
   * pattern is and which detector layers should have hits
   * @param detectorElements A set of detector elements to consider
  */
  std::array<unsigned int,4> expectedLayerPattern(std::span<const ActsDetectorElement*> detectorElements);

  void addToExpectedLayerPattern(std::array<unsigned int,4>& pattern, const ActsDetectorElement& detElement);
}

#endif
