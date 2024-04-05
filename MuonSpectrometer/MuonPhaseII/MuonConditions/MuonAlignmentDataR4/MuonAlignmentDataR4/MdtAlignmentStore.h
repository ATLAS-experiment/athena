/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNMENTDATA_MDTALIGNMENTSTORE_H
#define MUONALIGNMENTDATA_MDTALIGNMENTSTORE_H

#include <MuonAlignmentData/BLinePar.h>
#include <MuonAlignmentData/MdtAsBuiltPar.h>
#include <ActsGeometryInterfaces/DetectorAlignStore.h>
/**
 *  Helper struct to cache simulatenously the As-built and the
 *  BLine corrections of the Mdts for fast access within the new
 *  MdtReadout geometry
*/
class MdtAlignmentStore: public ActsTrk::DetectorAlignStore::InternalAlignStore {
    public:
        MdtAlignmentStore() = default;
        /// Helper struct to store the pointer to the 
        /// Mdt distrotion parameters, namely the As-built
        /// and the BLine chamber deformations
        struct chamberDistortions{
            const BLinePar* bLine{nullptr};
            const MdtAsBuiltPar* asBuilt{nullptr};
        };
        /// Returns a chamber distortion that's cached for the corresponding Mdt detector element
        chamberDistortions getDistortion(const Identifier& detElId) const {
            alignMap::const_iterator itr = m_alignMap.find(detElId);
            return itr != m_alignMap.end() ? itr->second : chamberDistortions{};
            return chamberDistortions{};
        }
        void storeDistortion(const Identifier& detElId, const BLinePar* bline, const MdtAsBuiltPar* asBuilt);
     private:
        using alignMap = std::unordered_map<Identifier, chamberDistortions>;
        alignMap m_alignMap{};
};

#endif