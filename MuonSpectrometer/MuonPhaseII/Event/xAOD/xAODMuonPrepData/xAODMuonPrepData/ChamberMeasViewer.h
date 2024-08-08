/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_CHAMBERVIEWER_H
#define XAODMUONPREPDATA_CHAMBERVIEWER_H

#include <type_traits>

#include <AthContainers/DataVector.h>
#include <xAODMeasurementBase/UncalibratedMeasurement.h>
#include <GeoModelHelpers/throwExcept.h>

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
    
    template<class MeasType> requires std::is_base_of_v<UncalibratedMeasurement, MeasType>
        class ChamberMeasViewer {
            public:                
                /** @brief Standard constructor
                 *  @param container: UncalibratedMeasurementContainer from which the views per chamber shall be generated*/
                ChamberMeasViewer(const DataVector<MeasType>& container) noexcept:
                    m_container{container} {
                    next();
                }
                /** @brief Delete the copy constructor */
                ChamberMeasViewer(const ChamberMeasViewer& other) = delete;
                /** @brief Delete the copy assignment operator */
                ChamberMeasViewer& operator=(const ChamberMeasViewer& other) = delete;

                /** @brief Begin iterator of the current chamber view */
                DataVector<MeasType>::const_iterator begin() const noexcept{
                    return m_begin;
                }
                /** @brief End iterator of the current chamber view */
                DataVector<MeasType>::const_iterator end() const noexcept {
                    return m_end;
                }
                /** @brief Returns how many hits are in the current chamber */
                std::size_t size() const noexcept{
                    return std::distance(m_begin, m_end);
                }
                /** @brief Returns the i-the measurement from the current chamber */
                const MeasType* at(const std::size_t idx) const {
                    if (idx >= size()) {
                        THROW_EXCEPTION("Invalid index given "<<typeid(MeasType).name()<<" Size: "<<size()<<", requested"<<idx);
                    }
                    return (*m_begin +idx);
                }
                /** @brief Loads the hits from the next chamber. 
                 *         Returns false if all chambers have been traversed. */
                bool next() noexcept {
                    if (m_end == m_container.end()) {
                        return false;
                    }
                    m_currentHash = (*m_end)->identifierHash();
                    m_begin = m_end;
                    m_end = std::find_if(m_begin, m_container.end(),[this](const MeasType* meas){
                                            return meas->identifierHash() != m_currentHash;
                                        });
                    return true;
                }

            private:               
                const DataVector<MeasType>& m_container;
                DetectorIDHashType m_currentHash{0};
                DataVector<MeasType>::const_iterator m_begin{m_container.begin()};
                DataVector<MeasType>::const_iterator m_end{m_container.begin()};
    };
}
#endif