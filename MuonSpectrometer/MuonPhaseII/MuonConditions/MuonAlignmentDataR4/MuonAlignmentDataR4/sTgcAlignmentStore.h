/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNMENTDATA_STGCALIGNMENTSTORE_H
#define MUONALIGNMENTDATA_STGCALIGNMENTSTORE_H

#include <MuonAlignmentData/BLinePar.h>
#include <MuonAlignmentData/NswAsBuiltDbData.h>
#include <ActsGeometryInterfaces/DetectorAlignStore.h>
/*  Alignment store to additionally ship the sTgc as-built parameters and
 *  B-Line deformation parameters through the Acts geometry context.
 */
class sTgcAlignmentStore : public ActsTrk::DetectorAlignStore::InternalAlignStore {
    public:
        sTgcAlignmentStore() = default;
        /// @brief   Pointer to the collection of passivation parameters
        ///          if the project is not AthSimulation. Otherwise, it's a char
        using sTgcAsBuiltPtr = NswAsBuiltDbData::sTgcAsBuiltPtr;
        sTgcAsBuiltPtr asBuiltPars{};

        /// Caches the micromega BLine parameter. 
        void cacheBLine(const Identifier& detElId, const BLinePar& bline);
        /// Returns the micromega BLine parameter if it exists
        const BLinePar* getBLine(const Identifier& bLine) const {
           BLineCache::const_iterator itr = m_blines.find(bLine);
           if (itr != m_blines.end()) return itr->second;
           return nullptr;
        }
    private:
        using BLineCache = std::unordered_map<Identifier, const BLinePar*>;
        BLineCache m_blines{};
};

#endif