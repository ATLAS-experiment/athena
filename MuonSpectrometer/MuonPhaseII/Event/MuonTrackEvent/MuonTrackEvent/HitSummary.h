

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKEVENT_HITSUMMARY_H
#define MUONTRACKEVENT_HITSUMMARY_H


#include "MuonStationIndex/MuonStationIndex.h"
#include "Acts/Utilities/Helpers.hpp"

#include <ostream>

namespace MuonR4{
    /** @brief Summary struct to hold the hit counts on the track per MS layer. Summaries
     *         are categorized by MS layer (Inner / Middle / Outer / Extended),
     *         by whether the hit is a precision, trigger eta or trigger phi hit & 
     *         by their contribution to the track fit.
     *         Note: Barrel extended is counted under the Extended category */
    struct HitSummary{
            using value_type = std::uint8_t;
            /** @brief Abrivation of the layer index */
            using LayerIndex = Muon::MuonStationIndex::LayerIndex;
            /** @brief Default constructor */
            HitSummary() = default;
            /** @brief Category of the hit */
            enum class HitCategory: std::uint8_t{
                Precision=0,         /// Precision hits (Mdt, NSW) on track
                TriggerEta,          /// Trigger eta hits (Tgc, Rpc)
                TriggerPhi,          /// Trigger phi hits (Tgc, Rpc)
                nCategories
            };
            /** @brief  Contribution to the track fit */
            enum class Status {
                OnTrack = 0,       /// Added to the trajectory & contributing to the fit 
                Outlier,           /// Added to the trajectory but rejected
                Hole,              /// Expected hit but missing
                MaxValue                
            };
            /** @brief Converts the hit category to a string */
            static std::string toString(const HitCategory c);
            /** @brief Converts the status to a string */
            static std::string toString(const Status s);
            /** @brief Returns the value type for a defined hit category & layer
             *  @param cat: Hit category
             *  @param status: Contribution to the fit
             *  @param layer: Spectrometer layer
             *  @param isSmall: Small sectors */
            value_type value(const HitCategory cat,
                            const Status status,
                            const LayerIndex layer,
                            const bool isSmall) const;

            /** @brief Returns the value type for a defined hit category & layer
             *         to modify the summary value
             *  @param cat: Hit category
             *  @param status: Contribution to the fit
             *  @param layer: Spectrometer layer
             *  @param isSmall: Small sectors */
            value_type& value(const HitCategory cat,
                              const Status status,
                              const LayerIndex layer,
                              const bool isSmall);
            /** @brief Output string stream operator */
            friend std::ostream& operator<<(std::ostream& ostr, const HitSummary& sum){
                sum.print(ostr);
                return ostr;
            }
        private:
            /** @brief Translates the 4 classification indices to a unique consecutive number
             *         (used for storage access)
             *  @param cat: Hit category
             *  @param status: Contribution to the fit
             *  @param layer: Spectrometer layer
             *  @param isSmall: Small sectors */
            unsigned translate(const HitCategory cat,
                               const Status status,
                               LayerIndex layer,
                               const bool isSmall) const;
            /** @brief Print the summary as an ASCII table */
            void print(std::ostream& ostr) const;
            /** @brief Abrivation to store the hits per layer*/
            using Counter_t = std::array<value_type,  Acts::toUnderlying(HitCategory::nCategories) * 
                                                      (Acts::toUnderlying(LayerIndex::LayerIndexMax) -1)* 
                                                      Acts::toUnderlying(Status::MaxValue) * 2>;

            Counter_t m_counts{};
    };
}

#endif