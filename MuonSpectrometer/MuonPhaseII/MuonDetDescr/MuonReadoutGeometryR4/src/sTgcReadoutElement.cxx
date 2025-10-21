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
  if (pars.stripDesign) ostr<<"Strips: "<<(*pars.stripDesign)<<std::endl;
  if (pars.wireGroupDesign) ostr<<"Wire Groups: "<<(*pars.wireGroupDesign)<<std::endl;   
  if (pars.padDesign) ostr<<"Pads: "<<(*pars.padDesign)<<std::endl;   
  return ostr;
}
sTgcReadoutElement::~sTgcReadoutElement() = default;
sTgcReadoutElement::sTgcReadoutElement(defineArgs&& args)
    : MuonReadoutElement(args),
      m_pars{std::move(args)} {
}

const parameterBook& sTgcReadoutElement::getParameters() const {return m_pars;}

StatusCode sTgcReadoutElement::initElement() {
   ATH_MSG_DEBUG("Parameter book "<<parameterBook());

   ATH_CHECK(createGeoTransform());
#ifndef SIMULATIONBASE

      ATH_CHECK(planeSurfaceFactory(geoTransformHash(), 
                     m_pars.layerBounds->makeBounds<Acts::TrapezoidBounds>(m_pars.sHalfChamberLength, 
                                                                           m_pars.lHalfChamberLength, 
                                                                           m_pars.halfChamberHeight)));
#endif

   if (m_pars.stripLayers.empty() || m_pars.wireGroupLayers.empty()) {
      ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" doesn't have any layers defined");
      return StatusCode::FAILURE;
   }
   for (unsigned int layer = 0; layer < m_pars.stripLayers.size(); ++layer) {
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
   for (unsigned int layer = 0; layer < m_pars.wireGroupLayers.size(); ++layer) {
      IdentifierHash layHash{layer};
      if (gasGapNumber(m_pars.wireGroupLayers[layer].hash()) != layHash) {
         ATH_MSG_FATAL("Layer "<<m_pars.wireGroupLayers[layer]<<" has a very strange hash. Expect "<<layer);
         return StatusCode::FAILURE;
      }
      ATH_CHECK(insertTransform<sTgcReadoutElement>(m_pars.wireGroupLayers[layer].hash()));
#ifndef SIMULATIONBASE
      const StripDesign& design{m_pars.wireGroupLayers[layer].design()};
      ATH_CHECK(planeSurfaceFactory(m_pars.wireGroupLayers[layer].hash(), 
                                    m_pars.layerBounds->makeBounds<Acts::TrapezoidBounds>(design.shortHalfHeight(), 
                                                                                          design.longHalfHeight(), 
                                                                                          design.halfWidth())));
#endif
   }
   for (unsigned int layer = 0; layer < m_pars.padLayers.size(); ++layer) {
      IdentifierHash layHash{layer};
      if (gasGapNumber(m_pars.padLayers[layer].hash()) != layHash) {
         ATH_MSG_FATAL("Layer "<<m_pars.padLayers[layer]<<" has a very strange hash. Expect "<<layer);
       return StatusCode::FAILURE;
      }
      ATH_CHECK(insertTransform<sTgcReadoutElement>(m_pars.padLayers[layer].hash()));
#ifndef SIMULATIONBASE
      const StripDesign& design{m_pars.padLayers[layer].design()};
      ATH_CHECK(planeSurfaceFactory(m_pars.padLayers[layer].hash(), 
                                    m_pars.layerBounds->makeBounds<Acts::TrapezoidBounds>(design.shortHalfHeight(), 
                                                                                          design.longHalfHeight(), 
                                                                                          design.halfWidth())));
#endif

   }
   ActsTrk::GeometryContext gctx{};
   m_gasGapPitch = (center(gctx, createHash(1, sTgcIdHelper::sTgcChannelTypes::Strip, 0)) -
                    center(gctx, createHash(2, sTgcIdHelper::sTgcChannelTypes::Strip, 0))).mag(); 
   return StatusCode::SUCCESS;
}

Amg::Transform3D sTgcReadoutElement::fromGapToChamOrigin(const IdentifierHash& measHash) const{
   return stripLayer(measHash).toOrigin();  
}

