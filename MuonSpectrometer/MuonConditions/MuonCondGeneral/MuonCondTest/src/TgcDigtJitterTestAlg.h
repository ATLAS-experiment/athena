/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TgcDigtJitterTestAlg_H
#define TgcDigtJitterTestAlg_H

// STL
#include <sstream>


// Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaKernel/IAthRNGSvc.h"

// Athena
#include "MuonCondData/TgcDigitJitterData.h"

/// Example class to show calling the MdtConditionsTestAlg
class TgcDigtJitterTestAlg : public AthAlgorithm {
public:
    TgcDigtJitterTestAlg(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~TgcDigtJitterTestAlg();

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;

private:
    SG::ReadCondHandleKey<TgcDigitJitterData> m_readKey{this, "ReadKey", "TgcJitterData", "Key of the Energy threshold data"};

    ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc",
                                        ""};  // Random number service
    Gaudi::Property<unsigned int> m_testJitters{this, "TestJitters", 250};
};  // end of class

#endif
