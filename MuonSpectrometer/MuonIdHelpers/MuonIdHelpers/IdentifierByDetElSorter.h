/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONIDHELPERS_IDENTIFIERBYDETELSORTER_H
#define MUONIDHELPERS_IDENTIFIERBYDETELSORTER_H

#include <MuonIdHelpers/IMuonIdHelperSvc.h>

/** @brief The IdentifierByDetElSorter groups objects / Identfiers in a vector by their associated 
 *         detectorElement Identifier and then by their actual Identifier. In order to work, it needs
 *         to be constructed with an IMuonIdHelperSvc*/
namespace Muon{
    /** @brief Define the concept that the object needs to have an Identifier method  */
    template <typename T> concept hasIdentifyConcept = requires (const T theObj){
        theObj.identify(); 
    }; 
    class IdentifierByDetElSorter {
        public:
            IdentifierByDetElSorter(const Muon::IMuonIdHelperSvc* idHelperSvc):
                m_idHelperSvc{idHelperSvc}{}
            
            bool operator() (const Identifier& a, const Identifier& b) const{
                const Identifier detA = m_idHelperSvc->detElId(a);
                const Identifier detB = m_idHelperSvc->detElId(b);
                if (detA != detB){
                    return detA < detB;
                }
                return a < b;
            }
            template <typename T>
            bool operator () (const T* a, const T* b) const requires hasIdentifyConcept<T> {
                return (*this)(a->identify(), b->identify());
            }

            template <typename T>
            bool operator () (const T& a, const T&b) const requires hasIdentifyConcept<T> {
                return (*this)(a.identify(), b.identify());
            }

        private:
            const Muon::IMuonIdHelperSvc* m_idHelperSvc{};

    }; 
}

#endif

