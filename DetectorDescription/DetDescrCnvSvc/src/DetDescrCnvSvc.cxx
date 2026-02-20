/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Include files
#include "DetDescrCnvSvc/DetDescrCnvSvc.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "DetDescrCnvSvc/IDetDescrCnvSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/IDataSelector.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/ISvcLocator.h"
#include "StoreGate/StoreGateSvc.h"

// External definitions
constexpr long DetDescr_StorageType = 0x44;

//-------------------------------------------------------------------------

/// Standard constructor
DetDescrCnvSvc::DetDescrCnvSvc(const std::string &name, ISvcLocator *svc)
    : ConversionSvc(name, svc, DetDescr_StorageType),
      m_detStore("DetectorStore", name)
 {}

//-------------------------------------------------------------------------

/// Standard Destructor
DetDescrCnvSvc::~DetDescrCnvSvc() = default;

//-------------------------------------------------------------------------

/// Initialize the service.
StatusCode DetDescrCnvSvc::initialize() {
    ATH_CHECK(ConversionSvc::initialize());
    ATH_MSG_INFO(" initializing ");

    // get DetectorStore service
    ATH_CHECK( m_detStore.retrieve() );
    ATH_MSG_INFO("Found DetectorStore service");

    // fill in the Addresses for Transient Detector Store objects
    ATH_MSG_INFO(" filling proxies for detector managers ");

    ATH_CHECK(addToDetStore(117659265, "CaloTTMgr"));
    ATH_CHECK(addToDetStore(4548337, "CaloMgr"));
    ATH_CHECK(addToDetStore(241807251, "CaloSuperCellMgr"));
    ATH_CHECK(addToDetStore(125856940, "CaloIdManager"));
    // IdDict:
    ATH_CHECK(addToDetStore(2411, "IdDict"));

    // IdHelpers
    ATH_CHECK(addToDetStore(164875623, "AtlasID"));
    ATH_CHECK(addToDetStore(2516, "PixelID"));
    ATH_CHECK(addToDetStore(2517, "SCT_ID"));
    ATH_CHECK(addToDetStore(2518, "TRT_ID"));
    ATH_CHECK(addToDetStore(131939624, "PLR_ID"));
    ATH_CHECK(addToDetStore(79264207, "HGTD_ID"));
    ATH_CHECK(addToDetStore(129452393, "SiliconID"));
    ATH_CHECK(addToDetStore(163583365, "LArEM_ID"));
    ATH_CHECK(addToDetStore(99488227, "LArEM_SuperCell_ID"));
    ATH_CHECK(addToDetStore(3870484, "LArHEC_ID"));
    ATH_CHECK(addToDetStore(254277678, "LArHEC_SuperCell_ID"));
    ATH_CHECK(addToDetStore(45738051, "LArFCAL_ID"));
    ATH_CHECK(addToDetStore(12829437, "LArFCAL_SuperCell_ID"));
    ATH_CHECK(addToDetStore(79264204, "LArMiniFCAL_ID"));
    ATH_CHECK(addToDetStore(158698068, "LArOnlineID"));
    ATH_CHECK(addToDetStore(38321944, "TTOnlineID"));
    ATH_CHECK(addToDetStore(115600394, "LArOnline_SuperCellID"));
    ATH_CHECK(addToDetStore(27863673, "LArHVLineID"));
    ATH_CHECK(addToDetStore(80757351, "LArElectrodeID"));
    ATH_CHECK(addToDetStore(2901, "TileID"));
    ATH_CHECK(addToDetStore(49557789, "Tile_SuperCell_ID"));
    ATH_CHECK(addToDetStore(2902, "TileHWID"));
    ATH_CHECK(addToDetStore(2903, "TileTBID"));

    if (m_hasMDT)
        ATH_CHECK(addToDetStore(4170, "MDTIDHELPER"));
    if (m_hasCSC)
        ATH_CHECK(addToDetStore(4171, "CSCIDHELPER"));

    if (m_hasRPC)
        ATH_CHECK(addToDetStore(4172, "RPCIDHELPER"));
    if (m_hasTGC)
        ATH_CHECK(addToDetStore(4173, "TGCIDHELPER"));
    if (m_hasSTGC)
        ATH_CHECK(addToDetStore(4174, "STGCIDHELPER"));
    if (m_hasMM)
        ATH_CHECK(addToDetStore(4175, "MMIDHELPER"));

    ATH_CHECK(addToDetStore(108133391, "CaloLVL1_ID"));
    ATH_CHECK(addToDetStore(123500438, "CaloCell_ID"));
    ATH_CHECK(addToDetStore(128365736, "CaloCell_SuperCell_ID"));
    ATH_CHECK(addToDetStore(167756483, "CaloDM_ID"));
    ATH_CHECK(addToDetStore(190591643, "ZdcID"));

    // for J/GTower
    ATH_CHECK(addToDetStore(218674799, "JTower_ID"));
    ATH_CHECK(addToDetStore(49678914, "GTower_ID"));

    return StatusCode::SUCCESS;
}

//-------------------------------------------------------------------------

/// Create a Generic address using explicit arguments to identify a single
/// object.
StatusCode DetDescrCnvSvc::createAddress(long /* svc_type */,
                                         const CLID & /* clid     */,
                                         const std::string * /* par      */,
                                         const unsigned long * /* ip       */,
                                         IOpaqueAddress *&refpAddress) {
    refpAddress = nullptr;
    return StatusCode::FAILURE;
}

//-------------------------------------------------------------------------

StatusCode DetDescrCnvSvc::createAddress(long /* svc_type */, const CLID &clid,
                                         const std::string &refAddress,
                                         IOpaqueAddress *&refpAddress) {
    try {
        refpAddress = new DetDescrAddress(clid);
        DetDescrAddress *ddAddr;
        ddAddr = dynamic_cast<DetDescrAddress *>(refpAddress);
        if (!ddAddr) {
            MsgStream log(msgSvc(), name());
            log << MSG::FATAL << "Could not cast to DetDescrAddress." << endmsg;
            return StatusCode::FAILURE;
        }
        if (ddAddr->fromString(refAddress).isFailure()) {
            MsgStream log(msgSvc(), name());
            log << MSG::FATAL << "Could not assign address " << refAddress
                << endmsg;
            return StatusCode::FAILURE;
        }
    } catch (...) {
        refpAddress = 0;
    }
    return (refpAddress != 0) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

//-------------------------------------------------------------------------

StatusCode DetDescrCnvSvc::convertAddress(const IOpaqueAddress *pAddress,
                                          std::string &refAddress) {
    const DetDescrAddress *addr =
        dynamic_cast<const DetDescrAddress *>(pAddress);
    if (!addr)
        return StatusCode::FAILURE;
    return addr->toString(refAddress);
}

//-------------------------------------------------------------------------

StatusCode DetDescrCnvSvc::addToDetStore(const CLID &clid,
                                         const std::string &name) const {
    // Based on input parameters, create StoreGate proxies with
    // DetDescrAddresses in the detector store for the different
    // detectors.

    // fill in the Addresses for Transient Detector Store objects

    CxxUtils::RefCountedPtr<DetDescrAddress> addr
      (new DetDescrAddress(clid, name, name));
    ATH_MSG_INFO(" filling address for " << (*addr->par()) << " with CLID "
                                         << addr->clID() << " and storage type "
                                         << addr->svcType()
                                         << " to detector store ");
    ATH_CHECK(m_detStore->recordAddress(std::move(addr)));
    return StatusCode::SUCCESS;
}
