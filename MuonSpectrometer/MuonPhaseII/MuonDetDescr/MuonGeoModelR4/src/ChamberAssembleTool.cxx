/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE

#include "ChamberAssembleTool.h"

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <ActsGeoUtils/SurfaceBoundSet.h>
#include <sstream>

#include <Acts/Geometry/TrapezoidVolumeBounds.hpp>
#include <Acts/Plugins/GeoModel/GeoModelToDetectorVolume.hpp>
#include <GeoModelHelpers/TransformToStringConverter.h>
#include <GeoModelKernel/GeoDefinitions.h>


namespace {
   constexpr int sign(int numb) { 
      return numb > 0 ? 1 : (numb == 0 ? 0 : -1);
   }
   bool isNsw(const MuonGMR4::MuonReadoutElement* re) {
       return re->detectorType() == ActsTrk::DetectorType::Mm ||
              re->detectorType() == ActsTrk::DetectorType::sTgc;
   }
   /// Orientation of the readout element coordinate system
   ///   x-axis: Points towards the sky
   ///   y-axis: Points along the chamber plane
   ///   z-axis: Points along the beam axis
   /// --> Transform into the coordinate system of the chamber
   ///   x-axis: Parallel to the eta channels
   ///   y-axis: Along the beam axis
   ///   z-axis: Towards the sky
   
   
   Amg::Transform3D axisRotation(ActsTrk::DetectorType t) {
      if (t == ActsTrk::DetectorType::sTgc) {
         return Amg::Transform3D::Identity();
      }
      return Amg::getRotateZ3D(-90. * Gaudi::Units::deg) *
              Amg::getRotateY3D(-90. * Gaudi::Units::deg);
   }
   
   using BoundEnum = Acts::TrapezoidVolumeBounds::BoundValues;

   Amg::Vector3D projectIntoXY(const Amg::Vector3D& v) {
      return Amg::Vector3D{v.x(), v.y(), 0.};
   }

}



