/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef H5_SUBGROUP_SVC_H
#define H5_SUBGROUP_SVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"

#include "HDF5Utils/IH5GroupSvc.h"

#include <memory>

namespace H5 {
  class Group;
}

class SubgroupSvc : public extends<AthService, IH5GroupSvc>
{
public:
  SubgroupSvc(const std::string& name, ISvcLocator* pSvcLocator);
  ~SubgroupSvc();
  virtual StatusCode initialize() override;
  virtual H5::Group* group() override;
private:
  ServiceHandle<IH5GroupSvc>   m_parent  {this, "parent", "",
    "parent group service"};
  Gaudi::Property<std::string> m_subgroup{this, "subgroup", "",
    "subgroup name"};
  Gaudi::Property<bool> m_mustBeNew{this, "mustBeNew", false,
    "fail if subgroup already exists"};
  std::unique_ptr<H5::Group>   m_group   {nullptr};
};

#endif
