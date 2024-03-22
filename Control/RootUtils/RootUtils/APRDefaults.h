// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

namespace APRDefaults {

    // using a struct so PyROOT can autoload
   struct TTreeNames {
      static constexpr const char* EventData  {"CollectionTree"};
      static constexpr const char* EventTag   {"POOLCollectionTree"};
      static constexpr const char* DataHeader {"POOLContainer"};
   };
   struct RNTupleNames {
      static constexpr const char* EventData  {"EventData"};
      static constexpr const char* EventTag   {"EventTag"};
      static constexpr const char* DataHeader {"DataHeader"};
   };

   static constexpr const char* IndexColName {"index_ref"};

};
  
