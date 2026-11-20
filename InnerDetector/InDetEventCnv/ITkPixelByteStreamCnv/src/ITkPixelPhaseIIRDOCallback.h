/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_PHASEIIRDOCALLBACK_H
#define ITKPIXEL_PHASEIIRDOCALLBACK_H

#include "InDetIdentifier/PixelID.h"
#include "Identifier/Identifier.h"
#include "InDetRawData/PhaseIIPixelRawDataContainerMT.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"
#include "itksw/pix/endec/intf/Errors.hpp"
#include "itksw/pix/endec/DecStreamDefs.hpp"
#include <array>
#include <limits>
#include <cstdint>

namespace PixelCallbacksPhaseIIRDO {

//Forms pixel RDOs from the decoded hits & fills them into a container
class PhaseIIRDOCallback {
    public:
        explicit PhaseIIRDOCallback(PhaseIIPixelRawDataContainerMT* cont_coll,
                PhaseIIPixelRawDataContainerMT::ContainerPtr rdo_container_dest,
                const PixelID* idHelper,
                bool transform_coords = true) :
            m_rdo_container_dest(rdo_container_dest),
            m_cont_coll(cont_coll),
            m_dest_range_guard(rdo_container_dest),
            m_currentIdentifierHash(std::numeric_limits<unsigned int>::max()),
            m_idHelper(idHelper),
            m_transformCoordinates(transform_coords)
            {};

        ~PhaseIIRDOCallback() = default;

        inline void evt_init([[maybe_unused]] uint8_t tag) {
            m_identifier = Identifier(m_offlineID);
            const auto waferHash = m_idHelper->wafer_hash(m_identifier);
            if (waferHash != m_currentIdentifierHash) {
                registerLastModule();
                m_currentIdentifierHash = waferHash;
                m_dest_range_guard =
                    PhaseII::ContainerRangeGuard<
                        PhaseII::DataRange,
                        PhaseIIPixelRawDataContainerMT::ContainerPtr
                    >(m_rdo_container_dest);
            }
        }

        inline void evt_next([[maybe_unused]] uint8_t tag) {};
        inline void evt_done() {};

        inline void add_hit(uint16_t col, uint16_t row, uint16_t tot) {
            if (m_transformCoordinates) {
                ITkPixelCabling::chipToModuleTransform(m_transform,m_chipID,col,row);
            }

            const int bcid = 0;
            const int lvl0a = 0;
            const int lvl0d = 0;

            PhaseII::addDataForModule(*m_cont_coll, m_dest_range_guard,
                std::array<std::int16_t, 2>{
                    static_cast<std::int16_t>(row),
                    static_cast<std::int16_t>(col)
                },
                PhaseII::PixelRawDataContainer::makeWord(
                    tot, bcid, lvl0a, lvl0d
                )
            );
        }
        inline void add_hmap([[maybe_unused]] uint8_t qcol,
                [[maybe_unused]] uint8_t qrow,
                [[maybe_unused]] uint16_t hmap,
                [[maybe_unused]] uint64_t tots)
        {};

        inline void add_qcore([[maybe_unused]] uint8_t qcol,
                [[maybe_unused]] uint8_t qrow,
                [[maybe_unused]] uint64_t qtot)
        {};

        inline itksw::pix::endec::intf::ErrorRecoveryStrategy on_error(
                itksw::pix::endec::intf::EventError error,
                std::optional<itksw::pix::endec::ErrorContext>)
        {
            std::ostringstream oss;
            oss << "pixel decoding error: " << error.to_string();
            throw std::runtime_error(oss.str());
            return itksw::pix::endec::intf::ErrorRecoveryStrategy::CONTINUE_DECODING;
        };
        //end of interface-mandated methods

        inline void registerLastModule() {
            if (!m_dest_range_guard.empty()) {
                if (!m_cont_coll->registerOrEraseNewData(
                        m_currentIdentifierHash,
                        m_dest_range_guard.range()))
                {
                    std::ostringstream oss;
                    oss << "pixel decoding error: "
                        << "data for this module was already registered, older data were discarded"
                        << ", module hash: " << m_currentIdentifierHash;
                    throw std::runtime_error(oss.str());
                }

                m_rdo_container_dest = m_dest_range_guard.ptr();
                m_n_rdos += m_dest_range_guard.range().size();
            }
        }
        inline void setOfflineID(const uint32_t& offlineID){
            m_offlineID = offlineID;
        };

        inline void setChipID(const uint8_t& chipID){
            m_chipID = chipID;
        }

        inline void setTransformType(const ITkPixelCabling::TransformType& transform){
            m_transform = transform;
        }

        uint32_t getNbRDOs() const { return m_n_rdos; }

    private:
        // the RDO (=hits) container
        PhaseIIPixelRawDataContainerMT::ContainerPtr m_rdo_container_dest;

        // the collection of RDO containers
        PhaseIIPixelRawDataContainerMT* m_cont_coll{};

        // RDO container range guard
        PhaseII::ContainerRangeGuard<PhaseII::DataRange, PhaseIIPixelRawDataContainerMT::ContainerPtr> m_dest_range_guard;

        // current offline ID hash
        unsigned int m_currentIdentifierHash{std::numeric_limits<unsigned int>::max()};

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

        // Whether the coordinates of hits are transformed according to transform type and chip ID
        const bool m_transformCoordinates;
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

        inline itksw::pix::endec::intf::ErrorRecoveryStrategy on_error(
                itksw::pix::endec::intf::EventError error,
                std::optional<itksw::pix::endec::ErrorContext>)
        {
            m_msg_source << MSG::DEBUG << "on_error(" << error.to_string() << ")" << std::endl;
            return itksw::pix::endec::intf::ErrorRecoveryStrategy::CONTINUE_DECODING;
        }

        MsgStream& m_msg_source;

};

} // PixelCallbacksPhaseIIRDO namespace

#endif
