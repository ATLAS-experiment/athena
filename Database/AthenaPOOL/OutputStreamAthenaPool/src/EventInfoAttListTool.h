/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENTINFOATTLISTTOOL_H 
#define EVENTINFOATTLISTTOOL_H 

/*****************************************************************************
Name    : EventInfoAttListTool.h
Purpose : Tool to buid the Global Event Tags
*****************************************************************************/

#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODEventInfo/EventInfo.h" //typedef

#include <string>
#include <memory> //unique_ptr

class AthenaAttributeList;
namespace coral{
 class AttributeListSpecification;
}


class EventInfoAttListTool : public AthAlgTool  {

public:
  
  /** Standard Constructor */
  using AthAlgTool::AthAlgTool;

  /** Overriding initialize, finalize and execute */
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  // interface 
  bool isValid() const;
  const coral::AttributeListSpecification& getAttributeSpecification() const;
  std::unique_ptr<AthenaAttributeList> getAttributeListPtr(const xAOD::EventInfo& einfo) const;

protected:

  /** the various components to build their own fragments of tag */
  StatusCode eventTag(AthenaAttributeList& eventTagCol, const xAOD::EventInfo& eventInfo) const;

  coral::AttributeListSpecification* m_attribListSpec{};

};

#endif // EVENTINFOATTLISTTOOL_H
