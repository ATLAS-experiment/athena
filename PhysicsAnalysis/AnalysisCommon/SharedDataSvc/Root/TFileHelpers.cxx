/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SharedDataSvc/TFileHelpers.h>

#include <AsgMessaging/MessageCheck.h>

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
    StatusCode readTObjectFromTFile (const ISharedDataSvc& svc, std::shared_ptr<TFile>& file, const std::string& fileName, const std::string& name, const std::type_info& type, const std::function<bool (const std::shared_ptr<const TObject>&)>& castSetter)
    {
      using namespace asg::msgUserCode;

      std::shared_ptr<const TObject> object;
      if (svc.getMakeShared (fileName + "/" + name, object, [&file, &fileName, &name, &type, &castSetter] (std::shared_ptr<const TObject>& data)
      {
        if (!file)
        {
          file.reset (TFile::Open (fileName.c_str (), "READ"));
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
        std::shared_ptr<TObject> mydata {dir->Get (name.substr(split).c_str ())};
        if (!mydata)
        {
          ANA_MSG_ERROR ("failed to read " << name << " from file " << fileName);
          return StatusCode::FAILURE;
        }
        if (auto hist = dynamic_cast<TH1*>(mydata.get()))
          hist->SetDirectory(nullptr);
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
