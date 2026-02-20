/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>

#include <AthenaBaseComps/AthCheckMacros.h>
#include <GaudiKernel/SystemOfUnits.h>

#ifndef SIMULATIONBASE
#   include "Acts/Surfaces/TrapezoidBounds.hpp"
#   include "Acts/Surfaces/Surface.hpp"
#endif

using namespace ActsTrk;

namespace MuonGMR4 {
using parameterBook = sTgcReadoutElement::parameterBook;
std::ostream& operator<<(std::ostream& ostr, const parameterBook& pars) {
   ostr<<"sTGC parameter halfX (S/L):"<<pars.sHalfChamberLength<<"/"<<pars.lHalfChamberLength
         <<", halfY: "<<pars.halfChamberHeight<<", thickness: "<<pars.halfChamberTck;
   return ostr;
}
sTgcReadoutElement::~sTgcReadoutElement() = default;
sTgcReadoutElement::sTgcReadoutElement(defineArgs&& args)
    : MuonReadoutElement(args),
      m_pars{std::move(args)} {
}

const parameterBook& sTgcReadoutElement::getParameters() const { return m_pars; }

StatusCode sTgcReadoutElement::initElement() {
   ATH_MSG_DEBUG("Parameter book "<<getParameters());

   ATH_CHECK(createGeoTransform());
#ifndef SIMULATIONBASE
      ATH_CHECK(planeSurfaceFactory(geoTransformHash(), 
                     m_pars.layerBounds->makeBounds<Acts::TrapezoidBounds>(m_pars.sHalfChamberLength, 
                                                                           m_pars.lHalfChamberLength, 
                                                                           m_pars.halfChamberHeight)));
#endif

   if (m_pars.stripLayers.empty()) {
      ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" doesn't have any layers defined");
      return StatusCode::FAILURE;
   }
   for (unsigned layer = 0; layer < m_pars.stripLayers.size(); ++layer) {
      IdentifierHash layHash{layer};
      if (gasGapNumber(m_pars.stripLayers[layer].hash()) != layHash) {
         ATH_MSG_FATAL("Layer "<<m_pars.stripLayers[layer]<<" has a very strange hash. Expect "<<layer);
         return StatusCode::FAILURE;
      }
      ATH_CHECK(insertTransform<sTgcReadoutElement>(m_pars.stripLayers[layer].hash()));
      
#ifndef SIMULATIONBASE
      const StripDesign& design{m_pars.stripLayers[layer].design()};
      ATH_CHECK(planeSurfaceFactory(m_pars.stripLayers[layer].hash(), 
                                    m_pars.layerBounds->makeBounds<Acts::TrapezoidBounds>(design.shortHalfHeight(), 
                                                                                          design.longHalfHeight(), 
                                                                                          design.halfWidth(),
                                                                                          90.*Gaudi::Units::deg)));
#endif

   }
   m_gasGapPitch = (fromGapToChamOrigin(createHash(1, sTgcIdHelper::sTgcChannelTypes::Strip, 0)).translation() -
                    fromGapToChamOrigin(createHash(2, sTgcIdHelper::sTgcChannelTypes::Strip, 0)).translation()).mag(); 
   return StatusCode::SUCCESS;
}

Amg::Transform3D sTgcReadoutElement::fromGapToChamOrigin(const IdentifierHash& measHash) const{
   return stripLayer(measHash).toOrigin();  
}

