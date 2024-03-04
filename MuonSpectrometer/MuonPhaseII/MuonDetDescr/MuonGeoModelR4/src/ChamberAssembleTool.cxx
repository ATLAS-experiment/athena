/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE

#include <MuonGeoModelR4/ChamberAssembleTool.h>

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>
#include <sstream>


namespace {
   int sign(int numb) { 
      return numb > 0 ? 1 : (numb == 0 ? 0 : -1);
   }
   bool isNsw(const MuonGMR4::MuonReadoutElement* re) {
       return re->detectorType() == ActsTrk::DetectorType::Mm ||
              re->detectorType() == ActsTrk::DetectorType::sTgc;
   }
}



namespace MuonGMR4{

using defineArgs = MuonChamber::defineArgs;

ChamberAssembleTool::ChamberAssembleTool(const std::string &type, const std::string &name,
                                         const IInterface *parent):
    AthAlgTool{type,name,parent}{}



StatusCode ChamberAssembleTool::buildReadOutElements(MuonDetectorManager &mgr) {
   ATH_CHECK(m_idHelperSvc.retrieve());
   ATH_CHECK(m_geoUtilTool.retrieve());

   /// TGC T4E chambers  & Mdt EIL chambers are glued together
   const int stIdx_T4E = m_idHelperSvc->hasTGC() ? m_idHelperSvc->tgcIdHelper().stationNameIndex("T4E") : -1;
   const int stIdx_EIL = m_idHelperSvc->hasMDT() ? m_idHelperSvc->mdtIdHelper().stationNameIndex("EIL") : -1;
   
   const auto isMdtTgcEILStation = [this, stIdx_T4E, stIdx_EIL](const MuonReadoutElement* a,
                                                                const MuonReadoutElement* b){
      /// At least one of the chambers needs to be EIL & T4E
      if ( !(a->stationName() == stIdx_EIL || b->stationName() == stIdx_EIL ||
             a->stationName() == stIdx_T4E || b->stationName() == stIdx_T4E)) return false;
      if (m_idHelperSvc->sector(a->identify()) != m_idHelperSvc->sector(b->identify())) {
          return false;
      }
      if(a->stationEta() * b->stationEta() < 0) return false;
      if (a->stationName() == b->stationName()) return true;
      return (a->stationName() == stIdx_EIL || b->stationName() == stIdx_EIL) &&
             (a->stationName() == stIdx_T4E || b->stationName() == stIdx_T4E);
   };
   std::vector<MuonReadoutElement*> allReadOutEles = mgr.getAllReadoutElements();

   std::vector<defineArgs> muonChamberCandidates{};

   /// Group the chambers together
   ///  NSW -> sector & side
   ///  Mdt + Tgc -> stationEta() + sector
   ///  Mdt + Rpc -> same mother volume
   for (const MuonReadoutElement* readOutEle : allReadOutEles) {
      std::vector<defineArgs>::iterator exist = 
            std::find_if(muonChamberCandidates.begin(), muonChamberCandidates.end(), 
                         [this, readOutEle, &isMdtTgcEILStation](const defineArgs& args){
                            const MuonReadoutElement* refEle = args.readoutEles[0];
                            const Identifier refId = refEle->identify();
                            const Identifier testId = readOutEle->identify();
                            /// Group the NSW according to their sector & side
                            if (isMdtTgcEILStation(refEle, readOutEle)) return true;
                            if (isNsw(readOutEle) && isNsw(refEle)) {
                                return sign(readOutEle->stationEta()) == sign(refEle->stationEta()) &&
                                       m_idHelperSvc->sector(testId) == m_idHelperSvc->sector(refId);
                            } else if (readOutEle->detectorType() == ActsTrk::DetectorType::Tgc ||
                                       refEle->detectorType() == ActsTrk::DetectorType::Tgc) {
                              /// Group the chambers by station Eta & sectors. Tgc chamber triplet
                              if (m_idHelperSvc->sector(refId) != m_idHelperSvc->sector(testId)) return false;
                              /// Ensure the same station eta
                              if (readOutEle->stationEta() != refEle->stationEta()) return false;
                              /// Last but not least the same Layer
                              return Muon::MuonStationIndex::toStationIndex(readOutEle->chamberIndex()) ==
                                     Muon::MuonStationIndex::toStationIndex(refEle->chamberIndex());
                            }
                            return readOutEle->getMaterialGeom()->getParent() ==
                                   refEle->getMaterialGeom()->getParent();

                         });
      /// If no chamber has been found, then create a new one
      if (exist == muonChamberCandidates.end()) {
         defineArgs newChamb{};
         newChamb.readoutEles.push_back(readOutEle);
         muonChamberCandidates.push_back(std::move(newChamb));
      } else {
         exist->readoutEles.push_back(readOutEle);
      }
    }
    /// Find the chamber middle and create the geometry from that
    ActsGeometryContext gctx{};
    
      /// Orientation of the chamber coordinate systems
      ///   x-axis: Points towards the sky
      ///   y-axis: Points along the chamber plane
      ///   z-axis: Points along the beam axis
      /// --> Transform into the coordinate system of the chamber
      ///   x-axis: Parallel to the eta channels
      ///   y-axis: Along the beam axis
      ///   z-axis: Towards the sky

      const Amg::Transform3D axisRotation{Amg::getRotateZ3D(-90. * Gaudi::Units::deg) *
                                          Amg::getRotateY3D(-90. * Gaudi::Units::deg)};
    
    for (defineArgs& candidate : muonChamberCandidates) {
         std::vector<Amg::Vector3D> edgePoints{};
         std::vector<Identifier> reIds{};
         const Amg::Transform3D toCenter = axisRotation * candidate.readoutEles[0]->globalToLocalTrans(gctx);
         for (const MuonReadoutElement* re : candidate.readoutEles) {
            const GeoShape* readOutShape = re->getMaterialGeom()->getLogVol()->getShape();
            std::vector<Amg::Vector3D> reEdges = m_geoUtilTool->shapeEdges(readOutShape,
                                                                           toCenter * re->localToGlobalTrans(gctx));
            if (msgLvl(MSG::VERBOSE)) {
               std::stringstream edgeStream{};
               for (const Amg::Vector3D& edge : reEdges) {
                  edgeStream<<" *** "<<Amg::toString(edge)<<std::endl;
               }
               ATH_MSG_VERBOSE(m_idHelperSvc->toStringDetEl(re->identify())<<" dumped shape "
                           <<m_geoUtilTool->dumpShape(readOutShape)<<std::endl<<std::endl<<edgeStream.str());
            }
            edgePoints.insert(edgePoints.end(), std::make_move_iterator(reEdges.begin()),
                                                std::make_move_iterator(reEdges.end()));            
            reIds.push_back(re->identify());
         }
         
         double minX{1.e6}, minY{1.e6}, minZ{1.e6}, maxX{-1.e6}, maxY{-1.e6}, maxZ{-1.e6};
         
         /// Determine the height and the width of the trapezoidal volume bounds
         for (const Amg::Vector3D& edge : edgePoints) {
            minY = std::min(minY, edge.y());
            maxY = std::max(maxY, edge.y());
            minZ = std::min(minZ, edge.z());
            maxZ = std::max(maxZ, edge.z());
            minX = std::min(minX, edge.x());
            maxX = std::max(maxX, edge.x());
         }
         const double midX = 0.5*(minX + maxX);
         const double midY = 0.5*(minY + maxY);
         const double midZ = 0.5*(minZ + maxZ);

         candidate.halfY = 0.5*(maxY - minY);
         candidate.halfZ = 0.5*(maxZ - minZ);
 
         /// Recenter the edges 
         minX = 1.e6, maxX = -1.e6;
         double minX2{1.e6}, maxX2{-1.e6};
         for (Amg::Vector3D& edge :edgePoints) {
            edge = Amg::Translation3D{-midX, -midY, -midZ} * edge;
            if (edge.y() < 0.){
               minX = std::min(minX, edge.x());
               maxX = std::max(maxX, edge.x());
            } else {
               minX2 = std::min(minX2, edge.x());
               maxX2 = std::max(maxX2, edge.x());
            }
         }
         candidate.halfXShort = 0.5*(maxX - minX);
         candidate.halfXShort = candidate.halfXLong = 0.5*(maxX2 - minX2);

         
         candidate.centerTrans = axisRotation.inverse() * Amg::Translation3D{midX, midY, midZ};
         
         if(msgLvl(MSG::VERBOSE)) {
            std::stringstream debugStream{};
            debugStream<<"minY: "<<minY<<", maxY: "<<maxY<<", minZ: "<<minZ<<", maxZ: "<<maxZ<<" -- ";
            debugStream<<candidate<<std::endl;
            const Amg::Transform3D globChambTrf{candidate.readoutEles[0]->localToGlobalTrans(gctx) * candidate.centerTrans};
            for (const MuonReadoutElement* ele: candidate.readoutEles){
                debugStream<<" **** "<<m_idHelperSvc->toStringDetEl(ele->identify())<<", local RE center: "
                           <<Amg::toString(globChambTrf.inverse() * ele->center(gctx))<<", global: "
                           <<Amg::toString(ele->center(gctx))<<std::endl;
            }
            ATH_MSG_VERBOSE(debugStream.str());
         }         
         GeoModel::TransientConstSharedPtr<MuonChamber> newChamber = std::make_unique<MuonChamber>(std::move(candidate));
         
         for (const Identifier& chId : reIds) {
            mgr.getReadoutElement(chId)->setChamberLink(newChamber);
         }
    }
    return StatusCode::SUCCESS;
}

}
#endif