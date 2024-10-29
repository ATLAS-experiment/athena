/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibData/MdtRtRelation.h"

namespace MuonCalib {
    MdtRtRelation::MdtRtRelation(GeoModel::TransientConstSharedPtr<IRtRelation> rt, 
                                 GeoModel::TransientConstSharedPtr<IRtResolution> reso, double t0) : 
        m_rt{std::move(rt)}, 
        m_rtRes{std::move(reso)}, 
        m_t0{t0} {
       if (m_rt) { m_tr = std::make_unique<TrRelation>(*m_rt); }
    }
    MdtRtRelation::MdtRtRelation(GeoModel::TransientConstSharedPtr<IRtRelation> rt, 
                                 GeoModel::TransientConstSharedPtr<IRtResolution> reso,
                                 GeoModel::TransientConstSharedPtr<TrRelation> tr, double t0) : 
        m_rt{std::move(rt)}, 
        m_rtRes{std::move(reso)}, 
        m_tr{std::move(tr)}, 
        m_t0{t0} {}
}  // end namespace MuonCalib
