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
  H5::Group* parent = m_parent->group();
  if (parent->nameExists(m_subgroup)) {
    if (m_mustBeNew) {
      ATH_MSG_ERROR("subgroup '" << m_subgroup.value()
                    << "' already exists");
      return StatusCode::FAILURE;
    }
    m_group = std::make_unique<H5::Group>(
      parent->openGroup(m_subgroup));
  } else {
    m_group = std::make_unique<H5::Group>(
      parent->createGroup(m_subgroup));
  }
  return StatusCode::SUCCESS;
}

H5::Group* SubgroupSvc::group() {
  return m_group.get();
}
