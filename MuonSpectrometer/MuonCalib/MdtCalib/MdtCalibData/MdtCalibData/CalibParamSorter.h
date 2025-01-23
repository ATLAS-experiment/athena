/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#pragma once

#include <MdtCalibData/MdtFullCalibData.h>

namespace MuonCalib{
    /** @brief Helper struct to group Mdt calibration constants which are equivalent within the target precision.
     *         The grouping is exploiting the std::set<>  or the std::map<> mechanism, where two objects are
     *         considered to be equivalent if !CalibParamSorter()(a,b) && CalibParamSorter()(b,a) are satisfied. */
    struct CalibParamSorter{
        using SingleTubeCalib = MdtTubeCalibContainer::SingleTubeCalib;
        using SingleTubeCalibPtr = MdtTubeCalibContainer::SingleTubeCalibPtr;

        CalibParamSorter(double _tol);

        bool operator()(const CalibFunc* a, const CalibFunc* b) const;

        bool operator()(const MdtRtRelation* a, const MdtRtRelation* b) const;
        bool operator()(const MdtFullCalibData::RtRelationPtr& a, 
                        const MdtFullCalibData::RtRelationPtr& b) const;

        bool operator()(const SingleTubeCalib* a, const SingleTubeCalib* b) const;
        bool operator()(const SingleTubeCalibPtr& a, const SingleTubeCalibPtr& b) const;


        private:
            int compare(const std::vector<double>& a , const std::vector<double>& b) const;
            int compare(const CalibFunc& a, const CalibFunc& b) const;
            int compare(const SingleTubeCalib& a, const SingleTubeCalib& b) const;
            int compare(const MdtRtRelation& a, const MdtRtRelation& b) const;

            double m_tolerance{1.e-6};
    };

}