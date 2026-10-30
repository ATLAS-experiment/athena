/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1RAWDATACOLL_RPC_DIGIT_H
#define VP1RAWDATACOLL_RPC_DIGIT_H

#include "VP1RawDataSystems/VP1RawDataCollBase.h"
#include <memory>
struct RpcDigitCuts;

class VP1RawDataColl_RPC_Digit : public VP1RawDataCollBase {
  Q_OBJECT
public:
  static QStringList availableCollections(IVP1System*);
  VP1RawDataColl_RPC_Digit(VP1RawDataCommonData*, const QString& key);
  ~VP1RawDataColl_RPC_Digit() override;
  bool cut(VP1RawDataHandleBase*) override;
public Q_SLOTS:
  void setRpcShowEta(bool);
  void setRpcShowPhi(bool);
  void setRpcShowPrimarySide(bool);
  void setRpcShowOppositeSide(bool);
  void setRpcKeepMissingToT(bool);
  void setRpcAllowedTime(const VP1Interval&);
  void setRpcAllowedToT(const VP1Interval&);
protected:
  void assignDefaultMaterial(SoMaterial*) const override;
  bool load() override;
  qint32 provideCollTypeID() const override { return 1; }
  QString provideSection() const override { return "Muon Stuff"; }
private:
  std::unique_ptr<RpcDigitCuts> m_cuts;
};

#endif
