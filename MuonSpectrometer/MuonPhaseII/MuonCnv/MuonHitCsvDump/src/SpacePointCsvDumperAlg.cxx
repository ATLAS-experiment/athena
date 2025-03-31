/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SpacePointCsvDumperAlg.h"

#include "StoreGate/ReadHandle.h"
#include "MuonSpacePoint/SpacePointPerLayerSplitter.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include <fstream>
#include <TString.h>

namespace {
    int encodeId(const int8_t stName, const int8_t stEta,
                 const int8_t sector, const int8_t tech) {
        return ( tech <<24 | sector <<16 |stEta << 8| stName);
    }
}


namespace MuonR4{

StatusCode SpacePointCsvDumperAlg::initialize() {
   ATH_CHECK(m_readKey.initialize());
   ATH_CHECK(m_idHelperSvc.retrieve());
   return StatusCode::SUCCESS;
 }

StatusCode SpacePointCsvDumperAlg::execute(){

   const EventContext& ctx{Gaudi::Hive::currentContext()};
 
   constexpr std::string_view delim = ",";
   std::ofstream file{std::string(Form("event%09zu-",++m_event))+"SpacePoints.csv"};
   
    /// Identifier to check whether the bucket is in the same sector
    file<<"sectorId"<<delim;
    // Bucket inside the sector layer    
    file<<"bucketId"<<delim;
    /// Local position of the hit
    file<<"locPositionX"<<delim;
    file<<"locPositionY"<<delim;
    file<<"locPositionZ"<<delim;
    /// Local sensor direction of the hit
    file<<"locSensorDirX"<<delim;
    file<<"locSensorDirY"<<delim;
    file<<"locSensorDirZ"<<delim;
    /// Normal vector on the sensor plane
    file<<"locPlaneNormX"<<delim;
    file<<"locPlaneNormY"<<delim;
    file<<"locPlaneNormZ"<<delim;
    /// Covariance entries of the uncalibrated space point
    file<<"covXX"<<delim;
    file<<"covXY"<<delim;
    file<<"covYX"<<delim;
    file<<"covYY"<<delim;
    /// Drift radius
    file<<"driftR"<<delim;
    /// Properties of the space point Identifier
    file<<"gasGap"<<delim;
    file<<"primaryCh"<<delim;
    file<<"measuresEta"<<delim;
    file<<"measuresPhi"<<delim;
    file<<std::endl;


   SG::ReadHandle readHandle{m_readKey, ctx};
   ATH_CHECK(readHandle.isPresent());

   auto dumpToFile = [&](const unsigned bucketId,
                         const SpacePoint& spacePoint,
                         const unsigned gasGap) {
        
        const Identifier measId = spacePoint.identify();
        int primaryCh{0};
        using TechIndex = Muon::MuonStationIndex::TechnologyIndex; 
        const TechIndex techIdx = m_idHelperSvc->technologyIndex(measId);
        switch (techIdx) {
            case TechIndex::MDT: {
                const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()}; 
                primaryCh = idHelper.tube(measId);
                break;
            }                
            case TechIndex::RPC: {
                const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
                primaryCh = idHelper.channel(measId);
                break;
            }
            case TechIndex::TGC: {
                const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
                primaryCh = idHelper.channel(measId);
                break;
            }
            case TechIndex::STGC: {
                const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
                primaryCh = idHelper.channel(measId);
                break;
            }
            case TechIndex::MM: {
                const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
                primaryCh = idHelper.channel(measId);
                break;
            }
            default:
                ATH_MSG_WARNING("Dude you can't have CSCs in R4 "<<m_idHelperSvc->toString(measId));
        };

        const int secId = encodeId(static_cast<int8_t>(spacePoint.msSector()->chamberIndex()),
                                   spacePoint.msSector()->side(),
                                   spacePoint.msSector()->sector(),
                                   static_cast<int8_t>(techIdx));

        file<<secId<<delim;
        file<<bucketId<<delim;
        file<<spacePoint.positionInChamber().x()<<delim;
        file<<spacePoint.positionInChamber().y()<<delim;
        file<<spacePoint.positionInChamber().z()<<delim;
        //
        file<<spacePoint.directionInChamber().x()<<delim;
        file<<spacePoint.directionInChamber().y()<<delim;
        file<<spacePoint.directionInChamber().z()<<delim;
        //
        file<<spacePoint.planeNormal().x()<<delim;
        file<<spacePoint.planeNormal().y()<<delim;
        file<<spacePoint.planeNormal().z()<<delim;
        //
        file<<spacePoint.covariance()(Amg::x, Amg::x)<<delim;
        file<<spacePoint.covariance()(Amg::x, Amg::y)<<delim;
        file<<spacePoint.covariance()(Amg::y, Amg::x)<<delim;
        file<<spacePoint.covariance()(Amg::y, Amg::y)<<delim;
        file<<spacePoint.driftRadius()<<delim;
        file<<gasGap<<delim;
        file<<primaryCh<<delim;
        file<<spacePoint.measuresEta()<<delim;
        file<<spacePoint.measuresPhi()<<delim;
        file<<std::endl;
   };

   for(const SpacePointBucket* bucket : *readHandle) {
       const SpacePointPerLayerSplitter splitter{*bucket};
       unsigned int gasGap{0};
       for (const SpacePointPerLayerSplitter::HitVec& mdtLayer : splitter.mdtHits()) {
            ++gasGap;
            for (const SpacePoint* spacePoint : mdtLayer) {
                dumpToFile(bucket->bucketId(), *spacePoint, gasGap);
            }
       }
       for (const SpacePointPerLayerSplitter::HitVec& mdtLayer : splitter.stripHits()) {
            ++gasGap;
            for (const SpacePoint* spacePoint : mdtLayer) {
                dumpToFile(bucket->bucketId(), *spacePoint, gasGap);
            }
        }
   }
   return StatusCode::SUCCESS;
}
}


