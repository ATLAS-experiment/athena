/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ChamberAssembleTool.h"

#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"

#include "Acts/Geometry/CuboidVolumeBounds.hpp"
#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Geometry/Volume.hpp"
#include "Acts/Surfaces/detail/PlanarHelper.hpp"

#include "GeoModelHelpers/TransformToStringConverter.h"
#include "GeoModelKernel/GeoDefinitions.h"

#include "MuonReadoutGeometryR4/TrapezoidBoundsExpander.h"

#include <sstream>


using namespace Acts::UnitLiterals;
using namespace Acts::PlanarHelper;

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
                                       const Acts::Transform3& toCenter,
                                       Acts::VolumeBoundFactory& volBoundSet,
                                       Acts::SurfaceBoundFactory& surfBoundSet,
                                       const double margin) const 
         requires (Acts::PointerConcept<ReObjType>){
      const Acts::GeometryContext tgContext{gctx.context()};
      TrapezoidBoundsExpander expandBounds{toCenter};
      expandBounds.setMargin(margin);
      ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Construct a new bounding box from "<<typeid(ReObjType).name()
                  <<" spanning over "<<constituents.size()<<" elements, align trf: "
                  <<Amg::toString(toCenter));
      for (const auto& element : constituents) {
         ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Append new element "<<toString(element));
         if constexpr(std::is_base_of_v<MuonReadoutElement, Acts::RemovePointer_t<ReObjType>>) {
            expandBounds.expand(tgContext, *element, volBoundSet);
         } else {
            expandBounds.expand(tgContext, *element);
         }
      }
      auto volBounds = expandBounds.makeBounds(volBoundSet);
      ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Box centered "
         <<Amg::toString(expandBounds.boxMidPoint())<<" "<<(*volBounds));
      return std::make_tuple(expandBounds.boxMidPoint(), volBounds, 
                             surfaceBounds(*volBounds, surfBoundSet));
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
         ATH_MSG_VERBOSE(__func__<<" () "<<__LINE__<<" - Open envelope "<<(envelopeCandidates.size()+1)
               <<" for "<<m_idHelperSvc->toStringDetEl(readOutEle->identify())
               <<", sector: "<<m_idHelperSvc->sector(readOutEle->identify())
               <<", chIdx: "<<Muon::MuonStationIndex::chName(readOutEle->chamberIndex()));
         envelopeCandidates.emplace_back().detEles.push_back(readOutEle);
      } else {
         ATH_MSG_VERBOSE(__func__<<" () "<<__LINE__<<" - Attach "<<m_idHelperSvc->toStringDetEl(readOutEle->identify())
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

         ATH_MSG_DEBUG(__func__<<" () "<<__LINE__<<" - New envelope candidate ");
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
            const Acts::Transform3 toChambCentre = Amg::toIsometry3D(refEle->globalToLocalTransform(gctx));
            ATH_MSG_DEBUG(__func__<<" () "<<__LINE__<<" - New chamber candidate "<<m_idHelperSvc->toStringChamber(refEle->identify()));
            const auto[chamberCentre, chamberBox, planeBounds] = boundingBox(gctx, detEles, toChambCentre, volBoundSet, 
                                                                              surfBoundSet, 0.1*Gaudi::Units::cm);
            chamberArgs chambArgs{};
            chambArgs.detEles = std::move(detEles);
            chambArgs.bounds = chamberBox;
            chambArgs.placement = std::make_unique<ActsTrk::VolumePlacement>(*refEle, chamberCentre);
            auto surface = Acts::Surface::makeShared<Acts::PlaneSurface>(Acts::Transform3::Identity(), planeBounds);
            chambArgs.placement->connectCenterSurface(surface);
            chambArgs.surface = surface;
            const Chamber* newChamber {sectorArgs.chambers.emplace_back(std::make_unique<Chamber>(std::move(chambArgs))).get()};
            for (const MuonReadoutElement* re : newChamber->readoutEles()) {
               reIds.insert(re->identify());
               mgr.getReadoutElement(re->identify())->setChamberLink(newChamber);
               ATH_MSG_VERBOSE(__func__<<" () "<<__LINE__<<" - Chamber element: "<<m_idHelperSvc->toStringDetEl(re->identify()));
                  
            }
            ATH_MSG_DEBUG(__func__<<" () "<<__LINE__<<" - Created new chamber: "<<(*newChamber));
         }
         if (sectorArgs.chambers.empty()) {
            continue;
         }
         std::ranges::sort(sectorArgs.chambers, [](const SpectrometerSector::ChamberPtr& a,
                                                   const SpectrometerSector::ChamberPtr& b) {
                                                      return (*a) < (*b);
                                                   });

         const Acts::Transform3 toCenter = sectorArgs.chambers.front()->globalToLocalTransform(gctx);
         const auto [envelopeCentre, envelopeBox, envelopePlane] = boundingBox(gctx, sectorArgs.chambers, toCenter, 
                                                                               volBoundSet, surfBoundSet, 2.* Gaudi::Units::cm);

         sectorArgs.bounds = envelopeBox;
         sectorArgs.localToGlobalTrf = make_intrusive<GeoAlignableTransform>(toCenter.inverse() * envelopeCentre);
         /// Temporarily use the CSC to avoid reporting issues with the alignment algorithms
         sectorArgs.placement = std::make_unique<ActsTrk::VolumePlacement>(ActsTrk::DetectorType::Csc,
                                                                           sectorArgs.localToGlobalTrf);

         auto surface = Acts::Surface::makeShared<Acts::PlaneSurface>(Acts::Transform3::Identity(), envelopePlane);
         sectorArgs.placement->connectCenterSurface(surface);
         sectorArgs.surface = surface ;
         const Acts::Transform3 globalToSector = sectorArgs.placement->globalToLocalTransform(gctx.context());

         /// now, build simplified 2D representations of the sorted chambers we collected. 
         for (auto & chamber : sectorArgs.chambers){
            // split by readout elements - MDT multilayers and trigger chambers for the sector
            for (auto & RE : chamber->readoutEles()){
               // get the center of the element in the sector frame 
               const Acts::Transform3& chamberToGlobal{RE->localToGlobalTransform(gctx)}; 
               const Amg::Vector3D origin = (globalToSector * chamberToGlobal).translation();
               // and then add the bounds of the element - this is technology dependent 
               sectorArgs.detectorLocs.emplace_back(origin, RE, BoundsExpander::makeBounds(*RE, volBoundSet));
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