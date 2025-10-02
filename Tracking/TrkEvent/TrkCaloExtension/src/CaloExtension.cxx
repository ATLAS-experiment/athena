/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkCaloExtension/CaloExtension.h"

Trk::CaloExtension::CaloExtension(std::unique_ptr<TrackParameters> caloEntry,
                             std::unique_ptr<TrackParameters> muonEntry,
                             std::vector<CurvilinearParameters>&& caloLayers)
    : m_caloEntryLayerIntersection(std::move(caloEntry)),
      m_muonEntryLayerIntersection(std::move(muonEntry)),
      m_caloLayerIntersections(std::move(caloLayers)) {}

