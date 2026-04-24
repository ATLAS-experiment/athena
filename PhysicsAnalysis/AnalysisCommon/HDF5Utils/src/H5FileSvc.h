/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef H5_FILE_SVC_H
#define H5_FILE_SVC_H

#include "AthenaBaseComps/AthService.h"
#include "Gaudi/Property.h"

#include "HDF5Utils/IH5GroupSvc.h"

#include <memory>

namespace H5 {
  class H5File;
  class Group;
}

class H5FileSvc : public extends<AthService, IH5GroupSvc>
{
public:
  H5FileSvc(const std::string& name, ISvcLocator* pSvcLocator);
  ~H5FileSvc();
  virtual StatusCode initialize() override;
  virtual H5::Group* group() override;
private:

  std::unique_ptr<H5::H5File>  m_file      {nullptr};
  std::unique_ptr<H5::Group>   m_root_group{nullptr};
  Gaudi::Property<std::string> m_file_path {this, "path", "", "path to file"};
};

#endif
