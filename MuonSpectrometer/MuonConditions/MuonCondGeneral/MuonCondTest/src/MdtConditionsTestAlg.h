/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTConditionsTestAlgMT_H
#define MDTConditionsTestAlgMT_H

// STL
#include <sstream>


// Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

// Athena
#include "Identifier/Identifier.h"
#include "MuonCondData/MdtCondDbData.h"

/// Example class to show calling the MdtConditionsTestAlg
class MdtConditionsTestAlg : public AthAlgorithm {
public:
    MdtConditionsTestAlg(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~MdtConditionsTestAlg();

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;

private:
    SG::ReadCondHandleKey<MdtCondDbData> m_readKey{this, "ReadKey", "MdtCondDbData", "Key of MdtCondDbData"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

};  // end of class

#endif
