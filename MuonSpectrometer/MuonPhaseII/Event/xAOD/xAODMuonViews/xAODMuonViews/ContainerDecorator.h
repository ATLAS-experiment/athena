/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONVIEWS_CONTAINERDECORATOR_H
#define XAODMUONVIEWS_CONTAINERDECORATOR_H
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthContainers/AuxElement.h"
#include "xAODMuonViews/FillContainer.h"
#include <unordered_set>

namespace xAOD{
    /** @brief Auxiliary class to instantiate WriteDecorHandles.The handles can be created in 
     *         an empty state. Once initialized, a default value is decorated to all objects
     *         in the container to ensure that the decoration is always set. */
    template<detail::PrimaryContainerConcept Cont_t, typename dType>
    class ContainerDecorator {
        public:
            using Handle_t = SG::WriteDecorHandle<Cont_t, dType>;
            /** Empty default constructor */
            explicit ContainerDecorator() = default;
            /** Construct the decorator with an initialized decorHandleKey,
             *  and the current EventContext
             *  @param key: The initialized decorHandleKey with which the 
             *              decorator is instantitated
             * @param ctx: EventContext to gain access to the underlying continer
             * @param defValue: Default value to be decorated onto the entire container
             *                  at instantiation */
            explicit ContainerDecorator(const SG::WriteDecorHandleKey<Cont_t>& key,
                                        const EventContext& ctx,
                                        dType defValue = {}) {
                initialize(key, ctx, std::move(defValue));
            }
            /** @brief Destructor */
            ~ContainerDecorator()  {
                initDefault();
            }
            /** @brief Default move constructor */
            ContainerDecorator(ContainerDecorator&& other) = default;
            /** @brief Default move assignment operator */
            ContainerDecorator& operator=(ContainerDecorator&& other) = default;

            /** @brief Instantiate the handle
             *  @param key: WriteDecorationKey to be used
             *  @param ctx: The current event context
             *  @param defValue: Default value to be used for all objects */
            void initialize(const SG::WriteDecorHandleKey<Cont_t>& key,
                            const EventContext& ctx,
                            dType defValue = {}) {
                if (key.empty()) {
                    m_decorHandle.reset();
                    return;
                }
                if (m_decorHandle) {
                    THROW_EXCEPTION("The Container decorator is already initialized to "<<m_decorHandle->fullKey()<<"."
                                <<m_decorHandle->decorKey());
                }
                m_decorHandle = std::make_unique<Handle_t>(key, ctx);
                m_defValue = std::move(defValue);
                initDefault();
            }
            /** @brief Access the decoration value */
            dType& operator()(const SG::AuxElement& auxElem) {
                if (!m_decorHandle) [[unlikely]] {
                    THROW_EXCEPTION("No decorator has been defined. Please use initialize() beforhand");
                }
                m_seenAux.insert(&auxElem);
                return (*m_decorHandle)(auxElem);
            }
            /** @brief Returns whether the handle is instantiated  */
            constexpr bool isValid() const { return m_decorHandle != nullptr; }
            /** @brief Returns whether the handle is instantiated */
            constexpr operator bool () const {return isValid(); }
            /** @brief Returns the underlying container */
            const Cont_t* container() const { return isValid() ? m_decorHandle->cptr() : nullptr; }
            /** @brief Returns the pointer to the underlying container */
            const Cont_t* operator->() const { return container(); }
        private:
            /** @brief Decorate the default value to all elements in the container */
            void initDefault() {
                if (!m_decorHandle){
                    return;
                } 
                if(!m_decorHandle->isPresent() ) {
                    return;
                }
                for (const SG::AuxElement* auxElem : **m_decorHandle) {
                    if (m_seenAux.insert(auxElem).second) {
                        (*m_decorHandle)(*auxElem) = m_defValue;
                    }
                }
            }
            std::unique_ptr<Handle_t> m_decorHandle{};
            std::unordered_set<const SG::AuxElement*> m_seenAux{};
            dType m_defValue{};
    };
    /** @brief Factory method for a vector of ContainerDecorators instantiated from a 
      *         WriteDecorHandleKeyArray
      * @param keyArray: The reference to the WriteDecorHandleKeyArray from which the particular
      *                  keys are taken
      * @param ctx: The current event context to instantiate each ContainerDecorator
      * @param defValue: The default value that shall be decorated to each element */
    template<detail::PrimaryContainerConcept Cont_t, typename dType>
    std::vector<ContainerDecorator<Cont_t, dType>> createDecorators(const SG::WriteDecorHandleKeyArray<Cont_t>& keyArray,
                                                                    const EventContext& ctx,
                                                                    dType defValue = {}) {
        std::vector<ContainerDecorator<Cont_t, dType>> retMe{};
        for (const SG::WriteDecorHandleKey<Cont_t>& key : keyArray) {
            retMe.emplace_back(key, ctx, defValue);
        }
        return retMe;
    }
}

#endif
