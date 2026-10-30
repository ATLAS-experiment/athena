/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1RAWDATACOLL_RPC_DIGIT_H
#define VP1RAWDATACOLL_RPC_DIGIT_H

#include "VP1RawDataSystems/VP1RawDataCollBase.h"

class VP1RawDataColl_RPC_Digit : public VP1RawDataCollBase {
public:
  static QStringList availableCollections(IVP1System*);
  VP1RawDataColl_RPC_Digit(VP1RawDataCommonData*, const QString& key);
  bool cut(VP1RawDataHandleBase*) override;
protected:
  void assignDefaultMaterial(SoMaterial*) const override;
  bool load() override;
  qint32 provideCollTypeID() const override { return 1; }
  QString provideSection() const override { return "Muon Stuff"; }
};

#endif
