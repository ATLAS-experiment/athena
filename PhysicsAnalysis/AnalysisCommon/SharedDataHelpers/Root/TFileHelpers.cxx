/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SharedDataHelpers/TFileHelpers.h>

#include <SharedDataHelpers/MessageCheck.h>
#include <SharedDataHelpers/SharedDataHelpers.h>

#include <TEfficiency.h>
#include <TFile.h>
#include <TH1.h>

#include <boost/core/demangle.hpp>

//
// method implementations
//

namespace asg
{
  namespace detail
  {
    StatusCode readTObjectFromTFile (std::shared_ptr<TFile>& file, const std::string& fileName, const std::string& name, const std::type_info& type, const std::function<bool (const std::shared_ptr<const TObject>&)>& castSetter)
    {
      using namespace msgSharedDataHelpers;

      std::shared_ptr<const TObject> object;
      // Technically, if you name your files and/or your objects really
      // weird you may encounter a name clash here, but as long as your
      // file name ends with ".root" and your object name does not
      // contain ".root" you ought to be ok.
      if (getMakeSharedData (fileName + "//" + name, object, [&file, &fileName, &name, &type, &castSetter] (std::shared_ptr<const TObject>& data)
      {
        // restore gDirectory on exit, TFile::Open changes it
        TDirectory::TContext directoryContext;
        if (!file)
        {
          file.reset (TFile::Open (fileName.c_str (), "READ"));
          if (file && file->IsZombie())
            file.reset ();
          if (!file)
          {
            ANA_MSG_ERROR ("failed to open file " << fileName);
            return StatusCode::FAILURE;
          }
        }
        TDirectory* dir = file.get();
        std::string::size_type split = 0, split2 = 0;
        while (split2 = name.find('/', split), split2 != std::string::npos)
        {
          auto subname = name.substr(split, split2 - split);
          if (!subname.empty())
          {
            dir = dynamic_cast<TDirectory*> (dir->Get (subname.c_str ()));
            if (!dir)
            {
              ANA_MSG_ERROR ("failed to navigate to directory " << name.substr(0, split2) << " in file " << fileName);
              return StatusCode::FAILURE;
            }
          }
          split = split2 + 1;
        }
        TObject *rawdata = dir->Get (name.substr(split).c_str ());
        if (!rawdata)
        {
          ANA_MSG_ERROR ("failed to read " << name << " from file " << fileName);
          return StatusCode::FAILURE;
        }
        // objects that stay owned by the file cannot outlive it, and
        // must not be wrapped in a shared_ptr (double delete)
        if (rawdata->InheritsFrom ("TTree") || dynamic_cast<TDirectory*>(rawdata))
        {
          ANA_MSG_ERROR ("object " << name << " in file " << fileName << " is of unsupported type " << rawdata->ClassName());
          return StatusCode::FAILURE;
        }
        std::shared_ptr<TObject> mydata {rawdata};
        if (auto hist = dynamic_cast<TH1*>(mydata.get()))
          hist->SetDirectory(nullptr);
        else if (auto eff = dynamic_cast<TEfficiency*>(mydata.get()))
          eff->SetDirectory(nullptr);
        data = mydata;
        return StatusCode::SUCCESS;
      }).isFailure())
      {
        ANA_MSG_ERROR ("while trying to read " << name << " from file " << fileName << " as type " << boost::core::demangle (type.name()));
        return StatusCode::FAILURE;
      }
      if (!castSetter(object))
      {
        ANA_MSG_ERROR ("failed to cast " << name << " from file " << fileName << " to the requested type " << boost::core::demangle (type.name()) << ", actual type is " << boost::core::demangle (typeid (*object).name()));
        return StatusCode::FAILURE;
      }
      return StatusCode::SUCCESS;
    }
  }
}
