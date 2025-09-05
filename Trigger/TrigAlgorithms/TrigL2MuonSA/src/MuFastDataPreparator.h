/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_MUFASTDATAPREPARATOR_H
#define  TRIGL2MUONSA_MUFASTDATAPREPARATOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "TrigT1Interfaces/RecMuonRoI.h"
#include "TrigT1Interfaces/ITrigT1MuonRecRoiTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "xAODTrigger/MuonRoI.h"

#include "MuFastDataPreparatorOptions.h"
#include "RpcDataPreparator.h"
#include "RpcRoadDefiner.h"
#include "RpcPatFinder.h"
#include "TgcDataPreparator.h"
#include "TgcRoadDefiner.h"
#include "MdtDataPreparator.h"
#include "MdtRegion.h"
#include "CscDataPreparator.h"
#include "CscData.h"
#include "StgcDataPreparator.h"
#include "StgcData.h"
#include "MmDataPreparator.h"
#include "MmData.h"

#include "TrigMuonToolInterfaces/ITrigMuonBackExtrapolator.h"

#include "StoreGate/ReadCondHandleKey.h"

#include "ClusterRoadDefiner.h"
#include "RpcClusterPreparator.h"
#include "ClusterPatFinder.h"

namespace TrigL2MuonSA {

class MuFastDataPreparator: public AthAlgTool {
 public:

  MuFastDataPreparator(const std::string& type,
		       const std::string& name,
		       const IInterface*  parent);

  virtual StatusCode initialize() override;

 public:

  StatusCode prepareData(const EventContext& ctx,
			 const xAOD::MuonRoI*        p_roi,
			 const TrigRoiDescriptor*    p_roids,
			 const bool                  insideOut,
			 TrigL2MuonSA::RpcHits&      rpcHits,
			 TrigL2MuonSA::MuonRoad&     muonRoad,
			 TrigL2MuonSA::MdtRegion&    mdtRegion,
			 TrigL2MuonSA::RpcFitResult& rpcFitResult,
			 TrigL2MuonSA::MdtHits&      mdtHits,
			 const bool                  dynamicDeltaRpc) const;

  StatusCode prepareData(const EventContext& ctx,
			 const xAOD::MuonRoI*        p_roi,
			 const TrigRoiDescriptor*    p_roids,
			 const bool                  insideOut,
			 TrigL2MuonSA::TgcHits&      tgcHits,
			 TrigL2MuonSA::MuonRoad&     muonRoad,
			 TrigL2MuonSA::MdtRegion&    mdtRegion,
			 TrigL2MuonSA::TgcFitResult& tgcFitResult,
			 TrigL2MuonSA::MdtHits&      mdtHits,
			 TrigL2MuonSA::CscHits&      cscHits,
			 TrigL2MuonSA::StgcHits&     stgcHits,
			 TrigL2MuonSA::MmHits&       mmHits) const;

  //for multi-track mode
  StatusCode prepareData(const EventContext& ctx,
                         const xAOD::MuonRoI*                 p_roi,
                         const TrigRoiDescriptor*             p_roids,
                         std::vector<TrigL2MuonSA::MuonRoad>& clusterRoad,
                         std::vector<TrigL2MuonSA::RpcFitResult>&  clusterFitResults,
                         TrigL2MuonSA::MdtHits&               mdtHits,
                         std::vector<TrigL2MuonSA::MdtHits>&  mdtHits_cluster_normal,
                         const bool                           dynamicDeltaRpc) const;


  void setRoadWidthForFailure(double rWidth_RPC_Failed, double rWidth_TGC_Failed);

  void setRpcGeometry(bool use_rpc);
  void setStgcGeometry(bool use_stgc){ m_use_stgc = use_stgc; };
  void setMmGeometry(bool use_mm){ m_use_mm = use_mm; };

  StatusCode setMCFlag(bool use_mcLUT);
  void setRoIBasedDataAccess(bool use_RoIBasedDataAccess_MDT,
			     bool use_RoIBasedDataAccess_RPC,
			     bool use_RoIBasedDataAccess_TGC,
			     bool use_RoIBasedDataAccess_CSC,
			     bool use_RoIBasedDataAccess_STGC,
			     bool use_RoIBasedDataAccess_MM);

  void setExtrapolatorTool(ToolHandle<ITrigMuonBackExtrapolator>* backExtrapolator);

 private:

  ToolHandle<LVL1::ITrigT1MuonRecRoiTool> m_recRPCRoiTool{this, "TrigT1RPCRecRoiTool", "LVL1::TrigT1RPCRecRoiTool/TrigT1RPCRecRoiTool"};
  ToolHandle<RpcDataPreparator>   m_rpcDataPreparator{this, "RPCDataPreparator", "TrigL2MuonSA::RpcDataPreparator"};
  ToolHandle<TgcDataPreparator>   m_tgcDataPreparator{this, "TGCDataPreparator", "TrigL2MuonSA::TgcDataPreparator"};
  ToolHandle<MdtDataPreparator>   m_mdtDataPreparator{this, "MDTDataPreparator", "TrigL2MuonSA::MdtDataPreparator"};
  ToolHandle<CscDataPreparator>   m_cscDataPreparator{this, "CSCDataPreparator", "TrigL2MuonSA::CscDataPreparator"};
  ToolHandle<StgcDataPreparator>  m_stgcDataPreparator{this,"STGCDataPreparator","TrigL2MuonSA::StgcDataPreparator"};
  ToolHandle<MmDataPreparator>    m_mmDataPreparator{this,  "MMDataPreparator",  "TrigL2MuonSA::MmDataPreparator"};

  ToolHandle<RpcRoadDefiner>      m_rpcRoadDefiner{this, "RpcRoadDefiner", "TrigL2MuonSA::RpcRoadDefiner"};
  ToolHandle<TgcRoadDefiner>      m_tgcRoadDefiner{this, "TgcRoadDefiner", "TrigL2MuonSA::TgcRoadDefiner"};
  ToolHandle<RpcPatFinder>        m_rpcPatFinder{"TrigL2MuonSA::RpcPatFinder"};

  //for multimu-in-pad mode
  ToolHandle<ClusterRoadDefiner>  m_clusterRoadDefiner{this, "ClusterRoadDefiner", "TrigL2MuonSA::ClusterRoadDefiner"};
  ToolHandle<ClusterPatFinder>    m_clusterPatFinder{this, "ClusterPatFinder", "TrigL2MuonSA::ClusterPatFinder"};
  //

  ToolHandle<ITrigMuonBackExtrapolator>* m_backExtrapolatorTool{nullptr};

  bool m_use_rpc{false};
  bool m_use_stgc{false};
  bool m_use_mm{false};
  bool m_use_mcLUT{false};

};

} // namespace TrigL2MuonSA

#endif  // MUFASTDATAPREPARATOR_H
