/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class LArSamples::D3PDConverter
   @brief storage of the time histories of all the cells
 */

#ifndef LArSamples_D3PDConverter_H
#define LArSamples_D3PDConverter_H

#include "LArSamplesMon/CaloD3PDClass.h"
#include "CxxUtils/checker_macros.h"

class TString;

#include <map>
#include <memory>


namespace LArSamples {
  class Interface;


  class ATLAS_NOT_THREAD_SAFE D3PDConverter : public CaloD3PDClass
  {

    public:

      /** @brief Constructor  */
      D3PDConverter(TTree& tree, const TString& templateFile, const TString& translatorFile);
      virtual ~D3PDConverter();

      bool makeSamplesTuple(const TString& outputFileName);

      bool initMapping(const TString& templateFile, const TString& translatorFile);
      
      std::map<unsigned int, unsigned int> m_id2hash;
      std::unique_ptr<Interface> m_template;
  };
}

#endif
