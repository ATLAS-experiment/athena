/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <AthenaBaseComps/AthCheckMacros.h>
#include <GaudiKernel/SystemOfUnits.h>

using namespace ActsTrk;

namespace MuonGMR4 {
using parameterBook = sTgcReadoutElement::parameterBook;
std::ostream& operator<<(std::ostream& ostr, const parameterBook& pars) {
  if (pars.stripDesign) ostr<<"Strips: "<<(*pars.stripDesign)<<std::endl;
  if (pars.wireGroupDesign) ostr<<"Wire Groups: "<<(*pars.wireGroupDesign)<<std::endl;   
  if (pars.padDesign) ostr<<"Pads: "<<(*pars.padDesign)<<std::endl;   
  return ostr;
}

sTgcReadoutElement::sTgcReadoutElement(defineArgs&& args)
    : MuonReadoutElement(std::move(args)),
      m_pars{std::move(args)} {
}

const parameterBook& sTgcReadoutElement::getParameters() const {return m_pars;}

StatusCode sTgcReadoutElement::initElement() {
   ATH_MSG_DEBUG("Parameter book "<<parameterBook());
       /// Check that the alignable node has been assigned
   if(!alignableTransform()) {
      ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" has no assigned alignable node");
      return StatusCode::FAILURE;
   } 
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
      ATH_CHECK(insertTransform(m_pars.stripLayers[layer].hash(), 
                                 [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                                    return toStation(store) * fromGapToChamOrigin(hash); 
                                 }));
   }
   for (unsigned int layer = 0; layer < m_pars.wireGroupLayers.size(); ++layer) {
      IdentifierHash layHash{layer};
      if (gasGapNumber(m_pars.wireGroupLayers[layer].hash()) != layHash) {
         ATH_MSG_FATAL("Layer "<<m_pars.wireGroupLayers[layer]<<" has a very strange hash. Expect "<<layer);
       return StatusCode::FAILURE;
      }
      ATH_CHECK(insertTransform(m_pars.wireGroupLayers[layer].hash(), 
                                 [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                                    return toStation(store) * fromGapToChamOrigin(hash); 
                                 }));
   }
   for (unsigned int layer = 0; layer < m_pars.padLayers.size(); ++layer) {
      IdentifierHash layHash{layer};
      if (gasGapNumber(m_pars.padLayers[layer].hash()) != layHash) {
         ATH_MSG_FATAL("Layer "<<m_pars.padLayers[layer]<<" has a very strange hash. Expect "<<layer);
       return StatusCode::FAILURE;
      }
      ATH_CHECK(insertTransform(m_pars.padLayers[layer].hash(), 
                                 [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                                    return toStation(store) * fromGapToChamOrigin(hash); 
                                 }));     
   }
   ActsGeometryContext gctx{};
   m_gasGapPitch = (center(gctx, createHash(1, sTgcIdHelper::sTgcChannelTypes::Strip, 0)) -
                   center(gctx, createHash(2, sTgcIdHelper::sTgcChannelTypes::Strip, 0))).mag(); 
   return StatusCode::SUCCESS;
}

Amg::Transform3D sTgcReadoutElement::fromGapToChamOrigin(const IdentifierHash& measHash) const{
   unsigned int layIdx = static_cast<unsigned int>(measHash);
   unsigned int gasGap = gasGapNumber(measHash);
   if(chType(measHash) == ReadoutChannelType::Strip && gasGap < m_pars.stripLayers.size()) {
      return m_pars.stripLayers[gasGap].toOrigin();
   }
   else if (chType(measHash) == ReadoutChannelType::Wire && gasGap < m_pars.wireGroupLayers.size()) {
      return m_pars.wireGroupLayers[gasGap].toOrigin();
   }
   else if (chType(measHash) == ReadoutChannelType::Pad && gasGap < m_pars.padLayers.size()) {
      return m_pars.padLayers[gasGap].toOrigin();
   }
   else {
      unsigned int maxReadoutLayers =  m_pars.stripLayers.size() + m_pars.wireGroupLayers.size() + m_pars.padLayers.size();
      ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<< maxReadoutLayers);
      return Amg::Transform3D::Identity();
   }
}

