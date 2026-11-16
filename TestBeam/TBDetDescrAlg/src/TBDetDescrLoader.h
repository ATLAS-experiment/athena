/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBDetDescrLoader_H
#define TBDetDescrLoader_H

#include "AthenaBaseComps/AthAlgorithm.h"

class TBDetDescrManager;

#include <string>

class TBDetDescrLoader : public AthAlgorithm {
public:
    //Constructor
    TBDetDescrLoader(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TBDetDescrLoader();                         
    
    //Gaudi Hooks
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual StatusCode finalize() override;

private:

    Gaudi::Property<std::string> m_TBDetDescrManager{this, "TBDetDescrManager", "TBDetDescrMgr", "DetectorStore key of the TBDetDescrManager"};
    Gaudi::Property<std::string> m_TBElementContainer{this, "TBElementContainer", "TBElementCnt", "StoreGate key of the TBElementContainer"};
    Gaudi::Property<int> m_readAction{this, "ReadAction", -1, "Read action: 0 - nothing, 1 - read once, 2 - read always, <0 - create manager"};
    Gaudi::Property<int> m_writeAction{this, "WriteAction", 0, "Write action: 0 - nothing, 1 - write once, 2 - write always"};

    TBDetDescrManager * m_TBDDM{nullptr};
    const TBDetDescrManager * c_TBDDM{nullptr};
};

#endif
