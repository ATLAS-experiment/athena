/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

///////////////////////////////////////////////////////////////////
// SharedObject.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUITLS_SHAREDONOTDELETE_H
#define TRKDETDESCRUITLS_SHAREDONOTDELETE_H

namespace Trk {

template<typename T>
const auto do_not_delete = [](T*) {};

} // end of namespace

#endif // TRKDETDESCRUITLS_SHAREDOBJECT_H