Amg::Vector2D sTgcReadoutElement::localChannelPosition(const IdentifierHash& measHash) const {
   if (chType(measHash) == ReadoutChannelType::Strip) {
      Amg::Vector2D stripCenter{Amg::Vector2D::Zero()};
      std::optional<Amg::Vector2D> stripCenterOpt = stripDesign(measHash).center(channelNumber(measHash));
      if (!stripCenterOpt) {
         ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The strip" << channelNumber(measHash) << "doesn't intersect with the edges of the trapezoid.");
         return stripCenter;
      }
      stripCenter = std::move(*stripCenterOpt);
      if (channelNumber(measHash) == 1 && firstStripPitch(measHash) < stripPitch(measHash)) {
         stripCenter.x() += 0.25 * stripWidth(measHash);
      }
      if (channelNumber(measHash) == numStrips(measHash) && firstStripPitch(measHash) == stripPitch(measHash)) {
         stripCenter.x() -= 0.25 * stripWidth(measHash);
      }
      return stripCenter;
   }
   else if (chType(measHash) == ReadoutChannelType::Wire) {
      Amg::Vector2D wireGroupCenter{Amg::Vector2D::Zero()};
      std::optional<Amg::Vector2D> wireGroupCenterOpt = wireDesign(measHash).center(channelNumber(measHash));
      if (!wireGroupCenterOpt) {
         ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The wireGroup" << channelNumber(measHash) << "doesn't intersect with the edges of the trapezoid.");
         return wireGroupCenter;
      }
      wireGroupCenter = std::move(*wireGroupCenterOpt);
      unsigned int gasGap = gasGapNumber(measHash) + 1;
      if (channelNumber(measHash) == 1) {
         ATH_MSG_DEBUG("The first wiregroup width is " <<firstWireGroupWidth(gasGap));
         ATH_MSG_DEBUG("The last wire pos is: " << wireGroupCenter.x() + ((firstWireGroupWidth(gasGap) + 1) / 2 - 1) * wirePitch(measHash) );
         /// Shifting the first wireGroup center to the last wire of the first wireGroup
         wireGroupCenter.x() = wireGroupCenter.x() + ((firstWireGroupWidth(gasGap) + 1) / 2 - 1) * wirePitch(measHash);
         /// Defining the wireGroup center as the mean of the position of the last wire in the first group
         /// and the left edge of the active area defined for pads to match the R3 description
         wireGroupCenter.x() = 0.5 * (wireGroupCenter.x() - 0.5 * lPadLength(measHash));
      }
      else if (channelNumber(measHash) == numWireGroups(gasGap)) {
         ATH_MSG_DEBUG("The last wire center before modification is: " << wireGroupCenter.x());
         unsigned int lastWireGroupWidth = numWires(gasGap) - firstWireGroupWidth(gasGap) - (numWireGroups(gasGap) - 2) * wireGroupWidth(gasGap);
         ATH_MSG_DEBUG("The last wire group width is: " << lastWireGroupWidth << " and half of that is: "<< lastWireGroupWidth / 2);                     
         /// Shifting the last wireGroup center to the last wire of the second-last wireGroup
         wireGroupCenter.x() = wireGroupCenter.x() - (lastWireGroupWidth / 2 + 1) * wirePitch(measHash);
         ATH_MSG_DEBUG("The last wire of the last second group is at: " << wireGroupCenter.x());
         /// Defining the wireGroup center as the mean of the position of the last wire in the second last group
         /// and the right edge of the active area defined for pads to match the R3 description
         wireGroupCenter.x() = 0.5 * (wireGroupCenter.x() + 0.5 * lPadLength(measHash));
      }
      else {
         /// In R3, the center of the normal wireGroup is defined on the 10th wire, whereas, in R4
         /// the center is defined on the 11th wire. So shifting by a wirePitch to match R3
         wireGroupCenter.x() = wireGroupCenter.x() - wirePitch(measHash);
      }
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

Amg::Vector3D sTgcReadoutElement::globalChannelPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const {
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
globalCornerArray sTgcReadoutElement::globalPadCorners(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const {
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
      return  m_pars.stripLayers[layIdx].stripPosition(channelNumber(measHash));
   }
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.stripLayers.size());
   return Amg::Vector3D::Zero();
}

}  // namespace MuonGMR4
