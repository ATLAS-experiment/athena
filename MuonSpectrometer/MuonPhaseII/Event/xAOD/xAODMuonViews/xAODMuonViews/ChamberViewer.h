/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_CHAMBERVIEWER_H
#define XAODMUONPREPDATA_CHAMBERVIEWER_H

#include <type_traits>

#include <xAODMeasurementBase/MeasurementDefs.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <GeoModelKernel/throwExcept.h>

#define BUILD_TRAIT(traitName, conceptPass) \
    template <typename T> struct traitName{}; \
    template <conceptPass T> struct traitName<T>{static constexpr bool value = true;}; \
    template <typename T> requires (!conceptPass <T>) struct traitName<T>{static constexpr bool value = false;};

namespace xAOD{
    /** @brief Under the assumption that all measurements in an uncalibrated measurement container are sorted by their
     *         IdentifierHash which is the unique hash of the associated ReadoutElement, the UnCalibMeasViewer provides
     *         a begin & end iterator where all measurements in between share the same identifierHash. That allows for 
     *         convenient range based for loops restricted over all measurements in chamber. A minimal example of how to use
     *         the ChamberMeasViewer is given below.
     *         
     *      /// Fetch some pointer to the full measruement container
     *      const xAOD::UnCalibMeasurementContainer* allMeasurements{};
     *         
     *      ChamberMeasViewer viewer{*allMeasurements};
     *      /// Start the loop over the chambers
     *      do {
     *         for (const xAOD::UnCalibMeasurement* meas : viewer) {
     *         }
     *      } 
     *      /// Load the hits from the next chamber
     *      while(viewer.next());  */
    
    
    namespace ChamberViewConcepts{
        /** @brief Define the concept that the object needs to have an Identifier method  */
        template <typename ObjType> concept hasIdentifyConcept = requires (const ObjType theObj) {
            theObj.identify(); 
        };
       /** @brief Define the concept that the object needs to have an IdentifierHash method  */
        template <typename ObjType> concept hasIdentifierHashConcept = requires(const ObjType theObj) {
            theObj.identifierHash();
        };

        BUILD_TRAIT(hasIdentify, hasIdentifyConcept)
        BUILD_TRAIT(hasIdentifyHash, hasIdentifierHashConcept)
        template<class HitObjContainer> concept ContainerConcept =
            (hasIdentify<typename std::remove_pointer_t<typename HitObjContainer::value_type>>::value ||
             hasIdentifyHash<typename  std::remove_pointer_t<typename HitObjContainer::value_type>>::value);
    }

    namespace ChamberView {
        /** @brief Switch setting the view mode if the chamber viewer is initialized with the IdHelperSvc */
        enum class Mode{
            DetElement /** @brief View ends if the detElementHash changes */,
            Chamber /** @brief View ends if the moduleHash changes */
        };
    }

      
    template<ChamberViewConcepts::ContainerConcept HitObjContainer>
        class ChamberViewer {
            public:
                using value_type = typename HitObjContainer::value_type;
                using element_type = typename std::remove_pointer_t<value_type>;
                /** @brief Type trait to find the proper refernce type for the lambda function access */
                template <typename HitObjType> struct ref_trait{};
                /** @brief Branch if the input value type is a pointer */
                template <typename HitObjType> requires (std::is_pointer_v<HitObjType>) struct ref_trait<HitObjType>{
                    /** Convert to the underlying object itself  */
                    using element_type = typename std::remove_pointer_t<HitObjType>;
                    /** Make the object to be const  */
                    using const_type = typename std::add_const_t<element_type>;
                    /** Declare the type to be a const pointer */
                    using type = std::add_pointer_t<const_type>; 
                };
                /** @brief Branch if the input value is an object */
                template <typename HitObjType> requires (!std::is_pointer_v<HitObjType>)struct ref_trait<HitObjType>{
                    /** @brief Const type */
                    using const_type = typename std::add_const_t<HitObjType>;
                    /** @brief Const reference type */
                    using type = typename std::add_lvalue_reference<const_type>::type;
                };
                using const_ref = ref_trait<value_type>::type;
                using ViewMode = ChamberView::Mode;
                
