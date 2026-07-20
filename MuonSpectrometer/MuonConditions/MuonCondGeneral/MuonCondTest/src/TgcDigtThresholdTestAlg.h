/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TgcDigtThresholdTestAlg_H
#define TgcDigtThresholdTestAlg_H

// STL
#include <sstream>


// Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

// Athena
#include "Identifier/Identifier.h"
#include "MuonCondData/TgcDigitThresholdData.h"

/// Example class to show calling the MdtConditionsTestAlg
class TgcDigtThresholdTestAlg : public AthAlgorithm {
public:
    TgcDigtThresholdTestAlg(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~TgcDigtThresholdTestAlg();

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;

private:
    SG::ReadCondHandleKey<TgcDigitThresholdData> m_readKey{this, "ReadKey", "TgcEnergyThresholds", "Key of the Energy threshold data"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

};  // end of class

#endif
