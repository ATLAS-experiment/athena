/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSegmentFilterAlg.h"
#include "TrkSurfaces/Surface.h"


StatusCode MuonSegmentFilterAlg::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_outKey.initialize());
    ATH_CHECK(m_inKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode MuonSegmentFilterAlg::execute(const EventContext& ctx) const {
    
    const Trk::SegmentCollection* segment_container{nullptr};
    ATH_CHECK(SG::get(segment_container, m_inKey, ctx));    
    SG::WriteHandle<ConstDataVector<Trk::SegmentCollection>> out_handle{m_outKey, ctx};
    ATH_CHECK(out_handle.record(std::make_unique<ConstDataVector<Trk::SegmentCollection>>(SG::VIEW_ELEMENTS)));
    for (const Trk::Segment* seg : *segment_container){
        if (keep_segment(seg)) out_handle->push_back(seg);
    }
    if (m_trash_unfiltered && !segment_container->empty() && out_handle->size() == segment_container->size()){
        ATH_MSG_DEBUG("The input and output container are the same. Clear output container");
        out_handle->clear();
    }
    return StatusCode::SUCCESS;
}
bool MuonSegmentFilterAlg::keep_segment(const Trk::Segment* segment) const {
    for (const Trk::MeasurementBase* meas : segment->containedMeasurements()){
         /// First find the identifier
         const Identifier id = meas->associatedSurface().associatedDetectorElementIdentifier();        
         if (m_thin_stations.value().count(toInt(m_idHelperSvc->stationIndex(id)))){
            return false;
        }
        if (m_thin_layers.value().count(toInt(m_idHelperSvc->layerIndex(id)))){
            return false;
        }
        if (m_thin_technology.value().count(toInt(m_idHelperSvc->technologyIndex(id)))) {
            return false;
        }
        if (m_thin_region.value().count(toInt(m_idHelperSvc->regionIndex(id)))){
            return false;
        }
        if (m_thin_chamber_idx.value().count(toInt(m_idHelperSvc->chamberIndex(id)))) {
           return false;
        }        
    }
    return true;    
}