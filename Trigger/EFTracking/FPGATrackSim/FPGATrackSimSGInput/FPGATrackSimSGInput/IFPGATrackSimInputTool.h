/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimSGInput_IFPGATrackSimInputTool_h
#define FPGATrackSimSGInput_IFPGATrackSimInputTool_h

#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"
#include "GaudiKernel/IAlgTool.h"

class FPGATrackSimEventInputHeader;
/**
 * @brief Interface of tools responsible for generation of the FPGATrackSim sim
 * wrapper files
 *
 */
class IFPGATrackSimInputTool : virtual public ::IAlgTool {
 public:
  DeclareInterfaceID(IFPGATrackSimInputTool, 1, 0);
  virtual ~IFPGATrackSimInputTool() = default;
  /**
   * @brief Reads the data from outer word (Athena  StoreGate) and fills the
   * output object called header
   *
   * @param header object to update
   * @return StatusCode
   */
  virtual StatusCode readData(FPGATrackSimEventInputHeader* header,
                              const EventContext& eventContext) const = 0;
};

#endif  // FPGATrackSimSGInput_IFPGATrackSimInputTool_h
