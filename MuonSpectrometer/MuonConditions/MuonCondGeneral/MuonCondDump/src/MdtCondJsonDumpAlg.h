/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Algorithm to dump the dead chambers & DCS voltages into a JSON file
*/

#ifndef MUONCONDDUMP_MDTCONDJSONDUMPALG_H
#define MUONCONDDUMP_MDTCONDJSONDUMPALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonCondData/MdtCondDbData.h"

class MdtCondJsonDumpAlg : public AthAlgorithm {
public:
    MdtCondJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MdtCondJsonDumpAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual unsigned int cardinality() const override final{return 1;}


private:
    /// Dumps all channels into a JSON format
    void dumpDeadChannels(const std::unordered_set<Identifier>& channels, 
                          std::ostream& ostr, 
                          bool dumpMultiLayer = false, 
                          bool dumpLayer = false, 
                          bool dumpTube = false) const;
    /// Dumps the Identifier into a json format
    void dumpIdentifier(const Identifier& id, 
                        std::ostream& ostr, 
                        bool dumpMultiLayer, 
                        bool dumpLayer, 
                        bool dumpTube,
                        bool trailingComma) const;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::ReadCondHandleKey<MdtCondDbData> m_readKey{this, "ReadKey", "MdtCondDbData", "Key of MdtCondDbData"};

    Gaudi::Property<std::string> m_deadChannelJSON{this, "DeadChannelJSON", "DeadChannels.json", "Json file to dump all dead channels"};
    Gaudi::Property<std::string> m_dcsJSON{this, "DcsVoltJSON", "DcsVoltage.json", "Dump all DCS values into a JSON"};
   
};

#endif
