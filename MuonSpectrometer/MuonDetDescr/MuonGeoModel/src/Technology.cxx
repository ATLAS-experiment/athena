/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>

#include "MuonGeoModel/Technology.h"

#include "MuonGeoModel/MYSQL.h"

namespace MuonGM {

    std::string Technology::GetName() const { return m_name; }

    Technology::Technology(MYSQL& mysql, std::string s) : m_name(std::move(s)), thickness(0.) {
        mysql.StoreTechnology(this);
    }

} // namespace MuonGM
