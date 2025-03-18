/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 03/2025
* Description: Initial implementation of the ITk Pixel RDO,
* heavily adapted from the existing ID
*/


#ifndef INDETRAWDATA_ITkPIXELRDO_CONTAINER_H
# define INDETRAWDATA_ITkPIXELRDO_CONTAINER_H

#include "AthenaKernel/CLASS_DEF.h"
#include "InDetRawData/InDetRawDataContainer.h"
#include "InDetRawData/InDetRawDataCollection.h"
#include "ITkPixelRDORawData.h"
#include "AthLinks/DeclareIndexingPolicy.h"

typedef InDetRawDataContainer<InDetRawDataCollection<ITkPixelRDORawData> > ITkPixelRDO_Container;

CLASS_DEF(ITkPixelRDO_Container, 1328667962, 1)
CONTAINER_IS_IDENTCONT(ITkPixelRDO_Container)

#endif

