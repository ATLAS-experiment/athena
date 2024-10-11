/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCalibStreamCnvSvc/MuonCalibStreamCnvSvc.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamAddress.h"

#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/IDataSelector.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/ISvcLocator.h"

// External definitions
//extern const long MuonCalibStream_StorageType = 0x43;

/// Standard constructor
MuonCalibStreamCnvSvc::MuonCalibStreamCnvSvc(const std::string &name, ISvcLocator *svc) :
    AthCnvSvc(name, svc, MuonCalibStreamAddress::storageType())  //, m_calibEvent(nullptr)
{
    m_initCnvs.push_back("xAOD::EventInfo");

    declareProperty("InitCnvs", m_initCnvs);
}

/// Standard Destructor
MuonCalibStreamCnvSvc::~MuonCalibStreamCnvSvc() {}

/// Initialize the service.
StatusCode MuonCalibStreamCnvSvc::initialize() {
    ATH_MSG_INFO("Initializing MuonCalibStreamCnvSvc");

    ATH_CHECK(AthCnvSvc::initialize());

    SmartIF<IClassIDSvc> clidSvc{service("ClassIDSvc")};
    ATH_CHECK( clidSvc.isValid() );

    // Initialize the converters
    std::vector<std::string>::const_iterator it = m_initCnvs.begin();
    std::vector<std::string>::const_iterator it_e = m_initCnvs.end();
    for (; it != it_e; ++it) {
        CLID id;
        ATH_MSG_DEBUG(" Try to obtain CLID for " << (*it));
        ATH_CHECK(clidSvc->getIDOfTypeName(*it, id));

        ATH_MSG_DEBUG(" " << *it << " has CLID " << id);
        IConverter *cnv = converter(id);
        if (!cnv) {
            ATH_MSG_WARNING(" Cannot get converter for  " << (*it));
        } else {
            ATH_MSG_DEBUG(" Converter   " << cnv->objType() << " for " << (*it));
        }
    }
    return StatusCode::SUCCESS;
}

/// Update state of the service
StatusCode MuonCalibStreamCnvSvc::updateServiceState(IOpaqueAddress *pAddress) {
    if (pAddress != 0) {
        GenericAddress *pAddr = dynamic_cast<GenericAddress *>(pAddress);
        if (pAddr != 0) { return StatusCode::SUCCESS; }
    }
    return StatusCode::FAILURE;
}
