/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE

#include "ChamberAssembleTool.h"

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <sstream>

#include <Acts/Geometry/CuboidVolumeBounds.hpp>
#include <Acts/Geometry/TrapezoidVolumeBounds.hpp>
#include <Acts/Surfaces/RectangleBounds.hpp>
#include <Acts/Surfaces/TrapezoidBounds.hpp>
#include <Acts/Surfaces/PlaneSurface.hpp>

#include <Acts/Geometry/Volume.hpp>

#include <GeoModelHelpers/TransformToStringConverter.h>
#include <GeoModelKernel/GeoDefinitions.h>

using namespace Acts::UnitLiterals;
namespace {
   /** @brief Project a position in the x-y plane. */
   Amg::Vector3D projectIntoXY(const Amg::Vector3D& v) {
      return Amg::Vector3D{v.x(), v.y(), 0.};
   }
}



namespace MuonGMR4{

using chamberArgs = Chamber::defineArgs;
using VolBoundPtr_t = ChamberAssembleTool::VolBoundPtr_t;
using ChamberPtr = SpectrometerSector::ChamberPtr;

std::string toString(const MuonGMR4::MuonReadoutElement* re) {
   return re->idHelperSvc()->toStringDetEl(re->identify());
}
std::string toString(const ChamberPtr& ch) {
   return ch->identString();
} 

double ChamberAssembleTool::trapezoidEdgeDist(const Amg::Vector3D& linePos,
                                              const Amg::Vector3D& lineDir,
                                              const Amg::Vector3D& testMe,
                                              bool leftEdge) {
   /// Construct the normal pointing inwards
   const Amg::Vector3D normal = lineDir.cross((leftEdge ? 1. : -1.) *Amg::Vector3D::UnitZ());
   const Amg::Vector3D closest = linePos + lineDir.dot(testMe - linePos) * lineDir;
   return normal.dot(testMe - closest);  
}

VolBoundPtr_t
   ChamberAssembleTool::boundingBox(const MuonReadoutElement* chambEle,
                                    Acts::VolumeBoundFactory& volBoundSet) {

   switch(chambEle->detectorType()) {
      case ActsTrk::DetectorType::Mdt: {
         const auto* techEle = static_cast<const MdtReadoutElement*>(chambEle);
         const auto& pars = techEle->getParameters();
         if (std::abs(pars.shortHalfX - pars.longHalfX) < Acts::s_epsilon) {
            return volBoundSet.makeBounds<Acts::CuboidVolumeBounds>(pars.shortHalfX, pars.halfY, pars.halfHeight);
         }
         return volBoundSet.makeBounds<Acts::TrapezoidVolumeBounds>(pars.shortHalfX, pars.longHalfX, 
                                                                    pars.halfY, pars.halfHeight );
      } case ActsTrk::DetectorType::Rpc: {
         const auto* techEle = static_cast<const RpcReadoutElement*>(chambEle);
         const auto& pars = techEle->getParameters();
         return volBoundSet.makeBounds<Acts::CuboidVolumeBounds>(pars.halfWidth, pars.halfLength, pars.halfThickness);
      } case ActsTrk::DetectorType::Tgc: {
         const auto* techEle = static_cast<const TgcReadoutElement*>(chambEle);
         const auto& pars = techEle->getParameters();
         return volBoundSet.makeBounds<Acts::TrapezoidVolumeBounds>(pars.halfWidthShort, pars.halfWidthLong, 
                                                                    pars.halfHeight, pars.halfThickness );
      } case ActsTrk::DetectorType::sTgc: {
         const auto* techEle = static_cast<const sTgcReadoutElement*>(chambEle);
         const auto& pars = techEle->getParameters();
         return volBoundSet.makeBounds<Acts::TrapezoidVolumeBounds>(pars.sHalfChamberLength, pars.lHalfChamberLength, 
                                                                    pars.halfChamberHeight, pars.halfChamberTck );
      } case ActsTrk::DetectorType::Mm: {
         const auto* techEle = static_cast<const MmReadoutElement*>(chambEle);
         const auto& pars = techEle->getParameters();
         return volBoundSet.makeBounds<Acts::TrapezoidVolumeBounds>(pars.halfShortWidth, pars.halfLongWidth, 
                                                                    pars.halfHeight, pars.halfThickness );
      } default:
         THROW_EXCEPTION("Unsupported detector type "<<to_string(chambEle->detectorType()));
   }
   return nullptr;
}
VolBoundPtr_t ChamberAssembleTool::boundingBox(const ChamberPtr& chamber, Acts::VolumeBoundFactory& /*boundSet*/) {
   return chamber->bounds();
}


std::array<Amg::Vector3D, 4> ChamberAssembleTool::cornerPointsPlane(const Amg::Transform3D& localToGlob, 
                                                                    const VolBounds_t& bounds) {
   std::array<Amg::Vector3D,4> planePoints{localToGlob * Amg::Vector3D(-halfXlowY(bounds), - halfY(bounds), 0.),
                                           localToGlob * Amg::Vector3D(-halfXhighY(bounds),  halfY(bounds), 0.),
                                           localToGlob * Amg::Vector3D( halfXlowY(bounds),  -halfY(bounds), 0.),
                                           localToGlob * Amg::Vector3D( halfXhighY(bounds),  halfY(bounds), 0.)};
   return planePoints;
}
std::array<Amg::Vector3D, 8> ChamberAssembleTool::cornerPoints(const Amg::Transform3D& localToGlob, 
                                                               const VolBounds_t& bounds) {
   std::array<Amg::Vector3D, 8> toRet{make_array<Amg::Vector3D,8>(Amg::Vector3D::Zero())};
   const std::array<Amg::Vector3D, 4> plane = cornerPointsPlane(localToGlob, bounds);
   for (unsigned int z : {0, 1}) {
      const Amg::Vector3D stretch = halfZ(bounds) * (z ? 1. : -1.) * Amg::Vector3D::UnitZ();
      for (unsigned int b = 0; b < plane.size(); ++b){
         toRet[b + z * plane.size()] = plane[b] + stretch;
      }
   }
   return toRet;
}
Amg::Transform3D ChamberAssembleTool::centerTrapezoid(const std::array<Amg::Vector3D, 8>& corners) {
   
   static constexpr double maxSize = 200._km;
   double minX{maxSize}, maxX{-maxSize}, minY{maxSize}, maxY{-maxSize}, minZ{maxSize}, maxZ{-maxSize};
   for (const Amg::Vector3D& corner : corners) {
      minX = std::min(corner.x(), minX); maxX = std::max(corner.x(), maxX);
      minY = std::min(corner.y(), minY); maxY = std::max(corner.y(), maxY);
      minZ = std::min(corner.z(), minZ); maxZ = std::max(corner.z(), maxZ);
   }
   return Amg::getTranslate3D(-0.5*(minX + maxX), -0.5*(minY + maxY), -0.5*(minZ + maxZ));
}

VolBoundPtr_t 
   ChamberAssembleTool::enlargeBounds(const VolBounds_t& enlargeMe,
                                      const double margin,
                                      Acts::VolumeBoundFactory& volBoundSet) {
   if (enlargeMe.type() == Acts::VolumeBounds::BoundsType::eCuboid) {
      return volBoundSet.makeBounds<Acts::CuboidVolumeBounds>(halfXlowY(enlargeMe) + 0.5*margin,
                                                              halfY(enlargeMe) + 0.5*margin,
                                                              halfZ(enlargeMe) + 0.5*margin);    
   } else if (enlargeMe.type()  == Acts::VolumeBounds::BoundsType::eTrapezoid) {
      return volBoundSet.makeBounds<Acts::TrapezoidVolumeBounds>(halfXlowY(enlargeMe) + 0.5*margin,
                                                                 halfXhighY(enlargeMe) + 0.5*margin,
                                                                 halfY(enlargeMe) + 0.5*margin,
                                                                 halfZ(enlargeMe) + 0.5*margin);
   }
   return nullptr;
}
ChamberAssembleTool::SurfBoundPtr_t 
   ChamberAssembleTool::surfaceBounds(const VolBounds_t& volBounds,
                                      Acts::SurfaceBoundFactory& surfBoundSet){
   if (volBounds.type() == Acts::VolumeBounds::BoundsType::eCuboid) {
      return surfBoundSet.makeBounds<Acts::RectangleBounds>(halfXlowY(volBounds),  halfY(volBounds));    
   } else if (volBounds.type()  == Acts::VolumeBounds::BoundsType::eTrapezoid) {
      return surfBoundSet.makeBounds<Acts::TrapezoidBounds>(halfXlowY(volBounds) , halfXhighY(volBounds) , halfY(volBounds));
   }
   return nullptr;
}
template <typename ReObjType>
 ChamberAssembleTool::TrfWithBounds 
      ChamberAssembleTool::boundingBox(const ActsTrk::GeometryContext& gctx,
                                       const std::vector<ReObjType>& constituents,
                                       const Amg::Transform3D& toCenter,
                                       Acts::VolumeBoundFactory& volBoundSet,
                                       Acts::SurfaceBoundFactory& surfBoundSet,
                                       const double margin) const 
         requires (Acts::PointerConcept<ReObjType>){

      VolBoundPtr_t envelopeBounds{};
      ATH_MSG_DEBUG("Conrstuct a new "<<typeid(Acts::RemovePointer_t<ReObjType>).name()<<" object.");
      Amg::Transform3D newCentreTrf{Amg::Transform3D::Identity()};
      for (const auto& chambEle :  constituents) {
            Amg::Transform3D trf = newCentreTrf * toCenter * 
                                   chambEle->localToGlobalTransform(gctx);
            VolBoundPtr_t bounds = boundingBox(chambEle, volBoundSet);
            /// Hack to cope with the RPCs which may be rotated by 180 degrees around the x or z-axis in cases,
            /// they're upside down.
            GeoTrf::CoordEulerAngles rotAngles = GeoTrf::getCoordRotationAngles(trf);
            if (std::abs(rotAngles.gamma - 180._degree)< Acts::s_epsilon){
               trf = trf *Amg::getRotateZ3D(180._degree);
            }
            if (std::abs(rotAngles.alpha - 180._degree)< Acts::s_epsilon){
               trf = trf *Amg::getRotateX3D(180._degree);
            }
            
            if (!envelopeBounds) {
               envelopeBounds = bounds;
               newCentreTrf = centerTrapezoid(cornerPoints(trf, *bounds)) * newCentreTrf;
               ATH_MSG_VERBOSE("Envelope "<<toString(chambEle)<<", "<<(*envelopeBounds)<<", transform: "<<Amg::toString(newCentreTrf));
               continue;
            }
            /// Check whether the bounds are already embedded in the trapezoid
            const std::array<Amg::Vector3D, 8> corners = cornerPoints(trf, *bounds);

            /// Everything is contained in the volume 
            if (std::ranges::none_of(corners, [&envelopeBounds](const Amg::Vector3D& v) { 
                  return !envelopeBounds->inside(v); 
               })) {
               ATH_MSG_VERBOSE("Element "<<toString(chambEle)<<" "
                           <<(*boundingBox(chambEle, volBoundSet))<<" fully contained. ");
               continue;
            }
            if (msgLvl(MSG::VERBOSE)) {
               std::stringstream debugStr{};
               for (const Amg::Vector3D& corner : corners) {
                  debugStr<<"   ***** "<<Amg::toString(corner)<<std::endl;
               }
               ATH_MSG_VERBOSE(toString(chambEle)<<" corner points "<<GeoTrf::toString(trf, true)<<std::endl<<debugStr.str());   
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
                                    <<" "<<dirRef.phi() / Gaudi::Units::deg <<" --- "<<toString(chambEle)<<" "
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
            const double lHalfX = 0.5*std::abs(newTrapBounds[3].x() - newTrapBounds[1].x());
            const double sHalfX = 0.5*std::abs(newTrapBounds[2].x() - newTrapBounds[0].x());
            const double halfZ  = 0.5*std::abs(newTrapBounds[4].z() - newTrapBounds[0].z());
            ATH_MSG_VERBOSE("New bounds "<<sHalfX<<"/"<<lHalfX<<", y:"<<halfY<<", "<<halfZ);
            if (std::abs(lHalfX - sHalfX) > Acts::s_epsilon) {
               envelopeBounds = volBoundSet.makeBounds<Acts::TrapezoidVolumeBounds>(sHalfX, lHalfX, halfY, halfZ);
            } else {
               envelopeBounds = volBoundSet.makeBounds<Acts::CuboidVolumeBounds>(sHalfX, halfY, halfZ);
            }
            ATH_MSG_VERBOSE(toString(chambEle)<<" "<<(*envelopeBounds));
         
            /// Finally re-center the trapezoid 
            const Amg::Transform3D centerShift = centerTrapezoid(newTrapBounds);
            newCentreTrf = centerShift * newCentreTrf;
            ATH_MSG_VERBOSE("New trapezoid centering "<<Amg::toString(centerShift)<<" combined: "
                           <<Amg::toString(newCentreTrf));
      }
      ATH_MSG_DEBUG("Done");
      return std::make_tuple(newCentreTrf.inverse(), 
                             enlargeBounds(*envelopeBounds, margin, volBoundSet), 
                             surfaceBounds(*envelopeBounds, surfBoundSet));               
}

StatusCode ChamberAssembleTool::buildReadOutElements(MuonDetectorManager &mgr) {
   ATH_CHECK(m_idHelperSvc.retrieve());
   /// TGC T4E chambers  & Mdt EIL chambers are glued together
   auto mdtStationIndex = [this] (const std::string& stName) {
      return m_idHelperSvc->hasMDT() ? m_idHelperSvc->mdtIdHelper().stationNameIndex(stName) : -1;
   };
   auto tgcStationIndex = [this] (const std::string& stName) {
      return m_idHelperSvc->hasTGC() ? m_idHelperSvc->tgcIdHelper().stationNameIndex(stName) : -1;   
   };
   
   const std::unordered_set<int> stIndicesEM{mdtStationIndex("EML"), mdtStationIndex("EMS"),
                                             tgcStationIndex("T1E"), tgcStationIndex("T1F"),
                                             tgcStationIndex("T2E"), tgcStationIndex("T2F"),
                                             tgcStationIndex("T3E"), tgcStationIndex("T3F")};


   std::unordered_set<Identifier> BIS78_ids{};
   auto fillBIS78 = [&BIS78_ids, this](const MuonIdHelper& idHelper) {
      const int BIS = idHelper.stationNameIndex("BIS");
      std::copy_if(idHelper.detectorElement_begin(), idHelper.detectorElement_end(), std::inserter(BIS78_ids, BIS78_ids.end()), 
                  [&](const Identifier& detId){
                     int stEta = idHelper.stationEta(detId);
                     if (m_isRun4) {
                        stEta = std::abs(stEta);
                     }
                     return stEta >= 7 && idHelper.stationName(detId) == BIS;
                  });
   };
   if (m_idHelperSvc->hasMDT()) {
      fillBIS78(m_idHelperSvc->mdtIdHelper());
   }
   if (m_idHelperSvc->hasRPC()) {
      fillBIS78(m_idHelperSvc->rpcIdHelper());
   }
   
   std::vector<MuonReadoutElement*> allReadOutEles = mgr.getAllReadoutElements();

   std::vector<chamberArgs> envelopeCandidates{};

   /** Group chambers by sectors && station layer. */
   for (const MuonReadoutElement* readOutEle : allReadOutEles) {
      std::vector<chamberArgs>::iterator exist = std::ranges::find_if(envelopeCandidates, 
                         [this, readOutEle, &stIndicesEM](const chamberArgs& args){
                           const MuonReadoutElement* refEle = args.detEles.front();
                           const Identifier refId = refEle->identify();
                           const Identifier testId = readOutEle->identify();
                           /// Check that the two readout elements are on the same side.
                           /// Exception BOG eta 0 -> attributes to positive sectors
                           if (Acts::copySign(1, refEle->stationEta()) * 
                               Acts::copySign(1, readOutEle->stationEta()) < 0) {
                                 return false;
                           }
                           /// The two readout elements shall be located in the same sector
                           if (m_idHelperSvc->sector(testId) != m_idHelperSvc->sector(refId)) {
                              return false;
                           }
                           if (stIndicesEM.count(readOutEle->stationName()) &&
                               stIndicesEM.count(refEle->stationName())) {
                              return true;
                           }
                           // /// Summarize all readout element in the same sector & layer
                           /// into a single chamber
                           return readOutEle->chamberIndex() == refEle->chamberIndex();
                         });
      /// If no chamber has been found, then create a new one
      if (exist == envelopeCandidates.end()) {
         ATH_MSG_VERBOSE("Open envelope "<<(envelopeCandidates.size()+1)
               <<" for "<<m_idHelperSvc->toStringDetEl(readOutEle->identify())
               <<", sector: "<<m_idHelperSvc->sector(readOutEle->identify())
               <<", chIdx: "<<Muon::MuonStationIndex::chName(readOutEle->chamberIndex()));
         envelopeCandidates.emplace_back().detEles.push_back(readOutEle);
      } else {
         ATH_MSG_VERBOSE("Attach "<<m_idHelperSvc->toStringDetEl(readOutEle->identify())
               <<", sector: "<<m_idHelperSvc->sector(readOutEle->identify())
               <<", chIdx: "<<Muon::MuonStationIndex::chName(readOutEle->chamberIndex())<<" to envelope "
            <<(std::distance(envelopeCandidates.begin(), exist) + 1)<<". ");
         exist->detEles.push_back(readOutEle);
      }
    }
    /// Find the chamber middle and create the geometry from that
    ActsTrk::GeometryContext gctx{};    


   Acts::VolumeBoundFactory volBoundSet{};
   Acts::SurfaceBoundFactory surfBoundSet{};  
   unsigned candId{0}; 
   for (chamberArgs& candidate : envelopeCandidates) {
         std::unordered_set<Identifier> reIds{};

         ATH_MSG_VERBOSE("New envelope candidate ");
         /// Define the spectrometer sector
         SpectrometerSector::defineArgs sectorArgs{};
         sectorArgs.id = (++candId);
         using namespace Muon::MuonStationIndex;
         std::vector<std::vector<const MuonReadoutElement*>> chamberElements{};
         if (toStationIndex(candidate.detEles.front()->chamberIndex()) != StIndex::EI) {
            /** Define the chamber envelopes */
            std::map<PVConstLink, std::vector<const MuonReadoutElement*>> stationMap{};
            /** Sort the readout elements by common parent volume */
            for (const MuonReadoutElement* re : candidate.detEles) {
               /// The BIS 7/8 chamber needs to be comparted into 4 subvolumes, each containing a single
               /// readout element to minimize the impact from the overlap between BIS78 & EIL-4 (TGC)
               if (BIS78_ids.count(re->identify())) {
                  stationMap[re->getMaterialGeom()].push_back(re);
               } else {
                  stationMap[re->getMaterialGeom()->getParent()].push_back(re);
               }
            }
            for (auto& [parent, stationEles ]: stationMap){
               chamberElements.push_back(std::move(stationEles));
           }
         } else {
            /// We need to ensure that the EIL 4/5 are split into a separate volume w.r.t.
            /// the New Small Wheel
            std::vector<std::vector<const MuonReadoutElement*>> endcapEles(2);

            for (const MuonReadoutElement* sortMe : candidate.detEles){
               if (sortMe->detectorType() == ActsTrk::DetectorType::Tgc) {
                  endcapEles.emplace_back(1, sortMe);
               } else {
                  endcapEles[sortMe->detectorType() == ActsTrk::DetectorType::sTgc ||
                             sortMe->detectorType() == ActsTrk::DetectorType::Mm].push_back(sortMe);
               }
            }
            for (auto& stationEles : endcapEles) {
               if (!stationEles.empty()) {
                  chamberElements.push_back(std::move(stationEles));
               }
            }
         }

         for (auto& detEles: chamberElements) {
            const MuonReadoutElement* refEle = detEles.front();
            const Amg::Transform3D toChambCentre = refEle->globalToLocalTransform(gctx);
            ATH_MSG_VERBOSE("New chamber candidate "<<m_idHelperSvc->toStringChamber(refEle->identify()));
            const auto[chamberCentre, chamberBox, planeBounds] = boundingBox(gctx, detEles, toChambCentre, volBoundSet, 
                                                                              surfBoundSet, 0.*Gaudi::Units::cm);
            chamberArgs chambArgs{};
            chambArgs.detEles = std::move(detEles);
            chambArgs.bounds = chamberBox;
            chambArgs.surface = Acts::Surface::makeShared<Acts::PlaneSurface>(toChambCentre.inverse() * chamberCentre, planeBounds);
            const Chamber* newChamber {sectorArgs.chambers.emplace_back(std::make_unique<Chamber>(std::move(chambArgs))).get()};
            for (const MuonReadoutElement* re : newChamber->readoutEles()) {
               reIds.insert(re->identify());
               mgr.getReadoutElement(re->identify())->setChamberLink(newChamber);
               ATH_MSG_VERBOSE("Chamber element: "<<m_idHelperSvc->toStringDetEl(re->identify()));
                  
            }
            ATH_MSG_VERBOSE("Created new chamber: "<<(*newChamber));
         }
         std::ranges::sort(sectorArgs.chambers, [](const SpectrometerSector::ChamberPtr& a,
                                                   const SpectrometerSector::ChamberPtr& b) {
                                                      return (*a) < (*b);
                                                   });

         const Amg::Transform3D toCenter = sectorArgs.chambers.front()->globalToLocalTransform(gctx);
         const auto [envelopeCentre, envelopeBox, envelopePlane] = boundingBox(gctx, sectorArgs.chambers, toCenter, 
                                                                               volBoundSet, surfBoundSet, 2.* Gaudi::Units::cm);

         sectorArgs.bounds = envelopeBox;
         sectorArgs.surface = Acts::Surface::makeShared<Acts::PlaneSurface>(toCenter.inverse() * envelopeCentre, envelopePlane);

         const Amg::Transform3D globalToSector = sectorArgs.surface->transform(gctx.context()).inverse();

         /// now, build simplified 2D representations of the sorted chambers we collected. 
         for (auto & chamber : sectorArgs.chambers){
            // split by readout elements - MDT multilayers and trigger chambers for the sector
            for (auto & RE : chamber->readoutEles()){
               // get the center of the element in the sector frame 
               const Amg::Transform3D& chamberToGlobal{RE->localToGlobalTransform(gctx)}; 
               const Amg::Vector3D origin = (globalToSector * chamberToGlobal).translation();
               // and then add the bounds of the element - this is technology dependent 
               sectorArgs.detectorLocs.emplace_back(origin, RE, boundingBox(RE, volBoundSet));
            }
         }
         /// Now construct the sector volume from the chamber envelopes
         auto newSector = std::make_unique<SpectrometerSector>(std::move(sectorArgs));
         
         for (const Identifier& chId : reIds) {
            mgr.getReadoutElement(chId)->setSectorLink(newSector.get());
         }
         mgr.addSpectrometerSector(std::move(newSector));
    }
    return StatusCode::SUCCESS;
}

}
#endif
