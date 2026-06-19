/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonCondDump_MdtAsBuiltJsonDumpAlg_H
#define MuonCondDump_MdtAsBuiltJsonDumpAlg_H

/*
 * Algorithm to dump the A & B line container content into a JSON file
*/

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"


class MdtAsBuiltJsonDumpAlg : public AthAlgorithm {
public:
    MdtAsBuiltJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MdtAsBuiltJsonDumpAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual unsigned int cardinality() const override final{return 1;}

private:
    SG::ReadCondHandleKey<MdtAsBuiltContainer> m_readKey{this, "ReadKey", "MdtAsBuiltContainer", "Key of input muon alignment ALine condition data"};
    
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
   

    Gaudi::Property<std::string> m_jsonFile{this, "OutFile", "MdtAsBuiltFile.txt", "Path of the file to dump the alignment constants"};

};
#endif