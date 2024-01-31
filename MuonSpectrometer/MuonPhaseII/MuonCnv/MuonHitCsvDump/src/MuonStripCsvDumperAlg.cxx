/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonStripCsvDumperAlg.h"
#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "StoreGate/ReadHandle.h"
#include <fstream>
#include <TString.h>


MuonStripCsvDumperAlg::MuonStripCsvDumperAlg(const std::string& name, ISvcLocator* pSvcLocator):
 AthAlgorithm{name, pSvcLocator} {}

 StatusCode MuonStripCsvDumperAlg::initialize() {
   ATH_CHECK(m_stripContainerKey.initialize());
   ATH_CHECK(m_idHelperSvc.retrieve());
   ATH_CHECK(m_surfaceProvTool.retrieve());
   return StatusCode::SUCCESS;
 }

 StatusCode MuonStripCsvDumperAlg::execute(){

   const EventContext& ctx{Gaudi::Hive::currentContext()};
   const ActsGeometryContext gctx{};
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
      Amg::Vector3D stripPos{Amg::Vector3D::Zero()};      
      const Identifier measId{(Identifier::value_type)strip->identifier()};

      if (strip->type() == xAOD::UncalibMeasType::RpcStripType) {
         const xAOD::RpcStrip* rpcStrip{static_cast<const xAOD::RpcStrip*>(strip)};
         const IdentifierHash hash{rpcStrip->measurementHash()};
         stripPos = m_surfaceProvTool->globalToChambCenter(gctx, measId) * 
                    rpcStrip->readoutElement()->stripPosition(gctx, hash);
      } else if (strip->type() == xAOD::UncalibMeasType::TgcStripType) {
        const xAOD::TgcStrip* tgcStrip{static_cast<const xAOD::TgcStrip*>(strip)};
        const IdentifierHash hash{tgcStrip->measurementHash()};
        stripPos = m_surfaceProvTool->globalToChambCenter(gctx, measId) * 
                   tgcStrip->readoutElement()->channelPosition(gctx, hash);                
      } else {
        ATH_MSG_FATAL("Readout type "<<m_idHelperSvc->toString(measId)
                <<" not implemented in "<<__FILE__);
        return StatusCode::FAILURE;
      }
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



