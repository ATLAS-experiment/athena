/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @brief Template utilities for merging sorted xAOD MuonMeasurement containers
 * 
 * This header provides template functions and concepts for efficiently merging
 * containers of MuonMeasurement objects while maintaining sorted order based on
 * detector element identifiers.
 * 
 * The merge operations use IdentifierSorter to determine the correct insertion
 * position for elements, ensuring the output container remains sorted throughout
 * the merge process.
 */

#ifndef XAODMUONVIEWS_MERGECONTAINER_H
#define XAODMUONVIEWS_MERGECONTAINER_H

#include "AthContainers/ConstDataVector.h"
#include "xAODMuonPrepData/MuonMeasurement.h"
#include "xAODMuonViews/IdentifierSorter.h"
#include "xAODMuonViews/FillContainer.h"

namespace xAOD{
    /** @brief Internal implementation details and type concepts for container merging */
    namespace detail {
        /**
         * @concept isMuonMeasurement
         * @brief Concept that verifies a type is derived from xAOD::MuonMeasurement
         * 
         * @tparam T The type to check
         */
        template <typename T>concept isMuonMeasurement  = std::is_base_of_v<xAOD::MuonMeasurement, T>;

        /**
         * @concept isMuonMeasurementCont
         * @brief Concept that verifies a type is a valid MuonMeasurement container
         * 
         * A valid container must satisfy PrimaryContainerConcept and contain elements
         * that are derived from xAOD::MuonMeasurement.
         * 
         * @tparam Cont_t The container type to check
         */
        template <typename Cont_t> concept isMuonMeasurementCont = 
            PrimaryContainerConcept<Cont_t> &&
            std::is_base_of_v<xAOD::MuonMeasurement, typename Cont_t::base_value_type>;

    }


    /**
     * @brief Merges a range of MuonMeasurement objects into a sorted output container
     * 
     * This function inserts elements from the input range into the output container,
     * maintaining sorted order according to IdentifierSorter. The input elements are
     * assumed to already be sorted.
     * 
     * If the output container is empty, elements are simply inserted at the end.
     * Otherwise, each input element is inserted at the correct sorted position.
     * 
     * @tparam inT The type of elements in the input range (must derive from MuonMeasurement)
     * @tparam outT The container type of the output (must contain MuonMeasurement-derived elements)
     * 
     * @param begin Iterator to the beginning of the input range
     * @param end Iterator to the end of the input range
     * @param outContainer The output container to merge elements into
     * 
     * @pre Input elements [begin, end) must be sorted according to IdentifierSorter
     * @pre outT::base_value_type must be a base class of inT
     * 
     * @note Output container may be reallocated if reserve() is called
     */
    template <detail::isMuonMeasurement inT,
              detail::isMuonMeasurementCont outT>
    void mergeInRange(typename DataVector<inT>::const_iterator begin,
                      const typename DataVector<inT>::const_iterator end,
                      ConstDataVector<outT>& outContainer) 
        requires(std::is_base_of_v<typename outT::base_value_type, inT>) {
        outContainer.reserve(std::distance(begin, end));
        if (outContainer.empty()) {
            outContainer.insert(outContainer.end(), begin, end);
            return;
        }
        MuonR4::IdentifierSorter sorter{};
        typename ConstDataVector<outT>::iterator outItr = outContainer.begin();
        for (; begin != end; ++begin){
            /// assume that the container is sorted under the identifier sorter
            assert(begin + 1 == end || sorter(*begin, *(begin + 1)));
            outItr = std::find_if(outItr, outContainer.end(),
                                [&](const outT::base_value_type* obj){
                                    return !sorter(obj, *begin);
                                });
            outItr = outContainer.insert(outItr, *begin);
        }
    }

