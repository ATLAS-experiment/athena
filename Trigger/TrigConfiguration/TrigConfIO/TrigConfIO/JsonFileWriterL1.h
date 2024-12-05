/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @brief Write Json file from L1Menu object
 * To validate correct loading of the L1 menu
 */

#ifndef TRIGCONFSTORAGE_JSONFILEWRITERL1_H
#define TRIGCONFSTORAGE_JSONFILEWRITERL1_H

#include "TrigConfBase/TrigConfMessaging.h"



namespace TrigConf {
  class L1Menu;
  class L1BunchGroupSet;
  class L1PrescalesSet;

   /**
    * @brief Loader of trigger configurations from Json files
    */
   class JsonFileWriterL1 : public TrigConfMessaging {
   public:

      /** Constructor */
      JsonFileWriterL1();

      /** Destructor */
      virtual ~JsonFileWriterL1() override = default;

      bool writeJsonFile(const std::string & filename, const L1Menu & l1menu) const;
      bool writeJsonFile(const std::string & filename, const L1BunchGroupSet & l1bgs) const;
      bool writeJsonFile(const std::string & filename, const L1PrescalesSet & l1bgs) const;

   };

}
#endif
