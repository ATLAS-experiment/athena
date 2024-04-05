
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelMmTest.h"
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <fstream>

using namespace ActsTrk;

namespace MuonGMR4{

GeoModelMmTest::GeoModelMmTest(const std::string& name, ISvcLocator* pSvcLocator):
    AthHistogramAlgorithm(name,pSvcLocator) {}

StatusCode GeoModelMmTest::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());
    /// Prepare the TTree dump
    ATH_CHECK(m_tree.init(this));

    const MmIdHelper& id_helper{m_idHelperSvc->mmIdHelper()};
    for (const std::string& testCham : m_selectStat) {
        if (testCham.size() != 6) {
            ATH_MSG_FATAL("Wrong format given " << testCham);
            return StatusCode::FAILURE;
        }
        /// Example string MML1A6
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
        for (const Identifier& id : m_testStations) {
            sstr<<" *** "<<m_idHelperSvc->toString(id)<<std::endl;
        }
        ATH_MSG_DEBUG("Test only the following stations "<<std::endl<<sstr.str());
    }
     ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMmTest::finalize() {
    ATH_CHECK(m_tree.write());
    return StatusCode::SUCCESS;
}
StatusCode GeoModelMmTest::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadHandle<ActsGeometryContext> geoContextHandle{m_geoCtxKey, ctx};
    ATH_CHECK(geoContextHandle.isPresent());
    const ActsGeometryContext& gctx{*geoContextHandle};

    for (const Identifier& test_me : m_testStations) {
      ATH_MSG_DEBUG("Test retrieval of Mm detector element "<<m_idHelperSvc->toStringDetEl(test_me));
      const MmReadoutElement* reElement = m_detMgr->getMmReadoutElement(test_me);
      if (!reElement) {
         continue;
      }
      /// Check that we retrieved the proper readout element
      if (reElement->identify() != test_me) {
         ATH_MSG_FATAL("Expected to retrieve "<<m_idHelperSvc->toStringDetEl(test_me)
                      <<". But got instead "<<m_idHelperSvc->toStringDetEl(reElement->identify()));
         return StatusCode::FAILURE;
      }
      ATH_CHECK(dumpToTree(ctx,gctx,reElement));
      const Amg::Transform3D globToLocal{reElement->globalToLocalTrans(gctx)};
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
      const MmIdHelper& id_helper{m_idHelperSvc->mmIdHelper()};
      for (unsigned int layer = 1; layer <= reElement->nGasGaps(); ++layer) {

            unsigned int numStrips = reElement->numStrips(layer);
            unsigned int fStrip = reElement->firstStrip(layer);
            unsigned int lStrip = fStrip+numStrips-1;

            for (unsigned int strip = fStrip; strip <= lStrip; ++strip) {
                bool isValid{false};
                
                const Identifier chId = id_helper.channelID(reElement->identify(),
                                                                reElement->multilayer(),
                                                                layer, strip, isValid);
                if (!isValid) {
                    continue;
                }
                /// Test the back and forth conversion of the Identifier
                const IdentifierHash channelHash = reElement->measurementHash(chId);
                const IdentifierHash layHash = reElement->layerHash(chId);
                const Identifier backCnv = reElement->measurementId(channelHash);
                if (backCnv != chId) {
                    ATH_MSG_FATAL("The back and forth conversion of "<<m_idHelperSvc->toString(chId)
                                    <<" failed. Got "<<m_idHelperSvc->toString(backCnv));
                    return StatusCode::FAILURE;
                }
                if (layHash != reElement->layerHash(channelHash)) {
                    ATH_MSG_FATAL("Constructing the layer hash from the identifier "<<
                                m_idHelperSvc->toString(chId)<<" leads to different layer hashes "<<
                                layHash<<" vs. "<< reElement->layerHash(channelHash));
                    return StatusCode::FAILURE;
                }

                ATH_MSG_DEBUG("numStrips "<< numStrips << "  ,Channel "<< m_idHelperSvc->toString(chId)<<" strip position "
                                    <<Amg::toString(reElement->stripPosition(gctx, channelHash)));
            }

      }
    
    }   

   return StatusCode::SUCCESS;
}
StatusCode GeoModelMmTest::dumpToTree(const EventContext& ctx,
                                       const ActsGeometryContext& gctx, 
                                       const MmReadoutElement* reElement) {



    m_stIndex    = reElement->stationName();
    m_stEta      = reElement->stationEta();
    m_stPhi      = reElement->stationPhi();
    m_stML       = reElement->multilayer();
    m_chamberDesign = reElement->chamberDesign();
    ///
   /// Dump the local to global transformation of the readout element
   const Amg::Transform3D& transform{reElement->localToGlobalTrans(gctx)};
   m_readoutTransform = transform;
  ///
    m_moduleHeight = reElement->moduleHeight();
    m_moduleWidthS = reElement->moduleWidthL();
    m_moduleWidthL = reElement->moduleWidthS();

    const MmIdHelper& id_helper{m_idHelperSvc->mmIdHelper()};
    for (unsigned int layer = 1; layer <= reElement->nGasGaps(); ++layer) {

        unsigned int numStrips = reElement->numStrips(layer);
        unsigned int fStrip = reElement->firstStrip(layer);
        unsigned int lStrip = fStrip+numStrips-1;

        for (unsigned int strip = fStrip; strip <= lStrip ; ++strip) {
            bool isValid{false};
            const Identifier chId = id_helper.channelID(reElement->identify(),
                                                            reElement->multilayer(),
                                                            layer, strip, isValid);
            if (!isValid) {
                ATH_MSG_WARNING("Invalid Identifier detected for readout element "
                                <<m_idHelperSvc->toStringDetEl(reElement->identify())
                                <<" layer: "<<layer<<" strip: "<<strip);
                continue;
            }
            m_locStripCenter.push_back(reElement->stripLayer(chId).design().center(strip).value_or(Amg::Vector2D::Zero()));
            m_isStereo.push_back(reElement->stripLayer(chId).design().hasStereoAngle());
            m_stripCenter.push_back(reElement->stripPosition(gctx, chId));
            m_stripLeftEdge.push_back(reElement->leftStripEdge(gctx,chId));
            m_stripRightEdge.push_back(reElement->rightStripEdge(gctx,chId));
            m_stripLength.push_back(reElement->stripLength(strip,layer));
            m_gasGap.push_back(layer);
            m_channel.push_back(strip);

            m_ActiveWidthS = reElement->gapLengthS(layer);
            m_ActiveWidthL = reElement->gapLengthL(layer);
            m_ActiveHeightR = reElement->gapHeight(layer);

            if (strip != fStrip) continue;
            const Amg::Transform3D stripGlobToLoc = reElement->globalToLocalTrans(gctx, chId);
            ATH_MSG_INFO("The global to local transformation on layers is: " << Amg::toString(stripGlobToLoc));
            ATH_MSG_INFO("The local to global transformation on layers is: " << Amg::toString(reElement->localToGlobalTrans(gctx, chId)));
            m_stripRot.push_back(stripGlobToLoc);
            m_stripRotGasGap.push_back(layer);
        }

    }
    return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

}

