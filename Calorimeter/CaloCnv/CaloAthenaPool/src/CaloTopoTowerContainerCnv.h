/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CaloTopoTowerContainerCnv_H
#define CaloTopoTowerContainerCnv_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
#include "CaloEvent/CaloTopoTowerContainer.h"
#include "CaloTPCnv/CaloTopoTowerContainer_p1.h"
#include "CaloTPCnv/CaloTopoTowerContainerCnv_p1.h"

class    CaloTopoTowerBuilderToolBase ; 
class    CaloTopoTowerBuilderTool ; 

typedef CaloTopoTowerContainer_p1 CaloTopoTowerContainerPERS;
typedef T_AthenaPoolCustomCnv<CaloTopoTowerContainer,CaloTopoTowerContainerPERS> CaloTopoTowerContainerCnvBase;

/**

 @class CaloTopoTowerContainerCnv
 @brief POOL Converter for CaloTopoTowerContainer

 **/

class CaloTopoTowerContainerCnv : public CaloTopoTowerContainerCnvBase
{
  friend class CnvFactory<CaloTopoTowerContainerCnv >;
public:
  CaloTopoTowerContainerCnv(ISvcLocator* svcloc);

  virtual CaloTopoTowerContainer* createTransient() override;
  virtual CaloTopoTowerContainerPERS* createPersistent(CaloTopoTowerContainer*) override;


  CaloTopoTowerBuilderToolBase* getTool(const std::string& type,
					const std::string& nm); 

  CaloTopoTowerBuilderToolBase* m_TopoTowerBldr{nullptr};
  pool::Guid  p0_guid{"00B7F56C-1E49-4469-BEBA-C74620575A00"};
  pool::Guid  p1_guid{"4ED29686-28E9-426F-B076-C4E4600F66A7"};
  CaloTopoTowerContainerCnv_p1 m_converter;
  
};

#endif


