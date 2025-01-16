/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_HITSUMMARYDATAUTILS_H
#define ACTSTRK_HITSUMMARYDATAUTILS_H

#include "ActsEvent/TrackContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"

#include <vector>
#include <cmath>
#include <array>
#include <tuple>
#include <type_traits>

namespace ActsTrk::detail {
   /** @brief Helper to convert class enum into an integer.
    */
   template <typename T_EnumClass >
   constexpr typename std::underlying_type<T_EnumClass>::type to_underlying(T_EnumClass an_enum) {
      return static_cast<typename std::underlying_type<T_EnumClass>::type>(an_enum);
   }
 
   namespace HitCategory {
      enum ESpecialHitCategories {
         DeadSensor,
         Hole,
         N
      };
   }
 
   /** @brief Helper class to gather hit summary information for e.g. tracks.
    */
   class HitSummaryData {
   public:
      /** @brief Regions for which hit counts are computed.
       */
      enum DetectorRegion {
         pixelBarrelFlat     = 0,
         pixelBarrelInclined = 1,
         pixelEndcap         = 2,
         stripBarrel         = 3,
         stripEndcap         = 4,
         unknown             = 5,
         pixelTotal          = 6,
         stripTotal          = 7,
         unknownTotal        = 8,
         Total               = 9
      };
 
      constexpr static unsigned short LAYER_REGION_MASK   = 0x1FF; // bits 0-8
      constexpr static unsigned short REGION_BITS = 3;             // bits 0-2
      constexpr static unsigned short REGION_MASK = 0x7;           // 3 bits
      constexpr static unsigned short LAYER_BITS  = 6;             // bits 3-8
      constexpr static unsigned short LAYER_MASK  = 0x3F;          // 6 bits
      constexpr static unsigned short SIGNED_ETA_MOD_BITS  = 7;    // bits 9-14 + 15(sign)
      constexpr static unsigned short SIGNED_ETA_MOD_MASK = 0x7F;  // 6 + 1(sign) bit
 
      /** @brief Compute a counter key for the given region, layer and module eta module index
       * @param region the detector region index (0..7).
       * @param layer the layer index (0..63).
       * @param the signed eta module index (-63..63).
       */
      constexpr static unsigned short makeKey(unsigned short region, unsigned short layer, int eta_mod) {
         //   3 bits region  :    0-7   : pixelBarrelFlat - unknown
         //   6 bits layer   :    0-63
         // 1+6 bits eta_mod : +- 0-63
         // @TODO endcap side A/C ?
         assert(region < (1<<REGION_BITS) );
         assert(layer  < (1<<LAYER_BITS));
         assert( std::abs(eta_mod) < (1<<(SIGNED_ETA_MOD_BITS-1)) );
         if (region != stripBarrel &&  region != pixelBarrelFlat) {
            layer |= (static_cast<uint8_t>(eta_mod) & SIGNED_ETA_MOD_MASK) << LAYER_BITS ;
         }
         return static_cast<uint8_t>(region) | (layer<<REGION_BITS);
      }
 
      /** @brief extract the region index from the given key.
       */
      constexpr static DetectorRegion regionFromKey(unsigned short key) {
         return static_cast<DetectorRegion>(key & REGION_MASK); // bits 0-2
      }
 
      /** @brief extract the layer index from the given key.
       */
      constexpr static uint8_t layerFromKey(unsigned short key) {
         return (key>>REGION_BITS) & LAYER_MASK;                // bits 3-8
      }
 
      // To select, hits, outliers or both.
      enum EHitSelection {
         Hit = 1,
         Outlier = 2,
         HitAndOutlier = 3
      };
 
      /** @brief reset all summary counters to zero.
       */
      void reset() {
         m_stat.clear();
         std::fill(m_hits.begin(),m_hits.end(), 0u);
         std::fill(m_outlierHits.begin(),m_outlierHits.end(), 0u);
         std::fill(m_layers.begin(),m_layers.end(), 0u);
      }
 
