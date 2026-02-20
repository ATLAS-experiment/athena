/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCOMBINEDDECORALGS_MUONRPCTIMINGDECORALG_H
#define MUONCOMBINEDDECORALGS_MUONRPCTIMINGDECORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "xAODMuon/MuonContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "StoreGate/WriteDecorHandle.h"
/*
    Algorithm to decorate the Rpc position and timing to the Muon particle
*/

class MuonRpcTimingDecorAlg: public AthReentrantAlgorithm {
   public:
        MuonRpcTimingDecorAlg(const std::string& name, ISvcLocator* pSvcLocator);
        ~MuonRpcTimingDecorAlg() = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;
    private:
      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
      ServiceHandle<Muon::IMuonEDMHelperSvc> m_edmHelperSvc{this, "edmHelper", "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
                                                              "Handle to the service providing the IMuonEDMHelperSvc interface"};

      SG::ReadHandleKey<xAOD::MuonContainer> m_MuonContainer{this, "MuonContainer", "Muons" ,"Muon container"};

      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_idKey{this, "IdKey", m_MuonContainer, "rpcHitIdentifier", "rpcHitIdentifier decoration key"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_posXKey{this, "PosXkey", m_MuonContainer, "rpcHitPositionX", "rpcHitPositionX decoration key"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_posYKey{this, "PosYkey", m_MuonContainer, "rpcHitPositionY", "rpcHitPositionY decoration key"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_posZKey{this, "PosZkey", m_MuonContainer, "rpcHitPositionZ", "rpcHitPositionZ decoration key"};
      SG::WriteDecorHandleKey<xAOD::MuonContainer> m_timeKey{this, "TimeKey", m_MuonContainer, "rpcHitTime", "rpcHitTime decoration key"};

      struct RpcInfo{
        RpcInfo(const Amg::Vector3D& pos_,
                const float time_,
                const unsigned int id_):
            pos{pos_},time{time_},id{id_}{}
        Amg::Vector3D pos{0.,0.,0.};
        float time{0.f};
        unsigned int id{0};
      };
      void getRpcTiming(const xAOD::TrackParticle* tp, std::vector<RpcInfo>& info) const;

      template <typename T> void insert(const std::vector<RpcInfo>& info,
                                      const xAOD::Muon* muon,
                                      SG::WriteDecorHandle<xAOD::MuonContainer,std::vector<T>>& decor,
                                      std::function<T(const RpcInfo&)>func) const;
};
#endif
