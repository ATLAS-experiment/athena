/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCDATABASESLBTOROD_HH
#define MUONTGC_CABLING_TGCDATABASESLBTOROD_HH

#include "MuonTGC_Cabling/TGCDatabase.h"

namespace MuonTGC_Cabling {

class TGCDatabaseSLBToROD : public TGCDatabase {
   public:
    // Constructor & Destructor
    TGCDatabaseSLBToROD(const std::string& filename,
                        const std::string& blockname);

    TGCDatabaseSLBToROD(const TGCDatabaseSLBToROD&) = default;

    virtual ~TGCDatabaseSLBToROD();

    virtual bool update(const std::vector<int>&);

    virtual int find(const std::vector<int>&) const;

   private:
    virtual void readDB();
};

}  // namespace MuonTGC_Cabling

#endif
