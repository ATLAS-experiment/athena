/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/WireGroupDesign.h>
#include <GaudiKernel/SystemOfUnits.h>

namespace MuonGMR4{
    void WireGroupDesign::print(std::ostream& ostr) const {
        ostr<<"Dimension  -- width x height [mm]: "<<halfWidth() * Gaudi::Units::mm<<" x ";
        ostr<<shortHalfHeight()<<"/"<<longHalfHeight()<<" [mm], ";
        if (hasStereoAngle()) ostr<<"stereo angle: "<<stereoAngle() / Gaudi::Units::deg<<", ";
        ostr<<"position first strip "<<Amg::toString(center(firstStripNumber()).value_or(Amg::Vector2D::Zero()),1);
        ostr<<" *** Trapezoid edges "<<Amg::toString(cornerBotLeft(),1)<<" - "<<Amg::toString(cornerBotRight(), 1)<<" --- ";
        ostr<<Amg::toString(cornerTopLeft(), 1)<<" - "<<Amg::toString(cornerTopRight(), 1);
        ostr<<" -- numWireGroups: "<<m_groups.size()<<", wire pitch: "<<stripPitch()<<", nWires: [";
        for (const wireGroup & grp : m_groups){
            ostr<<grp.numWires<<",";
        }
        ostr<<"] ";
    }
    bool WireGroupDesign::operator<(const WireGroupDesign& other) const {
        if (other.m_groups.size() != m_groups.size()) {
            return m_groups.size() < other.m_groups.size();
        }
        for (unsigned int grp = 0; grp < m_groups.size(); ++grp) {
            if (m_groups[grp].numWires != other.m_groups[grp].numWires) {
                return m_groups[grp].numWires < other.m_groups[grp].numWires;
            }
        }
        return static_cast<const StripDesign&>(*this) < other;
    }
    void WireGroupDesign::declareGroup(const unsigned int numWires) {
        m_groups.emplace_back(numWires, nAllWires());
    }
    unsigned int WireGroupDesign::nAllWires() const {
        return m_groups.empty() ? 0 : m_groups.back().accumlWires + m_groups.back().numWires;
    }
    /// Returns the number of wires in a given group.
    unsigned int WireGroupDesign::numWiresInGroup(unsigned int groupNum) const {
       unsigned int grpIdx = groupNum - firstStripNumber();
       if (grpIdx >= m_groups.size()) {
          ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The group number "<<groupNum
                        <<" is out of range. Expect ["<<firstStripNumber()
                        <<"-"<<m_groups.size()+firstStripNumber()<<").");
          return 0;
       }
       return m_groups[grpIdx].numWires;
    }
    double WireGroupDesign::wireCutout() const {return m_wireCutout;} 
    
    void WireGroupDesign::defineWireCutout(const double wireCutout){m_wireCutout=wireCutout;}
}