                /** @brief Standard constructor
                 *  @param container: UncalibratedMeasurementContainer from which the views per chamber shall be generated*/
                ChamberViewer(const HitObjContainer& container) noexcept:
                    m_container{container} {
                    static_assert(ChamberViewConcepts::hasIdentifyHash<element_type>::value, "Object needs to provide identifierHash" );
                    next();
                }
                /** @brief Standard constructor
                 *  @param container: UncalibratedMeasurementContainer from which the views per chamber shall be generated*/               
                ChamberViewer(const HitObjContainer& container, 
                             const Muon::IMuonIdHelperSvc* idHelperSvc,
                             const ViewMode mode = ViewMode::DetElement) noexcept:
                    m_container{container},
                    m_idHelperSvc{idHelperSvc},
                    m_mode{mode} {
                    static_assert(ChamberViewConcepts::hasIdentify<element_type>::value, "Object needs to provide identify()" );
                    next();
                }
                /** @brief Delete the copy constructor */
                ChamberViewer(const ChamberViewer& other) = delete;
                /** @brief Delete the copy assignment operator */
                ChamberViewer& operator=(const ChamberViewer& other) = delete;
                /** @brief Standard move constructor */
                ChamberViewer(ChamberViewer&& other) = default;
                /** @brief Standard move operator */
                ChamberViewer& operator=(ChamberViewer&& other) = default;
                /** @brief Begin iterator of the current chamber view */
                HitObjContainer::const_iterator begin() const noexcept{
                    return m_begin;
                }
                /** @brief End iterator of the current chamber view */
                HitObjContainer::const_iterator end() const noexcept {
                    return m_end;
                }
                /** @brief Returns how many hits are in the current chamber */
                std::size_t size() const noexcept{
                    return std::distance(m_begin, m_end);
                }
                /** @brief Returns the i-the measurement from the current chamber */
                HitObjContainer::const_iterator::reference at(const std::size_t idx) const {
                    if (idx >= size()) {
                        THROW_EXCEPTION("Invalid index given "<<typeid(value_type).name()<<" Size: "<<size()<<", requested: "<<idx);
                    }
                    return (*m_begin +idx);
                }
                /** @brief Loads the hits from the next chamber. 
                 *         Returns false if all chambers have been traversed. */
                bool next() noexcept {
                    if (m_end == m_container.end()) {
                        return false;
                    }
                    m_begin = m_end;
                    if constexpr (ChamberViewConcepts::hasIdentifyHash<element_type>::value) {
                        m_currentHash = (*m_end)->identifierHash();
                        m_end = std::find_if(m_begin, m_container.end(),
                                         [this](const_ref meas){
                                            return meas->identifierHash() != m_currentHash;
                                        });
                    } else {
                         m_currentHash = idHash((*m_end)->identify());
                         m_end = std::find_if(m_begin, m_container.end(),
                                         [this](const_ref meas){
                                            return idHash(meas->identify()) != m_currentHash;
                                        });                       
                    }
                    if (m_begin == m_end) return next(); // veto empty views
                    return true;
                }

                /** @brief  Loads the view matching the parsed identifier. I.e. the collection of hits sharing
                 *          the same IdentifierHash. Returns whether the view is empty or not
                 * @param chamberId: Identifier from the chamber / detElement to consider */
                bool loadView(const Identifier& chamberId) {
                    static_assert(ChamberViewConcepts::hasIdentify<element_type>::value, "Object needs to provide identify()" );
                    const IdentifierHash detId = idHash(chamberId);
                    m_begin = std::ranges::find_if(m_container,[this,&detId](const_ref meas) {
                                                        return idHash(meas->identify()) == detId;
                                                   });
                    m_end = std::find_if(m_begin, m_container.end(),[this,&detId](const_ref meas) {
                                                        return idHash(meas->identify()) != detId;
                                                   });
                    return m_begin != m_end;
                }
                /** @brief Loads the view matching the parsed IdentifierHash. I.e. either the detector element hash or 
                 *         the chamber module hash. There's no cross-check whether the interpretion of both hashes is the same
                 *  @param idHash: IdentifierHash to search */
                bool loadView(const IdentifierHash& idHash) {
                    static_assert(ChamberViewConcepts::hasIdentifyHash<element_type>::value, "Object needs to provide identify()" );
                    m_begin = std::ranges::find_if(m_container,[&idHash](const_ref meas) {
                                                        return meas->identifierHash() == idHash;
                                                   });
                    m_end = std::find_if(m_begin, m_container.end(),[&idHash](const_ref meas) {
                                                        return meas->identifierHash() != idHash;
                                                   });
                     return m_begin != m_end;
                }

                bool loadView(std::function<bool(const_ref)> selector) {
                    m_begin = std::ranges::find_if(m_container, selector);
                    m_end = std::find_if(m_begin, m_container.end(),
                                         [&selector](const_ref meas) {
                                            return !selector(meas);
                                        });
                    return m_begin != m_end;
                }

            private:
                /** @brief Returns the IdentifierHash from an Identifier */
                IdentifierHash idHash(const Identifier& id) const {
                    return m_mode == ViewMode::DetElement ? m_idHelperSvc->detElementHash(id)
                                                          : m_idHelperSvc->moduleHash(id);
                }
                const HitObjContainer& m_container;
                const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
                const ViewMode m_mode{ViewMode::DetElement};
                DetectorIDHashType m_currentHash{0};
                HitObjContainer::const_iterator m_end{m_container.begin()};
                HitObjContainer::const_iterator m_begin{m_container.begin()};
    };
}
#undef buildTrait
#endif
