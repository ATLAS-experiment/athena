/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MdtCalibData/RtRelationLookUp.h>

#include <AthenaKernel/getMessageSvc.h>
#include <GaudiKernel/MsgStream.h>
namespace MuonCalib{

    RtRelationLookUp::RtRelationLookUp(const ParVec &vec) : 
        IRtRelation(vec) {
        if (vec.size() < 4) {
            m_t_min = 9e9;
            m_bin_size = 1.0;  // will be always out of range
            MsgStream log(Athena::getMessageSvc(), "RtRelationLookUp");
            log << MSG::WARNING << "<to few parameters>" << endmsg;
        } else {
            m_t_min = par(0);
            m_bin_size = par(1);
            if (m_bin_size == 0) {
                MsgStream log(Athena::getMessageSvc(), "RtRelationLookUp");
                log << MSG::WARNING << "<bin size=0>" << endmsg;
            }
        }
    }
    unsigned RtRelationLookUp::nDoF() const{
        return nPar() -2;
    }

    inline int RtRelationLookUp::getBin(double t) const {
        double t_minus_tmin{t - m_t_min};
        double rel = t_minus_tmin / m_bin_size;
        if (rel < static_cast<double>(INT_MIN)) return INT_MIN;
        if (rel > static_cast<double>(INT_MAX)) return INT_MAX;
        return static_cast<int>(rel);
    }

    double RtRelationLookUp::radius(double t) const {
        // get best matching bin in rt range
        int bin = binInRtRange(t);

        // shift bin so we are using the last two bins for extrapolation
        if (static_cast<unsigned>(bin) >= nDoF() - 1) bin = nDoF() - 2;

        double r1 = getRadius(bin);      // get bin value
        double r2 = getRadius(bin + 1);  // get value of next bin
        double dr = r2 - r1;

        // scale factor for interpolation
        double scale = (t - m_t_min) / m_bin_size - (double)bin;

        double r = r1 + dr * scale;

        return r >= 0 ? r : 0;
    }

    double RtRelationLookUp::driftVelocity(double t) const {
        // get best matching bin in rt range
        int bin = binInRtRange(t);

        // shift bin so we are using the last two bins for extrapolation
        if (static_cast<unsigned>(bin) >= nDoF() - 1) bin = nDoF() - 2;

        double r1 = getRadius(bin);      // get bin value
        double r2 = getRadius(bin + 1);  // get value of next bin
        double dr = r2 - r1;

        double v = dr / m_bin_size;

        return v;
    }
    double RtRelationLookUp::driftAcceleration(double t) const {
        return (driftVelocity(t+1) - driftVelocity(t-1)) / 2.;
    }

    inline int RtRelationLookUp::binInRtRange(double t) const {
        // get bin
        int bin = getBin(t);

        // if t corresponds to a negativ bin return first
        if (bin < 0) bin = 0;

        // if t corresponds to a bin outside the range of the lookup table return the last bin
        if (static_cast<unsigned>(bin) >= nDoF()) bin = nDoF() - 1;

        return bin;
    }

    double RtRelationLookUp::tLower() const { return m_t_min; }
    double RtRelationLookUp::tUpper() const { return m_t_min + m_bin_size * nDoF(); }
    double RtRelationLookUp::tBinWidth() const {return m_bin_size; }
}

