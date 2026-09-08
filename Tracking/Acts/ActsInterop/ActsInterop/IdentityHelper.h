/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#pragma once
#include <variant>
namespace InDetDD {
  class SolidStateDetectorElementBase;
}

class PixelID;
class SCT_ID;
class HGTD_ID;

class IdentityHelper {

public:
  IdentityHelper(const InDetDD::SolidStateDetectorElementBase * detElement);
  int bec() const;
  int layer_disk() const;
  int phi_module() const;
  int eta_module() const;
  int side() const;

  int phi_module_max() const;
  int eta_module_max() const;

private:
  const InDetDD::SolidStateDetectorElementBase* m_elem;
  std::variant<const PixelID*, const SCT_ID*, const HGTD_ID*> m_helper;
  bool m_isInDet;
  const PixelID* getPixelIDHelper() const;
  const SCT_ID* getSCTIDHelper() const;
  const HGTD_ID* getHgtdIdHelper() const;
  
};
