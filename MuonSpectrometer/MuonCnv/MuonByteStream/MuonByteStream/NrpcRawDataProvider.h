/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAM_NRPCRAWDATAPROVIDER_H
#define MUONBYTESTREAM_NRPCRAWDATAPROVIDER_H

// Base class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"

namespace Muon {

    class NrpcRawDataProvider : public AthReentrantAlgorithm {
    public:
        //! Constructor.
        NrpcRawDataProvider(const std::string &name, ISvcLocator *pSvcLocator);

        //! Initialize
        virtual StatusCode initialize();

        //! Execute
        virtual StatusCode execute(const EventContext &ctx) const;

        //! Destructur
        ~NrpcRawDataProvider() = default;

    private:
        /// Tool handle for raw data provider tool
        ToolHandle<Muon::IMuonRawDataProviderTool> m_rawDataTool{this, "ProviderTool",
                                                                 "Muon::NRPC_RawDataProviderTool/NrpcRawDataProviderTool"};

    };
}  // namespace Muon

#endif
