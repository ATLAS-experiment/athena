//Dear emacs, this is -*-c++-*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCELLREC_LARCELLMERGER_H
#define LARCELLREC_LARCELLMERGER_H

/** 
@class LArCellMerger
@brief Replace cells in calo cell container with cells from alternate raw channel container
@brief  Use case: when full calo cell container is made from DSP computed cells, replace some of then with better energies from offline reconstructed digits

 AlgTool properties (name defined in cxx file): 
     Name of the alternate raw channel container (i.e made from the digits offline)

 Created July 09, 2009  G.Unal
*/



#include "GaudiKernel/ToolHandle.h"
#include "CaloInterface/ICaloCellMakerTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "LArCabling/LArOnOffIdMapping.h"

class CaloCell_ID;
class LArOnlineID;
class LArRawChannelContainer;

class LArCellMerger
  : public extends<AthAlgTool, ICaloCellMakerTool>
{
public:    

  using base_class::base_class;
  
  /** initialize the tool
  */
  virtual StatusCode initialize() override; 

  /** finalize the tool
  */
  virtual StatusCode finalize() override; 

  /** update theCellContainer
  */
  virtual StatusCode process (CaloCellContainer* theCellContainer,
                              const EventContext& ctx) const override;


 private:

  /** handle to LAr cabling
  */
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of Cabling obj"};


  /** key for raw channel container to merge to cells
  */  
  SG::ReadHandleKey<LArRawChannelContainer>  m_rawChannelContainerName{this,"RawChannelsName","LArRawChannels_digits",
									     "SG key of rebuilt LArRawChannelContainer"};

  const CaloCell_ID* m_calo_id=nullptr;
  const LArOnlineID* m_onlineID=nullptr;

};

#endif

