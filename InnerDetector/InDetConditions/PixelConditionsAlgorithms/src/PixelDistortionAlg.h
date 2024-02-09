/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsAlgorithms/PixelDistortionAlg.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date December, 2019
 * @brief Store pixel distortion data in PixelDistortionData.
 */

#ifndef PIXELDISTORTIONALG_H
#define PIXELDISTORTIONALG_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "DetDescrConditions/DetCondCFloat.h"

#include "StoreGate/WriteCondHandleKey.h"
#include "PixelConditionsData/PixelDistortionData.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "Gaudi/Property.h"

class PixelID;

class PixelDistortionAlg : public AthAlgorithm {  
  public:
    PixelDistortionAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~PixelDistortionAlg() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  private:
    const PixelID* m_pixelID{nullptr};
    ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};  //!< Random number service

    SG::ReadCondHandleKey<DetCondCFloat> m_readKey
    {this, "ReadKey", "/Indet/PixelDist", "Input readout distortion folder"};

    SG::WriteCondHandleKey<PixelDistortionData> m_writeKey
    {this, "WriteKey", "PixelDistortionData", "Output readout distortion data"};
    
    Gaudi::Property<int> m_distortionInputSource
    {this, "DistortionInputSource", 4, "Source of module distortions: 0 (none), 1 (constant), 2 (text file), 3 (random), 4 (database)"};
    
    Gaudi::Property<int> m_distortionVersion
    {this, "DistortionVersion", -1, "Version number for distortion model"};
    
    Gaudi::Property<bool> m_writeToFile
    {this, "DistortionWriteToFile", false, "Record data in storegate"};
    
    Gaudi::Property<std::string> m_inputFileName
    {this, "DistortionFileName", "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/TrackingCP/PixelDistortions/PixelDistortionsData_v2_BB.txt","Read distortions from this file"};
 
};

#endif
