/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTestR4/SpacePointTesterModule.h"
#include "StoreGate/ReadHandle.h"

using namespace Muon::MuonStationIndex;
namespace MuonValR4{
    SpacePointTesterModule::SpacePointTesterModule(MuonTesterTree& tree,
                                   const std::string& inContainer,
                                   MSG::Level msgLvl,
                                   const std::string& collName) :
        TesterModuleBase{tree, inContainer + collName, msgLvl},
        m_collName{collName},
        m_key{inContainer}{}

    unsigned int SpacePointTesterModule::push_back(const MuonR4::SpacePoint& spacePoint) {
        if (!m_internalFill) {
            m_applyFilter = true;
        }
        auto insert_itr = m_spacePointIdx.insert(std::make_pair(&spacePoint, m_spacePointIdx.size()));
        // Space point added before return the index of the space point
        if (!insert_itr.second) {
            return insert_itr.first->second;
        }
        
        m_spPos.push_back(spacePoint.positionInChamber());
        m_driftR.push_back(spacePoint.driftRadius());


        const AmgSymMatrix(2)& cov{spacePoint.covariance()};
        m_covXX.push_back(cov(Amg::x, Amg::x));
        m_covXY.push_back(cov(Amg::x, Amg::y));
        m_covYX.push_back(cov(Amg::y, Amg::x));
        m_covYY.push_back(cov(Amg::y, Amg::y));

        m_measEta.push_back(spacePoint.measuresEta());
        m_measPhi.push_back(spacePoint.measuresPhi());
        m_nEtaInstances.push_back(spacePoint.measuresEta() ? spacePoint.nEtaInstanceCounts() : 0);
        m_nPhiInstances.push_back(spacePoint.measuresPhi() ? spacePoint.nPhiInstanceCounts() : 0);
        

        using TechIndex = Muon::MuonStationIndex::TechnologyIndex; 
        const Identifier id = spacePoint.identify();
        const TechIndex techIdx = idHelperSvc()->technologyIndex(id);
        m_techIdx.push_back(toInt(techIdx));
        m_spacePointId.push_back(id);
        int phiChannel{-1};
        switch (techIdx) {
            case TechIndex::MDT: {
                    const MdtIdHelper& idHelper{idHelperSvc()->mdtIdHelper()}; 
                    m_layer.push_back((idHelper.multilayer(id) -1)*idHelper.tubeLayerMax(id) + 
                                        idHelper.tubeLayer(id));
                    m_channel.push_back(idHelper.tube(id));
                }
                break;
            case TechIndex::RPC: {
                    const RpcIdHelper& idHelper{idHelperSvc()->rpcIdHelper()};
                    m_layer.push_back( (idHelper.doubletR(id) -1) * idHelper.gasGapMax(id) + 
                                        idHelper.gasGap(id));
                    m_channel.push_back(idHelper.channel(id));
                    if (spacePoint.secondaryMeasurement()) {
                        phiChannel = idHelper.channel(xAOD::identify(spacePoint.secondaryMeasurement()));
                    }
                }
                break;
            case TechIndex::TGC: {
                    const TgcIdHelper& idHelper{idHelperSvc()->tgcIdHelper()};
                    m_layer.push_back(idHelper.gasGap(id));
                    m_channel.push_back(idHelper.channel(id));
                    if (spacePoint.secondaryMeasurement()) {
                        phiChannel = idHelper.channel(xAOD::identify(spacePoint.secondaryMeasurement()));
                    }
                }
                break;
            case TechIndex::STGC: {
                    const sTgcIdHelper& idHelper{idHelperSvc()->stgcIdHelper()};
                    m_layer.push_back( (idHelper.multilayer(id) -1) * 4 + idHelper.gasGap(id));
                    m_channel.push_back(idHelper.channel(id));
                    if (spacePoint.secondaryMeasurement()) {
                        phiChannel = idHelper.channel(xAOD::identify(spacePoint.secondaryMeasurement()));
                    }
                }
                break;
            case TechIndex::MM: {
                    const MmIdHelper& idHelper{idHelperSvc()->mmIdHelper()};
                    m_layer.push_back( (idHelper.multilayer(id) -1) * 4 + idHelper.gasGap(id));
                    m_channel.push_back(idHelper.channel(id)); 
                }
                break;
            default:
                ATH_MSG_WARNING("Dude you can't have CSCs in R4 "<<idHelperSvc()->toString(id));
        };
        m_phiChannel.push_back(phiChannel);
        return insert_itr.first->second;
    }
    unsigned int SpacePointTesterModule::push_back(const MuonR4::SpacePointBucket& bucket) {
        if (!m_internalFill) {
            m_applyFilter = true;
        } else if (m_applyFilter) {
            auto find_itr = m_bucketIdx.find(&bucket);
            return find_itr != m_bucketIdx.end() ? find_itr->second : m_bucketIdx.size();
        }
        auto insert_itr = m_bucketIdx.insert(std::make_pair(&bucket, m_bucketIdx.size()));
        // Bucket has been added before. Bail out
        if (!insert_itr.second) {
            return insert_itr.first->second;
        }
        

        m_bucketNumber.push_back(bucket.bucketId());
        m_bucketId.push_back(bucket.msSector()->chambers().front()->readoutEles().front()->identify());
        m_bucketMin.push_back(bucket.coveredMin());
        m_bucketMax.push_back(bucket.coveredMax());
        std::vector<uint16_t>& spacePoints = m_bucketPoints[m_bucketPoints.size()];
        for (const auto& spacePoint : bucket) {
            spacePoints.push_back(push_back(*spacePoint));
        }        

        return insert_itr.first->second;        
    }
    bool SpacePointTesterModule::declare_keys() {
        return declare_dependency(m_key);
    }
    bool SpacePointTesterModule::fill(const EventContext& ctx) {
        m_internalFill = true;
        const MuonR4::SpacePointContainer* container{nullptr};
        if (!SG::get(container, m_key, ctx).isSuccess()) {
            return false;
        }
        for (const MuonR4::SpacePointBucket* bucket : *container) {
            push_back(*bucket);
        }
        m_internalFill = false;
        m_spacePointIdx.clear();
        m_bucketIdx.clear();
        return true;
    }
}
