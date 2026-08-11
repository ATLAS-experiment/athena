/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SHARED_DATA_HELPERS_TFILE_HELPERS_H
#define SHARED_DATA_HELPERS_TFILE_HELPERS_H

#include <AsgMessaging/StatusCode.h>

#include <TObject.h>

#include <concepts>
#include <memory>
#include <string>
#include <functional>

class TFile;

namespace asg
{
  namespace detail
  {
    /// @brief helper function for @ref readFromTFile
    StatusCode readTObjectFromTFile (std::shared_ptr<TFile>& file, const std::string& fileName, const std::string& name, const std::type_info& type, const std::function<bool (const std::shared_ptr<const TObject>&)>& castSetter);
  }


  /// @brief read an object from a ROOT file
  ///
  /// This should (hopefully) make it straightforward to read individual
  /// objects from a ROOT file, as needed. It will open the file on the
  /// first object read, and base the object name on both the file name
  /// and the object name.
  ///
  /// The general way to use this is:
  /// ```
  /// std::shared_ptr<TFile> file;
  /// std::shared_ptr<const TH2> my_data;
  /// ASSERT_SUCCESS (readFromTFile (file, "my_file.root", "my_data", my_data));
  /// ```
  ///
  /// @warning Do **not** place the `TFile` object inside of your tool.
  /// This should not escape the scope of `initialize` (or wherever you
  /// are doing your file reading).
  ///
  /// @warning This only supports objects that can outlive the file
  /// they were read from. Objects that stay owned by the file are
  /// rejected (`TTree`, `TDirectory`), and directory-attached objects
  /// (`TH1`, `TEfficiency`) get detached from the file on read.
  ///
  /// @note In general the `fileName` should be one returned by @ref
  /// PathResolver::find_file or similar, to make sure it can find
  /// calibration files in various places.

  template<typename T>
    requires std::derived_from<T, TObject>
  StatusCode readFromTFile (std::shared_ptr<TFile>& file, const std::string& fileName, const std::string& name, std::shared_ptr<const T>& object)
  {
    return detail::readTObjectFromTFile (file, fileName, name, typeid (T), [&object] (const std::shared_ptr<const TObject>& baseObject)
    {
      object = std::dynamic_pointer_cast<const T> (baseObject);
      return static_cast<bool> (object);
    });
  }
}

#endif
