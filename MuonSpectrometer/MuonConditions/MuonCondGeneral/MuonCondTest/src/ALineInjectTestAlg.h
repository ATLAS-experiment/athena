/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDTEST_ALINEINJECTTESTALG_H
#define MUONCONDTEST_ALINEINJECTTESTALG_H

#include <AthenaBaseComps/AthCondAlgorithm.h>
#include <MuonAlignmentData/CorrContainer.h>
#include <StoreGate/WriteCondHandleKey.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

/**
 *  The ALineInjectTestAlg creates a complete arbitrary set of Alignment constants. Basically, it distorts each of the component by an
 *  increasing counter in mm or degrees
*/

class ALineInjectTestAlg : public AthCondAlgorithm{
    public:
       
        ALineInjectTestAlg(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~ALineInjectTestAlg() = default;
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;
    private:
        SG::WriteCondHandleKey<ALineContainer> m_writeKey{this, "WriteKey", "InjectedALines",
                                                          "Key of output muon alignment ALine condition data"};
        
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

};
#endif