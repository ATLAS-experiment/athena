/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUROI_TRIGMUROI_H
#define TRIGMUROI_TRIGMUROI_H

#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

class TrigmuRoI : public AthReentrantAlgorithm
{
   public:

      TrigmuRoI(const std::string& name, ISvcLocator* pSvcLocator); //!< std Gaudi algorthm constructor
      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;

   private:
   
      unsigned int getBitMaskValue( const unsigned int uintValue, const unsigned int mask ) const;
      StatusCode readAndAppendTrigRoiDescriptors(const EventContext& ctx, TrigRoiDescriptorCollection* roiColl, const SG::ReadHandleKey<xAOD::MuonRoIContainer>& readHandleKey, int bc_shift, unsigned int& roi_id, std::vector<int>& RpcBCShift, std::vector<int>& TgcBCShift, std::vector<float>& RoIEta, std::vector<float>& RoIPhi) const ;

      ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool","","Monitoring tool"};

      SG::WriteHandleKey<TrigRoiDescriptorCollection> m_roisWriteHandleKey {this,"RoisWriteHandleKey","Unspecified", "Output collection of RoIs"};
      SG::ReadHandleKey<xAOD::MuonRoIContainer> m_L1OutOfTimeRoIBCm2Key {this,"L1OutOfTimeRoIBCm2Key","LVL1MuonRoIsBCm2", "BC minus 2 out-of-time RoI container"};
      SG::ReadHandleKey<xAOD::MuonRoIContainer> m_L1OutOfTimeRoIBCm1Key {this,"L1OutOfTimeRoIBCm1Key","LVL1MuonRoIsBCm1", "BC minus 1 out-of-time RoI container"};
      SG::ReadHandleKey<xAOD::MuonRoIContainer> m_L1OutOfTimeRoIBCp1Key {this,"L1OutOfTimeRoIBCp1Key","LVL1MuonRoIsBCp1", "BC plus 1 out-of-time RoI container"};
      SG::ReadHandleKey<xAOD::MuonRoIContainer> m_L1OutOfTimeRoIBCp2Key {this,"L1OutOfTimeRoIBCp2Key","LVL1MuonRoIsBCp2", "BC plus 2 out-of-time RoI container"};

      Gaudi::Property<float> m_roiHalfWidthEta{this, "roiHalfWidthEta", 0.2, "Half width of RoI in eta"};
      Gaudi::Property<float> m_roiHalfWidthPhi{this, "roiHalfWidthPhi", 0.2, "Half width of RoI in phi"};

};

#endif
