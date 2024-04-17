/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonStripCsvDumperAlg.h"
#include "MuonSpacePoint/MuonSpacePoint.h"
#include "StoreGate/ReadHandle.h"
#include <fstream>
#include <TString.h>


MuonStripCsvDumperAlg::MuonStripCsvDumperAlg(const std::string& name, ISvcLocator* pSvcLocator):
 AthAlgorithm{name, pSvcLocator} {}

 StatusCode MuonStripCsvDumperAlg::initialize() {
   ATH_CHECK(m_stripContainerKey.initialize());
   ATH_CHECK(m_geoCtxKey.initialize());
   ATH_CHECK(m_idHelperSvc.retrieve());
   return StatusCode::SUCCESS;
 }

 StatusCode MuonStripCsvDumperAlg::execute(){

   const EventContext& ctx{Gaudi::Hive::currentContext()};
   
   SG::ReadHandle<ActsGeometryContext> gctxHandle{m_geoCtxKey, ctx};
   ATH_CHECK(gctxHandle.isPresent());
   const ActsGeometryContext& gctx{*gctxHandle};

   const std::string delim = ",";
   std::ofstream file{std::string(Form("event%09zu-",++m_event))+
                      m_preFix+"Strips.csv"};
   
    file<<"locPos"<<delim;
    file<<"locCov"<<delim;
    file<<"stripPositionx"<<delim;
    file<<"stripPositiony"<<delim;
    file<<"stripPositionz"<<delim;
    file<<"stationName"<<delim;    
    file<<"stationEta"<<delim;
    file<<"stationPhi"<<delim;
    file<<"gasGap"<<delim;
    file<<"measuresPhi"<<delim<<std::endl;


   SG::ReadHandle<xAOD::UncalibratedMeasurementContainer> readHandle{m_stripContainerKey, ctx};
   ATH_CHECK(readHandle.isPresent());

   for(const xAOD::UncalibratedMeasurement* strip : *readHandle){
      const MuonR4::MuonSpacePoint spacePoint{gctx, strip};
      const Amg::Vector3D& stripPos{spacePoint.positionInChamber()};
      const Identifier& measId{spacePoint.identify()};

      file<<strip->localPosition<1>().x()<<delim;
      file<<strip->localCovariance<1>().x()<<delim;
      
      file<<stripPos.x()<<delim;
      file<<stripPos.y()<<delim;
      file<<stripPos.z()<<delim;
      file<<m_idHelperSvc->stationName(measId)<<delim;
      file<<m_idHelperSvc->stationEta(measId)<<delim;
      file<<m_idHelperSvc->stationPhi(measId)<<delim;
      file<<m_idHelperSvc->gasGap(measId)<<delim;
      file<<m_idHelperSvc->measuresPhi(measId)<<delim;
      file<<std::endl;
   }

   return StatusCode::SUCCESS;


 }



