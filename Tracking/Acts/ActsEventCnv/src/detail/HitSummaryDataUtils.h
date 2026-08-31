/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRK_HITSUMMARYDATAUTILS_H
#define ACTSTRK_HITSUMMARYDATAUTILS_H

#include "ActsEvent/TrackContainer.h"
#include "ReadoutGeometryBase/SolidStateDetectorElementBase.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "HGTD_Identifier/HGTD_ID.h"

#include <vector>
#include <cmath>
#include <array>
#include <tuple>
#include <type_traits>

namespace ActsTrk::detail {
   enum class HitCategory: std::uint8_t {
         DeadSensor,
         Hole,
         N
   };
 
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
         pixelBarrel         = 6,
         pixelTotal          = 7,
         stripTotal          = 8,
         hgtdTotal           = 9,
         unknownTotal        = 10,
         Total               = 11
      };

      enum class CountType{
         Hit,
         Outlier,
         SharedHit,
         SplitHit,
         NCountTypes
      };

      constexpr static unsigned short LAYER_REGION_MASK   = 0xFF;  // bits 0-7
      constexpr static unsigned short REGION_BITS = 4;             // bits 0-3
      constexpr static unsigned short REGION_MASK = 0xF;           // 4 bits
      constexpr static unsigned short LAYER_BITS  = 4;             // bits 4-7
      constexpr static unsigned short LAYER_MASK  = 0xF;           // 4 bits
      constexpr static unsigned short SIGNED_ETA_MOD_BITS  = 7;    // bits 8-13 + 14(sign)
      constexpr static unsigned short SIGNED_ETA_MOD_MASK = 0x7F;  // 6 + 1(sign) bit
 
      /** @brief Compute a counter key for the given region, layer and module eta module index
       * @param region the detector region index (0..9).
       * @param layer the layer index (0..15).
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
      constexpr static unsigned short makeKey(unsigned short region) {
         assert(region < (1<<REGION_BITS) );
         return static_cast<uint8_t>(region);
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
 
      // To select, hits, outliers, and/or shared hits.
      enum EHitSelection {
         HitFlag = 1,
         OutlierFlag = 2,
         HitAndOutlierFlag = 3,
         SharedHitFlag = 4,
         SplitHitFlag = 8
      };
 
      /** @brief reset all summary counters to zero.
       */
      void reset() {
         m_stat.clear();
         for(unsigned int i=0;i<static_cast<unsigned int>(CountType::NCountTypes); ++i) {
            std::fill(m_hits[i].begin(),m_hits[i].end(), 0u);
         }
         std::fill(m_layers.begin(),m_layers.end(), 0u);
      }
 
      /** @brief update summaries to take the given hit into account.
       * @param det_type.
       * @param detector_elements detector element collection relevant for the given hit.
       * @param hit_selection should be set to bit mask for Hit, Outlier, SharedHit.
       * @return returns false in case the hit was not considered.
       * The hit is not considered if the given id_hash is not valid for the given detector element collectio.
       */
      bool addHit(xAOD::UncalibMeasType det_type,
                  const InDetDD::SolidStateDetectorElementBase *detEl,
                  EHitSelection hit_selection) {
         DetectorRegion region = unknown;
         uint8_t layer  = 255;
         int eta_module = 0;
         Identifier id(detEl ? detEl->identify() : Identifier() );
         switch (det_type) {
         case xAOD::UncalibMeasType::PixelClusterType: {
            if (!detEl) { return false; }
            assert(detEl ->getIdHelper()->is_pixel(id));
            InDetDD::DetectorType type = detEl->design().type();
            if(type==InDetDD::PixelInclined)  region = pixelBarrelInclined;
            else if(type==InDetDD::PixelBarrel) region = pixelBarrelFlat;
            else region = pixelEndcap;
 
            const PixelID* pixel_id = static_cast<const PixelID *>(detEl->getIdHelper());
            layer = pixel_id->layer_disk(detEl->identify());
            eta_module = pixel_id->eta_module(detEl->identify());
            break;
         }
         case xAOD::UncalibMeasType::StripClusterType: {
            if (!detEl) { return false; }
            assert(detEl ->getIdHelper()->is_sct(id));
            const SCT_ID* sct_id = static_cast<const SCT_ID *>(detEl->getIdHelper());
            region =  (sct_id->barrel_ec(id)==0 ?  stripBarrel : stripEndcap);

            layer = sct_id->layer_disk(detEl->identify());
            eta_module = sct_id->eta_module(detEl->identify());
            break;
         }
         case xAOD::UncalibMeasType::HGTDClusterType: {
            if (!detEl) { return false; }
            assert(detEl ->getIdHelper()->is_hgtd(id));
            const HGTD_ID* hgtd_id = static_cast<const HGTD_ID *>(detEl->getIdHelper());
            region=hgtdTotal;
            layer=hgtd_id->layer(id);
            break;
         }
         default: {
            return false;
         }
         }
 
         unsigned short key = makeKey(region, layer, eta_module);
         for (auto &[stat_key, stat_hits, stat_outlier_hits, stat_shared_hits, stat_split_hits] : m_stat) {
            if (stat_key == key) {
               stat_hits         += ((hit_selection & HitSummaryData::HitFlag)!=0);
               stat_outlier_hits += ((hit_selection & HitSummaryData::OutlierFlag)!=0);
               stat_shared_hits  += ((hit_selection & HitSummaryData::SharedHitFlag)!=0);
               stat_split_hits   += ((hit_selection & HitSummaryData::SplitHitFlag)!=0);
               return true;
            }
         }
         m_stat.emplace_back( std::make_tuple(key,
                                              ((hit_selection & HitSummaryData::HitFlag)!=0),
                                              ((hit_selection & HitSummaryData::OutlierFlag)!=0),
                                              ((hit_selection & HitSummaryData::SharedHitFlag)!=0),
                                              ((hit_selection & HitSummaryData::SplitHitFlag)!=0) ));
         return true;
      }
 
      /** @brief Compute the varius summaries.
       * Must be called only after all hits have been gathered, and must not be called more than once.
       */
      void computeSummaries() {
         for (const auto &[stat_key, stat_hits, stat_outlier_hits, stat_shared_hits, stat_split_hits] : m_stat) {
            unsigned short region=regionFromKey(stat_key);
            m_hits[static_cast<unsigned int>(CountType::Hit)].at(region) += stat_hits;
            m_hits[static_cast<unsigned int>(CountType::Outlier)].at(region) += stat_outlier_hits;
            m_hits[static_cast<unsigned int>(CountType::SharedHit)].at(region) += stat_shared_hits;
            m_hits[static_cast<unsigned int>(CountType::SplitHit)].at(region) += stat_split_hits;
            ++m_layers.at(region);
         }
         for (unsigned int region_i=0; region_i<unknown+1; ++region_i) {
            for (unsigned int count_type_i=0; count_type_i<static_cast<unsigned int>(CountType::NCountTypes);++count_type_i) {
               m_hits[count_type_i].at(s_type.at(region_i)) += m_hits[count_type_i][region_i];
               m_hits[count_type_i].at(Total) += m_hits[count_type_i][region_i];
            }
            m_layers.at(s_type.at(region_i)) += m_layers[region_i];
            m_layers.at(Total) += m_layers[region_i];
         }
         m_layers.at(pixelBarrel) += m_layers.at(pixelBarrelFlat) + m_layers.at(pixelBarrelInclined);
         for (unsigned int count_type_i=0; count_type_i<static_cast<unsigned int>(CountType::NCountTypes);++count_type_i) {
            assert( pixelBarrelFlat < m_hits[count_type_i].size() && pixelBarrelInclined < m_hits[count_type_i].size());
            m_hits[count_type_i].at(pixelBarrel) = m_hits[count_type_i][pixelBarrelFlat]+m_hits[count_type_i][pixelBarrelInclined];
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
      uint8_t contributingHits(DetectorRegion region, CountType hit_type=CountType::Hit) const {
         assert( static_cast<unsigned int>(hit_type) < static_cast<unsigned int>(CountType::NCountTypes));
         return m_hits[static_cast<unsigned int>(hit_type)].at(region);
      }
 
      /** @brief return the number of outliers in a certain detector region.
       * @param region the detector region.
       * Only meaningful after @ref computeSummaries was called.
       */
      uint8_t contributingOutlierHits(DetectorRegion region) const {
         return contributingHits(region, CountType::Outlier);
      }
 
      /** @brief return the number of shared hits in a certain detector region.
       * @param region the detector region.
       * Only meaningful after @ref computeSummaries was called.
       */
      uint8_t contributingSharedHits(DetectorRegion region) const {
         return contributingHits(region, CountType::SharedHit);
      }

 
      /** @brief return the total number of hits, outliers, shared hits and/or split hits in the given detector region and layer.
       * @param region the detector region.
       * @param layer the detector layer.
       */
      template <unsigned short HIT_SELECTION>
      uint8_t sum(DetectorRegion region, uint8_t layer) const {
         uint8_t total=0u;
         unsigned short key = makeKey(region, layer, 0);
         for (const auto &[stat_key, stat_hits, stat_outlier_hits, stat_shared_hits, stat_split_hits] : m_stat) {
            if ((stat_key & LAYER_REGION_MASK) == key) {
               if constexpr(HIT_SELECTION & HitSummaryData::HitFlag) {
                  total +=  stat_hits;
               }
               if constexpr(HIT_SELECTION & HitSummaryData::OutlierFlag) {
                  total +=  stat_outlier_hits;
               }
               if constexpr(HIT_SELECTION & HitSummaryData::SharedHitFlag) {
                  total +=  stat_shared_hits;
               }
               if constexpr(HIT_SELECTION & HitSummaryData::SplitHitFlag) {
                  total +=  stat_split_hits;
               }
            }
         }
         return total;
      }
      /** @brief return the total number of hits, outliers,  shared hits and split hits in the given detector region and layer.
       * @param region the detector region.
       * @param layer the detector layer.
       */
      std::array<uint8_t,4> sumPerCountType(DetectorRegion region, uint8_t layer) const {
         std::array<uint8_t,4> total{};
         unsigned short key = makeKey(region, layer, 0);
         for (const auto &[stat_key, stat_hits, stat_outlier_hits, stat_shared_hits, stat_split_hits] : m_stat) {
            if ((stat_key & LAYER_REGION_MASK) == key) {
               total[static_cast<unsigned int>(CountType::Hit)]       +=  stat_hits;
               total[static_cast<unsigned int>(CountType::Outlier)]   +=  stat_outlier_hits;
               total[static_cast<unsigned int>(CountType::SharedHit)] +=  stat_shared_hits;
               total[static_cast<unsigned int>(CountType::SplitHit)]  +=  stat_split_hits;
            }
         }
         return total;
      }

      /** Get a bit pattern with one bit set per layer which is set if the layer has a hit or optionally an outlier.
       * @param region selector
       * @param include_outlier if true also set the corresponding bit if the layer contains an outlier only.
       * @return a bit pattern, where each bit corresponds to one detector and the bit is set if the layer contains a hit (or outlier).
       */
      unsigned int layerPattern(DetectorRegion region, bool include_outlier) const {
         unsigned int layer_pattern=0u;
         unsigned short key = makeKey(region);
         std::array<uint8_t,2> stat_mask{ 0xff, static_cast<uint8_t>(include_outlier ? 0xff : 0) };
         for (const auto &[stat_key, stat_hits, stat_outlier_hits, stat_shared_hits, stat_split_hits] : m_stat) {
            if ((stat_key & REGION_MASK) == key) {
               if ((stat_hits & stat_mask[0])+(stat_outlier_hits & stat_mask[1]) >0u) {
                  layer_pattern |= 1u<<layerFromKey(stat_key);
               }
            }
         }
         return layer_pattern;
      }
 
   private:
      std::vector< std::tuple<unsigned short, uint8_t, uint8_t, uint8_t, uint8_t> > m_stat;
      std::array<std::array<uint8_t, Total+1>,static_cast<unsigned int>(CountType::NCountTypes)> m_hits{};
      std::array<uint8_t, Total+1>                                                               m_layers{};
      static constexpr std::array<uint8_t, unknown+1>                                            s_type
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

   struct TimeInfo {
      float mean;
      float resolution;
      double chi2;
   };
 
   /** Helper to gather track summary information from the track states of the specified track
    * @param track a track of the given acts track container for which the summary information is to be gathered
    * @param measurement_to_summary_type a LUT to map measurement types to the corresponding summary type for the measurement counts
    * @param chi2_stat_out output of the per track state chi-squared sums and squared sums to compute the per state chi2 variance.
    * @param hit_info_out output of the gathered measurement statistics per detector region, layer, ... .
    * @param param_state_idx_out output vector to be filled with the state index of all track states which are not holes.
    * @param special_hit_counts_out arrays to count holes (and @TODO dead sensors) per measurement type.
    * @param time_info will be filled with the mean time and the time resolution, provided at least one hit provides time information.
    */
   void gatherTrackSummaryData(const typename ActsTrk::TrackContainer::ConstTrackProxy &track,
                               const std::array<unsigned short,Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)>
                                        &measurement_to_summary_type,
                               SumOfValues &chi2_stat_out,
                               HitSummaryData &hit_info_out,
                               std::vector<ActsTrk::TrackStateBackend::ConstTrackStateProxy::IndexType > &param_state_idx_out,
                               std::array<std::array<uint8_t,Acts::toUnderlying(HitCategory::N)>,
                                          Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)> &special_hit_counts_out,
                               TimeInfo &time_info);
 
}
#endif
