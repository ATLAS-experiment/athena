/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCTRIGGERLUTREADER_H
#define TGCTRIGGERLUTREADER_H

class TGCTriggerLUTs;
class CondAttrListCollection;


class TGCTriggerLUTReader {

 public:
  TGCTriggerLUTReader(int lutType);
  ~TGCTriggerLUTReader() = default;

  virtual bool readLUT(TGCTriggerLUTs* ) = 0;
  virtual bool loadParameters(TGCTriggerLUTs* ,
                              const CondAttrListCollection* ) = 0;

 protected:
  int m_lutType{0};
};



#endif
