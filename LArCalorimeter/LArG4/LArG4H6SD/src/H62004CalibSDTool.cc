/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "H62004CalibSDTool.h"

// LArG4 includes
#include "LArG4Code/VolumeUtils.h"

// Local includes
#include "LArG4H62004CalibSD.h"

namespace LArG4
{

  //---------------------------------------------------------------------------
  // Tool constructor
  //---------------------------------------------------------------------------
  H62004CalibSDTool::H62004CalibSDTool(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent)
    : CalibSDTool(type, name, parent)
  {}

  //---------------------------------------------------------------------------
  // Create one SD
  //---------------------------------------------------------------------------
  LArG4CalibSD*
  H62004CalibSDTool::makeOneSD(const std::string& sdName, ILArCalibCalculatorSvc* calc,
                                const std::vector<std::string>& volumes) const
  {
    ATH_MSG_VERBOSE( name() << " makeOneSD" );

    // Parse the wildcard patterns for existing volume names
    auto parsedVolumes = findLogicalVolumes(volumes, msg());

    // Create the simple SD
    auto sd = std::make_unique<LArG4H62004CalibSD>(sdName, calc, m_doPID);
    auto* sdPtr = sd.get();
    sd->setupHelpers(m_larEmID, m_larFcalID, m_larHecID, m_caloDmID);

    // Assign the volumes to the SD
    if( assignSD( std::move(sd), parsedVolumes ).isFailure() ) {
      throw GaudiException("Failed to assign sd: " + sdName,
                           name(), StatusCode::FAILURE);
    }
    return sdPtr;
  }

} // namespace LArG4