Amg::Vector2D sTgcReadoutElement::localChannelPosition(const IdentifierHash& measHash) const {
   switch (chType(measHash)) {
      case ReadoutChannelType::Strip: {
         Amg::Vector2D stripCenter{Amg::Vector2D::Zero()};
         const StripDesign& design{stripDesign(measHash)};
         const int ch = channelNumber(measHash);

         std::optional<Amg::Vector2D> stripCenterOpt = design.center(ch);
         if (!stripCenterOpt) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The strip " << ch << " doesn't intersect with the edges of the trapezoid.");
            return stripCenter;
         }
         ATH_MSG_VERBOSE("Fetch local strip position "<<idHelperSvc()->toString(measurementId(measHash))<<" "
                        <<" "<<Amg::toString(*stripCenterOpt)<<" "<<design);
         stripCenter = std::move(*stripCenterOpt);
         if (ch == 1 && firstStripPitch(measHash) < 0.75 * design.stripPitch()) {
            stripCenter.x() += 0.25 * design.stripWidth();
         }
         if (ch == design.numStrips() && firstStripPitch(measHash) > 0.75 * design.stripPitch()) {
            stripCenter.x() -= 0.25 * design.stripWidth();
         }
         return stripCenter;
      } case  ReadoutChannelType::Wire: {
         Amg::Vector2D wireGroupCenter{Amg::Vector2D::Zero()};
         const WireGroupDesign& design{wireDesign(measHash)};
         const int ch = channelNumber(measHash);
         std::optional<Amg::Vector2D> wireGroupCenterOpt = design.center(ch);
         if (!wireGroupCenterOpt) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The wireGroup" << ch 
                        << "doesn't intersect with the edges of the trapezoid.");
            return wireGroupCenter;
         }
         wireGroupCenter = std::move(*wireGroupCenterOpt);
         ATH_MSG_VERBOSE("Fetch local wire position "<<idHelperSvc()->toString(measurementId(measHash))<<" "
                        <<" "<<Amg::toString(wireGroupCenter)<<" "<<design);
         if (ch == 1) {
            ATH_MSG_DEBUG("The first wiregroup width is " <<design.numWiresInGroup(ch) << " firstWirePos: " << design.firstStripPos());
            ATH_MSG_DEBUG("The last wire pos is: " << wireGroupCenter.x() + (0.5 * (design.numWiresInGroup(ch) + 1) - 1) * design.stripPitch() );
            /// Shifting the first wireGroup center to the last wire of the first wireGroup
            wireGroupCenter.x() = wireGroupCenter.x() + (0.5 * (design.numWiresInGroup(ch) + 1) - 1) * design.stripPitch();
            /// Defining the wireGroup center as the mean of the position of the last wire in the first group
            /// and the left edge of the active area defined for pads to match the R3 description
            wireGroupCenter.x() = 0.5 * (wireGroupCenter.x() - design.longHalfHeight());
         } else if (ch == design.numStrips()) {
            ATH_MSG_VERBOSE("The actual center of the last wire group is: " << wireGroupCenter.x());
            /// Shifting the last wireGroup center to the last wire of the second-last wireGroup
            wireGroupCenter.x() = wireGroupCenter.x() - 0.5 * (design.numWiresInGroup(ch) + 1) * design.stripPitch();
            ATH_MSG_VERBOSE("The last wire of the last second group is at: " << wireGroupCenter.x());
            /// Defining the wireGroup center as the mean of the position of the last wire in the second last group
            /// and the right edge of the active area defined for pads to match the R3 description
            wireGroupCenter.x() = 0.5 * (wireGroupCenter.x() + design.longHalfHeight());
         }
         /// In R3, the center of the normal wireGroup is defined on the 10th wire, whereas, in R4
         /// the center is at the center of the wireGroup (between 10th and 11th wire). 
         return wireGroupCenter;
      } case  ReadoutChannelType::Pad: {
         std::optional<Amg::Vector2D> padCenterOpt = padDesign(measHash).stripPosition(channelNumber(measHash));
         if (!padCenterOpt) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The pad" << channelNumber(measHash) 
                           << "doesn't is not a valid pad number.");
            return Amg::Vector2D::Zero();
         }
         return padCenterOpt.value();
   }  default:
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<"Invalid channel type: " << chType(measHash));
      return Amg::Vector2D::Zero();
   }
}

