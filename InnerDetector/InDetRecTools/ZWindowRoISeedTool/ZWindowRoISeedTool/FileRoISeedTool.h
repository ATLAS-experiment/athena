// -*- C++ -*-

/*
  Copyright (C) 2020-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for Z-window RoI from an input file
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiSpacePointsSeedTool_xk_FileRoISeedTool_h
#define SiSpacePointsSeedTool_xk_FileRoISeedTool_h

#include "InDetRecToolInterfaces/IZWindowRoISeedTool.h"
#include "GaudiKernel/EventContext.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODEventInfo/EventInfo.h"

#include <string>
#include <vector>


namespace InDet {

  class FileRoISeedTool final: 
    public extends<AthAlgTool, IZWindowRoISeedTool>
  {

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
      
  public:
      
    ///////////////////////////////////////////////////////////////////
    /// @name Standard tool methods
    ///////////////////////////////////////////////////////////////////
    //@{
    FileRoISeedTool(const std::string&,const std::string&,const IInterface*);
    virtual ~FileRoISeedTool() = default;
    virtual StatusCode               initialize() override;
    //@}

    /** @brief Compute RoI 
     * 
     * Compute region(s) of interest for the event.
     * @return vector of regions of interest.
     */
    virtual std::vector<ZWindow> getRoIs(const EventContext& ctx) const override;

  protected:

    /**  @name Disallow default instantiation, copy, assignment **/
    FileRoISeedTool() = delete;
    FileRoISeedTool(const FileRoISeedTool&) = delete;
    FileRoISeedTool &operator=(const FileRoISeedTool&) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @name Tool configuration properties
    /////////////////////////////////////////////////////////////////// 
    //@{
    StringProperty m_filename{this, "InputFileName", "", "Input file MUST be specified with LowPtRoIFile job option"};
    FloatProperty m_z0Window{this, "TrackZ0Window", 30.0, "Size of RoI along z-axis"}; 

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey", "EventInfo", "xAOD EventInfo object"};
    //@}
    
  }; // FileRoISeedTool
} //InDet namespace

#endif // SiSpacePointsSeedMaker_FileRoISeedTool

