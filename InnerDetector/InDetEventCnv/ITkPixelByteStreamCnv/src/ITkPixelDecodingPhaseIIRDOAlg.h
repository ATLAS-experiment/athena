/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_DECODINGPHASEIIRDOALG_H
#define ITKPIXEL_DECODINGPHASEIIRDOALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/ReadHandleKey.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "InDetIdentifier/PixelID.h"
#include "Identifier/Identifier.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/PhaseIIPixelRawDataContainerMT.h"
#include "InDetRawData/PixelRDO_Collection.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"
#include "itksw/pix/endec/DecCore.hpp"
#include <chrono>
#include <limits>
#include <cstdint> //for uint8_t etc.

/**
 * @class ITkPixelDecodingPhaseIIRDOAlg
 * This algorithm translates the event-formatted
 * bytestream into PhaseIIPixelRawDataContainer RDOs.
 * This is the main pixel decoding algorithm intended
 * for Run4, and the parallel using the old RDO will
 * be phased out.
 */


class ITkPixelDecodingPhaseIIRDOAlg : public AthReentrantAlgorithm 
{
  public:

    ITkPixelDecodingPhaseIIRDOAlg(const std::string &name, ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
  
    virtual StatusCode execute (const EventContext& ctx) const override;
  
    virtual StatusCode finalize() override;

  
  private:

    typedef std::vector< std::vector<uint32_t >> ITkPacketCollection;

    ServiceHandle<IROBDataProviderSvc> m_robDataProviderSvc
        { this, "ROBDataProvider", "ROBDataProviderSvc" };

    SG::ReadCondHandleKey<ITkPixelCablingData> m_pixelCablingKey
        {this, "PixelCablingKey", "ITkPixelCablingData", "Cond Key of Pixel Cabling"};

    SG::WriteHandleKey<PhaseIIPixelRawDataContainer> m_pixelRDOKey
        {this,    "pixelRDOKey", "PixelRDOs", "StoreGate Key of Pixel RDOs"};
    
    std::vector<uint32_t> m_sourceIDs;

    const PixelID* m_idHelper{};

    const Gaudi::Property<uint32_t> m_n_rdos_est {this, "nRDOs", 1300000, "Estimated number of RDOs per event"};

};

namespace ITkPixelDecodingPhaseIIRDO{
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

namespace PixelCallbacksPhaseIIRDO{
    //Forms pixel RDOs from the decoded hits & fills them into a container
    class PhaseIIRDOCallback {

        public:
            explicit PhaseIIRDOCallback(PhaseIIPixelRawDataContainerMT* cont_coll,
                    PhaseIIPixelRawDataContainerMT::ContainerPtr rdo_container_dest, const PixelID* idHelper, MsgStream& msg_source) :
                m_rdo_container_dest(rdo_container_dest),
                m_cont_coll(cont_coll),
                m_dest_range_guard(rdo_container_dest),
                m_currentIdentifierHash(0),
                m_idHelper(idHelper),
                m_msg_source(msg_source)
                {};

            ~PhaseIIRDOCallback() = default;

            //Obligatory members for the interface
            inline void evt_init([[maybe_unused]] uint8_t tag) {

                m_identifier = Identifier(m_offlineID);
                const auto waferHash = m_idHelper->wafer_hash(m_identifier);
                if(waferHash != m_currentIdentifierHash){
                    registerLastModule();
                    m_currentIdentifierHash = waferHash;
                    m_dest_range_guard=PhaseII::ContainerRangeGuard<PhaseII::DataRange, PhaseIIPixelRawDataContainerMT::ContainerPtr>(m_rdo_container_dest);
                }
            };

            inline void evt_next([[maybe_unused]] uint8_t tag) {};

            inline void evt_done() {
            };

            inline void add_hit(uint16_t col, uint16_t row, uint16_t tot){
                //Translate the col, row into module coordinates
                // after that, row and col are phi_index and eta_index, respectively
                ITkPixelCabling::chipToModuleTransform(m_transform, m_chipID, col, row);

                //Set dummy values at 0 for these:
                int bcid = 0;
                int lvl0a = 0;
                int lvl0d = 0;

                PhaseII::addDataForModule(*m_cont_coll,
                        m_dest_range_guard,
                        std::array<std::int16_t,2>{static_cast<std::int16_t>(row), static_cast<std::int16_t>(col)},
                        PhaseII::PixelRawDataContainer::makeWord( tot, bcid, lvl0a, lvl0d ));          
            };

            inline void add_hmap([[maybe_unused]] uint8_t qcol,
                    [[maybe_unused]] uint8_t qrow,
                    [[maybe_unused]] uint16_t hmap,
                    [[maybe_unused]] uint64_t tots)
                    {};

            inline void add_qcore([[maybe_unused]] uint8_t qcol,
                    [[maybe_unused]] uint8_t qrow,
                    [[maybe_unused]] uint64_t qtot)
                    {};

            inline uint8_t on_error([[maybe_unused]] itksw::pix::endec::intf::EventError error) {return 0;};
            //end of interface-mandated methods

            inline void registerLastModule() {
                if (!m_dest_range_guard.empty()) {
                    // register the RDO range for this module, or erase the newly added
                    if (!m_cont_coll->registerOrEraseNewData(m_currentIdentifierHash,m_dest_range_guard.range())) {
                        ++m_n_rejected_work;
                    }
                    // in the process of adding hits to the original container, its capacity may have
                    // been exceeded and the container may have been changed for the current module. To
                    // ensure that the same container is used for the next module get the container, that
                    // contains the hits for the current module from the the range_guard.
                    m_rdo_container_dest = m_dest_range_guard.ptr();
                    m_n_rdos += m_dest_range_guard.range().size();
                }
            }

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
        
            // the RDO (=hits) container
            PhaseIIPixelRawDataContainerMT::ContainerPtr m_rdo_container_dest;
            
            // the collection of RDO containers
            PhaseIIPixelRawDataContainerMT* m_cont_coll{};

            // RDO container range guard
            PhaseII::ContainerRangeGuard<PhaseII::DataRange, PhaseIIPixelRawDataContainerMT::ContainerPtr> m_dest_range_guard;

            // current offline ID hash
            unsigned int m_currentIdentifierHash{std::numeric_limits<unsigned int>::max()};

            // statistics 
            unsigned int m_n_rejected_work{};

            // module offline ID as defined in InnerDetector/InDetDetDescr/InDetIdentifier/InDetIdentifier/PixelID.h
            uint32_t m_offlineID = 0;

            // front-end chip ID, from 0 to 4 for merged quads, 0 otherwise
            uint8_t m_chipID = 0;

            // RDO counter
            uint32_t m_n_rdos = 0;

            //offline identifier
            Identifier m_identifier{};

            //transform type for cabling package
            ITkPixelCabling::TransformType m_transform{ITkPixelCabling::TransformType::UndefinedTransform};

            // Identifier helper
            const PixelID* m_idHelper{};
            
            // Athena message stream for debug output
            MsgStream& m_msg_source;
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

