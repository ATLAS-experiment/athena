/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
        MdtRtRelation(GeoModel::TransientConstSharedPtr<IRtRelation> rt, 
                      GeoModel::TransientConstSharedPtr<IRtResolution> reso, double t0);
        MdtRtRelation(GeoModel::TransientConstSharedPtr<IRtRelation> rt, 
                      GeoModel::TransientConstSharedPtr<IRtResolution> reso,
                      GeoModel::TransientConstSharedPtr<ITrRelation> tr, double t0);
        ~MdtRtRelation() = default;
        inline const IRtRelation* rt() const { return m_rt.get(); }          //!< rt relation
        inline const IRtResolution* rtRes() const { return m_rtRes.get(); }  //!< resolution
        inline const ITrRelation* tr() const { return m_tr.get(); }           //!< t(r) relationship
        inline double t0Global() const { return m_t0; }          //!< global t0
    private:
        GeoModel::TransientConstSharedPtr<IRtRelation> m_rt{};
        GeoModel::TransientConstSharedPtr<IRtResolution> m_rtRes{};
        GeoModel::TransientConstSharedPtr<ITrRelation> m_tr{};
        double m_t0{0.f};

    };

}  // namespace MuonCalib

#endif
