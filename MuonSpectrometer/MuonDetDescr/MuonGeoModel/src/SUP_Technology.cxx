/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 stiffening aluminum bars holding together RPC and DED components
 ----------------------------------------------------------------
 ***************************************************************************/

//<doc><file>	$Id: SUP_Technology.cxx,v 1.1 2008-07-31 10:57:55 dquarrie Exp $
//<version>	$Name: not supported by cvs2svn $

#include <utility>

#include "MuonGeoModel/SUP_Technology.h"

namespace MuonGM {

    SUP::SUP(MYSQL& mysql, std::string s)
       : Technology(mysql, std::move(s)){}
} // namespace MuonGM
