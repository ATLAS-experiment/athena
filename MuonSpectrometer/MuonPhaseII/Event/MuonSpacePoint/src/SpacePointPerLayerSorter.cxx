/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"

namespace MuonR4 {

    unsigned int SpacePointPerLayerSorter::sectorLayerNum(const SpacePoint& sp) const {
        
        switch (sp.primaryMeasurement()->type()){
            case xAOD::UncalibMeasType::MdtDriftCircleType:{
                auto mdtMeas = static_cast<const xAOD::MdtDriftCircle*>(sp.primaryMeasurement());
                return sp.msSector()->logicalLayerIdx(mdtMeas->readoutElement()).at(mdtMeas->tubeLayer()-1);
            }
            case xAOD::UncalibMeasType::RpcStripType:{
                auto rpcMeas = static_cast<const xAOD::RpcMeasurement*>(sp.primaryMeasurement());
                return sp.msSector()->logicalLayerIdx(rpcMeas->readoutElement()).at(rpcMeas->gasGap()-1);
            }
            case xAOD::UncalibMeasType::TgcStripType:{
                auto tgcMeas = static_cast<const xAOD::TgcStrip*>(sp.primaryMeasurement());
                return sp.msSector()->logicalLayerIdx(tgcMeas->readoutElement()).at(tgcMeas->gasGap()-1);
            }
            case xAOD::UncalibMeasType::sTgcStripType:{
                auto stgcMeas = static_cast<const xAOD::sTgcMeasurement*>(sp.primaryMeasurement());
                return sp.msSector()->logicalLayerIdx(stgcMeas->readoutElement()).at(stgcMeas->gasGap()-1);
            }
            case xAOD::UncalibMeasType::MMClusterType:{
                auto mmMeas = static_cast<const xAOD::MMCluster*>(sp.primaryMeasurement());
                return sp.msSector()->logicalLayerIdx(mmMeas->readoutElement()).at(mmMeas->gasGap()-1);
            }
            default:
                THROW_EXCEPTION("Unexpected Measurement Type in sectorLayerNum()");
        }
    }
    
    bool SpacePointPerLayerSorter::operator()(const SpacePoint& sp1, const SpacePoint& sp2) const {

        const unsigned int lay1 {sectorLayerNum(sp1)};
        const unsigned int lay2 {sectorLayerNum(sp2)};

        if (lay1 == lay2) {
            const double dy = sp1.positionInChamber().y() - sp2.positionInChamber().y();
            if ( std::abs(dy) > 20 * Gaudi::Units::micrometer ){ 
                return dy < 0;
            }
            return sp1.positionInChamber().x() < sp2.positionInChamber().x();
        }
        return lay1 < lay2;
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