      /** @brief update summaries to take the given hit into account.
       * @param detector_elements detector element collection relevant for the given hit.
       * @param id_hash the id_hash of the hit
       * @param hit_selection should be set to either Hit, or Outlier.
       * @param returns false in case the hit was not considered.
       * The hit is not considered if the given id_hash is not valid for the given detector element collectio.
       */
      bool addHit(const InDetDD::SiDetectorElementCollection *detector_elements, unsigned int id_hash, EHitSelection hit_selection) {
         if (!detector_elements ||  id_hash>=detector_elements->size() || !(*detector_elements)[id_hash]) {
            return false;
         }
         const InDetDD::SiDetectorElement *detEl=(*detector_elements)[id_hash];
         DetectorRegion region = unknown;
         uint8_t layer  = 255;
         int eta_module = 0;
         if (detEl->isPixel()) {
            InDetDD::DetectorType type = detEl->design().type();
            if(type==InDetDD::PixelInclined)  region = pixelBarrelInclined;
            else if(type==InDetDD::PixelBarrel) region = pixelBarrelFlat;
            else region = pixelEndcap;
 
            const PixelID* pixel_id = static_cast<const PixelID *>(detEl->getIdHelper());
            layer = pixel_id->layer_disk(detEl->identify());
            eta_module = pixel_id->eta_module(detEl->identify());
         }
         else if (detEl->isSCT()) {
            region =  (detEl->isBarrel() ?  stripBarrel : stripEndcap);
 
            const SCT_ID* sct_id = static_cast<const SCT_ID *>(detEl->getIdHelper());
            layer = sct_id->layer_disk(detEl->identify());
            eta_module = sct_id->eta_module(detEl->identify());
         }
 
         unsigned short key = makeKey(region, layer, eta_module);
         for (auto &[stat_key, stat_hits, stat_outlier_hits] : m_stat) {
            if (stat_key == key) {
               stat_hits         += ((hit_selection & HitSummaryData::Hit)!=0);
               stat_outlier_hits += ((hit_selection & HitSummaryData::Outlier)!=0);
               return true;
            }
         }
         m_stat.emplace_back( std::make_tuple(key,
                                              ((hit_selection & HitSummaryData::Hit)!=0),
                                              ((hit_selection & HitSummaryData::Outlier)!=0)) );
         return true;
      }
 
      /** @brief Compute the varius summaries.
       * Must be called only after all hits have been gathered, and must not be called more than once.
       */
      void computeSummaries() {
         for (const auto &[stat_key, stat_hits, stat_outlier_hits] : m_stat) {
            unsigned short region=regionFromKey(stat_key);
            m_hits.at(region) += stat_hits;
            m_outlierHits.at(region) += stat_outlier_hits;
            ++m_layers.at(region);
         }
         for (unsigned int region_i=0; region_i<unknown+1; ++region_i) {
            m_hits.at(s_type.at(region_i)) += m_hits[region_i];
            m_outlierHits.at(s_type.at(region_i)) += m_outlierHits[region_i];
            m_layers.at(s_type.at(region_i)) += m_layers[region_i];
            m_hits.at(Total) += m_hits[region_i];
            m_outlierHits.at(Total) += m_outlierHits[region_i];
            m_layers.at(Total) += m_layers[region_i];
         }
      }
 
      /** @brief return the number of layers contributing to the hit collection in the given detector region.
       * @param region the detector region.
       * Only meaningful after @ref computeSummaries was called.
       */
      uint8_t contributingLayers(DetectorRegion region) const {
         return m_layers.at(region);
      }
 
      /** @brief return the number of hits in a certain detector region.
       * @param region the detector region.
       * Only meaningful after @ref computeSummaries was called.
       */
      uint8_t contributingHits(DetectorRegion region) const {
         return m_hits.at(region);
      }
 
