/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
#include "MagFieldConditions/AtlasFieldMapCondObj.h"

// setter
void
AtlasFieldMapCondObj::setFieldMap(std::unique_ptr<MagField::AtlasFieldMap> fieldMap)
{
    m_fieldMap = std::move(fieldMap);
}

