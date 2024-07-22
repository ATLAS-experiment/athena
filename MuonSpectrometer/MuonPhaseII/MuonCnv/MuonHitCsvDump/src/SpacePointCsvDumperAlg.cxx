/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SpacePointCsvDumperAlg.h"

#include "StoreGate/ReadHandle.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include <fstream>
#include <TString.h>

namespace {
    union bucketId{
        int8_t fields[4];
        int hash;
    };

}


namespace MuonR4{
SpacePointCsvDumperAlg::SpacePointCsvDumperAlg(const std::string& name, ISvcLocator* pSvcLocator):
 AthAlgorithm{name, pSvcLocator} {}

 StatusCode SpacePointCsvDumperAlg::initialize() {
   ATH_CHECK(m_readKey.initialize());
   ATH_CHECK(m_idHelperSvc.retrieve());
   return StatusCode::SUCCESS;
 }

 StatusCode SpacePointCsvDumperAlg::execute(){

   const EventContext& ctx{Gaudi::Hive::currentContext()};
   


   constexpr std::string_view delim = ",";
   std::ofstream file{std::string(Form("event%09zu-",++m_event))+"SpacePoints.csv"};
   
    file<<"bucketId"<<delim;
    file<<"localPositionX"<<delim;
    file<<"localPositionY"<<delim;
    file<<"localPositionZ"<<delim;
    file<<"covX"<<delim;
    file<<"covXY"<<delim;
    file<<"covYX"<<delim;
    file<<"covY"<<delim;
    file<<"driftR"<<delim;
    file<<"stationName"<<delim;
    file<<"stationEta"<<delim;
    file<<"stationPhi"<<delim;
    file<<"gasGap"<<delim;
    file<<"primaryCh"<<delim;
    file<<"secondaryCh"<<delim;
    file<<"measuresEta"<<delim;
    file<<"measuresPhi"<<delim<<std::endl;


   SG::ReadHandle<MuonSpacePointContainer> readHandle{m_readKey, ctx};
   ATH_CHECK(readHandle.isPresent());

   for(const MuonSpacePointBucket* bucket : *readHandle) {
      
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
            
            bucketId buckId{};
            buckId.fields[0] = spacePoint->muonChamber()->stationName();
            buckId.fields[1] = spacePoint->muonChamber()->stationEta();
            buckId.fields[2] = spacePoint->muonChamber()->stationPhi();
            buckId.fields[3] = bucket->bucketId();
            
            file<<buckId.hash<<delim;
            file<<spacePoint->positionInChamber().x()<<delim;
            file<<spacePoint->positionInChamber().y()<<delim;
            file<<spacePoint->positionInChamber().z()<<delim;
            file<<spacePoint->covariance()(Amg::x, Amg::x)<<delim;
            file<<spacePoint->covariance()(Amg::x, Amg::y)<<delim;
            file<<spacePoint->covariance()(Amg::y, Amg::x)<<delim;
            file<<spacePoint->covariance()(Amg::y, Amg::y)<<delim;
            file<<spacePoint->driftRadius()<<delim;
            file<<m_idHelperSvc->stationName(measId)<<delim;
            file<<m_idHelperSvc->stationEta(measId)<<delim;
            file<<m_idHelperSvc->stationPhi(measId)<<delim;
            file<<gasGap<<delim;
            file<<primaryCh<<delim;
            file<<secondCh<<delim;            
            file<<spacePoint->measuresEta()<<delim;
            file<<spacePoint->measuresPhi()<<delim;
            file<<std::endl;
       }
   }

   return StatusCode::SUCCESS;


 }
}


