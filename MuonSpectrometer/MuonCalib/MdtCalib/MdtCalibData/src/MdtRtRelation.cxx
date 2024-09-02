/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibData/MdtRtRelation.h"

namespace MuonCalib {
    MdtRtRelation::MdtRtRelation(std::unique_ptr<IRtRelation>&& rt, 
                                 std::unique_ptr<IRtResolution>&& reso, float t0) : 
        m_rt(std::move(rt)), m_rtRes(std::move(reso)), m_t0(t0) {
       if (m_rt) { m_tr = std::make_unique<TrRelation>(*m_rt); }
    }
    MdtRtRelation::MdtRtRelation(std::unique_ptr<IRtRelation>&& rt, 
                                 std::unique_ptr<IRtResolution>&& reso,
                                 std::unique_ptr<TrRelation>&& tr, float t0) : 
        m_rt(std::move(rt)), m_rtRes(std::move(reso)), m_tr(std::move(tr)), m_t0(t0) {}
}  // end namespace MuonCalib
