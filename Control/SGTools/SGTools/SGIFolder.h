/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SGTOOLS_IFOLDER_H
#define SGTOOLS_IFOLDER_H

#include <set>
#include <string>

#include "GaudiKernel/IAlgTool.h"
#include "SGTools/SGFolderItem.h"

namespace SG {  
  /** @class SG::IFolder
   * @brief a run-time configurable list of data objects
   *
   * @author pcalafiura@lbl.gov - ATLAS Collaboration
   **/
  class IFolder : public virtual IAlgTool
  {
  public:
    /// Interface ID
    DeclareInterfaceID( IFolder, 1, 0 );

    /// the list we manage
    typedef std::set<FolderItem> ItemList; //FIXME would be nice to move to SG::Folder

    /// \name access the ItemList
    //@{
    typedef ItemList::const_iterator const_iterator;
    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
    //@}
    ///add a data object identifier to the list
    virtual StatusCode add(const std::string& typeName, const std::string& skey) = 0;
    ///add a data object identifier to the list
    virtual StatusCode add(const CLID& clid, const std::string& skey) = 0;

    ///clear the folder contents
    virtual void clear() = 0;

    ///update list of items
    virtual void updateItemList(bool checkValidCLID) = 0;

  };
} //ns SG

#endif
