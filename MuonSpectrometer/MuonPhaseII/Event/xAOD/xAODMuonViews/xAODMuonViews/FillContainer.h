/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONVIEWS_FILLCONTAINER_H
#define XAODMUONVIEWS_FILLCONTAINER_H

#include "xAODCore/AuxContainerBase.h"
#include "AthContainers/DataVector.h"
#include "AthContainers/AuxElement.h"
#include "GaudiKernel/MsgStream.h"
#include "AthenaKernel/getMessageSvc.h"
#include "GeoModelKernel/throwExcept.h"
#include "StoreGate/WriteHandle.h"

#include <concepts>
namespace xAOD {

/** @brief Auxiliary class to create a xAOD container together with its associated
 *         AuxContainer without manually instantiating the corresponding pointers.
 *         Further, methods are provided to write the container to store gate using
 *         a passed WriteHandle key object */
namespace detail{
    /** @brief Define that the templated class needs to inherit from the AuxContainerBase class */
     template <typename AuxCont_t> concept AuxContainerConcept = std::is_base_of_v<xAOD::AuxContainerBase, AuxCont_t>;
    /** @brief Define the primary container */
    template <typename Cont_t> concept PrimaryContainerConcept = requires(const Cont_t& container) {
        requires std::is_base_of_v<SG::AuxElement, typename Cont_t::base_value_type>;
        requires std::is_same_v<Cont_t, DataVector<typename Cont_t::base_value_type>>;
    };
}

template <detail::PrimaryContainerConcept Cont_t, 
          detail::AuxContainerConcept AuxCont_t>
class FillContainer {
    public:
        /** @brief Abrivation of the write handle type */
        using WriteHandle_t = SG::WriteHandle<Cont_t>;
        /** @brief default constructor */
        FillContainer() {
            m_cont->setStore(m_auxCont.get());
        }

        /** @brief Move constructor */
        FillContainer(FillContainer&& other) = default;
        /** @brief Default move operator */
        FillContainer& operator=(FillContainer&& other) = default;
        /** @brief get operator */
        Cont_t* get() const { 
            if (!m_writeHandle) {
                return m_cont.get();
            }
            WriteHandle_t& handle ATLAS_THREAD_SAFE{*m_writeHandle};
            return handle.ptr(); 
        }
        /** @brief Define the arrow operator */
        Cont_t* operator->() const { return  get(); }
        /** @brief Define the dereference operator */
        Cont_t& operator*() const { return *get(); }
        /** @brief Record the container to store gate using the passed write handle
         *         key. If the key is empty nothing is written
         *  @param key: Reference to the StoreGate write handle key
         *  @param ctx: Event context to be passed to the StoreGate instance */
        StatusCode record(const SG::WriteHandleKey<Cont_t>& key,
                          const EventContext& ctx) {
            if (key.empty()) {
                return StatusCode::SUCCESS;
            }
            if (m_writeHandle) {
                MsgStream msg{Athena::getMessageSvc(), "FillContainer"};
                msg<<MSG::ERROR<<__func__<<"() - "<<__LINE__<<": Write handle under key "
                    <<m_writeHandle->key()<<" already exists."<<endmsg;
                return StatusCode::FAILURE;
            }
            m_writeHandle = std::make_unique<WriteHandle_t>(key, ctx);
            return m_writeHandle->record(std::move(m_cont), std::move(m_auxCont));
        }
        /** @brief Record the container to store gate using the passed write handle
         *         key. If the key is empty nothing is written
         *  @param key: Reference to the StoreGate write handle key
         *  @param ctx: Event context to be passed to the StoreGate instance */
        StatusCode recordNonConst(const SG::WriteHandleKey<Cont_t>& key,
                                  const EventContext& ctx) {
            if (key.empty()) {
                return StatusCode::SUCCESS;
            }
            if (m_writeHandle) {
                MsgStream msg{Athena::getMessageSvc(), "FillContainer"};
                msg<<MSG::ERROR<<__func__<<"() - "<<__LINE__<<": Write handle under key "
                    <<m_writeHandle->key()<<" already exists."<<endmsg;
                return StatusCode::FAILURE;
            }
            m_writeHandle = std::make_unique<WriteHandle_t>(key, ctx);
            return m_writeHandle->record(std::move(m_cont), std::move(m_auxCont));
        }
        /** @brief Returns the reference to the active write handle. Throws an 
         *         exception if the handle has not been instantiated yet. */
        WriteHandle_t& getHandle() {
            if (!m_writeHandle) {
                THROW_EXCEPTION("The write handle has not been initialized. Please call record or"
                              <<" recordNonConst before calling getHandle()");
            }
            return *m_writeHandle;
        }
    private:
        /** @brief Unique pointer to the container type */
        std::unique_ptr<Cont_t> m_cont{std::make_unique<Cont_t>()};
        /** @brief Unique pointer to the aux container type */
        std::unique_ptr<AuxCont_t> m_auxCont{std::make_unique<AuxCont_t>()};
        /** @brief Pointer to the write handle. Is instantiated with a record call */
        std::unique_ptr<WriteHandle_t> m_writeHandle{};
};

}
#endif