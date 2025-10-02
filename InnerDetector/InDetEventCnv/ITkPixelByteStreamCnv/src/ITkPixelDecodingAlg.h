/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_DECODINGALG_H
#define ITKPIXEL_DECODINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/ReadHandleKey.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "InDetIdentifier/PixelID.h"
#include "Identifier/Identifier.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/PixelRDO_Collection.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"
#include "itksw/pix/endec/DecCore.hpp"
#include <chrono>


using namespace itksw::pix::endec;

//class PixelID;

class ITkPixelDecodingAlg : public AthReentrantAlgorithm 
{
  public:

    ITkPixelDecodingAlg(const std::string &name, ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
  
    virtual StatusCode execute (const EventContext& ctx) const override;

    virtual StatusCode finalize() override;

  
  private:

    typedef std::vector< std::vector<uint32_t >> ITkPacketCollection;

    ServiceHandle<IROBDataProviderSvc>    m_robDataProviderSvc{ this, "ROBDataProvider", "ROBDataProviderSvc" };

    SG::ReadCondHandleKey<ITkPixelCablingData> m_pixelCablingKey{this, "PixelCablingKey", "ITkPixelCablingData", "Cond Key of Pixel Cabling"};

    SG::WriteHandleKey<PixelRDO_Container> m_pixelRDOKey{this,    "pixelRDOKey", "PixelRDOs", "StoreGate Key of Pixel RDOs"};
    
    std::vector<uint32_t> m_sourceIDs;

    const PixelID* m_idHelper{};

};

namespace ITkPixelDecoding{
    struct ScopedTimer {
        std::string m_label;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
        MsgStream& m_msg_source;

        ScopedTimer(std::string lbl, MsgStream& src) : m_label(std::move(lbl)), m_start(std::chrono::high_resolution_clock::now()), m_msg_source(src) {}
        ~ScopedTimer() {
            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();
            m_msg_source << MSG::DEBUG << m_label << " took " << ms << " ms" << endmsg;
        }

    };
}

namespace PixelCallbacks{
    //Forms pixel RDOs from the decoded hits & fills them into a container
    class RDOCallback {

        public:
            explicit RDOCallback(PixelRDO_Container* rdoContainer, const PixelID* idHelper) :
            m_rdoContainer(rdoContainer),
            m_idHelper(idHelper)
            {};

            ~RDOCallback() = default;

            //non-negotiable concepts
            inline void evt_init([[maybe_unused]] uint8_t tag) {
                m_identifier = Identifier(m_offlineID);
                const auto waferHash = m_idHelper->wafer_hash(m_identifier);
                if (m_rdoContainer->indexFind(waferHash) == m_rdoContainer->end()){
                    m_rdoCollection = std::make_unique<PixelRDO_Collection>(waferHash);
                    m_rdoCollection->reserve(1000);
                }
                else {
                    m_rdoCollection.reset(m_rdoContainer->removeCollection(waferHash));
                }

            };

            inline void evt_next([[maybe_unused]] uint8_t tag) {};

            inline void evt_done() {
                m_rdoContainer->addCollection(m_rdoCollection.release(), m_idHelper->wafer_hash(m_identifier)).ignore();
            };

            inline void add_hit(uint16_t col, uint16_t row, uint16_t tot){
                //Translate the col, row into module coordinates
                ITkPixelCabling::chipToModuleTransform(m_transform, m_chipID, col, row);
                m_rdoCollection->emplace_back(new Pixel1RawData(m_idHelper->pixel_id(m_identifier, row, col), tot, 0, 0, 0));                
            };

            inline void add_hmap([[maybe_unused]] uint8_t qcol, [[maybe_unused]] uint8_t qrow, [[maybe_unused]] uint16_t hmap, [[maybe_unused]] uint64_t tots) {};

            inline void add_qcore([[maybe_unused]] uint8_t qcol, [[maybe_unused]] uint8_t qrow, [[maybe_unused]] uint64_t qtot) {};

            inline uint8_t on_error([[maybe_unused]] itksw::pix::endec::intf::EventError error) {return 0;};
            //end of non-negotiable concepts

            //extra members

            //this is needed to properly identify the module.
            inline void setOfflineID(const uint32_t& offlineID){
                m_offlineID = offlineID;
            };

            //this is needed to properly translate hits on a quad
            inline void setChipID(const uint8_t& chipID){
                m_chipID = chipID;
            }

            //translate
            inline void setTransformType(const ITkPixelCabling::TransformType& transform){
                m_transform = transform;
            }

        private:
            PixelRDO_Container* m_rdoContainer;
            uint32_t m_offlineID = 0;
            uint8_t m_chipID = 0;
            Identifier m_identifier;
            ITkPixelCabling::TransformType m_transform;
            std::unique_ptr<PixelRDO_Collection> m_rdoCollection;
            const PixelID* m_idHelper;
    };

    //This prints the decoded hits on the screen,
    //useful for testing purposes
    class TestEventCallback {
        public:
            TestEventCallback(MsgStream& src): m_msg_source(src) {};
            ~TestEventCallback() = default;
          
            inline void evt_init(uint8_t tag) {
                m_msg_source << MSG::DEBUG << "evt_init(" << tag << ")" << std::endl;
            }
        
            inline void evt_next(uint8_t tag) {
                m_msg_source << MSG::DEBUG << "evt_next(" << tag << ")" << std::endl;
            }
        
            inline void evt_done() {
                m_msg_source << MSG::DEBUG << "evt_done()" << std::endl;
            }
        
            inline void add_hit(uint16_t col, uint16_t row, uint16_t tot) {
                m_msg_source << MSG::DEBUG << "add_hit(" << col << "," << row << "," << tot << ")" << std::endl;
            }
        
            inline void add_hmap(uint8_t qcol, uint8_t qrow, uint16_t hmap, uint64_t tots) {
                m_msg_source << MSG::DEBUG << "add_hmap(qcol=" << qcol << ",qrow=" << qrow
                    << ",hmap=" << hmap << ",tots=" << tots << ")" << std::endl;
            }
        
            inline void add_qcore(uint8_t qcol, uint8_t qrow, uint64_t qtot) {
                m_msg_source << MSG::DEBUG << "add_qcore(qcol=" << qcol << ",qrow=" << qrow
                    << ",qtot=" << qtot << ")" << std::endl;
            }
        
            inline uint8_t on_error(itksw::pix::endec::intf::EventError error) {
                m_msg_source << MSG::DEBUG << "on_error(" << error.code << ")" << std::endl;
                return 0;
            }
            
            MsgStream& m_msg_source;
  
    };
}


#endif

