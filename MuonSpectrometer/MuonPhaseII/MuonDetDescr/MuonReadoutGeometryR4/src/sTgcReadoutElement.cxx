/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
   if (m_pars.stripLayers.empty()) {
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
   ActsGeometryContext gctx{};
   m_gasGapPitch = (center(gctx, createHash(1, sTgcIdHelper::sTgcChannelTypes::Strip, 0)) -
                   center(gctx, createHash(2, sTgcIdHelper::sTgcChannelTypes::Strip, 0))).mag();
    
   return StatusCode::SUCCESS;
}

Amg::Transform3D sTgcReadoutElement::fromGapToChamOrigin(const IdentifierHash& hash) const{
   unsigned int layIdx = static_cast<unsigned int>(hash);
   unsigned int gasGap = gasGapNumber(hash);
   if (gasGap < m_pars.stripLayers.size()) return m_pars.stripLayers[gasGap].toOrigin();
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.stripLayers.size());
   return Amg::Transform3D::Identity();
}

Amg::Vector3D sTgcReadoutElement::stripPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   unsigned int layIdx = static_cast<unsigned int>(lHash);
   unsigned int gasGap = gasGapNumber(measHash);
   if (gasGap < m_pars.stripLayers.size()) {
      return localToGlobalTrans(ctx, lHash) * m_pars.stripLayers[gasGap].localStripPos(stripNumber(measHash));
   }
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.stripLayers.size());
   return Amg::Vector3D::Zero();
}

Amg::Vector3D sTgcReadoutElement::chamberStripPos(const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   unsigned int layIdx = static_cast<unsigned int>(lHash);
   if (layIdx < m_pars.stripLayers.size()) {
      return  m_pars.stripLayers[layIdx].stripPosition(stripNumber(measHash));
   }
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.stripLayers.size());
   return Amg::Vector3D::Zero();
}

}  // namespace MuonGMR4
