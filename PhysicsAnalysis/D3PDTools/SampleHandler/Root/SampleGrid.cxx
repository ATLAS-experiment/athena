/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/SampleGrid.h>

#include <stdexcept>
#include <RootCoreUtils/Assert.h>
#include <SampleHandler/GridTools.h>
#include <SampleHandler/MetaFields.h>
#include <SampleHandler/MetaObject.h>
#include <SampleHandler/SampleLocal.h>
#include <memory>

//
// method implementations
//

ClassImp (SH::SampleGrid)

namespace SH
{
  void SampleGrid ::
  testInvariant () const
  {
  }



  SampleGrid ::
  SampleGrid ()
    : Sample ("unnamed")
  {
    RCU_NEW_INVARIANT (this);
  }



  SampleGrid ::
  SampleGrid (const std::string& name)
    : Sample (name)
  {
    RCU_NEW_INVARIANT (this);
  }



  std::size_t SampleGrid ::
  getNumFiles () const
  {
    RCU_READ_INVARIANT (this);
    throw std::runtime_error ("Sample::numFiles not supported for SampleGrid");
  }



  std::string SampleGrid ::
  getFileName (const std::size_t /*index*/) const
  {
    RCU_READ_INVARIANT (this);
    throw std::runtime_error ("Sample::fileName not supported for SampleGrid");
  }



  std::unique_ptr<SampleLocal> SampleGrid ::
  doMakeLocal () const
  {
    RCU_READ_INVARIANT (this);

    auto result = std::make_unique<SampleLocal> (name());
    for (auto& file : makeFileList ())
      result->add (file);
    return result;
  }



  std::vector<std::string> SampleGrid ::
  doMakeFileList () const
  {
    RCU_READ_INVARIANT (this);
    using namespace msgGridTools;

    const char *downloadDir = getenv (downloadStageEnvVar().c_str());

    const std::string sampleName
      = meta()->castString (MetaFields::gridName, name());
    const std::string fileFilter
      = meta()->castString (MetaFields::gridFilter, MetaFields::gridFilter_default);

    if (downloadDir)
    {
      ANA_MSG_DEBUG ("download dir set, trying download");
      if (downloadDir[0] != '/')
        throw std::runtime_error ("rucio download path in variable " + downloadStageEnvVar() + " should start with /");
      return rucioCacheDatasetGlob (downloadDir, sampleName, fileFilter);
    } else
    {
      ANA_MSG_DEBUG ("download dir not set, trying direct access");
      const std::string sourceOptions
        = meta()->castString (MetaFields::gridSourceOptions);
      return rucioDirectAccessGlob (sampleName, fileFilter, sourceOptions);
    }
  }
}
