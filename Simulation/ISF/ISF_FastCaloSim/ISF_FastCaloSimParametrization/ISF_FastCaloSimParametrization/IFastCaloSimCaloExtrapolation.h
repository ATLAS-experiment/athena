/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IFastCaloSimCaloExtrapolation_H
#define IFastCaloSimCaloExtrapolation_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

class TFCSTruthState;
class TFCSExtrapolationState;
class G4FieldTrack;

static const InterfaceID IID_IFastCaloSimCaloExtrapolation("IFastCaloSimCaloExtrapolation", 1, 0);

class IFastCaloSimCaloExtrapolation : virtual public IAlgTool
{
 public:
   /** AlgTool interface methods */
   static const InterfaceID& interfaceID() { return IID_IFastCaloSimCaloExtrapolation; }

   virtual void extrapolate(TFCSExtrapolationState& result,const TFCSTruthState* truth) const = 0;
   virtual void extrapolate(TFCSExtrapolationState& result,const TFCSTruthState* truth, const std::vector<G4FieldTrack>& caloSteps) const = 0;

};

#endif // IFastCaloSimCaloExtrapolation_H