Amg::Vector3D sTgcReadoutElement::localPosition(const IdentifierHash& measHash) const {
   return stripLayer(measHash).to3D(localChannelPosition(measHash),
                                    chType(measHash) != ReadoutChannelType::Strip);
}

Amg::Vector3D sTgcReadoutElement::globalChannelPosition(const ActsTrk::GeometryContext& ctx, 
                                                        const IdentifierHash& measHash) const {
   return localToGlobalTransform(ctx, layerHash(measHash)) * localPosition(measHash);
}

using localCornerArray = std::array<Amg::Vector2D, 4>;
using globalCornerArray = std::array<Amg::Vector3D, 4>;
globalCornerArray sTgcReadoutElement::globalPadCorners(const ActsTrk::GeometryContext& ctx, 
                                                       const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   if (chType(measHash) == ReadoutChannelType::Pad && gasGapNumber(measHash) < m_pars.padLayers.size()) {
      globalCornerArray gPadCorners{make_array<Amg::Vector3D, 4>(Amg::Vector3D::Zero())};
      const auto& layer = stripLayer(lHash);
      localCornerArray lPadCorners = localPadCorners(measHash);
      for (unsigned corner = 0; corner < lPadCorners.size(); ++corner) {
         gPadCorners[corner] = localToGlobalTransform(ctx, lHash)* layer.to3D(std::move(lPadCorners[corner]), true);
      }
      return gPadCorners;
   }
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<lHash
                 <<" is out of range. Maximum range "<<m_pars.padLayers.size());
   return make_array<Amg::Vector3D, 4>(Amg::Vector3D::Zero());
}
   
int sTgcReadoutElement::padNumber(const Amg::Vector2D& hitPos, const IdentifierHash& measHash) const {
   const auto& design{padDesign(measHash)};
   int padEta = design.channelNumber(hitPos).first;
   int padPhi = design.channelNumber(hitPos).second;  
   const Identifier padID = m_idHelper.padID(identify(), multilayer(), gasGapNumber(measHash) + 1, chType(measHash),
                                                padEta, padPhi);
   return m_idHelper.channel(padID);
}

Amg::Vector3D sTgcReadoutElement::leftStripEdge(const ActsTrk::GeometryContext& ctx, const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);  
   switch (chType(measHash)) {
      case ReadoutChannelType::Strip:
      case ReadoutChannelType::Wire:      
         return localToGlobalTransform(ctx, lHash) *
                stripLayer(measHash).localStripLeftEdge(channelNumber(measHash), 
                                                        chType(measHash) == ReadoutChannelType::Wire);
      default:
         break;
   }  
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<lHash
                <<" is not valid Type "<< chType(measHash));
   return Amg::Vector3D::Zero();
   
}


Amg::Vector3D sTgcReadoutElement::rightStripEdge(const ActsTrk::GeometryContext& ctx, const IdentifierHash& measHash) const {
     const IdentifierHash lHash = layerHash(measHash);  
   switch (chType(measHash)) {
      case ReadoutChannelType::Strip:
      case ReadoutChannelType::Wire:      
         return localToGlobalTransform(ctx, lHash) *
                stripLayer(measHash).localStripRightEdge(channelNumber(measHash), 
                                                         chType(measHash) == ReadoutChannelType::Wire);
      default:
         break;
   }  
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<lHash
                <<" is not valid Type "<< chType(measHash));
   return Amg::Vector3D::Zero(); 

   
}

bool sTgcReadoutElement::isEtaZero(const IdentifierHash& measurementHash, const Amg::Vector2D& localPosition) const {
   if(std::abs(stationEta()) != 1 ) {
      return false; // if we are not in a Q1 ro element we do not have to check further
   }
   const WireGroupDesign& design = wireDesign(measurementHash); // function is not checking for channel type so we just use its gas gap info
   return localPosition.x() < design.halfWidth() - design.wireCutout();
}


}  // namespace MuonGMR4