namespace MuonGMR4{

using chamberArgs = Chamber::defineArgs;

ChamberAssembleTool::ChamberAssembleTool(const std::string &type, const std::string &name,
                                         const IInterface *parent):
    base_class{type,name,parent}{}



double ChamberAssembleTool::trapezoidEdgeDist(const Amg::Vector3D& linePos,
                                              const Amg::Vector3D& lineDir,
                                              const Amg::Vector3D& testMe,
                                              bool leftEdge) {
   /// Construct the normal pointing inwards
   const Amg::Vector3D normal = lineDir.cross((leftEdge ? 1. : -1.) *Amg::Vector3D::UnitZ());
   const Amg::Vector3D closest = linePos + lineDir.dot(testMe - linePos) * lineDir;
   return normal.dot(testMe - closest);  
}

std::shared_ptr<ChamberAssembleTool::BoundType> 
      ChamberAssembleTool::boundingBox(const MuonReadoutElement* chambEle,
                                       ActsTrk::SurfaceBoundSet<BoundType>& boundSet) {

   switch(chambEle->detectorType()) {
      case ActsTrk::DetectorType::Mdt: {
         const auto* techEle = static_cast<const MdtReadoutElement*>(chambEle);
         const auto& pars = techEle->getParameters();
         return boundSet.make_bounds(pars.shortHalfX, pars.longHalfX, 
                                     pars.halfY, pars.halfHeight );
         break;
      } case ActsTrk::DetectorType::Rpc: {
         const auto* techEle = static_cast<const RpcReadoutElement*>(chambEle);
         const auto& pars = techEle->getParameters();
         return boundSet.make_bounds(pars.halfWidth, pars.halfWidth, 
                                     pars.halfLength, pars.halfThickness );
         break;
      } case ActsTrk::DetectorType::Tgc: {
         const auto* techEle = static_cast<const TgcReadoutElement*>(chambEle);
         const auto& pars = techEle->getParameters();
         return boundSet.make_bounds(pars.halfWidthShort, pars.halfWidthLong, 
                                     pars.halfHeight, pars.halfThickness );
         break;
      } case ActsTrk::DetectorType::sTgc: {
         const auto* techEle = static_cast<const sTgcReadoutElement*>(chambEle);
         const auto& pars = techEle->getParameters();
         return boundSet.make_bounds(pars.sHalfChamberLength, pars.lHalfChamberLength, 
                                     pars.halfChamberHeight, pars.halfChamberTck );
         break;
      } case ActsTrk::DetectorType::Mm: {
         const auto* techEle = static_cast<const MmReadoutElement*>(chambEle);
         const auto& pars = techEle->getParameters();
         return boundSet.make_bounds(pars.halfShortWidth, pars.halfLongWidth, 
                                     pars.halfHeight, pars.halfThickness );
         break;
      } default:
         THROW_EXCEPTION("Unsupported detector type "<<to_string(chambEle->detectorType()));
   }
   return nullptr;
}


std::array<Amg::Vector3D, 4> ChamberAssembleTool::cornerPointsPlane(const Amg::Transform3D& localToGlob, 
                                                                    const BoundType& bounds) {
   std::array<Amg::Vector3D,4> planePoints{
            localToGlob * Amg::Vector3D(-bounds.get(BoundEnum::eHalfLengthXnegY), -bounds.get(BoundEnum::eHalfLengthY), 0. ),
            localToGlob * Amg::Vector3D(-bounds.get(BoundEnum::eHalfLengthXposY), bounds.get(BoundEnum::eHalfLengthY), 0. ),
            localToGlob * Amg::Vector3D(bounds.get(BoundEnum::eHalfLengthXnegY), -bounds.get(BoundEnum::eHalfLengthY), 0. ),
            localToGlob * Amg::Vector3D(bounds.get(BoundEnum::eHalfLengthXposY), bounds.get(BoundEnum::eHalfLengthY), 0. ),
   };
   return planePoints;
}
std::array<Amg::Vector3D, 8> ChamberAssembleTool::cornerPoints(const Amg::Transform3D& localToGlob, 
                                                               const BoundType& bounds) {
   std::array<Amg::Vector3D, 8> toRet{make_array<Amg::Vector3D,8>(Amg::Vector3D::Zero())};
   std::array<Amg::Vector3D, 4> plane = cornerPointsPlane(localToGlob, bounds);
   for (unsigned int z : {0, 1}){
      const double sign {z ? 1. : -1.};
      const Amg::Vector3D stretch = (sign * bounds.get(BoundEnum::eHalfLengthZ)) * Amg::Vector3D::UnitZ();
      for (unsigned int b = 0; b < plane.size(); ++b){
         toRet[b + z * plane.size()] = plane[b] + stretch;
      }
   }
   return toRet;
}
Amg::Transform3D ChamberAssembleTool::centerTrapezoid(const std::array<Amg::Vector3D, 8>& corners) {
   
   static constexpr double maxSize = 200.*Gaudi::Units::km;
   double minX{maxSize}, maxX{-maxSize}, minY{maxSize}, maxY{-maxSize}, minZ{maxSize}, maxZ{-maxSize};
   for (const Amg::Vector3D& corner : corners) {
      minX = std::min(corner.x(), minX);
      maxX = std::max(corner.x(), maxX);
      minY = std::min(corner.y(), minY);
      maxY = std::max(corner.y(), maxY);
      minZ = std::min(corner.z(), minZ);
      maxZ = std::max(corner.z(), maxZ);
   }
   return Amg::getTranslate3D(-0.5*(minX + maxX), -0.5*(minY + maxY), -0.5*(minZ + maxZ));
}


ChamberAssembleTool::BoundTrfPair 
      ChamberAssembleTool::boundingBox(const ActsGeometryContext& gctx,
                                       const std::vector<const MuonReadoutElement*>& readoutEles,
                                       const Amg::Transform3D& toCenter,
                                       ActsTrk::SurfaceBoundSet<BoundType>& boundSet,
                                       const double margin) const {

      std::shared_ptr<BoundType> envelopeBounds{};
     
      Amg::Transform3D newCentreTrf{Amg::Transform3D::Identity()};
      for (const MuonReadoutElement* chambEle :  readoutEles) {
            Amg::Transform3D trf = newCentreTrf * toCenter * 
                                   chambEle->localToGlobalTrans(gctx) * 
                                   axisRotation(chambEle->detectorType()).inverse();
            std::shared_ptr<BoundType> bounds = boundingBox(chambEle, boundSet);
            if (!envelopeBounds) {
               envelopeBounds = bounds;
               newCentreTrf = centerTrapezoid(cornerPoints(trf, *bounds)) * newCentreTrf;
               ATH_MSG_VERBOSE("Envelope "<<(*envelopeBounds)<<", transform: "<<Amg::toString(newCentreTrf));
               continue;
            }
            /// Hack to cope with the RPCs which may be rotated by 180 degrees around the z-axis in cases,
            /// they're upside down.
            GeoTrf::CoordEulerAngles rotAngles = GeoTrf::getCoordRotationAngles(trf);
            if (std::abs(rotAngles.gamma - 180.*Gaudi::Units::deg)< std::numeric_limits<float>::epsilon()){
               trf = trf *Amg::getRotateZ3D(180.*Gaudi::Units::deg);
            }
            /// Check whether the bounds are already embedded in the trapezoid
            const std::array<Amg::Vector3D, 8> corners = cornerPoints(trf, *bounds);

            Acts::Volume volume{Amg::Transform3D::Identity(), envelopeBounds};
            /// Everything is contained in the volume 
            if (std::ranges::find_if(corners, [&volume](const Amg::Vector3D& v) {
                              return !volume.inside(v);}) == corners.end()) {
               ATH_MSG_VERBOSE("Readout element "<<m_idHelperSvc->toStringDetEl(chambEle->identify())<<" "
                           <<(*boundingBox(chambEle, boundSet))<<" fully contained. ");
               continue;
            }
            if (msgLvl(MSG::VERBOSE)) {
               std::stringstream debugStr{};
               for (const Amg::Vector3D& corner : corners) {
                  debugStr<<"   ***** "<<Amg::toString(corner)<<std::endl;
               }
               ATH_MSG_VERBOSE(m_idHelperSvc->toString(chambEle->identify())<<" corner points "
                              <<GeoTrf::toString(trf, true)<<std::endl<<debugStr.str());   
            }
            /// Fetch the edges of the best known trapezoid to extend the dimensions
            const std::array<Amg::Vector3D, 8> refCorners{cornerPoints(Amg::Transform3D::Identity(), *envelopeBounds)};
            /// Reserve space for the new envelope trapezoid 
            std::array<Amg::Vector3D, 8> newTrapBounds{make_array<Amg::Vector3D, 8>(Amg::Vector3D::Zero())};

            /// The first 4 indices in the trpezoidal array are the bottom corners
            for (unsigned  lowZ : {0, 4}) {
               for (bool isLeft : {false, true}) {

                  const size_t iHigh = 1 + (!isLeft)*2 + lowZ;
                  const size_t iLow  = 0 + (!isLeft)*2 + lowZ;
                  /// Check the angles of the bounding trapzeoids
                  const Amg::Vector3D dirRef{projectIntoXY(refCorners[iHigh] - refCorners[iLow]).unit()};
                  const Amg::Vector3D dirCan{projectIntoXY(corners[iHigh] - corners[iLow]).unit()};
            
                  
                  ATH_MSG_VERBOSE((isLeft ? "Left" : "Right")<<" edge "<<Amg::toString(dirRef)
                                    <<" "<<dirRef.phi() / Gaudi::Units::deg <<" --- "
                                    <<m_idHelperSvc->toStringDetEl(chambEle->identify())<<" "
                                    <<Amg::toString(dirCan)<<", phi: "<<dirCan.phi() / Gaudi::Units::deg);

                  /// On the left edge the trapezoid opening angle needs to be larger while on the right side it's smaller
                  const Amg::Vector3D& pickDir{(dirRef.phi() > dirCan.phi()) == isLeft ? dirRef : dirCan};
                  /// Calculate the distance to the trapezoidal edge. If it's less than 0, it's not embedded by
                  /// the trapezoid and the smaller the number the farer it's from the trapezoid
                  const double cornerLowD = trapezoidEdgeDist(refCorners[iLow], pickDir, corners[iLow], isLeft);
                  const double cornerHighD = trapezoidEdgeDist(refCorners[iLow], pickDir, corners[iHigh], isLeft);
                  ATH_MSG_VERBOSE("Distance "<<cornerLowD<<"/ "<<cornerHighD);
                  /// Choose as reference point the one which is farest away from the edge
                  const Amg::Vector3D& pickPos{cornerLowD < 0 || cornerHighD < 0  ? 
                                               cornerLowD < cornerHighD ? corners[iLow] : corners[iHigh]: refCorners[iLow]};
               
                  ATH_MSG_VERBOSE("Low points "<<Amg::toString(corners[iLow])<<" - "<<Amg::toString(refCorners[iLow]));
                  ATH_MSG_VERBOSE("High points "<<Amg::toString(corners[iHigh])<<" - "<<Amg::toString(refCorners[iHigh]));

                  ///Extend the trapezoid in the x-y plane
                  newTrapBounds[iHigh] = pickPos + Amg::intersect<3>(pickPos, pickDir, Amg::Vector3D::UnitY(),
                                                      std::max(corners[iHigh].y(), refCorners[iHigh].y())).value_or(0.) * pickDir;

                  newTrapBounds[iHigh].z() = lowZ ? std::max(corners[iHigh].z(),refCorners[iHigh].z())
                                                   : std::min(corners[iHigh].z(), refCorners[iHigh].z());
                  newTrapBounds[iLow] = pickPos + Amg::intersect<3>(pickPos, pickDir, Amg::Vector3D::UnitY(),
                                                                     std::min(corners[iLow].y(), refCorners[iLow].y())).value_or(0.) * pickDir;
                  newTrapBounds[iLow].z() = lowZ ? std::max(corners[iLow].z(), refCorners[iLow].z())
                                                 : std::min(corners[iLow].z(), refCorners[iLow].z());

                  ATH_MSG_VERBOSE("New end points "<<Amg::toString(newTrapBounds[iLow])<<" - "<<Amg::toString(newTrapBounds[iHigh]));
               }  
            }
            if (msgLvl(MSG::VERBOSE)) {
               std::stringstream debugStr{};
               for (const Amg::Vector3D& edge : newTrapBounds) {
                  debugStr<<"***** "<<Amg::toString(edge)<<std::endl;
               }
               ATH_MSG_VERBOSE("#############################################################"<<std::endl<<
                     debugStr.str()<<"#############################################################");
            }
            /// Edge points of the best-known trapezoid
            const double halfY  = 0.5*std::max(newTrapBounds[1].y() - newTrapBounds[0].y(),
                                               newTrapBounds[3].y() - newTrapBounds[2].y());
            const double lHalfX = 0.5*(newTrapBounds[3].x() - newTrapBounds[1].x());
            const double sHalfX = 0.5*(newTrapBounds[2].x() - newTrapBounds[0].x());
            const double halfZ  = 0.5*(newTrapBounds[4].z() - newTrapBounds[0].z());
            envelopeBounds = boundSet.make_bounds(sHalfX, lHalfX, halfY, halfZ);
            ATH_MSG_VERBOSE(m_idHelperSvc->toStringDetEl(chambEle->identify())<<" "<<(*envelopeBounds));
         
            /// Finally re-center the trapezoid 
            const Amg::Transform3D centerShift = centerTrapezoid(newTrapBounds);
            newCentreTrf = centerShift * newCentreTrf;
            ATH_MSG_VERBOSE("New trapezoid centering "<<Amg::toString(centerShift)<<" combined: "
                           <<Amg::toString(newCentreTrf));
      }
      
      envelopeBounds = boundSet.make_bounds(envelopeBounds->get(BoundEnum::eHalfLengthXnegY)+margin,
                                            envelopeBounds->get(BoundEnum::eHalfLengthXposY)+margin,
                                            envelopeBounds->get(BoundEnum::eHalfLengthY)+margin,
                                            envelopeBounds->get(BoundEnum::eHalfLengthZ)+margin);
      return std::make_pair(envelopeBounds, newCentreTrf.inverse());               
}

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

