/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class VP1RawDataSysController             //
//                                                            //
//  Description: Raw data system controller    .              //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: January 2009                             //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef VP1RAWDATASYSCONTROLLER_H
#define VP1RAWDATASYSCONTROLLER_H

#include "VP1Base/VP1Controller.h"
#include "VP1RawDataSystems/VP1RawDataFlags.h"
class SoGroup;

class VP1RawDataSysController : public VP1Controller {

  Q_OBJECT

public:

  VP1RawDataSysController(IVP1System * sys);
  virtual ~VP1RawDataSysController();

  void actualRestoreSettings(VP1Deserialise&);
  int currentSettingsVersion() const;
  void actualSaveSettings(VP1Serialise&) const;

  VP1CollectionWidget * collWidget() const;

  ///////////////////////////////////
  //  Access Methods for settings  //
  ///////////////////////////////////

  //To put at start of the system scenegraph:
  SoGroup * drawOptions() const;

  //Materials:
  SoMaterial * trtHTMaterial() const;
  SoMaterial * bcmHighAttMaterial() const;

  //Interactions (no change signals needed):
  bool printInfoOnClick() const;
  bool printVerboseInfoOnClick() const;
  bool zoomOnClick() const;

  //General cuts:
  VP1Interval cutAllowedEta() const;
  QList<VP1Interval> cutAllowedPhi() const;//All off: empty list. All on: list with one entry: ]-inf,inf[

  //Other cuts:
  VP1RawDataFlags::InDetPartsFlags enabledInDetParts() const;
  bool trtRequireHT() const;
  VP1Interval trtAllowedToT() const;
  unsigned lucidMinQDC() const;

  // Inclusive muon digit ranges; unchecked bounds are unrestricted.
  bool tgcShowStrips() const;
  bool tgcShowWires() const;
  bool tgcShowPrevious() const;
  bool tgcShowCurrent() const;
  bool tgcShowNext() const;
  bool tgcShowNextNext() const;
  bool tgcShowUndefined() const;
  bool tgcShowUnknown() const;
  bool mdtExcludeMasked() const;
  bool stgcShowPads() const;
  bool stgcShowStrips() const;
  bool stgcShowWires() const;
  bool stgcExcludeDead() const;
  bool stgcExcludePileup() const;

  VP1Interval mdtAllowedADC() const;
  VP1Interval mdtAllowedTDC() const;
  VP1Interval mmAllowedCharge() const;
  VP1Interval mmAllowedTime() const;
  VP1Interval stgcAllowedCharge() const;
  VP1Interval stgcAllowedTime() const;


  //Display options:
  bool showVolumeOutLines() const;
  bool useSpecialTRTHTMaterial() const;
  bool useSpecialBCMHighAttMaterial() const;

  ///////////////////////////////////////
  //  Signals for changes in settings  //
  ///////////////////////////////////////
signals:
  void cutAllowedEtaChanged(const VP1Interval&);
  void cutAllowedPhiChanged(const QList<VP1Interval>&);
  void showVolumeOutLinesChanged(bool);
  void useSpecialTRTHTMaterialChanged(bool);
  void useSpecialBCMHighAttMaterialChanged(bool);
  void enabledInDetPartsChanged(VP1RawDataFlags::InDetPartsFlags);
  void trtRequireHTChanged(bool);
  void trtAllowedToTChanged(const VP1Interval&);//nano-seconds
  void lucidMinQDCChanged(unsigned);
  void tgcShowStripsChanged(bool);
  void tgcShowWiresChanged(bool);
  void tgcShowPreviousChanged(bool);
  void tgcShowCurrentChanged(bool);
  void tgcShowNextChanged(bool);
  void tgcShowNextNextChanged(bool);
  void tgcShowUndefinedChanged(bool);
  void tgcShowUnknownChanged(bool);
  void mdtExcludeMaskedChanged(bool);
  void stgcShowPadsChanged(bool);
  void stgcShowStripsChanged(bool);
  void stgcShowWiresChanged(bool);
  void stgcExcludeDeadChanged(bool);
  void stgcExcludePileupChanged(bool);

  void mdtAllowedADCChanged(const VP1Interval&);
  void mdtAllowedTDCChanged(const VP1Interval&);
  void mmAllowedChargeChanged(const VP1Interval&);
  void mmAllowedTimeChanged(const VP1Interval&);
  void stgcAllowedChargeChanged(const VP1Interval&);
  void stgcAllowedTimeChanged(const VP1Interval&);


private:

  class Imp;
  Imp * m_d;

//   //For verbose output:
//   template <class T> static QString toString( const T& t ) { return VP1Controller::toString(t); }//unhide base methods
//   static QString toString( const QPair<bool,double>& par );
private Q_SLOTS:
  void possibleChange_cutAllowedEta();
  void possibleChange_cutAllowedPhi();
  void possibleChange_showVolumeOutLines();
  void possibleChange_useSpecialTRTHTMaterial();
  void possibleChange_useSpecialBCMHighAttMaterial();
  void possibleChange_enabledInDetParts();
  void possibleChange_trtRequireHT();
  void possibleChange_trtAllowedToT();
  void possibleChange_lucidMinQDC();
  void possibleChange_tgcShowStrips();
  void possibleChange_tgcShowWires();
  void possibleChange_tgcShowPrevious();
  void possibleChange_tgcShowCurrent();
  void possibleChange_tgcShowNext();
  void possibleChange_tgcShowNextNext();
  void possibleChange_tgcShowUndefined();
  void possibleChange_tgcShowUnknown();
  void possibleChange_mdtExcludeMasked();
  void possibleChange_stgcShowPads();
  void possibleChange_stgcShowStrips();
  void possibleChange_stgcShowWires();
  void possibleChange_stgcExcludeDead();
  void possibleChange_stgcExcludePileup();

  void possibleChange_mdtAllowedADC();
  void possibleChange_mdtAllowedTDC();
  void possibleChange_mmAllowedCharge();
  void possibleChange_mmAllowedTime();
  void possibleChange_stgcAllowedCharge();
  void possibleChange_stgcAllowedTime();

};

#endif
