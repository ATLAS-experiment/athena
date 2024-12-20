/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "NRPC_RawDataProviderTool.h"

#include "xAODMuonRDO/NRPCRDOAuxContainer.h"

namespace Muon {


NRPC_RawDataProviderTool::NRPC_RawDataProviderTool(const std::string& t, const std::string& n, const IInterface* p) :
    base_class(t, n, p) {
    declareInterface<Muon::IMuonRawDataProviderTool>(this);

}

StatusCode NRPC_RawDataProviderTool::initialize() {

    // Get ROBDataProviderSvc
    ATH_CHECK(m_robDataProvider.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());

    ATH_CHECK(m_rdoContainerKey.initialize());

    ATH_CHECK(m_readKey.initialize()); 

    ATH_MSG_DEBUG("initialize() successful in " << name());
    return StatusCode::SUCCESS;
}

StatusCode NRPC_RawDataProviderTool::convertIntoContainer(
    const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs, xAOD::NRPCRDOContainer& nrpcContainer) const {
    ATH_MSG_VERBOSE("convert(): " << vecRobs.size() << " ROBFragments.");

    for (const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment* frag : vecRobs) {
        // convert only if data payload is delivered
        if (frag->rod_ndata() != 0) {
            ATH_CHECK(fillCollections(*frag, nrpcContainer).ignore() );
        } else {
            ATH_MSG_DEBUG(" ROB " << MSG::hex << frag->source_id() << " is delivered with an empty payload" );
        }
    }

    return StatusCode::SUCCESS;
}

StatusCode NRPC_RawDataProviderTool::convert() const  // call decoding function using list of all detector ROBId's
{
    return convert(Gaudi::Hive::currentContext());
}

StatusCode NRPC_RawDataProviderTool::convert(
    const EventContext& ctx) const  // call decoding function using list of all detector ROBId's
{

    SG::ReadCondHandle<RpcCablingMap> readHandle{m_readKey, ctx};
    const RpcCablingMap* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }
    
    const std::vector<uint32_t>& robIds=readCdo->getAllROBId();    
    
    return convert(robIds, ctx);
}

StatusCode NRPC_RawDataProviderTool::convert(const std::vector<IdentifierHash>& HashVec) const {
    return convert(HashVec, Gaudi::Hive::currentContext());
}

StatusCode NRPC_RawDataProviderTool::convert(const std::vector<IdentifierHash>& HashVec, const EventContext& ctx) const {
    SG::ReadCondHandle<RpcCablingMap> readHandle{m_readKey, ctx};
    const RpcCablingMap* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }
    return convert(readCdo->getROBId(HashVec, msgStream()), ctx);
}

StatusCode NRPC_RawDataProviderTool::convert(const std::vector<uint32_t>& robIds) const {
    return convert(robIds, Gaudi::Hive::currentContext());
}

StatusCode NRPC_RawDataProviderTool::convert(const std::vector<uint32_t>& robIds, const EventContext& ctx) const {
    std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecOfRobf;
    m_robDataProvider->getROBData(ctx, robIds, vecOfRobf);
    return convert(vecOfRobf, ctx); 
}

StatusCode NRPC_RawDataProviderTool::convert(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
                                                    const std::vector<IdentifierHash>&) const {
    return convert(vecRobs, Gaudi::Hive::currentContext());
}

StatusCode NRPC_RawDataProviderTool::convert(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
                                                    const std::vector<IdentifierHash>& /*collection*/, const EventContext& ctx) const {
    return convert(vecRobs, ctx);
}

StatusCode NRPC_RawDataProviderTool::convert(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs) const {
    return convert(vecRobs, Gaudi::Hive::currentContext());
}

StatusCode NRPC_RawDataProviderTool::convert(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
                                                    const EventContext& ctx) const {
    ATH_MSG_VERBOSE("convert(): " << vecRobs.size() << " ROBFragments.");

    SG::WriteHandle<xAOD::NRPCRDOContainer> rdoContainerHandle(m_rdoContainerKey, ctx);
    ATH_CHECK(rdoContainerHandle.record(std::make_unique<xAOD::NRPCRDOContainer>(), std::make_unique<xAOD::NRPCRDOAuxContainer>()));
    xAOD::NRPCRDOContainer* rdoContainer = rdoContainerHandle.ptr();

    // use the convert function in the NRPC_RawDataProviderTool class
    ATH_CHECK(convertIntoContainer(vecRobs, *rdoContainer));

    return StatusCode::SUCCESS;
}


