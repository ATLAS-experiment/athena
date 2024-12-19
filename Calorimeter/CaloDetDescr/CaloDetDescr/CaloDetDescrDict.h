/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloDetDescr/CaloDetDescrManager.h"
/**
 * These headers should not (from pure C++ pov) be necessary, 
 * but are needed for building the ROOT dictionary.
 * Omitting them results in warnings of this kind:
 * Unused class rule: ICaloRecoSimpleGeomTool
 **/
#include "CaloDetDescr/CaloDetectorElements.h"
#include "CaloDetDescr/CaloDetDescriptor.h"
#include "CaloDetDescr/CaloDepthTool.h"
#include "CaloDetDescr/CaloDescriptors.h"
#include "CaloDetDescr/CaloSubdetNames.h"
#include "CaloDetDescr/ICaloCoordinateTool.h"
#include "CaloDetDescr/ICaloRecoMaterialTool.h"
#include "CaloDetDescr/ICaloRecoSimpleGeomTool.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"


struct CaloDetDescrDict
{
  CaloDetDescrManager::calo_descr_const_iterator m_rit;
  CaloDetDescrManager::calo_descr_range m_rrange;

  CaloDetDescrManager::calo_element_const_iterator m_eit;
  CaloDetDescrManager::calo_element_range m_ebrange;
};

namespace CaloDetDescrDictInstan
{
  bool ritdum (CaloDetDescrManager::calo_descr_const_iterator /*a*/,
               CaloDetDescrManager::calo_descr_const_iterator /*b*/)
  { return true;}
  bool eitdum (CaloDetDescrManager::calo_element_const_iterator /*a*/,
               CaloDetDescrManager::calo_element_const_iterator /*b*/)
  { return true; }
}
