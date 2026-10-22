/** emacs: this is  -*- c++ -*- 
 **
 **  @file    RoiSelector.h
 **  @author  marco aparo
 **
 **  @brief   fetches an Roi vector using the TrigDecisionTool
 **
 **  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/

#ifndef IDTPM2_ROISELECTOR_H
#define IDTPM2_ROISELECTOR_H

#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include <string>
#include <vector>

namespace IDTPM2 {

class RoiSelector {

public:
  
  using roi_info_t = TrigCompositeUtils::LinkInfo<TrigRoiDescriptorCollection>;
  
public:
  
  /// Constructor
  // not going to waste time messing about with ToolHandles for simple utility classes
  RoiSelector( const ToolHandle<Trig::TrigDecisionTool>* trigDecTool=0 ) : m_trigDecTool(trigDecTool) { } 
    
  /// Destructor
  virtual ~RoiSelector() = default;
  
  /// Retrieve RoIs
  std::vector<roi_info_t> retrieveRois( const std::string& chainName, const std::string& roiKey, const int& chainLeg=-1 ) const;

  void setTrigDecisionTool( const ToolHandle<Trig::TrigDecisionTool>* trigDecTool ) { m_trigDecTool = trigDecTool; }  
  
private:
  
  /// do we really need this to be a ToolHandle ?? Can we not just make do with 
  /// a pointer passed in by the parent tool ? Yes, yes we can.
  const ToolHandle<Trig::TrigDecisionTool>* m_trigDecTool; 
  
}; // class RoiSelectionTool

} // namespace IDTPM2


#endif //> !INDETTRACKPERFMON2_ROISELECTOR_H
