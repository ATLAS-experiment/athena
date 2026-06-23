/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef ATLASHEPMC_PDFINFO_H
#define ATLASHEPMC_PDFINFO_H
#include "HepMC3/GenEvent.h"
#include "HepMC3/PrintStreams.h"
namespace HepMC {
typedef std::shared_ptr<HepMC3::GenPdfInfo>  GenPdfInfoPtr;
}
#endif