StatusCode NRPC_RawDataProviderTool::fillCollections(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag, xAOD::NRPCRDOContainer& rdoIdc) const {
#define WARNING_WITH_LINE(msg) ATH_MSG_WARNING(__FILE__ << ":" << __LINE__<< " " << msg)
    try {
        robFrag.check();
    } catch (eformat::Issue& ex) {
        ATH_MSG_VERBOSE(ex.what());
        return StatusCode::SUCCESS;  // error in fragment
    }

    //uint32_t nstat = robFrag.nstatus();
    uint32_t version = robFrag.rod_version();
    uint32_t sourceId = robFrag.source_id();
    uint32_t rod_sourceId = robFrag.rod_source_id();

    // Unpack sub-detector and tdc sector from sourceId
    uint16_t subDetector = sourceId >> 16;
    uint16_t tdcSector = (sourceId & 0x00ffff);

    ATH_MSG_VERBOSE("ROD version: " << MSG::hex << version << MSG::dec << "  ROB source ID: " << MSG::hex << sourceId << MSG::dec
                                    << "  ROD source ID: " << MSG::hex << rod_sourceId << MSG::dec << "  Subdetector: " << MSG::hex
                                    << subDetector << MSG::dec << "  tdcSector: " << std::hex 
                                    << tdcSector << std::dec );



    // get the pointer to the data
    BS data;
    robFrag.rod_data(data);

    const unsigned int data_size = robFrag.rod_ndata();
    

    // Loop on words
    unsigned int idata=0;
    while (idata<data_size) {
        if (data[idata]==6 && data[idata+4]==0xa0 && data[idata+5]==0) {
            ATH_MSG_DEBUG("NRPC: Empty tdc " << std::hex << data[idata+4] << std::dec << " for " << std::hex << data[idata+1] << std::dec );
        } else if (data[idata]<6) {
            WARNING_WITH_LINE("NRPC: Corrupted: Number of words from tdc " << std::hex << data[idata+4] << std::dec << " is <6 :" << data[idata] );
            break;
        } else if ( (data[idata+data[idata]-2] & 0x000000ff) != 0xa0) {
            WARNING_WITH_LINE("NRPC: Missing expected trailer a0" );
            break;
        } else {

            // Bit inversion and manipulation needed to decode nominal BCID
            uint32_t bcid12 = (data[idata+2] & 0xff000000) >> 24;
            uint32_t bcid34 = (data[idata+2] & 0x00ff0000) >> 8;
            uint32_t bcid56 = (data[idata+2] & 0x0000ff00) << 8;
            uint32_t bcid78 = (data[idata+2] & 0x000000ff) << 24;
            uint32_t bcid_nom =  (bcid12 | bcid34 | bcid56 | bcid78) >> 4;
            
            // Decode data
            for (unsigned int i=0; i<data[idata]-6; i++) {
                uint16_t tdc = (data[idata+4+i] & 0x000000ff) ;
                uint16_t chan = (data[idata+4+i] & 0x0000ff00) >> 8 ;
                float tot = ((data[idata+4+i] & 0x007f0000) >> 16)*0.4 ;
                float time = ((data[idata+4+i] & 0x0f000000) >> 24)*1.6 ;
                uint32_t bcid_hit = (data[idata+4+i] & 0xf0000000) >> 28 ;

                // Compute the BCID of the hit combining the nominal BCID with the last 4 bits from the hit (bcid_hit)
                uint32_t bcid_nom_4bits = (bcid_nom & 0x0000000f) ;
                uint32_t bcid = 0 ;
                if (bcid_hit >= bcid_nom_4bits) {  // The BCID of the hit is subsequent to the nominal 
                    bcid = ( (bcid_nom & 0xfffffff0) | bcid_hit );
                } else {
                    bcid = ( (bcid_nom & 0xfffffff0) | bcid_hit ) + 0x00000010;
                }

                // Build the RDO
                xAOD::NRPCRDO* NrpcRdo = new xAOD::NRPCRDO();
                rdoIdc.push_back(NrpcRdo);
                NrpcRdo->setBcid(bcid);
                NrpcRdo->setTime(time);
                NrpcRdo->setSubdetector(subDetector);
                NrpcRdo->setTdcsector(tdcSector);
                NrpcRdo->setTdc(tdc);
                NrpcRdo->setChannel(chan);
                NrpcRdo->setTimeoverthr(tot);

            }
        }

        idata+=data[idata];
    }
 

    return StatusCode::SUCCESS;

}  // end fillCollections




}  // namespace Muon
