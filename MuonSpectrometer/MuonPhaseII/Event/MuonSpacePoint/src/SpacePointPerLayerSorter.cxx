
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonSpacePoint/SpacePointPerLayerSorter.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

namespace MuonR4 {
    using HitVec = SpacePointPerLayerSorter::HitVec;
    inline HitVec stripSmartPtr(const SpacePointBucket& bucket) {
        HitVec hits{};
        hits.reserve(bucket.size());
        std::transform(bucket.begin(),bucket.end(),std::back_inserter(hits), 
                      [](const SpacePointBucket::value_type& hit){return hit.get();});
        return hits;
    }
    SpacePointPerLayerSorter::SpacePointPerLayerSorter(const SpacePointBucket& bucket):
        SpacePointPerLayerSorter(stripSmartPtr(bucket)){}

    SpacePointPerLayerSorter::SpacePointPerLayerSorter(HitVec hits) {
        if (hits.empty()) return;

        /// Sort space points by z
        std::ranges::stable_sort(hits, [](const SpacePoint* a, const SpacePoint*b){
                        const double dZ = a->positionInChamber().z() - b->positionInChamber().z(); 
                        if (std::abs(dZ) > 50.*Gaudi::Units::micrometer) {
                            return dZ < 0.;
                        }
                        return a->positionInChamber().y() < b->positionInChamber().y();
        });
        const Muon::IMuonIdHelperSvc* idHelperSvc{hits.front()->msSector()->idHelperSvc()};
        m_mdtLayers.reserve(8);

        /// The hits are radially sorted from low local-z to high local z. Build the gasGap Identifier
        /// to find out to which layer the hit belongs to and then use the layer counting map as auxillary object
        /// fetch the indices for the sorted measurements
        using LayerCounting = std::unordered_map<Identifier, unsigned int>;
        LayerCounting mdtLayerCounting{}, stripLayerCounting{};
        
        auto push_StripHit = [this, &stripLayerCounting](const Identifier& layId, const SpacePoint* hit) {
            const unsigned int layer = stripLayerCounting.insert(std::make_pair(layId,stripLayerCounting.size())).first->second;

            if (layer >= m_stripLayers.size()) {
                    m_stripLayers.resize(layer + 1);
                }
                HitVec& pushTo{m_stripLayers[layer]};
                if (pushTo.capacity() == pushTo.size()){
                    pushTo.reserve(pushTo.size() + 5);
                }
                pushTo.push_back(hit);
                ++m_nStripHits;
        };
        
        for (const SpacePoint* hit : hits) {
            const Identifier& id {hit->identify()};
            switch (hit->type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType: {
                    const MdtIdHelper& idHelper{idHelperSvc->mdtIdHelper()};
                    const Identifier layId = idHelper.channelID(idHelper.stationName(id), 1, idHelper.stationPhi(id), 
                                                                idHelper.multilayer(id), idHelper.tubeLayer(id), 1);

                    const unsigned int layer = mdtLayerCounting.insert(std::make_pair(layId, mdtLayerCounting.size())).first->second;
                    if (layer >= m_mdtLayers.size()) {
                        m_mdtLayers.resize(layer + 1);
                    }
                    HitVec& pushTo{m_mdtLayers[layer]};
                    if (pushTo.capacity() == pushTo.size()){
                        pushTo.reserve(pushTo.size() + 5);
                    }
                    pushTo.push_back(hit);
                    ++m_nMdtHits;
                    if (!m_tubeLaySwitch && layer &&  
                        m_mdtLayers[layer -1].front()->primaryMeasurement()->identifierHash() != 
                        hit->primaryMeasurement()->identifierHash()) {
                        m_tubeLaySwitch = layer;
                    }
                    break;
                } case xAOD::UncalibMeasType::RpcStripType: {
                    const RpcIdHelper& idHelper{idHelperSvc->rpcIdHelper()};
                    /** Separate gasGaps by doubletR but not by doubletZ or doubletPhi */
                    const Identifier layId = idHelper.panelID(idHelper.stationName(id), 1, idHelper.stationPhi(id), 
                                                              idHelper.doubletR(id), 1, 1, idHelper.gasGap(id), false);
                    push_StripHit(layId, hit);
                    break;
                } case xAOD::UncalibMeasType::TgcStripType: {
                    const TgcIdHelper& idHelper{idHelperSvc->tgcIdHelper()};
                    const Identifier layId = idHelper.channelID(idHelper.stationName(id), 1, 1, idHelper.gasGap(id), false, 1);
                    push_StripHit(layId, hit);
                    break;
                } case xAOD::UncalibMeasType::sTgcStripType: {
                    const sTgcIdHelper& idHelper{idHelperSvc->stgcIdHelper()};
                    const Identifier layId = idHelper.channelID(idHelper.stationName(id), 1, 1, 
                                                                idHelper.multilayer(id), idHelper.gasGap(id), 
                                                                sTgcIdHelper::sTgcChannelTypes::Strip, 1);
                    push_StripHit(layId, hit);
                    break;
                } case xAOD::UncalibMeasType::MMClusterType: {
                    const MmIdHelper& idHelper{idHelperSvc->mmIdHelper()};
                    const Identifier layId = idHelper.channelID(idHelper.stationName(id), 1, 1, 
                                                                idHelper.multilayer(id), idHelper.gasGap(id), 500);
                    push_StripHit(layId, hit);
                    break;
                } default: {
                    break;
                }
            }
        }
    }
}