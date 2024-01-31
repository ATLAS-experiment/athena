/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSimHitCsvDumperAlg.h"

#include <MuonReadoutGeometry/MuonReadoutElement.h>
#include <MuonReadoutGeometry/GlobalUtilities.h>
#include <GaudiKernel/SystemOfUnits.h>

#include <fstream>
#include <TString.h>

/// Helper struct to map all muons in the same chamber
struct TrueHitInChamb {
    Identifier stationId{};
    Amg::Vector3D lDir{Amg::Vector3D::Zero()};
    Amg::Vector3D lPos{Amg::Vector3D::Zero()};
    const xAOD::MuonSimHit* simHit{};
    bool operator<(const TrueHitInChamb& other) const{
       if (stationId != other.stationId) return stationId < other.stationId;
       return simHit->genParticleLink().barcode() < other.simHit->genParticleLink().barcode();
    }
};

MuonSimHitCsvDumperAlg::MuonSimHitCsvDumperAlg(const std::string& name, ISvcLocator* pSvcLocator):
   AthAlgorithm{name, pSvcLocator} {}

StatusCode MuonSimHitCsvDumperAlg::initialize() {
   ATH_CHECK(m_inSimHitKey.initialize());
   ATH_CHECK(m_idHelperSvc.retrieve());
   ATH_CHECK(m_surfaceProvTool.retrieve());
   ATH_CHECK(detStore()->retrieve(m_r4DetMgr));

   return StatusCode::SUCCESS;

}

StatusCode MuonSimHitCsvDumperAlg::execute(){
    
   const ActsGeometryContext gctx{};

   // these are the conventions by ACTS
   std::ofstream file{std::string(Form("event%09zu-",++m_event))+"MuonSimHit.csv"};
   const std::string delim = ",";
   file<<"pdgId"<<delim;
   file<<"StationName"<<delim;
   file<<"StationEta"<<delim;
   file<<"StationPhi"<<delim;
   file<<"LocalPositionExtrx"<<delim;
   file<<"LocalPositionExtry"<<delim;
   file<<"LocalPositionExtrz"<<delim;
   file<<"LocalDirectionx"<<delim;
   file<<"LocalDirectiony"<<delim;
   file<<"LocalDirectionz"<<std::endl;

   const EventContext & context = Gaudi::Hive::currentContext();
   std::set<TrueHitInChamb> usedStations{};
   for (const SG::ReadHandleKey<xAOD::MuonSimHitContainer>& key : m_inSimHitKey) {

      SG::ReadHandle<xAOD::MuonSimHitContainer> readSimHits{key, context};
      ATH_CHECK(readSimHits.isPresent());
   

      for (const xAOD::MuonSimHit* simHit : *readSimHits) {

         //The sim hit collection contains non-muon hits, while the fast digi only writes out muon 
         // deposits. Here remove sim hits that are not expected to be accounted for in the digi. 

         if (std::abs(simHit->pdgId()) != 13) continue;
         const Identifier ID = simHit->identify();

         TrueHitInChamb newHit{};
         newHit.stationId = m_idHelperSvc->chamberId(ID);
         newHit.simHit = simHit;
         ATH_MSG_VERBOSE("Dump simulation hit for " <<m_idHelperSvc->toString(ID));
         const MuonGMR4::MuonReadoutElement* reElement = m_r4DetMgr->getReadoutElement(ID);
      
         //transform from local (w.r.t tube's frame) to global (ATLAS frame) and then to chamber's frame
         const Amg::Transform3D toChamber = m_surfaceProvTool->globalToChambCenter(gctx, ID) *
                                            reElement->localToGlobalTrans(gctx, reElement->measurementHash(ID));

         const Amg::Vector3D localPos{toChamber * xAOD::toEigen(simHit->localPosition())};
         newHit.lDir = toChamber.linear() * xAOD::toEigen(simHit->localDirection());

         const std::optional<double> lambda = Amg::intersect<3>(localPos, newHit.lDir, Amg::Vector3D::UnitZ(), 0.);
         newHit.lPos = localPos + (*lambda)*newHit.lDir;

         auto insert_itr = usedStations.insert(newHit);
         /// No check needed that the muon is extrapolated to the same point
         if (insert_itr.second) continue;
      
         const TrueHitInChamb prevMuon{*insert_itr.first};
         constexpr double tolerance =  0.1 * Gaudi::Units::millimeter;
         double angleTolerance = std::cos(0.1*Gaudi::Units::deg);
         if ((prevMuon.lPos - newHit.lPos).mag() > tolerance ||
              prevMuon.lDir.dot(newHit.lDir) < angleTolerance ) {
               ATH_MSG_WARNING("Muon "<<newHit.simHit->genParticleLink()<<" has a different ending point when it starts from "
                        <<m_idHelperSvc->toString(prevMuon.simHit->identify())<<" "<<Amg::toString(prevMuon.lPos)
                        <<" pointing to "<<Amg::toString(prevMuon.lDir)
                        <<" --  vs. "<<m_idHelperSvc->toString(newHit.simHit->identify())
                        <<" "<<Amg::toString(newHit.lPos) <<" pointing to "<<Amg::toString(newHit.lDir)<<" difference: "
                        <<(prevMuon.lPos - newHit.lPos).mag() <<" && "<< std::acos(std::min(1.,(std::max(-1.,prevMuon.lDir.dot(newHit.lDir))))) / Gaudi::Units::deg  ) ;
         }
      }
   }

   for (const TrueHitInChamb& hit : usedStations) {
      file<<hit.simHit->pdgId()<<delim;
      file<<m_idHelperSvc->stationName(hit.stationId)<<delim;
      file<<m_idHelperSvc->stationEta(hit.stationId)<<delim;
      file<<m_idHelperSvc->stationPhi(hit.stationId)<<delim;
   
      file<<hit.lPos.x()<<delim;
      file<<hit.lPos.y()<<delim;
      file<<hit.lPos.z()<<delim;

      file<<hit.lDir.x()<<delim;
      file<<hit.lDir.y()<<delim;
      file<<hit.lDir.z()<<std::endl;
   }
   return StatusCode::SUCCESS;
}


