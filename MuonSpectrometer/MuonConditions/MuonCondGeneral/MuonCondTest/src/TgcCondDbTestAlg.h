/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TgcCondDbTestAlg_H
#define TgcCondDbTestAlg_H

// STL
#include <sstream>


// Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

// Athena
#include "Identifier/Identifier.h"
#include "MuonCondData/TgcCondDbData.h"

/// Example class to show calling the MdtConditionsTestAlg
class TgcCondDbTestAlg : public AthAlgorithm {
public:
    TgcCondDbTestAlg(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~TgcCondDbTestAlg();

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;

private:
    SG::ReadCondHandleKey<TgcCondDbData> m_readKey{this, "ReadKey", "TgcCondDbData", "Key of TgcCondDbData"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

};  // end of class

#endif
