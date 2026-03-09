/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef L0MuonMDTTools_LOMDTSEGMENT_H
#define L0MuonMDTTools_LOMDTSEGMENT_H


namespace L0MDT {
/**
 * @class L0MDTSegment
 * @brief Class describing a reconstructed MDT segment used by the L0Muon trigger.
 *
 * The class stores the parameters of the segment reconstructed from MDT hits
 * such as slope, intercept and associated hit information. It is used by the
 * MDT segment finding tools in the L0Muon trigger simulation.*/
    class Segment {
    public:
        Segment(const float slope, const float intercept): m_slope(slope), m_intercept(intercept) {}
        ~Segment() = default;

        float slope() const { return m_slope; }
        float intercept() const { return m_intercept; }

    private:

        float m_slope{0.f};
        float m_intercept{0.f};

    };

}// end of namespace

#endif
