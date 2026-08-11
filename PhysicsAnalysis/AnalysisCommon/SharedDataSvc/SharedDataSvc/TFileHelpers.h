/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SHARED_DATA_SVC_TFILE_HELPERS_H
#define SHARED_DATA_SVC_TFILE_HELPERS_H

#include <SharedDataSvc/ISharedDataSvc.h>
#include <TObject.h>

class TFile;

namespace asg
{
  namespace detail
  {
    StatusCode readTObjectFromTFile (const ISharedDataSvc& svc, std::shared_ptr<TFile>& file, const std::string& fileName, const std::string& name, const std::type_info& type, const std::function<bool (const std::shared_ptr<const TObject>&)>& castSetter);
  }


  /// @brief read an object from a ROOT file
  ///
  /// This should (hopefully) make it straightforward to read individual
  /// objects from a ROOT file, as needed. It will open the file on the
  /// first object read, and base the object name on both the file name
  /// and the object name.
  ///
  /// @warn Do **not** place the `TFile` object inside of your tool.
  /// This should not escape the scope of `initialize` (or wherever you
  /// are doing your file reading).

  template<typename T>
  StatusCode readFromTFile (const ISharedDataSvc& svc, std::shared_ptr<TFile>& file, const std::string& fileName, const std::string& name, std::shared_ptr<const T>& object)
  {
    return detail::readTObjectFromTFile (svc, file, fileName, name, typeid (T), [&object] (const std::shared_ptr<const TObject>& baseObject)
    {
      object = std::dynamic_pointer_cast<const T> (baseObject);
      return static_cast<bool> (object);
    });
  }
}

#endif