Amg::Vector2D sTgcReadoutElement::localChannelPosition(const IdentifierHash& measHash) const {
   if (chType(measHash) == ReadoutChannelType::Strip) {
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
   } else if (chType(measHash) == ReadoutChannelType::Wire) {
      Amg::Vector2D wireGroupCenter{Amg::Vector2D::Zero()};
      const WireGroupDesign& design{wireDesign(measHash)};
      const int ch = channelNumber(measHash);
      std::optional<Amg::Vector2D> wireGroupCenterOpt = design.center(ch);
      if (!wireGroupCenterOpt) {
         ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The wireGroup" << ch << "doesn't intersect with the edges of the trapezoid.");
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
   }
   else if (chType(measHash) == ReadoutChannelType::Pad) {
      Amg::Vector2D padCenter{Amg::Vector2D::Zero()};
      std::optional<Amg::Vector2D> padCenterOpt = padDesign(measHash).stripPosition(channelNumber(measHash));
      if (!padCenterOpt) {
         ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The pad" << channelNumber(measHash) << "doesn't is not a valid pad number.");
         return padCenter;
      }
      padCenter = std::move(*padCenterOpt);
      return padCenter;
   }
   else {
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<"Invalid channel type: " << chType(measHash));
      return Amg::Vector2D::Zero();
   }
}

Amg::Vector3D sTgcReadoutElement::globalChannelPosition(const ActsTrk::GeometryContext& ctx, const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   unsigned int layIdx = static_cast<unsigned int>(lHash);
   unsigned int gasGap = gasGapNumber(measHash);
   if((chType(measHash) < ReadoutChannelType::Pad || chType(measHash) > ReadoutChannelType::Wire) && gasGap < m_pars.padLayers.size()) {
         ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The channel type "<<chType(measHash)
                  <<"with the layer hash "<<layIdx<<" is invalid. Maximum range "<<m_pars.stripLayers.size());
         return Amg::Vector3D::Zero();
   }
   Amg::Vector3D channelPos{Amg::Vector3D::Zero()};
   Amg::Vector2D localChannel = localChannelPosition(measHash);
   channelPos.block<2,1>(0,0) = std::move(localChannel);
   return localToGlobalTrans(ctx, lHash) * channelPos;
}

using localCornerArray = std::array<Amg::Vector2D, 4>;
using globalCornerArray = std::array<Amg::Vector3D, 4>;
globalCornerArray sTgcReadoutElement::globalPadCorners(const ActsTrk::GeometryContext& ctx, const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   unsigned int layIdx = static_cast<unsigned int>(lHash);
   unsigned int gasGap = gasGapNumber(measHash);
   globalCornerArray gPadCorners{make_array<Amg::Vector3D, 4>(Amg::Vector3D::Zero())};
   if (chType(measHash) == ReadoutChannelType::Pad && gasGap < m_pars.padLayers.size()) {
      localCornerArray lPadCorners = localPadCorners(measHash);
      for (unsigned int corner = 0; corner < lPadCorners.size(); ++corner) {
         gPadCorners[corner].block<2,1>(0,0) = std::move(lPadCorners[corner]);
         gPadCorners[corner] = localToGlobalTrans(ctx, lHash) * gPadCorners[corner];
      }
      return gPadCorners;
   }
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.padLayers.size());
   return gPadCorners;
}
   
Amg::Vector3D sTgcReadoutElement::chamberStripPos(const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   unsigned int layIdx = static_cast<unsigned int>(lHash);
   if (layIdx < m_pars.stripLayers.size()) {
      const StripLayer& layout{m_pars.stripLayers[layIdx]};
      return layout.toOrigin() * layout.localStripPosition(channelNumber(measHash));
   }
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.stripLayers.size());
   return Amg::Vector3D::Zero();
}
int sTgcReadoutElement::padNumber(const Amg::Vector2D& hitPos, const IdentifierHash& measHash) const {
   int padEta = padDesign(measHash).channelNumber(hitPos).first;
   int padPhi = padDesign(measHash).channelNumber(hitPos).second;
   bool is_valid{true};
   const Identifier padID = m_idHelper.padID(identify(), multilayer(), gasGapNumber(measHash) + 1, chType(measHash),
                                                padEta, padPhi, is_valid);
   int channel = m_idHelper.channel(padID);
   return channel;   
}

