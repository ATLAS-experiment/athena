/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODPrimitives/tools/getIsolationCorrectionDecorator.h"
#include <stdexcept>
namespace xAOD {

const SG::Decorator< uint32_t >
  getIsolationCorrectionBitsetDecorator( Iso::IsolationFlavour type ){
    std::string name(Iso::toCString(type));
    name+="CorrBitset";
    return SG::Decorator< uint32_t >( name );
  }

const SG::Decorator< float >
  getIsolationCorrectionDecorator( Iso::IsolationFlavour type, Iso::IsolationCaloCorrection corr, 
                                  Iso::IsolationCorrectionParameter param  ){
    std::string name(Iso::toCString(type));                                                                       
    if (corr == Iso::coreCone || corr == Iso::coreConeSC)
      name+=toCString(corr); 
    else{
        name = toCString(corr);
    }

    if (param==xAOD::Iso::coreEnergy || param==xAOD::Iso::coreArea){
      name+=toCString(param );    
     }else{
      throw std::runtime_error("IsolationCorrectionParameter out of bounds");
    }
    name+="Correction";
    return SG::Decorator< float >( name );                                                                                                              
  }

// Isolation Calo 
const SG::Decorator< float >
  getIsolationCorrectionDecorator( Iso::IsolationType type, Iso::IsolationCaloCorrection corr){
    std::string name(Iso::toCString(type));
    name+=toCString(corr);
    name+="Correction";
    return SG::Decorator< float >( name );
  }

const SG::Decorator< float >
  getIsolationCorrectionDecorator( Iso::IsolationFlavour type, Iso::IsolationTrackCorrection corr ){
      std::string name(Iso::toCString(type));                                                                         
      name+=toCString(corr);    
      name+="Correction";
      return SG::Decorator< float >( name );

  }
} // namespace xAOD

