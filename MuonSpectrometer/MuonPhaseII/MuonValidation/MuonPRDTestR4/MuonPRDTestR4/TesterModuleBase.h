/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTERR4_TESTERMODULEBASE_H
#define PRDTESTERR4_TESTERMODULEBASE_H

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"
#include "StoreGate/ReadHandleKey.h"
namespace MuonValR4 {
    using namespace MuonVal;

    class TesterModuleBase: public MuonTesterBranch{
      public:
          TesterModuleBase(MuonTesterTree& tree, 
                          const std::string& grp_name, 
                          MSG::Level msglvl = MSG::Level::INFO);

        virtual ~TesterModuleBase() = default;

        bool init() override final;

      protected:
          const Muon::IMuonIdHelperSvc* idHelperSvc() const;
          const MuonGMR4::MuonDetectorManager* getDetMgr() const;
          const ActsTrk::GeometryContext& getGeoCtx(const EventContext& ctx) const;
          virtual bool declare_keys() = 0;

      private:
          const MuonGMR4::MuonDetectorManager* m_detMgr{};
          ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{"Muon::MuonIdHelperSvc/MuonIdHelperSvc", name()};
          SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{"ActsAlignment"};


    };
}
#endif