   const std::unordered_set<int> stIndicesEIL{mdtStationIndex("EIL"), mdtStationIndex("T4E")};
   const std::unordered_set<int> stIndicesEM{mdtStationIndex("EML"), mdtStationIndex("EMS"),
                                             tgcStationIndex("T1E"), tgcStationIndex("T1F"),
                                             tgcStationIndex("T2E"), tgcStationIndex("T2F"),
                                             tgcStationIndex("T3E"), tgcStationIndex("T3F")};
   
   const std::set<Identifier> BOE_ids{m_idHelperSvc->hasMDT() ? m_idHelperSvc->mdtIdHelper().elementID("BOL", 7,7): Identifier{},
                                      m_idHelperSvc->hasMDT() ? m_idHelperSvc->mdtIdHelper().elementID("BOL", -7,7): Identifier{},
                                      m_idHelperSvc->hasRPC() ? m_idHelperSvc->rpcIdHelper().elementID("BOL", -8, 7, 1) : Identifier{},
                                      m_idHelperSvc->hasRPC() ? m_idHelperSvc->rpcIdHelper().elementID("BOL", 8, 7, 1) : Identifier{}};

   std::vector<MuonReadoutElement*> allReadOutEles = mgr.getAllReadoutElements();

   std::vector<chamberArgs> envelopeCandidates{};

