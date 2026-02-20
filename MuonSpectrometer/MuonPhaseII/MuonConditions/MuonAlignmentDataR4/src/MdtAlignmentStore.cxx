/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonAlignmentDataR4/MdtAlignmentStore.h>
#include <MuonAlignmentData/MdtAsBuiltPar.h>
#include <MuonAlignmentData/BLinePar.h>
#include "GeoModelKernel/throwExcept.h"



MdtAlignmentStore::MdtAlignmentStore(const Muon::IMuonIdHelperSvc* idHelperSvc):
    m_idHelperSvc{idHelperSvc}{
    /// Reserve enough space in the vector
    m_alignMap.resize(m_idHelperSvc->mdtIdHelper().module_hash_max());
}
void MdtAlignmentStore::storeDistortion(const Identifier& detElId, const BLinePar* bline, const MdtAsBuiltPar* asBuilt) {
    if (!bline && !asBuilt) return;
    unsigned int idx = m_idHelperSvc->moduleHash(detElId);
    if (idx >= m_alignMap.size()) {
        THROW_EXCEPTION("Invalid hash for "<<m_idHelperSvc->toString(detElId));
    }
    chamberDistortions& distorts = m_alignMap[idx];
    if (distorts &&(distorts.bLine != bline || distorts.asBuilt != asBuilt)) {
        THROW_EXCEPTION("The alignment parameter "<<m_idHelperSvc->toString(detElId)<<" is already cached ");
    }
    distorts.bLine = bline;
    distorts.asBuilt = asBuilt;
}