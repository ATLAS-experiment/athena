/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDALG_MUONNSWASBUILTCONDALG_H
#define MUONCONDALG_MUONNSWASBUILTCONDALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "MuonAlignmentData/NswAsBuiltDbData.h"


class NswAsBuiltCondAlg : public AthReentrantAlgorithm {
public:
    NswAsBuiltCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~NswAsBuiltCondAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

    virtual bool isReEntrant() const override final { return false; }

private:
    SG::ReadCondHandleKey<CondAttrListCollection> m_readMmAsBuiltParamsKey{this, "ReadMmAsBuiltParamsKey", "/MUONALIGN/ASBUILTPARAMS/MM",
                                                                            "Key of MM/ASBUILTPARAMS input condition data"};
     
    Gaudi::Property<std::string> m_MmJsonPath{this,"MicroMegaJSON",   "", "Pass As-Built parameters for MM chambers from an Ascii file"};
     
    SG::WriteCondHandleKey<NswAsBuiltDbData> m_writeNswAsBuiltKey{this, "WriteNswAsBuiltKey", "NswAsBuiltDbData",
                                                                     "Key of output muon alignment MM+STGC/AsBuilt condition data"};



};



#endif
