/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "SubgroupSvc.h"
#include "H5Cpp.h"

SubgroupSvc::SubgroupSvc(const std::string& name,
                         ISvcLocator* pSvcLocator):
  base_class(name, pSvcLocator)
{
}

SubgroupSvc::~SubgroupSvc() = default;

StatusCode SubgroupSvc::initialize() {
  if (m_subgroup.value().empty()) {
    ATH_MSG_ERROR("subgroup name is empty");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(m_parent.retrieve());
  m_group = std::make_unique<H5::Group>(
    m_parent->group()->createGroup(m_subgroup));
  return StatusCode::SUCCESS;
}

H5::Group* SubgroupSvc::group() {
  return m_group.get();
}