   /** Group chambers by sectors && station layer. */
   for (const MuonReadoutElement* readOutEle : allReadOutEles) {
      std::vector<chamberArgs>::iterator exist = std::ranges::find_if(envelopeCandidates, 
                         [this, readOutEle, &stIndicesEIL, &stIndicesEM, &BOE_ids](const chamberArgs& args){
                            const MuonReadoutElement* refEle = args.detEles.front();
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
                            return false;// readOutEle->chamberIndex() == refEle->chamberIndex();
                         });
      /// If no chamber has been found, then create a new one
      if (exist == envelopeCandidates.end()) {
         chamberArgs newChamb{};
         newChamb.detEles.push_back(readOutEle);
         envelopeCandidates.push_back(std::move(newChamb));
      } else {
         exist->detEles.push_back(readOutEle);
      }
    }
    /// Find the chamber middle and create the geometry from that
    ActsGeometryContext gctx{};    


   ActsTrk::SurfaceBoundSet<BoundType> boundSet{};    
   for (chamberArgs& candidate : envelopeCandidates) {
         std::unordered_set<Identifier> reIds{};
         const MuonReadoutElement* refEle = candidate.detEles.front();
         const Amg::Transform3D toCenter = axisRotation(refEle->detectorType()) * refEle->globalToLocalTrans(gctx);
         ATH_MSG_VERBOSE("New envelope candidate ");
         const auto [envelopeBox, envelopeCentre] = boundingBox(gctx, candidate.detEles, toCenter, boundSet);
       
         /// Define the spectrometer sector
         SpectrometerSector::defineArgs sectorArgs{};
         sectorArgs.bounds = envelopeBox;
         sectorArgs.locToGlobTrf = toCenter.inverse() * envelopeCentre;
         
         if (!isNsw(candidate.detEles.front())) {
            /** Define the chamber envelopes */
            std::map<PVConstLink, std::vector<const MuonReadoutElement*>> stationMap{};
            /** Sort the readout elements by common parent volume */
            for (const MuonReadoutElement* re : candidate.detEles) {
               stationMap[re->getMaterialGeom()->getParent()].push_back(re);
            }
            /** Create the enclosing chamber volume */
            for (auto& [parent, detEles]: stationMap) {
               const MuonReadoutElement* refEle = detEles.front();
               const Amg::Transform3D toChambCentre = axisRotation(refEle->detectorType()) * refEle->globalToLocalTrans(gctx);
               ATH_MSG_VERBOSE("New chambre candidate "<<m_idHelperSvc->toStringChamber(refEle->identify()));
               const auto[chamberBox, chamberCentre] = boundingBox(gctx, detEles, toChambCentre, boundSet, 0.1*Gaudi::Units::cm);
               chamberArgs chambArgs{};
               chambArgs.detEles =std::move(detEles);
               chambArgs.bounds = chamberBox;
               chambArgs.locToGlobTrf = toChambCentre.inverse() * chamberCentre;
               const Chamber* newChamber {sectorArgs.chambers.emplace_back(std::make_unique<Chamber>(std::move(chambArgs))).get()};
               for (const MuonReadoutElement* re : newChamber->readoutEles()) {
                  reIds.insert(re->identify());
                  mgr.getReadoutElement(re->identify())->setChamberLink(newChamber);
               }
            }
         } else {
               const MuonReadoutElement* refEle = candidate.detEles.front();
            const Amg::Transform3D toChambCentre = axisRotation(refEle->detectorType()) * refEle->globalToLocalTrans(gctx);
            ATH_MSG_VERBOSE("New chambre candidate "<<m_idHelperSvc->toStringChamber(refEle->identify()));
            const auto[chamberBox, chamberCentre] = boundingBox(gctx, candidate.detEles, toChambCentre, boundSet, 0.1*Gaudi::Units::cm);
            chamberArgs chambArgs{};
            chambArgs.detEles = candidate.detEles;
            chambArgs.bounds = chamberBox;
            chambArgs.locToGlobTrf = toChambCentre.inverse() * chamberCentre;
            const Chamber* newChamber {sectorArgs.chambers.emplace_back(std::make_unique<Chamber>(std::move(chambArgs))).get()};
            for (const MuonReadoutElement* re : newChamber->readoutEles()) {
               reIds.insert(re->identify());
               mgr.getReadoutElement(re->identify())->setChamberLink(newChamber);
            }
         }
         std::ranges::sort(sectorArgs.chambers, [](const SpectrometerSector::ChamberPtr& a,
                                                   const SpectrometerSector::ChamberPtr& b) {
                                                      return (*a) < (*b);
                                                   });
         auto globalToSector = sectorArgs.locToGlobTrf.inverse(); 

         /// now, build simplified 2D representations of the sorted chambers we collected. 
         for (auto & chamber : sectorArgs.chambers){
            // split by readout elements - MDT multilayers and trigger chambers 
            for (auto & RE : chamber->readoutEles()){
               // get the center of the element in the sector frame 
               auto chamberToGlobal = RE->localToGlobalTrans(gctx); 
               Amg::Vector3D origin {0, 0,  0}; 
               origin = globalToSector * chamberToGlobal * origin;
               // and then add the bounds of the element - this is technology dependent 
               if (RE->detectorType()==ActsTrk::DetectorType::Mdt){
                     const MuonGMR4::MdtReadoutElement* MDT = dynamic_cast<const MuonGMR4::MdtReadoutElement*>(RE); 
                     sectorArgs.detectorLocs.emplace_back(origin.y() - MDT->getParameters().halfY, 
                                                         origin.y() + MDT->getParameters().halfY, 
                                                         origin.z() - MDT->getParameters().halfHeight, 
                                                         origin.z() + MDT->getParameters().halfHeight, 
                                                         RE); 
               }
               else if (RE->detectorType()==ActsTrk::DetectorType::Rpc){
                     const MuonGMR4::RpcReadoutElement* RPC = dynamic_cast<const MuonGMR4::RpcReadoutElement*>(RE);
                     sectorArgs.detectorLocs.emplace_back(origin.y() - RPC->getParameters().halfLength, 
                                                         origin.y() + RPC->getParameters().halfLength, 
                                                         origin.z() - RPC->getParameters().halfThickness, 
                                                         origin.z() + RPC->getParameters().halfThickness, 
                                                         RE); 
               }
               else if (RE->detectorType()==ActsTrk::DetectorType::Tgc){
                     const MuonGMR4::TgcReadoutElement* TGC = dynamic_cast<const MuonGMR4::TgcReadoutElement*>(RE);
                     sectorArgs.detectorLocs.emplace_back(origin.y() - TGC->getParameters().halfHeight, 
                                                         origin.y() + TGC->getParameters().halfHeight, 
                                                         origin.z() - TGC->getParameters().halfThickness, 
                                                         origin.z() + TGC->getParameters().halfThickness, 
                                                         RE); 
               }
               else if (RE->detectorType() == ActsTrk::DetectorType::Mm){
                   const MuonGMR4::MmReadoutElement* MM = dynamic_cast<const MuonGMR4::MmReadoutElement*>(RE);
                   sectorArgs.detectorLocs.emplace_back(origin.y() - MM->getParameters().halfHeight,
                                                       origin.y() + MM->getParameters().halfHeight,
                                                       origin.z() - MM->getParameters().halfThickness,
                                                       origin.z() + MM->getParameters().halfThickness,
                                                       RE);
               }
            }
         }
         auto newSector = std::make_unique<SpectrometerSector>(std::move(sectorArgs));
         
         for (const Identifier& chId : reIds) {
            mgr.getReadoutElement(chId)->setSectorLink(newSector.get());
         }
         ATH_MSG_VERBOSE("Add new sector "<<(*newSector));
         mgr.addSpectrometerSector(std::move(newSector));
    }
    return StatusCode::SUCCESS;
}

}
#endif
