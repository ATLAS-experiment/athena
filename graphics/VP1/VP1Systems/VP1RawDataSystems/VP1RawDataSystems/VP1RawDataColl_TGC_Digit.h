/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1RAWDATACOLL_TGC_DIGIT_H
#define VP1RAWDATACOLL_TGC_DIGIT_H

#include "VP1RawDataSystems/VP1RawDataCollBase.h"
#include <memory>
struct TgcDigitCuts;

class VP1RawDataColl_TGC_Digit : public VP1RawDataCollBase {
  Q_OBJECT
public:
  static QStringList availableCollections(IVP1System*);
  VP1RawDataColl_TGC_Digit(VP1RawDataCommonData* common, const QString& key);
  ~VP1RawDataColl_TGC_Digit() override;
  bool cut(VP1RawDataHandleBase*) override;
public Q_SLOTS:
  void setTgcShowStrips(bool);
  void setTgcShowWires(bool);
  void setTgcShowPrevious(bool);
  void setTgcShowCurrent(bool);
  void setTgcShowNext(bool);
  void setTgcShowNextNext(bool);
  void setTgcShowUndefined(bool);
  void setTgcShowUnknown(bool);
protected:
  void assignDefaultMaterial(SoMaterial*) const override;
  bool load() override;
  qint32 provideCollTypeID() const override { return 1; }
  QString provideSection() const override { return "Muon Stuff"; }
private:
  std::unique_ptr<TgcDigitCuts> m_cuts;
};

#endif
