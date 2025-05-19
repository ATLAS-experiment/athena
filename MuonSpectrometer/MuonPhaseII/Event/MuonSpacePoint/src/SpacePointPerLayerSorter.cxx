/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonStationIndex/MuonStationIndex.h"

namespace MuonR4 {

    SpacePointPerLayerSorter::SpacePointPerLayerSorter(const Muon::IMuonIdHelperSvc* idHelperSvc): 
        m_idHelperSvc{idHelperSvc} {}

    Identifier SpacePointPerLayerSorter::detectorLayerId(const Identifier& id) const {

        Muon::MuonStationIndex::TechnologyIndex techIdx{m_idHelperSvc->technologyIndex(id)};

        switch (techIdx){
            using enum Muon::MuonStationIndex::TechnologyIndex;
            case MDT:{
                const MdtIdHelper& idHelper {m_idHelperSvc->mdtIdHelper()};

                Identifier detLayId {idHelper.channelID(idHelper.stationName(id), 1,
                                                        idHelper.stationPhi(id), 
                                                        idHelper.multilayer(id),
                                                        idHelper.tubeLayer(id), 1)};
                return detLayId;
            }
            case RPC:{
                const RpcIdHelper& idHelper {m_idHelperSvc->rpcIdHelper()};

                Identifier detLayId {idHelper.channelID(idHelper.stationName(id), 1, 
                                                        idHelper.stationPhi(id),
                                                        idHelper.doubletR(id), 1, 1, 
                                                        idHelper.gasGap(id), 0, 1)};
                return detLayId;
            }
            case TGC:{
                const TgcIdHelper& idHelper {m_idHelperSvc->tgcIdHelper()};

                Identifier detLayId {idHelper.channelID(idHelper.stationName(id), 1,
                                                        idHelper.stationPhi(id), 
                                                        idHelper.gasGap(id), 0, 1)};
                return detLayId;
            }
            case STGC:{
                const sTgcIdHelper& idHelper {m_idHelperSvc->stgcIdHelper()};

                Identifier detLayId {idHelper.channelID(idHelper.stationName(id), 1,
                                                        idHelper.stationPhi(id), 
                                                        idHelper.multilayer(id),
                                                        idHelper.gasGap(id), 
                                                        idHelper.channelType(id), 1)};
                return detLayId;
            }
            case MM:{
                const MmIdHelper& idHelper {m_idHelperSvc->mmIdHelper()};

                Identifier detLayId {idHelper.channelID(idHelper.stationName(id), 1,
                                                        idHelper.stationPhi(id), 
                                                        idHelper.multilayer(id),
                                                        idHelper.gasGap(id), 1)};
                return detLayId;
            }
            default:
                return id;
        }
    }

    bool SpacePointPerLayerSorter::operator()(const SpacePoint& sp1, const SpacePoint& sp2) const{

        const Identifier& id1 = sp1.identify();
        const Identifier& id2 = sp2.identify();

        const Identifier lay1 {detectorLayerId(id1)};
        const Identifier lay2 {detectorLayerId(id2)};

        if (lay1 == lay2) {
            const double dy = sp1.positionInChamber().y() - sp2.positionInChamber().y();
            if ( std::abs(dy) > 20 * Gaudi::Units::micrometer ){ 
                return dy < 0;
            }
            return sp1.positionInChamber().x() < sp2.positionInChamber().x();
        }
        return sp1.positionInChamber().z() < sp2.positionInChamber().z();
    }

    bool SpacePointPerLayerSorter::operator()(const std::shared_ptr<SpacePoint>& sp1, const std::shared_ptr<SpacePoint>& sp2) const {
        return (*this)(*sp1, *sp2);
    }

    bool SpacePointPerLayerSorter::operator()(const std::unique_ptr<SpacePoint>& sp1, const std::unique_ptr<SpacePoint>& sp2) const {
        return (*this)(*sp1, *sp2);
    }
    bool SpacePointPerLayerSorter::operator()(const SpacePoint* sp1, const SpacePoint* sp2) const {
        return (*this)(*sp1, *sp2);
    }

}