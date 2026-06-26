/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for Z-window(s) RoI tool
/////////////////////////////////////////////////////////////////////////////////

#ifndef InDetRecToolInterfaces_IZWindowRoISeedTool_H
#define InDetRecToolInterfaces_IZWindowRoISeedTool_H

#include "GaudiKernel/IAlgTool.h"


#include <vector>

class EventContext;

namespace InDet
{

  static const InterfaceID IID_IZWindowRoISeedTool("IZWindowRoISeedTool", 1, 0);

  /**
   * @class IZWindowRoISeedTool
   *
   * Compute Region-Of-Interest (RoI) along z-axis for track-reconstruction.
   * See the SiSPSeededTrackRecontructionRoI algorithm for its usage.
   *
   */
  class IZWindowRoISeedTool : virtual public IAlgTool
  {
  public:

    /// @name Interface ID
    //@{
    DeclareInterfaceID(IZWindowRoISeedTool, 1, 0);
    //@}

    ///////////////////////////////////////////////////////////////////
    /// @name Public methods and return type
    ///////////////////////////////////////////////////////////////////
    //@{
    class ZWindow {
    public:
      //* Lower and Upper z bound of the window */
      float zWindow[2] = {-999., -999.};
      //* Reference z-position (if any) */
      float zReference = {-999.};
      //* The perigee z-positions of the tracks (if any) */
      float zPerigeePos[2] = {-999., -999.};

      //* Constructor setting default values */
      ZWindow() = default;

      ~ZWindow() = default;
    };

    /*** @brief Compute RoI z-window(s) */
    virtual std::vector<ZWindow> getRoIs(const EventContext& ctx) const = 0;

    //@}

  }; // End of IZWindowRoISeedTool class definition 


} // End of namespace InDet
#endif
