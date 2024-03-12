/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSPACEPOINT_MUONSPACEPOINTCONTAINER_H
#define MUONSPACEPOINT_MUONSPACEPOINTCONTAINER_H

#include "MuonSpacePoint/MuonSpacePoint.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "AthContainers/DataVector.h"
#include <vector>
namespace MuonR4{
    /**  @brief: The muon space point bucket represents a collection of points that 
     *           will bre processed together in the pattern seeding. Buckets represent a 
     *           a collection of hits that are close in a layer & sector of the muon spectrometer
     *           The area covered may varied across the different regions of the spectrometers & may
     *           also partially overlap with other buckets close-by.
     * 
    */
    class MuonSpacePointBucket : 
        public std::vector<std::shared_ptr<MuonSpacePoint>> {
         public:
            /** @brief Standard constructor*/
            MuonSpacePointBucket() = default;
            /** @brief set the range in the precision plane covered by the bucket*/
            void setCoveredRange(double min, double max){
                m_min = min;
                m_max = max;
            }
            /** @brief lower interval value covered by the bucket */
            double coveredMin() const { return m_min; }
            /** @brief upper interval value covered by the bucket */
            double coveredMax() const { return m_max; }
            /** @brief returns th associated muonChamber */
            const MuonGMR4::MuonChamber* muonChamber() const {
                return empty() ? nullptr : front()->muonChamber();
            }
            /** @brief sets the Identifier of the MuonSpacePointBucket in context
             *         of the associated muonChamber
            */
            void setBucketId(unsigned int id) {
                m_bucketId = id;
            }
            /** @brief  Returns the Identifier in the context of the MuonChamber*/
            unsigned int bucketId() const { return m_bucketId; }
            bool operator<(const MuonSpacePointBucket& other) const {
                using ChamberSorter = MuonGMR4::MuonDetectorManager::ChamberSorter;
                static const ChamberSorter sorter{};
                int chambCompare = -sorter(muonChamber(), other.muonChamber()) + 
                                    sorter(other.muonChamber(), muonChamber());
                if (chambCompare) return chambCompare < 0;
                return bucketId() < other.bucketId();
            }
        private:
            unsigned int m_bucketId{0};
            double m_min{-20. *Gaudi::Units::m};
            double m_max{20. * Gaudi::Units::m};
    };

    using MuonSpacePointContainer = DataVector<MuonSpacePointBucket>;
}
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( MuonR4::MuonSpacePointContainer , 1177013528 , 1 );

#endif