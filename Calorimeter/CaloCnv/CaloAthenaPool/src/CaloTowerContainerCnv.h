/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CaloTowerContainerCnv_H
#define CaloTowerContainerCnv_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
#include "CaloEvent/CaloTowerContainer.h"
#include "CaloTPCnv/CaloTowerContainer_p1.h"
#include "CaloTPCnv/CaloTowerContainerCnv_p1.h"

class    CaloTowerBuilderToolBase ; 
class    CaloTowerBuilderTool ; 

typedef CaloTowerContainer_p1 CaloTowerContainerPERS;
typedef T_AthenaPoolCustomCnv<CaloTowerContainer,CaloTowerContainerPERS> CaloTowerContainerCnvBase;

/**

 @class CaloTowerContainerCnv
 @brief POOL Converter for CaloTowerContainer

 **/

class CaloTowerContainerCnv : public CaloTowerContainerCnvBase
{
  friend class CnvFactory<CaloTowerContainerCnv >;
public:
  CaloTowerContainerCnv(ISvcLocator* svcloc);

  virtual CaloTowerContainer* createTransient() override;
  virtual CaloTowerContainerPERS* createPersistent(CaloTowerContainer*) override;
  
  CaloTowerBuilderToolBase* getTool(const std::string& type,
				    const std::string& nm); 

  CaloTowerBuilderTool* m_emHecTowerBldr{nullptr};
  CaloTowerBuilderToolBase* m_fcalTowerBldr{nullptr}; 
  CaloTowerBuilderToolBase* m_tileTowerBldr{nullptr}; 
  
  pool::Guid  p0_guid{"8F94A938-3C19-4509-BBAA-E8EB0A64B524"};
  pool::Guid  p1_guid{"E56D5471-A9E2-4787-B413-D3BD9F2AC55D"};
  CaloTowerContainerCnv_p1 m_converter;
};

#endif


