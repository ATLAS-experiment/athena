/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "H5FileSvc.h"
#include "H5Cpp.h"

H5FileSvc::H5FileSvc(const std::string& name, ISvcLocator* pSvcLocator):
  base_class(name, pSvcLocator)
{
}

H5FileSvc::~H5FileSvc() = default;

StatusCode H5FileSvc::initialize() {
  if (m_file_path.empty()) {
    return StatusCode::FAILURE;
  }
  m_file = std::make_unique<H5::H5File>(m_file_path, H5F_ACC_TRUNC);
  m_root_group = std::make_unique<H5::Group>(m_file->openGroup("/"));
  return StatusCode::SUCCESS;
}

H5::Group* H5FileSvc::group() {
  return m_root_group.get();
}
