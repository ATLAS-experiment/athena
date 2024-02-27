/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/RadialStripDesign.h>
namespace MuonGMR4{
    bool RadialStripDesign::operator<(const RadialStripDesign& other) const {
        if (other.m_strips.size() != m_strips.size()) {
            return m_strips.size() < other.m_strips.size();
        }
        for (unsigned int strip = 0; strip < m_strips.size(); ++strip) {
            if (m_strips[strip].distOnBottom != other.m_strips[strip].distOnBottom) {
                return m_strips[strip].distOnBottom < other.m_strips[strip].distOnBottom;
            }
            if (m_strips[strip].distOnTop != other.m_strips[strip].distOnTop) {
                return m_strips[strip].distOnTop < other.m_strips[strip].distOnTop;
            }
        }
        return static_cast<const StripDesign&>(*this) < other;
    }
    void RadialStripDesign::addStrip(const double posOnBottom,
                                     const double posOnTop) {        
        m_strips.emplace_back( 0.5*lenBottomEdge() + posOnBottom, 0.5* lenTopEdge()+ posOnTop);
    }
}