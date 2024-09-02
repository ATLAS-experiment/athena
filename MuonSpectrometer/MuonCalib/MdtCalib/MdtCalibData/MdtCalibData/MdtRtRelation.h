/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_MDTRTRELATION_H
#define MUONCALIB_MDTRTRELATION_H

#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/IRtResolution.h"
#include "MdtCalibData/TrRelation.h"

#include <iostream>
#include <memory>

namespace MuonCalib {

    /** class which holds calibration constants per rt-region */
    class MdtRtRelation {
    public:
        MdtRtRelation(std::unique_ptr<IRtRelation>&& rt, 
                      std::unique_ptr<IRtResolution>&& reso, float t0);
        MdtRtRelation(std::unique_ptr<IRtRelation>&& rt, 
                      std::unique_ptr<IRtResolution>&& reso,
                      std::unique_ptr<TrRelation>&& tr, float t0);
        ~MdtRtRelation() = default;
        inline const IRtRelation* rt() const { return m_rt.get(); }          //!< rt relation
        inline const IRtResolution* rtRes() const { return m_rtRes.get(); }  //!< resolution
        inline const TrRelation* tr() const { return m_tr.get(); }           //!< t(r) relationship
        inline float t0Global() const { return m_t0; }          //!< global t0
    private:
        std::unique_ptr<IRtRelation> m_rt{};
        std::unique_ptr<IRtResolution> m_rtRes{};
        std::unique_ptr<TrRelation> m_tr{};
        float m_t0{0.f};

    };

}  // namespace MuonCalib

#endif
