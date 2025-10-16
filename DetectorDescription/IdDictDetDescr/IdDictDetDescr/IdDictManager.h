/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 IdDictDetDescr package
 -----------------------------------------
 ***************************************************************************/

#ifndef IDDICTDETDESCR_IDDICTMANAGER_H
# define IDDICTDETDESCR_IDDICTMANAGER_H

#include "AthenaKernel/CLASS_DEF.h"

class IdHelper;
class IdDictDictionary;
class IdDictMgr;

/**
 *  IdDictManager is the interface to identifier dictionaries. This
 *  class inherits most of its functionality from IdDictMgr and adds
 *  the capabilities to be an object in the detector store.
 */

class IdDictManager
{
public:
    IdDictManager();
    IdDictManager(const IdDictMgr& mgr);
    ~IdDictManager();

    //IdDictDictionary* 	find_dictionary (const std::string& name) const;  

    const IdDictMgr*	manager			(void) const;

    /// Return value: 0 OK, >0 error
    int                 initializeHelper        (IdHelper& helper) const;

private:
    const IdDictMgr* 	m_mgr;
};


//using the macros below we can assign an identifier (and a version)
//This is required and checked at compile time when you try to record/retrieve
CLASS_DEF(IdDictManager, 2411, 1)

#endif // IDDICTDETDESCR_IDDICTMANAGER_H
