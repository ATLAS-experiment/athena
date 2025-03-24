/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCEINTERACES_LAYERBUCKET_H
#define MUONINFERENCEINTERACES_LAYERBUCKET_H

#include "MuonSpacePoint/SpacePointContainer.h"
#include "GaudiKernel/SystemOfUnits.h"
namespace MuonML{
    /** @brief The LayerSpBucket is a space pointbucket where the points are internally 
     *         sorted by their layer number as defined in the SpacePointLayerSorter. The
     *         bucket also provides the layer number tag for each space point & the number of total layers
      */
    class LayerSpBucket : public std::vector<const MuonR4::SpacePoint*> {
        public:
          /** @brief Standard constructor taking the space point bucket */  
          LayerSpBucket(const MuonR4::SpacePointBucket& bucket);
          /** @brief Returns how many Mdt layers are inside the bucket */
          uint8_t nMdtLayers() const {
              return m_nMdtLay;
          }
          /** @brief Returns how many Strip layers are inside the bucket */
          uint8_t nStripLayers() const {
              return m_nStripLay;
          }
          /** @brief Returns the associated layer number of the i-the space point inside the bucket */
          uint8_t layerNum(const size_t i) const {
            return m_layNum[i];
          }
          /** @brief Returns the max covered position of the bucket */
          double coveredMax() const {
              return m_max;
          }
          /** @brief Returns the min covered position of the bucket */
          double coveredMin() const {
              return m_min;
          }
        private:
            uint8_t m_nMdtLay{0};
            uint8_t m_nStripLay{0};
            std::vector<uint8_t> m_layNum{};
            double m_min{-20. *Gaudi::Units::m};
            double m_max{20. * Gaudi::Units::m};
    };
}
#endif