/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @class TRT_CablingSvc: interface class for TRT Mapping
 */

#ifndef TRT_CABLINGSVC_H
#define TRT_CABLINGSVC_H

#include "TRT_Cabling/ITRT_CablingSvc.h"

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/IdContext.h"

#include "TRT_CablingData.h"
#include "TRT_FillCablingData_SR1_ECC.h"
#include "TRT_FillCablingData_SR1.h"
#include "TRT_FillCablingData_TB04.h"
#include "TRT_FillCablingData_DC3.h"

#include "TRT_ReadoutGeometry/TRT_DetectorManager.h" 

#include "AthenaBaseComps/AthService.h"

#include "eformat/SourceIdentifier.h"

#include <stdint.h> 


class TRT_CablingSvc : public extends<AthService, ITRT_CablingSvc>
{

public:

    // Constructor
  TRT_CablingSvc( const std::string& name, ISvcLocator *pSvcLocator ) ;

    // Destructor	
  virtual ~TRT_CablingSvc() = default;

  virtual StatusCode initialize() override;

  // Get Identifier for each straw from BS
  virtual Identifier getIdentifier(const eformat::SubDetector& subdetector,
				   const unsigned& rod, const int& bufferOffset, IdentifierHash& hashId) const override;
  
  // Get buffer offset from staw Identifier
  virtual uint32_t getBufferOffset( const Identifier &StrawId ) override;

  // Get ROBIDs for each Detector Element (Straw Layer)
  virtual std::vector<uint32_t> getRobID(Identifier& id) const override;

  // Get all RODIDs for TRT
  virtual const std::vector<uint32_t>& getAllRods() const override;

private:
  const InDetDD::TRT_DetectorManager *m_manager{nullptr};
  TRT_CablingData* m_cabling{nullptr};
  
  TRT_FillCablingData_SR1_ECC* m_cablingTool_SR1_ECC{nullptr};
  TRT_FillCablingData_SR1* m_cablingTool_SR1{nullptr};
  TRT_FillCablingData_TB04* m_cablingTool_TB{nullptr};
  TRT_FillCablingData_DC3* m_cablingTool_DC3{nullptr};
  int m_TRTLayout{0};
};

#endif     // TRT_CABLINGSVC_H
