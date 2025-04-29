/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCondAlg/NswAsBuiltCondAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteCondHandle.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "PathResolver/PathResolver.h"
#include <iostream>
#include <fstream>

 NswAsBuiltCondAlg::NswAsBuiltCondAlg(const std::string& algName, ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm{algName, pSvcLocator} {
}
StatusCode NswAsBuiltCondAlg::initialize() {
    ATH_CHECK(m_readMmAsBuiltParamsKey.initialize(m_MmJsonPath.value().empty() && !m_readMmAsBuiltParamsKey.empty()));
    ATH_CHECK(m_writeNswAsBuiltKey.initialize());
    if (!m_MmJsonPath.value().empty()) ATH_MSG_INFO("Load MicroMega as-built from external JSON "<< m_MmJsonPath);
    else if (!m_readMmAsBuiltParamsKey.empty()) ATH_MSG_INFO("Load MicroMega as-built from COOL <"<<m_readMmAsBuiltParamsKey.key()<<">");
    else ATH_MSG_INFO("MicroMega as-built is deactiviated");
    return StatusCode::SUCCESS;
}

StatusCode NswAsBuiltCondAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<NswAsBuiltDbData> writeHandle{m_writeNswAsBuiltKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));

    std::unique_ptr<NswAsBuiltDbData> writeCdo{std::make_unique<NswAsBuiltDbData>()};

    if (!m_readMmAsBuiltParamsKey.empty()) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readMmAsBuiltParamsKey, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_ERROR("Null pointer to the read MM/ASBUILTPARAMS conditions object");
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readHandle);
        ATH_MSG_INFO("Size of MM/ASBUILTPARAMS CondAttrListCollection " << readHandle.fullKey()
                                                                     << " ->size()= " << readHandle->size());
        unsigned int nLines{0};
        for( CondAttrListCollection::const_iterator itr = readHandle->begin(); itr != readHandle->end(); ++itr) {
            const coral::AttributeList& atr = itr->second;
            const std::string data{*(static_cast<const std::string*>((atr["data"]).addressOfData()))};
            ATH_MSG_DEBUG(__FILE__<<":"<<__LINE__<<" data load is " << data << " FINISHED HERE ");            
            writeCdo->microMegaData = std::make_unique<NswAsBuilt::StripCalculator>();
            writeCdo->microMegaData->parseJSON(data);          
            ++nLines;
        }
        if(nLines>1) {
            ATH_MSG_FATAL(nLines << " data objects were loaded for MM/ASBUILTPARAMS! Expected only one for this validity range!");
            return StatusCode::FAILURE;
        }
    }
    if (!m_MmJsonPath.value().empty()){
        ATH_MSG_INFO("Load micromega as-built constants from a JSON file");
        std::ifstream thefile{PathResolverFindCalibFile(m_MmJsonPath)};
        if (!thefile.good()) {
            ATH_MSG_FATAL("No such file or directory "<<m_MmJsonPath);
            return StatusCode::FAILURE;
        }
        std::stringstream buffer;
        buffer << thefile.rdbuf();
        writeCdo->microMegaData = std::make_unique<NswAsBuilt::StripCalculator>();
        writeCdo->microMegaData->parseJSON(buffer.str());       
    }

    if (!writeCdo->microMegaData) {
        ATH_MSG_ERROR("No AsBuilt constants were loaded. Please check the algorithm configucration");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    return StatusCode::SUCCESS;
}
