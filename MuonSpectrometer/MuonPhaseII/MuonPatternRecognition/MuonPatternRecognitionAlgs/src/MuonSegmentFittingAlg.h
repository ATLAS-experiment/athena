/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNRECOGNTIONALGS_SEGMENTFITTINGALG__H
#define MUONR4_MUONPATTERNRECOGNTIONALGS_SEGMENTFITTINGALG__H

#include "MuonPatternEvent/MuonPatternContainer.h"
#include "MuonPatternEvent/MuonSegment.h"
#include "MuonPatternEvent/MuonSegmentFitterEventData.h"
#include "MuonPatternEvent/MuonHoughDefs.h"

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
            
            StatusCode prepareSegmentFit(const SegmentSeed & seed,  MuonSegmentFitterEventData & data) const; 
            StatusCode fitSegment(MuonSegmentFitterEventData & data) const; 

            MuonSegment buildSegment(MuonSegmentFitterEventData & data) const; 

            // read handle key for the input maxima (from a previous eta-transform)
            SG::ReadHandleKey<SegmentSeedContainer> m_inHoughSegmentSeedKey{this, "ReadKey", "MuonHoughStationSegmentSeeds"};
            // write handle key for the output segment seeds 
            SG::WriteHandleKey<MuonSegmentContainer> m_outSegments{this, "MuonSegmentContainer", "R4MuonSegments"};

            // access to the ACTS geometry context 
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            // data members
            /// Add beamline constraint
            Gaudi::Property<bool> m_doBeamspotConstraint{this, "doBeamspotConstraint", true};
    };
}


#endif
