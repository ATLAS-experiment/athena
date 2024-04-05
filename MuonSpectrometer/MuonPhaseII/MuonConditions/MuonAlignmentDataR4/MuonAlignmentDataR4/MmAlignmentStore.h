/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONALIGNMENTDATA_MMALIGNMENTSTORE_H
#define MUONALIGNMENTDATA_MMALIGNMENTSTORE_H

#include <MuonAlignmentData/BLinePar.h>
#include <MuonAlignmentData/NswPassivationDbData.h>
#include <MuonAlignmentData/NswAsBuiltDbData.h>
#include <ActsGeometryInterfaces/DetectorAlignStore.h>
/*  Alignment store class to additionally ship the Micromega passivation,
 *  the As built parameters and the BLines through the Acts Geometry context
 * 
 */
class MmAlignmentStore: public ActsTrk::DetectorAlignStore::InternalAlignStore  {
    public:
        MmAlignmentStore() = default;
        /// @brief  Passivation is subdivided into several PCBs
        ///         Ship the complete container with the alignment store
        const NswPassivationDbData* passivation{nullptr};
        /// @brief   Pointer to the collection of passivation parameters
        ///          if the project is not AthSimulation. Otherwise, it's a char
        using MmAsBuiltPtr = NswAsBuiltDbData::MmAsBuiltPtr;
        MmAsBuiltPtr asBuiltPars{};

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