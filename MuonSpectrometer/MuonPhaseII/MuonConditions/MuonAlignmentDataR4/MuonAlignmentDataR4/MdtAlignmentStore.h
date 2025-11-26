/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNMENTDATA_MDTALIGNMENTSTORE_H
#define MUONALIGNMENTDATA_MDTALIGNMENTSTORE_H


#include <ActsGeometryInterfaces/DetectorAlignStore.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h" //for moduleHash
#include <vector>

class BLinePar;
class MdtAsBuiltPar;
namespace Muon{
  class IMuonIdHelperSvc;
}
/**
 *  Helper struct to cache simulatenously the As-built and the
 *  BLine corrections of the Mdts for fast access within the new
 *  MdtReadout geometry
*/
class MdtAlignmentStore final: public ActsTrk::DetectorAlignStore::InternalAlignStore {
    public:
        MdtAlignmentStore(const Muon::IMuonIdHelperSvc* idHelperSvc);
        /// Helper struct to store the pointer to the 
        /// Mdt distrotion parameters, namely the As-built
        /// and the BLine chamber deformations
        struct chamberDistortions{
            const BLinePar* bLine{nullptr};
            const MdtAsBuiltPar* asBuilt{nullptr};
            operator bool() const {
                return bLine || asBuilt;
            }
        };
        /// Returns a chamber distortion that's cached for the corresponding Mdt chamber element
        chamberDistortions getDistortion(const Identifier& detElId) const {
            const unsigned int idx = m_idHelperSvc->moduleHash(detElId);
            assert(m_idHelperSvc->technologyIndex(detElId) == Muon::MuonStationIndex::TechnologyIndex::MDT);
            return  idx < m_alignMap.size() ? m_alignMap[idx] : chamberDistortions{};
        }
        void storeDistortion(const Identifier& detElId, const BLinePar* bline, const MdtAsBuiltPar* asBuilt);
     private:
        const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
        std::vector<chamberDistortions> m_alignMap{};
};

#endif