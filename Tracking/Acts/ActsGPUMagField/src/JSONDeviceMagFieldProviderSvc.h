/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUDATAPREPARATION_JSONDEVICEMAGFIELDPROVIDERSVC_H
#define ACTSGPUDATAPREPARATION_JSONDEVICEMAGFIELDPROVIDERSVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ToolHandle.h"
#include "PathResolver/PathResolver.h"
#include "IDeviceMagFieldProviderTool.h"

#include <string>

namespace ActsTrk {

/**
 * @class JSONDeviceMagFieldProviderSvc
 *
 * @brief Service providing magnetic field description from JSON files
 *
 * This service loads magnetic field data from JSON files,
 * which are needed for executing track parameter estimation and track reconstruction on GPU.
 *
 * All objects are recorded to detector store as pointers to device objects.
 * Note that the different implementations require different tools to make the device copy.
 *
 * @author Neža Ribarič <neza.ribaric@cern.ch>
 */
class JSONDeviceMagFieldProviderSvc
    : public AthService
{
public:

    using AthService::AthService;

    /// Function initializing and executing the file loading
    virtual StatusCode initialize() override;

private:

    ServiceHandle<StoreGateSvc> m_detStore{this, "DetectorStore", "StoreGateSvc/DetectorStore"};
    
    /// The device magnetic field provider tool
    ToolHandle<ActsTrk::IDeviceMagFieldProviderTool> m_deviceFieldProviderTool{
        this, "DeviceMagFieldProviderTool", "",
        "The device magnetic field provider tool"};    

    /// @name The input JSON file names, path resolved with PathResolver
    /// @{
    Gaudi::Property<std::string> m_magFieldFile{
        this, "MagFieldFile", "",
        "Magnetic field JSON file"};   
    /// @}

    /// @name The output object names
    /// @{
    Gaudi::Property<std::string> m_hostMagFieldObjectName{
        this, "HostMagFieldObjectName", "",
        "Traccc host magnetic field object"};  
    Gaudi::Property<std::string> m_deviceMagFieldObjectName{
        this, "DeviceMagFieldObjectName", "",
        "Traccc device magnetic field object"};    
    /// @}

};

} // namespace ActsTrk

#endif // ACTSGPUDATAPREPARATION_JSONDEVICEMAGFIELDPROVIDERSVC_H