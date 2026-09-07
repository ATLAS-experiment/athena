/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MOOSEGMENTFINDERS_MUONSEGMENTMERGINGALG_H
#define MOOSEGMENTFINDERS_MUONSEGMENTMERGINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrkSegment/SegmentCollection.h"

class MuonSegmentMergingAlg: public AthReentrantAlgorithm {
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;
        virtual StatusCode initialize() override final;
        virtual StatusCode execute(const EventContext& ctx) const final;
    private:
        SG::ReadHandleKeyArray<Trk::SegmentCollection> m_inKeys{this, "ReadKeys", {}} ;

        SG::WriteHandleKey<Trk::SegmentCollection> m_writeKey{this, "WriteKey", "TrackMuonSegments"};

        Gaudi::Property<bool> m_hardCopy{this, "hardCopy", false};
};

#endif
