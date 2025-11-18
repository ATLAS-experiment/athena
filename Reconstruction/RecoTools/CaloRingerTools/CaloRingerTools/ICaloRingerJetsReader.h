/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// $Id: ICaloRingerJetsReader.h $
#ifndef CALORINGERTOOLS_ICALORINGERJETSREADER
#define CALORINGERTOOLS_ICALORINGERJETSREADER

/**
   @class ICaloRingerJetsReader
   @brief Interface for tool CaloRingerJetsReader

   $$$
*/

// Core Include
#include "GaudiKernel/IAlgTool.h"

// Interface Includes:
#include "ICaloRingerInputReader.h"


namespace Ringer {

static const InterfaceID IID_ICaloRingerJetsReader("ICaloRingerJetsReader", 1, 0);

class ICaloRingerJetsReader : virtual public ICaloRingerInputReader
{
 public:
  /** @brief Virtual destructor*/
  virtual ~ICaloRingerJetsReader() {};
	
  /** @brief AlgTool interface methods */
  static const InterfaceID& interfaceID();

  /** @brief initialize method*/
  virtual StatusCode initialize() = 0;
  /** @brief execute method **/                                                    
  virtual StatusCode execute() = 0;
  /** @brief finalize method*/
  virtual StatusCode finalize() = 0;

};

inline const InterfaceID& ICaloRingerJetsReader::interfaceID()
{
  return IID_ICaloRingerJetsReader;
}

} // namespace Ringer

#endif
