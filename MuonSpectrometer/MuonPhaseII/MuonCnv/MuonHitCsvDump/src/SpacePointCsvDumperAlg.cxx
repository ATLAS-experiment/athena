/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SpacePointCsvDumperAlg.h"

#include "StoreGate/ReadHandle.h"
#include <fstream>
#include <TString.h>

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
    file<<"measuresEta"<<delim;
    file<<"measuresPhi"<<delim<<std::endl;


   SG::ReadHandle<MuonSpacePointContainer> readHandle{m_readKey, ctx};
   ATH_CHECK(readHandle.isPresent());

   for(const MuonSpacePointBucket* bucket : *readHandle) {
      
       for (const auto& spacePoint : *bucket) {
           const Identifier measId = spacePoint->identify();
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
            file<<m_idHelperSvc->gasGap(measId)<<delim;
            file<<spacePoint->measuresEta()<<delim;
            file<<spacePoint->measuresPhi()<<delim;
            file<<std::endl;
       }
       file<<std::endl;
   }

   return StatusCode::SUCCESS;


 }
}


