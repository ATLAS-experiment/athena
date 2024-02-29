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
   std::vector<MuonReadoutElement*> allReadOutEles = mgr.getAllReadoutElements();

   std::vector<defineArgs> muonChamberCandidates{};

   /// Group the chambers together
   ///  NSW -> sector & side
   ///  Mdt + Tgc -> stationEta() + sector
   ///  Mdt + Rpc -> same mother volume
   for (const MuonReadoutElement* readOutEle : allReadOutEles) {
      std::vector<defineArgs>::iterator exist = 
            std::find_if(muonChamberCandidates.begin(), muonChamberCandidates.end(), 
                         [this, readOutEle](const defineArgs& args){
                            const MuonReadoutElement* refEle = args.readoutEles[0];
                            const Identifier refId = refEle->identify();
                            const Identifier testId = readOutEle->identify();
                            /// Group the NSW according to their sector & side
                            if (isNsw(readOutEle) && isNsw(refEle)) {
                                return sign(readOutEle->stationEta()) == sign(refEle->stationEta()) &&
                                       m_idHelperSvc->sector(testId) == m_idHelperSvc->sector(refId);
                            } else if (readOutEle->detectorType() == ActsTrk::DetectorType::Tgc ||
                                       refEle->detectorType() == ActsTrk::DetectorType::Tgc) {
                              /// Group the chambers by station Eta & sectors. Tgc chamber triplet
                              return m_idHelperSvc->sector(refId) == m_idHelperSvc->sector(testId) &&
                                     readOutEle->stationEta() == refEle->stationEta() &&
                                     Muon::MuonStationIndex::toLayerIndex(readOutEle->chamberIndex()) ==
                                     Muon::MuonStationIndex::toLayerIndex(readOutEle->chamberIndex());
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
      /// x-axis: Parallel to the eta channels
      /// y-axis: Along the beam axis
      /// z-axis: Towards the sky
      /// Record the thickness for the top and bottom of the chamber
      const Amg::Transform3D axisRotation{Amg::getRotateX3D(90. * Gaudi::Units::deg)*
                                          Amg::getRotateZ3D(90. * Gaudi::Units::deg)};
    
    for (defineArgs& candidate : muonChamberCandidates) {
         std::vector<Amg::Vector3D> edgePoints{};
         std::vector<Identifier> reIds{};
         const Amg::Transform3D toCenter = axisRotation * candidate.readoutEles[0]->globalToLocalTrans(gctx);
         for (const MuonReadoutElement* re : candidate.readoutEles) {
            const GeoShape* readOutShape = re->getMaterialGeom()->getLogVol()->getShape();
            std::vector<Amg::Vector3D> reEdges = m_geoUtilTool->shapeEdges(readOutShape,
                                                                           re->localToGlobalTrans(gctx) * toCenter);
            edgePoints.insert(edgePoints.end(), std::make_move_iterator(reEdges.begin()),
                                                std::make_move_iterator(reEdges.end()));            
            reIds.push_back(re->identify());
         }
         double minX1{1.e6},  minX2{1.e6},  minY{1.e6},  minZ{1.e6},
                maxX1{-1.e6}, maxX2{-1.e6}, maxY{-1.e6}, maxZ{-1.e6};
         
         for (const Amg::Vector3D& edge : edgePoints) {
            if (edge.z() > 0.) {
               minX1 = std::min(minX1, edge.x());
               maxX1 = std::max(maxX1, edge.x());
              
            } else {
               minX2 = std::min(minX2, edge.x());
               maxX2 = std::max(maxX2, edge.x());              
            }
            minY = std::min(minY, edge.y());
            maxY = std::max(maxY, edge.y());
            minZ = std::min(minZ, edge.z());
            maxZ = std::max(maxZ, edge.z());
         }
         candidate.halfXLong = maxX1 - minX1;
         candidate.halfXShort = maxX2 - minX2;
         candidate.halfY = maxY - minY;
         candidate.halfZ = maxZ - minZ;

         candidate.centerTrans = axisRotation * Amg::Translation3D{0.5*(maxX1 + minX1),
                                                                   0.5*(maxY + minY),
                                                                   0.5*(maxZ + minZ)};

         GeoModel::TransientConstSharedPtr<MuonChamber> newChamber = std::make_unique<MuonChamber>(std::move(candidate));
         for (const Identifier chId : reIds) {
            mgr.getReadoutElement(chId)->setChamberLink(newChamber);
         }
    }
    return StatusCode::SUCCESS;
}

}
#endif