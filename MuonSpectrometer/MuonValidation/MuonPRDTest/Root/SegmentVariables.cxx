
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPRDTest/SegmentVariables.h>
#include <StoreGate/ReadHandle.h>

namespace MuonPRDTest {
    SegmentVariables::SegmentVariables(MuonTesterTree& tree, 
                                       const std::string& containerKey,
                                       const std::string& outName,
                                       MSG::Level msglvl) :
        PrdTesterModule(tree, "Segments"+ containerKey+outName, msglvl), 
            m_key{containerKey},
            m_name{outName}{}

   bool SegmentVariables::fill(const EventContext& ctx) {
        SG::ReadHandle readHandle{m_key, ctx};
        if(!readHandle.isPresent()) {
            ATH_MSG_FATAL("Failed to retrieve "<<m_key.fullKey());
            return false;
        }
        if (!m_filterMode) {
            for (const xAOD::MuonSegment* segment : *readHandle){
                fill(*segment);
            }
        }
        m_idxLookUp.clear();
        return true;
   }
    bool SegmentVariables::declare_keys() {
        return declare_dependency(m_key);
    }
    unsigned int SegmentVariables::push_back(const xAOD::MuonSegment& segment) {
        m_filterMode = true;
        return fill(segment);
    }
    bool SegmentVariables::addVariable(std::shared_ptr<IAuxElementDecorationBranch> br) {
        m_addBranches.push_back(br);
        return parent().addBranch(br);
    }
    unsigned int SegmentVariables::fill(const xAOD::MuonSegment& segment){ 
       auto insert_itr = m_idxLookUp.insert(std::make_pair(&segment, m_idxLookUp.size()));
       if (!insert_itr.second) {
            return insert_itr.first->second;
       }
       m_pos.push_back(segment.position());
       m_dir.push_back(segment.direction());
       m_etaIdx += segment.etaIndex();
       m_sector += segment.sector();
       using namespace Muon::MuonStationIndex;
       m_chamberIdx += toInt(segment.chamberIndex());
       m_chi2 +=segment.chiSquared();
       m_nDoF +=segment.numberDoF();

        m_nPrecHits += segment.nPrecisionHits();
        m_nTrigEtaLayers += segment.nTrigEtaLayers();
        m_nTrigPhiLayers += segment.nPhiLayers();
        for(const auto& br : m_addBranches){
            br->push_back(segment);
        }
 
        return insert_itr.first->second;
    } 
}