    /**
     * @brief Merges a single container of MuonMeasurement objects into an output container
     * 
     * This is a convenience wrapper around mergeInRange() that merges an entire
     * input container into the output container while maintaining sorted order.
     * 
     * @tparam outT The output container type (must contain MuonMeasurement-derived elements)
     * @tparam inT The input element type (must derive from MuonMeasurement)
     * 
     * @param outContainer The output container to merge elements into
     * @param inCont The input container to merge from
     * 
     * @pre outT::base_value_type must be a base class of inT
     * @pre Input container must be sorted according to IdentifierSorter
     * 
     * @see mergeInRange()
     */
    template <detail::isMuonMeasurementCont outT,
              detail::isMuonMeasurement inT>
    void mergeContainer(ConstDataVector<outT>& outContainer,
                        const DataVector<inT>& inCont) {
        mergeInRange<inT>(inCont.begin(), inCont.end(), outContainer);
    }

    /**
     * @brief Merges two containers of MuonMeasurement objects into an output container
     * 
     * This function performs a sorted merge of two input containers into the output
     * container, similar to the merge step in merge-sort. Elements from both input
     * containers are interleaved based on IdentifierSorter ordering to maintain
     * a globally sorted output.
     * 
     * The algorithm uses two pointers to traverse both input containers simultaneously,
     * selecting the next smallest element according to IdentifierSorter and inserting
     * it at the correct position in the output container.
     * 
     * @tparam outT The output container type (must contain MuonMeasurement-derived elements)
     * @tparam inT1 The first input element type (must derive from MuonMeasurement)
     * @tparam inT2 The second input element type (must derive from MuonMeasurement)
     * 
     * @param outContainer The output container to merge elements into
     * @param inCont1 The first input container to merge from
     * @param inCont2 The second input container to merge from
     * 
     * @pre Both input containers must be sorted according to IdentifierSorter
     * @pre outT::base_value_type must be a base class of both inT1 and inT2
     * 
     * @complexity O(n + m + k) where n = inCont1.size(), m = inCont2.size(),
     *            and k = outContainer.size() (due to insertion operations)
     * 
     * @note Uses two-pointer merging algorithm to efficiently combine sorted sequences
     * 
     * @see mergeInRange()
     */
    template <detail::isMuonMeasurementCont outT,
              detail::isMuonMeasurement inT1,
              detail::isMuonMeasurement inT2>
    void mergeContainer(ConstDataVector<outT>& outContainer,
                        const DataVector<inT1>& inCont1,
                        const DataVector<inT2>& inCont2) 
        requires(std::is_base_of_v<typename outT::base_value_type, inT1> &&
                 std::is_base_of_v<typename outT::base_value_type, inT2>){
        const std::size_t startSize = outContainer.size();
        outContainer.reserve(startSize + inCont1.size() + inCont2.size());
        typename DataVector<inT1>::const_iterator begin1 = inCont1.begin();
        typename DataVector<inT2>::const_iterator begin2 = inCont2.begin();
       
        const typename DataVector<inT1>::const_iterator end1 = inCont1.end();
        const typename DataVector<inT2>::const_iterator end2 = inCont2.end();
        MuonR4::IdentifierSorter sorter{};
        typename ConstDataVector<outT>::iterator insert = outContainer.begin();
        while ( begin1!= end1 || begin2 != end2) {
            if (begin1== end1) {
                mergeInRange<inT2>(begin2, end2, outContainer);
                break;
            } else if (begin2 == end2) {
                mergeInRange<inT1>(begin1, end1, outContainer);
                break;
            } else {
                const typename outT::base_value_type* mergeMe = nullptr;
                if (sorter(*begin1, *begin2)){
                    mergeMe = *begin1;
                    ++begin1;
                } else {
                    mergeMe = *begin2;
                    ++begin2;
                }
                insert = std::find_if(insert, outContainer.end(),
                                    [&](const outT::base_value_type* obj){
                                        return !sorter(obj, mergeMe);
                                    });
                insert = outContainer.insert(insert, mergeMe);
            }
        }
        assert(inCont1.size() + inCont2.size() + startSize == outContainer.size());
    }
}


#endif
