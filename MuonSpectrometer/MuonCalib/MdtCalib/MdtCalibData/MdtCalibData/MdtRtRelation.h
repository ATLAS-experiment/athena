/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_MDTRTRELATION_H
#define MUONCALIB_MDTRTRELATION_H

#include "GeoModelUtilities/TransientConstSharedPtr.h"
#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/IRtResolution.h"
#include "MdtCalibData/ITrRelation.h"

#include <iostream>
#include <memory>

namespace MuonCalib {

    /** class which holds calibration constants per rt-region */
    class MdtRtRelation {
    public:
        MdtRtRelation(IRtRelationPtr rt, IRtResolutionPtr reso, ITrRelationPtr tr = nullptr);
        ~MdtRtRelation() = default;
        inline const IRtRelation* rt() const { return m_rt.get(); }          //!< rt relation
        inline const IRtResolution* rtRes() const { return m_rtRes.get(); }  //!< resolution
        inline const ITrRelation* tr() const { return m_tr.get(); }           //!< t(r) relationship

        /// Smart pointer access
        const IRtRelationPtr& smartRt() const { return m_rt; }
        const ITrRelationPtr& smartTr() const { return m_tr; }
        const IRtResolutionPtr& smartReso() const { return m_rtRes; }
    private:
        IRtRelationPtr m_rt{};
        IRtResolutionPtr m_rtRes{};
        ITrRelationPtr m_tr{};
    };

}  // namespace MuonCalib

#endif
