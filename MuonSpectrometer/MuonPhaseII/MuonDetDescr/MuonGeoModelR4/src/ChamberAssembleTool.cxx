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
   auto mdtStationIndex = [this] (const std::string& stName) {
      return m_idHelperSvc->hasMDT() ? m_idHelperSvc->mdtIdHelper().stationNameIndex(stName) : -1;
   };
   auto tgcStationIndex = [this] (const std::string& stName) {
      return m_idHelperSvc->hasTGC() ? m_idHelperSvc->tgcIdHelper().stationNameIndex(stName) : -1;
   };

   const std::set<int> stIndicesEIL{mdtStationIndex("EIL"), mdtStationIndex("T4E")};
   const std::set<int> stIndicesEM{mdtStationIndex("EML"), mdtStationIndex("EMS"),
                                   tgcStationIndex("T1E"), tgcStationIndex("T1F"),
                                   tgcStationIndex("T2E"), tgcStationIndex("T2F"),
                                   tgcStationIndex("T3E"), tgcStationIndex("T3F")};
   
   const std::set<Identifier> BOE_ids{m_idHelperSvc->hasMDT() ? m_idHelperSvc->mdtIdHelper().elementID("BOL", 7,7): Identifier{},
                                      m_idHelperSvc->hasMDT() ? m_idHelperSvc->mdtIdHelper().elementID("BOL", -7,7): Identifier{},
                                      m_idHelperSvc->hasRPC() ? m_idHelperSvc->rpcIdHelper().elementID("BOL", -8, 7, 1) : Identifier{},
                                      m_idHelperSvc->hasRPC() ? m_idHelperSvc->rpcIdHelper().elementID("BOL", 8, 7, 1) : Identifier{}};

   std::vector<MuonReadoutElement*> allReadOutEles = mgr.getAllReadoutElements();

   std::vector<defineArgs> muonChamberCandidates{};

   /// Group the chambers together
   ///  NSW  / Mdt + Tgc -> sector & side
   ///  Mdt + Rpc -> same mother volume
   for (const MuonReadoutElement* readOutEle : allReadOutEles) {
      std::vector<defineArgs>::iterator exist = 
            std::find_if(muonChamberCandidates.begin(), muonChamberCandidates.end(), 
                         [this, readOutEle, &stIndicesEIL, &stIndicesEM, &BOE_ids](const defineArgs& args){
                            const MuonReadoutElement* refEle = args.readoutEles[0];
                            const Identifier refId = refEle->identify();
                            const Identifier testId = readOutEle->identify();
                            /// Check that the two readout elements are on the same side
                            if (sign(refEle->stationEta()) != sign(readOutEle->stationEta())) {
                                 return false;
                            }
                            /// The two readout elements shall be located in the same sector
                            if (m_idHelperSvc->sector(testId) != m_idHelperSvc->sector(refId)) {
                                 return false;
                            }
                            /// Separate out the BOE chambers
                            if (BOE_ids.count(m_idHelperSvc->chamberId(refId)) !=
                                 BOE_ids.count(m_idHelperSvc->chamberId(testId))){
                                    return false;
                            }
                            /// Summarize all readout element in the same sector & layer
                            /// into a single chamber
                            if (readOutEle->stationName() == refEle->stationName()) {
                                 return true;
                            }
                            /// sTgcs && Micromegas should belong to the same chamber
                            if (isNsw(readOutEle) && isNsw(refEle)) return true;
                            ///  EM readout element shall be grouped together too
                            if (stIndicesEM.count(readOutEle->stationName()) && 
                                stIndicesEM.count(refEle->stationName())) {
                                 return true;
                            }
                            /// Finally the EIL chambers should belong to the same chamber
                            if (stIndicesEIL.count(readOutEle->stationName()) &&
                                stIndicesEIL.count(refEle->stationName())) {
                                 return true;    
                            }
                            return false;
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

      constexpr double margin = 1.*Gaudi::Units::cm;
    
    for (defineArgs& candidate : muonChamberCandidates) {
         std::vector<Amg::Vector3D> edgePoints{};
         std::vector<Identifier> reIds{};
         double minPhi{1.e6};
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
         
         double minX{1.e6}, minY{1.e6}, minZ{1.e6}, maxX{-1.e6}, maxY{-1.e6}, maxZ{-1.e6}, maxXNegY{-1.e6};;
         
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

         const double hLengthY = 0.5 * (maxY - minY);
         const double hLengthZ = 0.5 * (maxZ - minZ);
         
         candidate.centerTrans = axisRotation.inverse() * Amg::Translation3D{midX, midY, midZ};
         
         for (const Amg::Vector3D &edge : edgePoints) {
            // translate the edges to the local frame of the chamber
            const Amg::Vector2D &localedge =
                (Amg::getTranslate3D(-midX, -midY, -midZ) * edge)
                    .block<2, 1>(0, 0);

            if (std::abs(localedge.y() + hLengthY) < 15. * Gaudi::Units::cm) {
                maxXNegY = std::max(maxXNegY, localedge.x());
            }
         }

           const Amg::Vector2D refEdge{maxXNegY, -hLengthY};

        // find the minimum angle between the edge points to define the
        // trapezoidal bounds of the chamber

        for (const Amg::Vector3D &edge : edgePoints) {
            const Amg::Vector2D &localedge =
                (Amg::getTranslate3D(-midX, -midY, -midZ) * edge)
                    .block<2, 1>(0, 0);
            if (std::abs(localedge.y() - refEdge.y()) < 1.e-10) continue;

            if (localedge.x() > refEdge.x() ||
                std::abs(localedge.x() - refEdge.x()) < 1.e-10) {
                double phi = std::atan2((localedge.y() - refEdge.y()),
                                   (localedge.x() - refEdge.x()));
                minPhi = std::min(minPhi, phi);
            }
        }

          // Define the trapezoidal bounds of the chamber
        candidate.halfXShort = maxXNegY + margin;
        candidate.halfY = hLengthY + margin;
        candidate.halfZ = hLengthZ + margin;
        candidate.halfXLong =
        candidate.halfXShort + 2 * candidate.halfY / std::tan(minPhi) + margin;

         if(msgLvl(MSG::VERBOSE)) {
            std::stringstream debugStream{};
            debugStream<<"minY: "<<minY<<", maxY: "<<maxY<<", minZ: "<<minZ<<", maxZ: "<<maxZ<<" -- ";
            debugStream<<candidate<<std::endl;
            const Amg::Transform3D globChambTrf{candidate.readoutEles[0]->localToGlobalTrans(gctx) * candidate.centerTrans};
            for (const MuonReadoutElement* ele: candidate.readoutEles){
                if(m_idHelperSvc->stationNameString(ele->identify()) == "BIR") {
                  const MdtReadoutElement* mdtRe = static_cast<const MdtReadoutElement*>(ele);

                  debugStream<<" *** "<<m_idHelperSvc->toString(ele->identify())<<" "
                             <<Amg::toString(globChambTrf.inverse()*mdtRe->highVoltPos(gctx, mdtRe->identify()))
                             <<" -- "<<Amg::toString(globChambTrf.inverse()*mdtRe->readOutPos(gctx, mdtRe->identify()))<<std::endl;
                }else
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
