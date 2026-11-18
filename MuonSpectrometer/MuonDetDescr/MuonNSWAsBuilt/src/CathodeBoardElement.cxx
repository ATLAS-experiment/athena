/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>

#include "MuonNSWAsBuilt/CathodeBoardElement.h"
using namespace NswAsBuilt;

//coverity[PASS_BY_VALUE]
CathodeBoardElement::CathodeBoardElement(stgcStripConfiguration_t config, const Element& element)
  : m_config_stgc(std::move(config)), m_element_stgc(element)
{
}
/* In the sTGC strip (cathode) boards due to the staggering, the first/last strip could be a half 
* or a full strip depending on the strip_board_type. Therefore first and last strip are handled 
* separately from second or any i-strip   
*/
CathodeBoardElement::stgcStrip_t CathodeBoardElement::getStgcStrip(ParameterClass iclass, int stripNumber) const {
  Eigen::Matrix3d vectorset{Eigen::Matrix3d::Identity()};
  if (stripNumber == 1){
    vectorset.col(0) = m_config_stgc.fCenterPoint.pos;
    vectorset.col(1) = m_config_stgc.fLeftPoint.pos;
    vectorset.col(2) = m_config_stgc.fRightPoint.pos;
  }
  else if (stripNumber == m_config_stgc.lastStripNumber){
    vectorset.col(0) = m_config_stgc.lCenterPoint.pos;
    vectorset.col(1) = m_config_stgc.lLeftPoint.pos;
    vectorset.col(2) = m_config_stgc.lRightPoint.pos;
  }
  else {
    vectorset.col(0) = m_config_stgc.sCenterPoint.pos + (stripNumber - 2) * m_config_stgc.sCenterPoint.pitch;
    vectorset.col(1) = m_config_stgc.sLeftPoint.pos   + (stripNumber - 2) * m_config_stgc.sLeftPoint.pitch;
    vectorset.col(2) = m_config_stgc.sRightPoint.pos  + (stripNumber - 2) * m_config_stgc.sRightPoint.pitch;
  }

  m_element_stgc.transformToFrame(iclass, vectorset, nullptr);

  stgcStrip_t ret;
  ret.center = vectorset.col(0);
  ret.left   = vectorset.col(1);
  ret.right  = vectorset.col(2);
  return ret;
}

Amg::Vector3D CathodeBoardElement::getPositionAlongStgcStrip(ParameterClass iclass, int stripNumber, double sx, double sy) const {
  Amg::Vector3D center{Amg::Vector3D::Zero()}, left{Amg::Vector3D::Zero()}, right{Amg::Vector3D::Zero()};
  if (stripNumber == 1){
    center = m_config_stgc.fCenterPoint.pos;
    left   = m_config_stgc.fLeftPoint.pos;
    right  = m_config_stgc.fRightPoint.pos;
  }
  else if (stripNumber == m_config_stgc.lastStripNumber){
    center = m_config_stgc.lCenterPoint.pos;
    left   = m_config_stgc.lLeftPoint.pos;
    right  = m_config_stgc.lRightPoint.pos;
  }
  else {
    center = m_config_stgc.sCenterPoint.pos + (stripNumber - 2) * m_config_stgc.sCenterPoint.pitch;
    left   = m_config_stgc.sLeftPoint.pos   + (stripNumber - 2) * m_config_stgc.sLeftPoint.pitch;
    right  = m_config_stgc.sRightPoint.pos  + (stripNumber - 2) * m_config_stgc.sRightPoint.pitch;
  }

  // sy is the radial distance from the strip center / pitch, so in [-0.5, 0.5]
  center += sy * m_config_stgc.sCenterPoint.pitch;
  left   += sy * m_config_stgc.sLeftPoint.pitch;
  right  += sy * m_config_stgc.sRightPoint.pitch;

  // Get strip-local coordinate of point along strip
  // Note: left, center and right are exactly in line in local coordinates
  // (i.e. strip is not deformed in local coordinates)
  Amg::Vector3D ret = center;
  if (sx < 0.0) {
    ret = (sx+1.0)*center - sx*left;
  } else {
    ret = (1.0-sx)*center + sx*right;
  }
  m_element_stgc.transformToFrame(iclass, ret, nullptr);
  return ret;
}

