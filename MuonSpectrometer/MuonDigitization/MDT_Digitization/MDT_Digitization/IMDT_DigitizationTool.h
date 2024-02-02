/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDT_DIGITIZATION_IMDT_DIGITIZATIONTOOL_H
#define MDT_DIGITIZATION_IMDT_DIGITIZATIONTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "MDT_Digitization/MdtDigiToolOutput.h"
/*-----------------------------------------------

   Created 7-5-2004 by Niels van Eldik

 Interface for tools which convert MDT digitization input quantities into
 the signal
-----------------------------------------------*/
namespace CLHEP {
    class HepRandomEngine;
}
class MdtDigiToolInput;


class IMDT_DigitizationTool : virtual public IAlgTool {
public:
    virtual ~IMDT_DigitizationTool() = default;
    
    virtual MdtDigiToolOutput digitize(const EventContext& ctx,
                                       const MdtDigiToolInput& input, 
                                       CLHEP::HepRandomEngine* rndmEngine) const = 0;

    DeclareInterfaceID(IMDT_DigitizationTool, 1, 0);
};

#endif
