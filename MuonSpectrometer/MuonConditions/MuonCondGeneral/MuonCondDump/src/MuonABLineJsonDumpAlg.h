/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonCondDump_MuonABLineJsonDumpAlg_H
#define MuonCondDump_MuonABLineJsonDumpAlg_H

/*
 * Algorithm to dump the A & B line container content into a JSON file
*/

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"


class MuonABLineJsonDumpAlg : public AthAlgorithm {
public:
    MuonABLineJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MuonABLineJsonDumpAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual unsigned int cardinality() const override final{return 1;}

private:
    SG::ReadCondHandleKey<ALineContainer> m_readALineKey{this, "ReadALineKey", "ALineContainer", "Key of input muon alignment ALine condition data"};
    SG::ReadCondHandleKey<BLineContainer> m_readBLineKey{this, "ReadBLineKey", "BLineContainer", "Key of input muon alignment BLine condition data"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
   

    Gaudi::Property<std::string> m_jsonFile{this, "OutFile", "ABLineFile.txt", "Path of the file to dump the alignment constants"};

};

#endif