      /** @brief return the number of outliers in a certain detector region.
       * @param region the detector region.
       * Only meaningful after @ref computeSummaries was called.
       */
      uint8_t contributingOutlierHits(DetectorRegion region) const {
         return m_outlierHits.at(region);
      }
 
 
      /** @brief return the total number of hits, outliers or hits+outliers in the givrn detector region and layer.
       * @param region the detector region.
       * @param layer the detector layer.
       */
      template <unsigned short HIT_SELECTION>
      uint8_t sum(DetectorRegion region, uint8_t layer) const {
         uint8_t total=0u;
         unsigned short key = makeKey(region, layer, 0);
         for (auto &[stat_key, stat_hits, stat_outlier_hits] : m_stat) {
            if ((stat_key & LAYER_REGION_MASK) == key) {
               if constexpr(HIT_SELECTION & HitSummaryData::Hit) {
                  total +=  stat_hits;
               }
               if constexpr(HIT_SELECTION & HitSummaryData::Outlier) {
                  total +=  stat_outlier_hits;
               }
            }
         }
         return total;
      }
 
   private:
      std::vector< std::tuple<unsigned short, uint8_t, uint8_t> > m_stat;
      std::array<uint8_t, Total+1>                                m_hits;
      std::array<uint8_t, Total+1>                                m_outlierHits;
      std::array<uint8_t, Total+1>                                m_layers;
      static constexpr std::array<uint8_t, unknown+1>             s_type
        { pixelTotal, pixelTotal, pixelTotal, stripTotal, stripTotal, unknownTotal};
   };
 
    /** Helper class to gather statistics and compute the biased variance.
     */
   class SumOfValues {
   private:
      double m_sum = 0.;
      double m_sum2 = 0.;
      unsigned int m_n =0u;
   public:
      void reset() {
         m_sum=0.;
         m_sum2=0.;
         m_n=0u;
      }
      void add(double value) {
         m_sum += value;
         m_sum2 += value * value;
         ++m_n;
      }
      std::array<double,2> meanAndBiasedVariance() const {
         double inv_n = m_n>0 ? 1/m_n : 0 ;
         double mean = m_sum * inv_n;
         return std::array<double, 2> { mean, (m_sum2 - m_sum * mean) * inv_n };
      }
      double biasedVariance() const {
         double inv_n = m_n>0 ? 1./m_n : 0 ;
         return (m_sum2 - m_sum * m_sum *inv_n) * inv_n;
      }
   };
 
   /** Helper to gather track summary information from the track states of the specified track
    * @param tracksContainer the Acts track container
    * @param track a track of the given acts track container for which the summary information is to be gathered
    * @param siDetEleColl array of SiDetectorElement collections per measurement type.
    * @param measurement_to_summary_type a LUT to map measurement types to the corresponding summary type for the measurement counts
    * @param chi2_stat_out output of the per track state chi-squared sums and squared sums to compute the per state chi2 variance.
    * @param hit_info_out output of the gathered measurement statistics per detector region, layer, ... .
    * @param param_state_idx_out output vector to be filled with the state index of all track states which are not holes.
    * @param special_hit_counts_out arrays to count holes (and @TODO dead sensors) per measurement type.
    */
   void gatherTrackSummaryData(const ActsTrk::TrackContainer &tracksContainer,
                               const typename ActsTrk::TrackContainer::ConstTrackProxy &track,
                               const std::array<const InDetDD::SiDetectorElementCollection *,
                                                to_underlying(xAOD::UncalibMeasType::nTypes)> &siDetEleColl,
                               const std::array<unsigned short,to_underlying(xAOD::UncalibMeasType::nTypes)>
                                        &measurement_to_summary_type,
                               SumOfValues &chi2_stat_out,
                               HitSummaryData &hit_info_out,
                               std::vector<ActsTrk::TrackStateBackend::ConstTrackStateProxy::IndexType > &param_state_idx_out,
                               std::array<std::array<uint8_t,to_underlying(HitCategory::N)>,
                                          to_underlying(xAOD::UncalibMeasType::nTypes)> &special_hit_counts_out);
 
}
#endif
