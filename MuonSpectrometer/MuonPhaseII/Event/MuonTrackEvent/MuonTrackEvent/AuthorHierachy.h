/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKEVENT_AuthorHierachy_H
#define MUONTRACKEVENT_AuthorHierachy_H

#include "xAODMuon/Muon.h"

#include "Acts/Utilities/PointerTraits.hpp"
#include "AthContainers/DataVector.h"

#include <concepts>

/** @brief Utility class to sort single objects (e.g. xAOD::Muon, MuonTags) or even collections of 
 *         MuonTags by their primary author. The order is defined by the purity in reconstructed muons
 *         and by the best expected momentum resolution:
 *              - MuidCo: Outside -> in combined reconstruction
 *              - MuGirl: Inside -> out combined reconstruction
 *              - STACO: Placeholder for the combined muon
 *              - MuTagIMO: Segment tagging
 *              - CaloScore: Calorimeter tagged muons
 *              - MuidSA: Standalone muon tracks */
namespace MuonR4 {

    namespace detail {
        /** @brief All objects can be arranged by the AuthorHierachy, if they satisfy the
          *        hasAuthor concept */
        template <typename ObjType> concept hasAuthor = requires(const ObjType& obj) {
            { obj.author() } -> std::same_as<xAOD::Muon::Author>;
        };
        /** @brief Check that the xAOD::Muon satisfies this constraint */
        static_assert(hasAuthor<xAOD::Muon>);
    }
    /** @brief Implemenation of the utility class. */
    struct AuthorHierachy {
        using Author = xAOD::Muon::Author;
        /** @brief Ranks the author according to the list above */
        unsigned authorRank(const Author author) const;
        /** @brief Sorting operator between two Author values based
         *         on the author rank*/
        bool operator()(const Author a, const Author b) const;
        /** @brief Ranks two objects satisfying the hasAuthor concept
         *         according to their Author rank */
        template <detail::hasAuthor ObjType>
        bool operator()(const ObjType&a ,const  ObjType& b) const {
            return (*this)(a.author(), b.author());
        }
        /** @brief Ranks two pointers according to the Author rank
         *         of the objects they are pointing to */
        template <Acts::PointerConcept ObjPtr>
        bool operator()(const ObjPtr& a, const ObjPtr& b) const  {
            assert(a != nullptr);
            assert(b != nullptr);
            return (*this)(*a, *b);
        }
        /** @brief Ranks two Data vector collections of objects satisfying the
         *         hasAuthor concept according to their Author rank. It is assumed
         *         that all objects in the collection have the same author. */
        template <detail::hasAuthor ObjType>
        bool operator()(const DataVector<ObjType>&a, const DataVector<ObjType>& b) const {
            if (a.empty() || b.empty()) {
                return !a.empty();
            }
            // Check in the debug build that there is only one author in each vector
            assert(std::all_of(a.begin(), a.end(), [&](const auto obj){
                              return obj->author() == a.at(0)->author();
                            }));
            assert(std::all_of(b.begin(), b.end(), [&](const auto obj){
                              return obj->author() == b.at(0)->author();
                            }));

            return (*this)(a.front(), b.front());
        }
        /** @brief Ranks two pointers of data vectors according to the authorRank of the
         *         underlying data vector */
        template <detail::hasAuthor ObjType>
        bool operator()(const DataVector<ObjType>* a, const DataVector<ObjType>* b) const {
            assert( a != nullptr);
            assert( b != nullptr);
            return (*this)(*a, *b);
        }
    };

}

#endif