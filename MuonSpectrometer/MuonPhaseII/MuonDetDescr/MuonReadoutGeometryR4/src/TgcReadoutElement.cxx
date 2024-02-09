/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <AthenaBaseComps/AthCheckMacros.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <optional>
using namespace ActsTrk;

namespace MuonGMR4 {
using parameterBook = TgcReadoutElement::parameterBook;
std::ostream& operator<<(std::ostream& ostr, const parameterBook& pars) {
   ostr<<"chamber dimensions --- ";
   ostr<<"thickness: "<<pars.halfThickness<<" ";
   ostr<<"height: "<<pars.halfHeight<<" ";
   ostr<<"shortWidth: "<<pars.halfWidthShort<<" ";
   ostr<<"longWidth: "<<pars.halfWidthLong<<" --- ";
   return ostr;
}

TgcReadoutElement::TgcReadoutElement(defineArgs&& args)
    : MuonReadoutElement(std::move(args)),
      m_pars{std::move(args)} {
}

const parameterBook& TgcReadoutElement::getParameters() const { return m_pars; }
StatusCode TgcReadoutElement::initElement() {
    
   if(!alignableTransform()) {
      ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" has no assigned alignable node");
      return StatusCode::FAILURE;
   } 

    /// Check that the readoutelement has sensor layouts
    bool hasSensor{false};
    for (size_t s = 0; s < m_pars.sensorLayouts.size(); ++s) {
       const StripLayerPtr& layPtr{m_pars.sensorLayouts[s]};
       if (!layPtr) continue;
       if (layPtr->hash() != s) {
          ATH_MSG_FATAL("Layer "<<(*layPtr)<<" has an unexpected hash "<<s);
          return StatusCode::FAILURE;
       }
       hasSensor = true;
    }
    if (!hasSensor) {
         ATH_MSG_FATAL("No active layer is defined for "<<idHelperSvc()->toStringDetEl(identify()));
         return StatusCode::FAILURE;
    }
    if (std::find_if(m_pars.sensorLayouts.begin(), m_pars.sensorLayouts.end(),
                     [](const StripLayerPtr & ptr)->bool{ return ptr; }) == m_pars.sensorLayouts.end()) {
         ATH_MSG_FATAL("No sensor structure is provided to "<<idHelperSvc()->toStringDetEl(identify()));
         return StatusCode::FAILURE;
    }
#ifndef SIMULATIONBASE
    ATH_CHECK(planeSurfaceFactory(geoTransformHash(),
                                  m_pars.layerBounds->make_bounds(m_pars.halfWidthShort,
                                                                  m_pars.halfWidthLong,
                                                                  m_pars.halfHeight)));
#endif
    for (unsigned int gap = 1; gap <= nGasGaps(); ++gap) {
         if (numWireGangs(gap)) {
            const IdentifierHash layHash{constructHash(0, gap, false)}; 
            ATH_CHECK(insertTransform(layHash, 
                                 [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                                    return toStation(store) * fromGapToChamOrigin(hash); 
                                 }));
#ifndef SIMULATIONBASE
            const StripDesign& layout{wireGangLayout(gap)};
            ATH_CHECK(planeSurfaceFactory(layHash, m_pars.layerBounds->make_bounds(layout.shortHalfHeight(),
                                                                                   layout.longHalfHeight(),
                                                                                   layout.halfWidth())));
#endif
         }
         if (numStrips(gap)) {
            const IdentifierHash layHash{constructHash(0, gap, true)}; 
            ATH_CHECK(insertTransform(layHash, 
                                 [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                                    return toStation(store) * fromGapToChamOrigin(hash); 
                                 }));
#ifndef SIMULATIONBASE
            const StripDesign& layout{stripLayout(gap)};
            /// We probably need a rotated version of these bounds. However, that's not part
            /// of Acts yet
            ATH_CHECK(planeSurfaceFactory(layHash, m_pars.layerBounds->make_bounds(layout.shortHalfHeight(),
                                                                                   layout.longHalfHeight(),
                                                                                   layout.halfWidth())));
#endif
         }
    }
#ifndef SIMULATIONBASE
    m_pars.layerBounds.reset();
#endif
    const IdentifierHash firstLay  = constructHash(0, 1, false);
    const IdentifierHash secondLay = constructHash(0, 2, false);
    ActsGeometryContext gctx{};
    m_gasThickness =(center(gctx, firstLay) - center(gctx, secondLay)).mag(); 
    return StatusCode::SUCCESS;
}

Amg::Transform3D TgcReadoutElement::fromGapToChamOrigin(const IdentifierHash& layHash) const {
      const unsigned layIdx{static_cast<unsigned>(layHash)};
      return m_pars.sensorLayouts[layIdx]->toOrigin();
}
Amg::Vector3D TgcReadoutElement::channelPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const { 
   const StripLayerPtr& layDesign{sensorLayout(gasGapNumber(measHash), isStrip(measHash))};
   if (!layDesign) {
       ATH_MSG_WARNING("The gasGap "<<gasGapNumber(measHash)<<" & strip:"<<isStrip(measHash)<<" is unknown");
       return Amg::Vector3D::Zero();
   }
   return localToGlobalTrans(ctx, layerHash(measHash)) * layDesign->localStripPos(channelNumber(measHash));
}
}