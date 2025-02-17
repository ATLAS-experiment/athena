/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SpacePointCsvDumperAlg.h"

#include "StoreGate/ReadHandle.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include <fstream>
#include <TString.h>

namespace {
    union SectorId{
        int8_t fields[4];
        int hash;
    };
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
    file<<"localPositionX"<<delim;
    file<<"localPositionY"<<delim;
    file<<"localPositionZ"<<delim;
    /// Local sensor direction of the hit
    file<<"locSensorDirX"<<delim;
    file<<"locSensorDirY"<<delim;
    file<<"locSensorDirZ"<<delim;
    /// Normal vector on the sensor plane
    file<<"locPlaneNormX"<<delim;
    file<<"locPlaneNormY"<<delim;
    file<<"locPlaneNormZ"<<delim;
    /// Covariance entries of the uncalibrated space point
    file<<"covX"<<delim;
    file<<"covXY"<<delim;
    file<<"covYX"<<delim;
    file<<"covY"<<delim;
    /// Drift radius
    file<<"driftR"<<delim;
    /// Properties of the space point Identifier
    file<<"technology"<<delim;
    file<<"gasGap"<<delim;
    file<<"primaryCh"<<delim;
    file<<"secondaryCh"<<delim;
    file<<std::endl;


   SG::ReadHandle readHandle{m_readKey, ctx};
   ATH_CHECK(readHandle.isPresent());

   for(const SpacePointBucket* bucket : *readHandle) {
      
       for (const auto& spacePoint : *bucket) {
            const Identifier measId = spacePoint->identify();
            int gasGap{0}, primaryCh{0}, secondCh{-1};
            using TechIndex = Muon::MuonStationIndex::TechnologyIndex; 
            const TechIndex techIdx = m_idHelperSvc->technologyIndex(measId);
            switch (techIdx) {
                case TechIndex::MDT: {
                    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()}; 
                    gasGap = (idHelper.multilayer(measId) -1)*idHelper.tubeLayerMax(measId) + idHelper.tubeLayer(measId);
                    primaryCh = idHelper.tube(measId);
                    break;
               }                
                case TechIndex::RPC: {
                    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
                    gasGap = (idHelper.doubletR(measId) -1) * idHelper.gasGapMax(measId) +  idHelper.gasGap(measId);
                    primaryCh = idHelper.channel(measId);
                    if (spacePoint->secondaryMeasurement()){
                        secondCh = idHelper.channel(xAOD::identify(spacePoint->secondaryMeasurement()));
                    }
                    break;
                }
                case TechIndex::TGC: {
                    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
                    gasGap = idHelper.gasGap(measId);
                    primaryCh = idHelper.channel(measId);
                    if (spacePoint->secondaryMeasurement()){
                        secondCh = idHelper.channel(xAOD::identify(spacePoint->secondaryMeasurement()));
                    }
                    break;
                }
                case TechIndex::STGC: {
                    const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
                    gasGap = (idHelper.multilayer(measId) -1) * 4 + idHelper.gasGap(measId);
                    primaryCh = idHelper.channel(measId);
                    if (spacePoint->secondaryMeasurement()){
                        secondCh = idHelper.channel(xAOD::identify(spacePoint->secondaryMeasurement()));
                    }
                    break;
                }
                case TechIndex::MM: {
                    const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
                    gasGap = (idHelper.multilayer(measId) -1) * 4 + idHelper.gasGap(measId);
                    primaryCh = idHelper.channel(measId);
                    if (spacePoint->secondaryMeasurement()){
                        secondCh = idHelper.channel(xAOD::identify(spacePoint->secondaryMeasurement()));
                    }
                    break;

                }
                default:
                  ATH_MSG_WARNING("Dude you can't have CSCs in R4 "<<m_idHelperSvc->toString(measId));
            };
            
            SectorId secId{};
            secId.fields[0] = static_cast<int>(spacePoint->msSector()->chamberIndex());
            secId.fields[1] = spacePoint->msSector()->side();
            secId.fields[2] = spacePoint->msSector()->sector();
            file<<secId.hash<<delim;
            file<<bucket->bucketId()<<delim;
            file<<spacePoint->positionInChamber().x()<<delim;
            file<<spacePoint->positionInChamber().y()<<delim;
            file<<spacePoint->positionInChamber().z()<<delim;
            //
            file<<spacePoint->directionInChamber().x()<<delim;
            file<<spacePoint->directionInChamber().y()<<delim;
            file<<spacePoint->directionInChamber().z()<<delim;
            //
            file<<spacePoint->planeNormal().x()<<delim;
            file<<spacePoint->planeNormal().y()<<delim;
            file<<spacePoint->planeNormal().z()<<delim;
            //
            file<<spacePoint->covariance()(Amg::x, Amg::x)<<delim;
            file<<spacePoint->covariance()(Amg::x, Amg::y)<<delim;
            file<<spacePoint->covariance()(Amg::y, Amg::x)<<delim;
            file<<spacePoint->covariance()(Amg::y, Amg::y)<<delim;
            file<<spacePoint->driftRadius()<<delim;
            file<<static_cast<int>(techIdx)<<delim;
            file<<gasGap<<delim;
            file<<primaryCh<<delim;
            file<<secondCh<<delim;            
            file<<std::endl;
       }
   }

   return StatusCode::SUCCESS;


 }
}


