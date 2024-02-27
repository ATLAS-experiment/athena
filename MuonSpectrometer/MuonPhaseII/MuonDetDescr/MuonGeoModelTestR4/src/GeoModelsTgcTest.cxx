
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelsTgcTest.h"
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <fstream>

using namespace ActsTrk;
namespace MuonGMR4{

GeoModelsTgcTest::GeoModelsTgcTest(const std::string& name, ISvcLocator* pSvcLocator):
AthHistogramAlgorithm(name,pSvcLocator) {}

StatusCode GeoModelsTgcTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());    
    ATH_CHECK(m_surfaceProvTool.retrieve());
    /// Prepare the TTree dump
    ATH_CHECK(m_tree.init(this));

    
    const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
    for (const std::string& testCham : m_selectStat) {
        if (testCham.size() != 6) {
            ATH_MSG_FATAL("Wrong format given " << testCham);
            return StatusCode::FAILURE;
        }
        /// Example string STS3A3
        std::string statName = testCham.substr(0, 3);
        unsigned int statEta = std::atoi(testCham.substr(3, 1).c_str()) *
                               (testCham[4] == 'A' ? 1 : -1);
        unsigned int statPhi = std::atoi(testCham.substr(5, 1).c_str());
        bool is_valid{false};
        const Identifier eleId = id_helper.elementID(statName, statEta, statPhi, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("Failed to deduce a station name for " << testCham);
            return StatusCode::FAILURE;
        }
        std::copy_if(id_helper.detectorElement_begin(), 
                     id_helper.detectorElement_end(), 
                     std::inserter(m_testStations, m_testStations.end()), 
                        [&](const Identifier& id) {
                            return id_helper.elementID(id) == eleId;
                        });
    }
    /// Add all stations for testing if nothing has been specified
    if (m_testStations.empty()){
        std::copy(id_helper.detectorElement_begin(), 
                  id_helper.detectorElement_end(), 
                  std::inserter(m_testStations, m_testStations.end()));
    } else {
        std::stringstream sstr{};
        for (const Identifier& id : m_testStations){
            sstr<<" *** "<<m_idHelperSvc->toString(id)<<std::endl;
        }
        ATH_MSG_INFO("Test only the following stations "<<std::endl<<sstr.str());
    }
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
StatusCode GeoModelsTgcTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelsTgcTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<ActsGeometryContext> geoContextHandle{m_geoCtxKey, ctx};
    if (!geoContextHandle.isValid()){
      ATH_MSG_FATAL("Failed to retrieve "<<m_geoCtxKey.fullKey());
      return StatusCode::FAILURE;
    }
    
    const ActsGeometryContext& gctx{**geoContextHandle};
    for (const Identifier& test_me : m_testStations) {
      ATH_MSG_DEBUG("Test retrieval of sTgc detector element "<<m_idHelperSvc->toStringDetEl(test_me));
      const sTgcReadoutElement* reElement = m_detMgr->getsTgcReadoutElement(test_me);
      if (!reElement) {         
         continue;
      }
      /// Check that we retrieved the proper readout element
      if (m_idHelperSvc->stgcIdHelper().elementID(reElement->identify()) != m_idHelperSvc->stgcIdHelper().elementID(test_me)) {
         ATH_MSG_FATAL("Expected to retrieve "<<m_idHelperSvc->toString(test_me)
                      <<". But got instead "<<m_idHelperSvc->toString(reElement->identify()));
         return StatusCode::FAILURE;
      }
      ATH_CHECK(dumpToTree(ctx,gctx,reElement));
      const Amg::Transform3D& globToLocal{reElement->globalToLocalTrans(gctx)};
      const Amg::Transform3D& localToGlob{reElement->localToGlobalTrans(gctx)};
      /// Closure test that the transformations actually close
      const Amg::Transform3D transClosure = globToLocal * localToGlob;
      for (Amg::Vector3D axis :{Amg::Vector3D::UnitX(),Amg::Vector3D::UnitY(),Amg::Vector3D::UnitZ()}) {
         const double closure_mag = std::abs( (transClosure*axis).dot(axis) - 1.);
         if (closure_mag > std::numeric_limits<float>::epsilon() ) {
            ATH_MSG_FATAL("Closure test failed for "<<m_idHelperSvc->toStringDetEl(test_me)<<" and axis "<<Amg::toString(axis, 0)
            <<". Ended up with "<< Amg::toString(transClosure*axis) );
            return StatusCode::FAILURE;
         }         
      }
      const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
      for (int layer = 1; layer <= reElement->numLayers(); ++layer) {
        for (int chType = sTgcIdHelper::sTgcChannelTypes::Strip/*Pad*/; chType <= sTgcIdHelper::sTgcChannelTypes::Wire; ++chType) {
            unsigned int numChannel = 0;
            bool isValidLay{false};
            const Identifier layID = id_helper.channelID(reElement->identify(),
                                                        reElement->multilayer(),
                                                        layer, chType, 1, isValidLay);
            if (!isValidLay) {
                continue;
            }
            switch(chType) {
                case sTgcIdHelper::sTgcChannelTypes::Strip:
                    numChannel = reElement->numStrips(layID);
                break;
                
                case sTgcIdHelper::sTgcChannelTypes::Wire:
                    numChannel = reElement->numWireGroups(layer);
                break;
            }
            for (unsigned int channel = 1; channel < numChannel ; ++channel) {
                bool isValidCh{false};
                const Identifier chID = id_helper.channelID(reElement->identify(),
                                                                reElement->multilayer(),
                                                                layer, chType, channel, isValidCh);
                if (!isValidCh) {
                    continue;
                }
                /// Test the back and forth conversion of the Identifier
                const IdentifierHash measHash = reElement->measurementHash(chID);
                const IdentifierHash layHash = reElement->layerHash(chID);
                ATH_MSG_VERBOSE("layer: "<<layer<<", chType: "<<chType
                               <<" --> layerHash: "<<static_cast<unsigned>(layHash));
                const Identifier backCnv = reElement->measurementId(measHash);
                if (backCnv != chID) {
                    ATH_MSG_FATAL("The back and forth conversion of "<<m_idHelperSvc->toString(chID)
                                    <<" failed. Got "<<m_idHelperSvc->toString(backCnv));
                    return StatusCode::FAILURE;
                }
                if (layHash != reElement->layerHash(measHash)) {
                    ATH_MSG_FATAL("Constructing the layer hash from the identifier "<<
                                m_idHelperSvc->toString(chID)<<" leads to different layer hashes "<<
                                layHash<<" vs. "<< reElement->layerHash(measHash));
                    return StatusCode::FAILURE;
                }
                if (chType == sTgcIdHelper::sTgcChannelTypes::Strip) {
                    ATH_MSG_VERBOSE("Channel "<<m_idHelperSvc->toString(chID)<<" strip position "
                                    <<Amg::toString(reElement->globalChannelPosition(gctx, measHash)));
                } 
                else if (chType == sTgcIdHelper::sTgcChannelTypes::Wire) {
                    ATH_MSG_VERBOSE("Channel "<<m_idHelperSvc->toString(chID)<<" wireGroup position "
                                    <<Amg::toString(reElement->globalChannelPosition(gctx, measHash)));
                }
            }
        }
      }
    
    }   
    return StatusCode::SUCCESS;
}

