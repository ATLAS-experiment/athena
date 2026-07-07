/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsInterop/IdentityHelper.h"

#include <stdexcept>

#include "HGTD_Identifier/HGTD_ID.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "ReadoutGeometryBase/SolidStateDetectorElementBase.h"
IdentityHelper::IdentityHelper(
    const InDetDD::SolidStateDetectorElementBase* elem)
    : m_elem(elem), m_isInDet(false) {
  if (const auto* siElem = dynamic_cast<const InDetDD::SiDetectorElement*>(m_elem)) {
    m_isInDet = true;
    if (siElem->isPixel()) {
      m_helper = static_cast<const PixelID*>(siElem->getIdHelper());
    } else if (siElem->isSCT()) {
      m_helper = static_cast<const SCT_ID*>(siElem->getIdHelper());
    }
  } else if (const auto* hgtdElem = dynamic_cast<const InDetDD::HGTD_DetectorElement*>(m_elem)) {
    m_helper = static_cast<const HGTD_ID*>(hgtdElem->getIdHelper());
  } else {
    throw std::invalid_argument(
        "ActsInterop IdentityHelper constructed with "
        "SolidStateDetectorElementBase... can only be used for Pixel, SCT and "
        "HGTD det elements");
  }
}

const PixelID* 
IdentityHelper::getPixelIDHelper() const 
{
  return std::get<const PixelID*>(m_helper);
}
const SCT_ID* 
IdentityHelper::getSCTIDHelper() const
{
  return std::get<const SCT_ID*>(m_helper);
}

const HGTD_ID*
IdentityHelper::getHgtdIdHelper() const
{
  return std::get<const HGTD_ID*>(m_helper);
}

int 
IdentityHelper::bec() const 
{
  auto id = m_elem->identify();
  if (m_isInDet) {
    const InDetDD::SiDetectorElement* thisElement =
        static_cast<const InDetDD::SiDetectorElement*>(m_elem);
    if (thisElement->isPixel()) {
      return getPixelIDHelper()->barrel_ec(id);
    } else {
      return getSCTIDHelper()->barrel_ec(id);
    }
  } else {
      return getHgtdIdHelper()->endcap(id);
  }
}

int 
IdentityHelper::layer_disk() const
{
  auto id = m_elem->identify();
  if (m_isInDet) {
     const InDetDD::SiDetectorElement* thisElement =
        static_cast<const InDetDD::SiDetectorElement*>(m_elem);
     if (thisElement->isPixel()) {
    return getPixelIDHelper()->layer_disk(id);
  } else {
    return getSCTIDHelper()->layer_disk(id);
  }
  } else {
    return getHgtdIdHelper()->layer(id);
  }
}

int 
IdentityHelper::phi_module() const
{
  auto id = m_elem->identify();
  if (m_isInDet) {
     const InDetDD::SiDetectorElement* thisElement =
        static_cast<const InDetDD::SiDetectorElement*>(m_elem);
  if (thisElement->isPixel()) {
    return getPixelIDHelper()->phi_module(id);
  } else {
    return getSCTIDHelper()->phi_module(id);
  }
  } else {
    return getHgtdIdHelper()->phi_module(id);
  }
}

int 
IdentityHelper::eta_module() const
{
  auto id = m_elem->identify();
  if (m_isInDet) {
     const InDetDD::SiDetectorElement* thisElement =
        static_cast<const InDetDD::SiDetectorElement*>(m_elem);
  if (thisElement->isPixel()) {
    return getPixelIDHelper()->eta_module(id);
  } else {
    return getSCTIDHelper()->eta_module(id);
  }
  } else {
    return getHgtdIdHelper()->eta_module(id);
  }
}

int 
IdentityHelper::side() const
{
  auto id = m_elem->identify();
  if (m_isInDet) {
     const InDetDD::SiDetectorElement* thisElement =
        static_cast<const InDetDD::SiDetectorElement*>(m_elem);
  if (thisElement->isPixel()) {
    return 0;
  } else {
    return getSCTIDHelper()->side(id);
  }
  } else {
    return 0;
  }
}

int 
IdentityHelper::phi_module_max() const
{
  auto id = m_elem->identify();
  if (m_isInDet) {
     const InDetDD::SiDetectorElement* thisElement =
        static_cast<const InDetDD::SiDetectorElement*>(m_elem);
  if (thisElement->isPixel()) {
    return getPixelIDHelper()->phi_module_max(id);
  } else {
    return getSCTIDHelper()->phi_module_max(id);
  }
  } else {
    return getHgtdIdHelper()->phi_module_max(id);
  }
}

int 
IdentityHelper::eta_module_max() const
{
  auto id = m_elem->identify();
  if (m_isInDet) {
     const InDetDD::SiDetectorElement* thisElement =
        static_cast<const InDetDD::SiDetectorElement*>(m_elem);
  if (thisElement->isPixel()) {
    return getPixelIDHelper()->eta_module_max(id);
  } else {
    return getSCTIDHelper()->eta_module_max(id);
  }
  } else {
    return getHgtdIdHelper()->eta_module_max(id);
  }
}
