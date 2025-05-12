/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINT_MUONSPACEPOINTCONTAINER_H
#define MUONSPACEPOINT_MUONSPACEPOINTCONTAINER_H

#include "MuonSpacePoint/SpacePoint.h"
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
    class SpacePointBucket : 
        public std::vector<std::shared_ptr<SpacePoint>> {
         public:
            using chamberLocation = MuonGMR4::SpectrometerSector::chamberLocation;
            /** @brief Standard constructor*/
            using std::vector<std::shared_ptr<SpacePoint>>::vector;
            /** @brief set the range in the precision plane covered by the bucket*/
            void setCoveredRange(double min, double max);
            /** @brief lower interval value covered by the bucket */
            double coveredMin() const { return m_min; }
            /** @brief upper interval value covered by the bucket */
            double coveredMax() const { return m_max; }
            /** @brief returns th associated muonChamber */
            const MuonGMR4::SpectrometerSector* msSector() const {
                return empty() ? nullptr : front()->msSector();
            }
            /** @brief sets the Identifier of the MuonSpacePointBucket in context
             *         of the associated muonChamber */
            void setBucketId(unsigned int id);
            /** @brief  Returns the Identifier in the context of the MuonChamber*/
            unsigned int bucketId() const { return m_bucketId; }
            bool operator<(const SpacePointBucket& other) const;
            /// populate the chamber location list. 
            /// This should be done once all the hits have been added. 
            void populateChamberLocations();
            /// returns the list of all tracking chambers in the bucket for fast navigation
            const std::vector<chamberLocation> & chamberLocations() const{
                return m_chamberLocs; 
            }
        private:
            unsigned int m_bucketId{0};
            double m_min{-20. *Gaudi::Units::m};
            double m_max{20. * Gaudi::Units::m};
            std::vector<chamberLocation> m_chamberLocs{}; 
    };
    /** @brief Abrivation of the space point container type */
    using SpacePointContainer = DataVector<SpacePointBucket>;
}
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( MuonR4::SpacePointContainer , 1138818003 , 1 );

#endif
