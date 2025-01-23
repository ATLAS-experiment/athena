/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MdtCalibData/CalibParamSorter.h>

namespace MuonCalib{

    CalibParamSorter::CalibParamSorter(double _tol) :
        m_tolerance{_tol}{}
                
    int CalibParamSorter::compare(const std::vector<double>& a , const std::vector<double>& b) const {
        if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
        for (unsigned int i =0 ; i < a.size() ; ++i) {
            const double d = a[i] - b[i];
            if (std::abs(d) > m_tolerance) {
                return d < 0. ? -1 : 1;
            }
        }
        return 0;
    }
    int CalibParamSorter::compare(const CalibFunc& a, const CalibFunc& b) const {
        if (a.name() != b.name()) return a.name() < b.name() ? -1 : 1;
        return compare(a.parameters() , b.parameters());
    }
    int CalibParamSorter::compare(const MdtRtRelation& a, const MdtRtRelation& b) const {
        const int rtCmp = compare(*a.rt() , *b.rt());
        if (rtCmp) return rtCmp;
        return compare(*a.rtRes(), *b.rtRes());
    }
    int CalibParamSorter::compare(const SingleTubeCalib& a, const SingleTubeCalib& b) const {
        if ( std::abs(a.t0 - b.t0) > m_tolerance) {
            return a.t0 < b.t0 ? -1 : 1;
        }
        if (std::abs(a.adcCal - b.adcCal) > m_tolerance) {
            return a.adcCal < b.adcCal ? - 1 : 1;
        }
        if (a.statusCode != b.statusCode) {
            return a.statusCode  < b.statusCode ? -1 : 1;
        }
        return 0;
    }

    bool CalibParamSorter::operator()(const SingleTubeCalib* a, const SingleTubeCalib* b) const {
        return compare(*a, *b) < 0;
    }
    bool CalibParamSorter::operator()(const CalibFunc* a, const CalibFunc* b) const {
        return compare(*a, *b) < 0;
    }
    bool CalibParamSorter::operator()(const MdtFullCalibData::RtRelationPtr& a, 
                                      const MdtFullCalibData::RtRelationPtr& b) const {
        return compare(*a, *b) < 0;
    }
    bool CalibParamSorter::operator()(const SingleTubeCalibPtr& a, const SingleTubeCalibPtr& b) const {
        return compare(*a, *b) < 0;
    }
    bool CalibParamSorter::operator()(const MdtRtRelation* a, const MdtRtRelation* b) const {
        return compare(*a, *b) < 0;
    }

}