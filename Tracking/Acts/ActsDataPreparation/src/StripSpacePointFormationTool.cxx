/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "StripSpacePointFormationTool.h"

#include <cmath>

namespace ActsTrk {

    StatusCode StripSpacePointFormationTool::makeStripSpacePoint(
       std::vector<StripSP>& collection,
       const StripInformationHelper& firstInfo,
       const StripInformationHelper& secondInfo,
       const Amg::Vector3D& /*beamSpotVertex*/,
       bool isEndcap,
       double limit,
       double slimit) const
    {
        // The vertex enters through StripInformationHelper::trajDirection().

        double a  =-firstInfo.trajDirection().dot(secondInfo.normal());
        double b  = firstInfo.stripDirection().dot(secondInfo.normal());
        double l0 = firstInfo.oneOverStrip()*slimit+limit ;

        if(std::abs(a) > (std::abs(b)*l0)) {
          ATH_MSG_DEBUG("SP fails geometric cuts (first)");
          if(m_useBeamSpotConstraint) {
            return StatusCode::SUCCESS;
          }
        }

        double c  =-secondInfo.trajDirection().dot(firstInfo.normal());
        double d  = secondInfo.stripDirection().dot(firstInfo.normal());
        double l1 = secondInfo.oneOverStrip()*slimit+limit ;

        if(std::abs(c) > (std::abs(d)*l1)) {
          ATH_MSG_DEBUG("SP fails geometric cuts (second)");
          if(m_useBeamSpotConstraint) {
            return StatusCode::SUCCESS;
          }
        }

        double m = a/b;

        if(slimit!=0.) {
            double n = c/d;
            if (m >  limit || n >  limit) {
                double cs  = firstInfo.stripDirection().dot(secondInfo.stripDirection())*(firstInfo.oneOverStrip()*firstInfo.oneOverStrip());
                double dm  = (m-1);
                double dmn = (n-1.)*cs;
                if(dmn > dm) dm = dmn;
                m-=dm; n-=(dm/cs);
                if(std::abs(m) > limit || std::abs(n) > limit) {
                  ATH_MSG_DEBUG("SP falls outside of limit");
                  if(m_useBeamSpotConstraint) {
                    return StatusCode::SUCCESS;
                  }
                }
            } else if (m < -limit || n < -limit) {
                double cs  = firstInfo.stripDirection().dot(secondInfo.stripDirection())*(firstInfo.oneOverStrip()*firstInfo.oneOverStrip());
                double dm  = -(1.+m);
                double dmn = -(1.+n)*cs;
                if(dmn > dm) dm = dmn;
                m+=dm; n+=(dm/cs);
                if(std::abs(m) > limit || std::abs(n) > limit) {
                  ATH_MSG_DEBUG("SP falls outside of limit");
                  if(m_useBeamSpotConstraint) {
                    return StatusCode::SUCCESS;
                  }
                }
            }
        }

        Eigen::Matrix<double, 3, 1> globalPosition(m_useTopSp ? secondInfo.position(m) : firstInfo.position(m));

        collection.push_back(makeStripSP(globalPosition, firstInfo, secondInfo, isEndcap));

        return StatusCode::SUCCESS;
    }
}