StatusCode GeoModelsTgcTest::dumpToTree(const EventContext& ctx,
                                       const ActsGeometryContext& gctx, 
                                       const sTgcReadoutElement* reElement){
    
    m_stIndex    = reElement->stationName();
    m_stEta      = reElement->stationEta();
    m_stPhi      = reElement->stationPhi();
    m_stML       = reElement->multilayer();
    m_chamberDesign = reElement->chamberDesign();
    ///
    m_numLayers = reElement->numLayers();
    m_yCutout = reElement->yCutout();
    m_gasTck = reElement->gasGapThickness();
    ///
    m_sChamberLength = reElement->sChamberLength();
    m_lChamberLength = reElement->lChamberLength();
    m_chamberHeight = reElement->chamberHeight();
    ///
    ///Pads will come here 
 
   /// Dump the local to global transformation of the readout element
   const Amg::Transform3D& transform{reElement->localToGlobalTrans(gctx)};
   m_readoutTransform = transform;
   const sTgcIdHelper& id_helper{m_idHelperSvc->stgcIdHelper()};
   for (int layer = 1; layer <= reElement->numLayers(); ++layer) {
        for (int chType = sTgcIdHelper::sTgcChannelTypes::/*Pad*/Strip; chType <= sTgcIdHelper::sTgcChannelTypes::Wire; ++chType) {
            unsigned int numWireGroup = 0;
            /// Use idHelper to get the identifier
            bool isValidLay{false};
            const Identifier layID = id_helper.channelID(reElement->identify(),
                                                        reElement->multilayer(),
                                                        layer, chType, 1, isValidLay);
            if (!isValidLay) {
                continue;
            }
            /// Gas Gap dimensions
            m_sGapLength = reElement->sGapLength(layID);
            m_lGapLength = reElement->lGapLength(layID);
            m_gapHeight = reElement->gapHeight(layID);
            switch (chType) {
                case sTgcIdHelper::sTgcChannelTypes::Strip:
                    m_numStrips = reElement->numStrips(layID);
                    m_stripPitch = reElement->stripPitch(layID);
                    m_stripWidth = reElement->stripWidth(layID); 
                    for (unsigned int strip = 1; strip <= reElement->numStrips(layID); ++strip) {
                        bool isValidStrip{false};
                        const Identifier stripID = id_helper.channelID(reElement->identify(), 
                                                                   reElement->multilayer(),
                                                                    layer, chType, strip, isValidStrip);
                    if (!isValidStrip) {
                        ATH_MSG_WARNING("Invalid Identifier detected for readout element "
                                       <<m_idHelperSvc->toStringDetEl(reElement->identify())
                                       <<" layer: "<<layer<<" strip: "<<strip<<" channelType: "<<chType);
                        continue;
                    }
                    m_localStripPos.push_back((reElement->localChannelPosition(stripID)).block<2,1>(0,0));
                    m_globalStripPos.push_back(reElement->globalChannelPosition(gctx, stripID));
                    m_stripGasGap.push_back(layer);
                    m_stripNum.push_back(strip);
                    m_stripLengths.push_back(reElement->stripLength(stripID));

                    if (strip != 1) continue;
                    const Amg::Transform3D locToGlob = reElement->localToGlobalTrans(gctx, stripID);
                    ATH_MSG_DEBUG("The local to global transformation on layers is: " << Amg::toString(locToGlob));
                    m_stripRot.push_back(locToGlob);
                    m_stripRotGasGap.push_back(layer);

                    }
                    break;
                  
                case sTgcIdHelper::sTgcChannelTypes::Wire:
                    m_wireGroupWidth = reElement->wireGroupWidth(layer);
                    numWireGroup = reElement->numWireGroups(layer);                    
                    m_wirePitch = reElement->wirePitch(layID);
                    m_wireWidth = reElement->wireWidth(layID);
                    m_numWires.push_back(reElement->numWires(layer));
                    m_firstWireGroupWidth.push_back(reElement->firstWireGroupWidth(layer));
                    m_numWireGroups.push_back(numWireGroup);
                    m_wireCutout.push_back(reElement->wireCutout(layer)); 
                    std::cout << "The number of wire groups are:" << numWireGroup << std::endl;
                    for (unsigned int wireGroup = 1; wireGroup <= numWireGroup; ++wireGroup) {
                        bool isValidWire{false};
                        const Identifier wireGroupID = id_helper.channelID(reElement->identify(), 
                                                                   reElement->multilayer(),
                                                                    layer, chType, wireGroup, isValidWire);
                    if (!isValidWire) {
                        ATH_MSG_WARNING("Invalid Identifier detected for readout element "
                                       <<m_idHelperSvc->toStringDetEl(reElement->identify())
                                       <<" layer: "<<layer<<" wireGroup: "<<wireGroup<<" channelType: "<<chType);
                        continue;
                    }  
                    m_localWireGroupPos.push_back(reElement->localChannelPosition(wireGroupID));
                    m_globalWireGroupPos.push_back(reElement->globalChannelPosition(gctx, wireGroupID));
                    m_wireGroupGasGap.push_back(layer);
                    m_wireGroupNum.push_back(wireGroup);
                  
                    if (wireGroup != 1) continue;
                    const Amg::Transform3D locToGlob = reElement->localToGlobalTrans(gctx, wireGroupID);
                    ATH_MSG_DEBUG("The local to global transformation on layers is: " << Amg::toString(locToGlob));
                    m_wireGroupRot.push_back(locToGlob);
                    m_wireGroupRotGasGap.push_back(layer);
                    }
                    break;
            }
        }
   }
   return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

}

