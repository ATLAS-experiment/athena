/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRK_CALOEXTENSION_H
#define TRK_CALOEXTENSION_H

#include <vector>
#include "TrkParameters/TrackParameters.h"

namespace Trk {

/**
  Tracking class to hold the extrapolation through calorimeter Layers
   Both the caloEntryLayerIntersection and
   the muonEntryLayerIntersection can return nullptr if the particle failed to
   reach the layer
 */
class CaloExtension {
 public:
  /** constructor taking result of the intersection as arguments*/
  CaloExtension(std::unique_ptr<TrackParameters> caloEntry,
                std::unique_ptr<TrackParameters>  muonEntry,
                std::vector<CurvilinearParameters>&& caloLayers);

  /** access to intersection with the calorimeter entry layer
      return nullptr if the intersection failed
   */
  const TrackParameters* caloEntryLayerIntersection() const;

  /** access to intersection with the muon entry layer
      return nullptr if the intersection failed
   */
  const TrackParameters* muonEntryLayerIntersection() const;

  /** access to the intersections with the calorimeter layers.
      The intersections are stored as curvilinear parameters,
      the layers are identified by their cIdentifier() */
  const std::vector<CurvilinearParameters>& caloLayerIntersections() const;

 private:

  /// parameters at the calorimeter entrance
  std::unique_ptr<TrackParameters> m_caloEntryLayerIntersection{nullptr};
  /// parameters at the muon entrance
  std::unique_ptr<TrackParameters> m_muonEntryLayerIntersection{nullptr};
  /// parameters at the different calorimeter layers
  std::vector<CurvilinearParameters> m_caloLayerIntersections{};
};

inline const TrackParameters* CaloExtension::caloEntryLayerIntersection() const {
  return m_caloEntryLayerIntersection.get();
}

inline const TrackParameters* CaloExtension::muonEntryLayerIntersection() const {
  return m_muonEntryLayerIntersection.get();
}

inline const std::vector<CurvilinearParameters>& CaloExtension::caloLayerIntersections() const {
  return m_caloLayerIntersections;
}

}  // namespace Trk

#endif