Amg::Vector3D sTgcReadoutElement::leftStripEdge(const ActsTrk::GeometryContext& ctx, const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   unsigned int gasGap = gasGapNumber(measHash);
   unsigned int layIdx = static_cast<unsigned int>(lHash);

   if(chType(measHash) == ReadoutChannelType::Strip){
    if(gasGap > m_pars.stripLayers.size()){
      ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.stripLayers.size());
      return Amg::Vector3D::Zero();
      }      
   Amg::Vector3D stripleftEdge{Amg::Vector3D::Zero()};
   Amg::Vector2D localstripleftEdge{Amg::Vector2D::Zero()};
   std::optional<Amg::Vector2D> stripleftEdgeOpt = stripDesign(measHash).leftEdge(channelNumber(measHash));
   localstripleftEdge = std::move(*stripleftEdgeOpt);
   stripleftEdge.block<2,1>(0,0) = std::move(localstripleftEdge);

   return localToGlobalTrans(ctx, lHash)*stripleftEdge;

   }else if(chType(measHash) == ReadoutChannelType::Wire){
      if(gasGap > m_pars.wireGroupLayers.size()){
         ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.wireGroupLayers.size());
         return Amg::Vector3D::Zero();

      }

   Amg::Vector3D wireleftEdge{Amg::Vector3D::Zero()};
   Amg::Vector2D localwireleftEdge{Amg::Vector2D::Zero()};
   std::optional<Amg::Vector2D> wireleftedgeOpt = wireDesign(measHash).leftEdge(channelNumber(measHash));
   localwireleftEdge = std::move(*wireleftedgeOpt);
   wireleftEdge.block<2,1>(0,0) = std::move(localwireleftEdge);
     
     return localToGlobalTrans(ctx, lHash)*wireleftEdge;

   }

   ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is not valid Type "<< chType(measHash));
   return Amg::Vector3D::Zero();
   
}


Amg::Vector3D sTgcReadoutElement::rightStripEdge(const ActsTrk::GeometryContext& ctx, const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   unsigned int gasGap = gasGapNumber(measHash);
   unsigned int layIdx = static_cast<unsigned int>(lHash);

   if(chType(measHash) == ReadoutChannelType::Strip){
    if(gasGap > m_pars.stripLayers.size()){
      ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.stripLayers.size());
      return Amg::Vector3D::Zero();
      }      
   Amg::Vector3D striprightEdge{Amg::Vector3D::Zero()};
   Amg::Vector2D localstriprightEdge{Amg::Vector2D::Zero()};
   std::optional<Amg::Vector2D> striprightEdgeOpt = stripDesign(measHash).rightEdge(channelNumber(measHash));
   localstriprightEdge = std::move(*striprightEdgeOpt);
   striprightEdge.block<2,1>(0,0) = std::move(localstriprightEdge);

   return localToGlobalTrans(ctx, lHash)*striprightEdge;

   }else if(chType(measHash) == ReadoutChannelType::Wire){
      if(gasGap > m_pars.wireGroupLayers.size()){
         ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.wireGroupLayers.size());
         return Amg::Vector3D::Zero();

      }

   Amg::Vector3D wirerightEdge{Amg::Vector3D::Zero()};
   Amg::Vector2D localwirerightEdge{Amg::Vector2D::Zero()};
   std::optional<Amg::Vector2D> wirerightedgeOpt = wireDesign(measHash).rightEdge(channelNumber(measHash));
   localwirerightEdge = std::move(*wirerightedgeOpt);
   wirerightEdge.block<2,1>(0,0) = std::move(localwirerightEdge);
     
     return localToGlobalTrans(ctx, lHash)*wirerightEdge;

   }

   ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is not valid Type "<< chType(measHash));
   return Amg::Vector3D::Zero(); 

   
}

bool sTgcReadoutElement::isEtaZero(const IdentifierHash& measurementHash, const Amg::Vector2D& localPosition) const {
   if(std::abs(m_idHelper.stationEta(identify())) != 1 ) return false; // if we are not in a Q1 ro element we do not have to check further
   const WireGroupDesign& design = wireDesign(measurementHash); // function is not checking for channel type so we just use its gas gap info

   double lpos  = (chType(measurementHash) == ReadoutChannelType::Strip  ? localPosition.x() : localPosition.y() );
   if (lpos < design.halfWidth() - design.wireCutout()) return true;
   return false;
}


}  // namespace MuonGMR4
