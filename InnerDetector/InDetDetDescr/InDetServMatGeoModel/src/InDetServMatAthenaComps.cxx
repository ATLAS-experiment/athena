/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetServMatAthenaComps.h"
#include "InDetGeoModelUtils/IInDetServMatBuilderTool.h"

InDetServMatAthenaComps::InDetServMatAthenaComps()
  : InDetDD::AthenaComps("InDetServMatGeoModel"),
    m_builderTool(nullptr)
{}

 
void 
InDetServMatAthenaComps::setBuilderTool(IInDetServMatBuilderTool * builderTool) 
{
  m_builderTool = builderTool;
}

const IInDetServMatBuilderTool * 
InDetServMatAthenaComps::builderTool() const
{
  return m_builderTool;
}
