/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBCALOCONDITIONS_ITBCALOPOSTOOL
# define TBCALOCONDITIONS_ITBCALOPOSTOOL

#include "GaudiKernel/IAlgTool.h"

/** 
 ** Class ITBCaloPosTool
 ** 
 ** This is an interface for tool used access the Calorimeter position
 ** in Combined TB

   Sept 28, 2004   Hong Ma 
	

 **/

class ITBCaloPosTool : virtual public IAlgTool
{

public:    
  
    /// Declare interface ID
    DeclareInterfaceID(ITBCaloPosTool, 1 , 0);

    ///  access eta value 
    virtual double  eta () = 0 ;

    ///  access eta value 
    virtual double  theta () = 0 ;

    ///  access eta value 
    virtual double  z () = 0 ;

    ///  access delta value 
    virtual double  delta () = 0 ;


};



#endif
