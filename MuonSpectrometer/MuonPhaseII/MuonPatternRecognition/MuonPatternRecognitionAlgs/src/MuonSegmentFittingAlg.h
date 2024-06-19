/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__SEGMENTFITTINGALG__H
#define MUONR4__SEGMENTFITTINGALG__H

#include "MuonPatternEvent/StationHoughMaxContainer.h"
#include "MuonPatternEvent/MuonSegment.h"
#include "MuonPatternEvent/MuonSegmentFitterEventData.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "Gaudi/Property.h"

// muon includes


namespace MuonR4{
    
    /// @brief Algorithm to handle segment fits  
    /// 
    /// This is currently a placeholder to test ideas! 
    class MuonSegmentFittingAlg: public AthReentrantAlgorithm{
        public:
                MuonSegmentFittingAlg(const std::string& name, ISvcLocator* pSvcLocator);
                virtual ~MuonSegmentFittingAlg() = default;
                virtual StatusCode initialize() override;
                virtual StatusCode execute(const EventContext& ctx) const override;

        private:
            
            /// Helper method to fetch data from StoreGate. If the key is empty, a nullptr is assigned to the container ptr
            /// Failure is returned in cases, of non-empty keys and failed retrieval
            template <class ContainerType> StatusCode retrieveContainer(const EventContext& ctx,
                                                                        const SG::ReadHandleKey<ContainerType>& key,
                                                                        const ContainerType* & contToPush) const;
            StatusCode prepareEventData(const EventContext & ctx, MuonSegmentFitterEventData & data) const; 
            
            StatusCode prepareSegmentFit(const HoughSegmentSeed & seed,  MuonSegmentFitterEventData & data) const; 
            StatusCode fitSegment(MuonSegmentFitterEventData & data) const; 

            MuonR4::MuonSegment buildSegment(MuonSegmentFitterEventData & data) const; 

            // read handle key for the input maxima (from a previous eta-transform)
            SG::ReadHandleKey<MuonR4::StationHoughSegmentSeedContainer> m_inHoughSegmentSeedKey{this, "StationHoughSegmentSeedContainer", "MuonHoughStationSegmentSeeds"};
            // write handle key for the output segment seeds 
            SG::WriteHandleKey<MuonSegmentContainer> m_outSegments{this, "MuonSegmentContainer", "R4MuonSegments"};

            // access to the ACTS geometry context 
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            // data members
    };
}


#endif
