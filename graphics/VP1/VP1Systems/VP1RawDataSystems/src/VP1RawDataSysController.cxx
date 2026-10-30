/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1RawDataSysController           //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: January 2009                             //
//                                                            //
////////////////////////////////////////////////////////////////

#define VP1IMPVARNAME m_d

#include "VP1RawDataSystems/VP1RawDataSysController.h"
#include "ui_vp1rawdatacontrollerform.h"
#include "ui_rawdata_settings_display_form.h"
#include "ui_rawdata_settings_interactions_form.h"
#include "ui_rawdata_settings_cuts_form.h"
#include "VP1Base/VP1Serialise.h"
#include "VP1Base/VP1Deserialise.h"
#include "VP1Base/VP1CollectionWidget.h"

#include "GaudiKernel/SystemOfUnits.h"

#include <QCursor>
#include <QDialog>
#include <QGuiApplication>
#include <QScreen>

#include <cmath>


//____________________________________________________________________
class VP1RawDataSysController::Imp {
public:
  Ui::VP1RawDataControllerForm ui{};
  Ui::RawDataSysSettingsCutsForm ui_cuts{};
  Ui::RawDataSysSettingsInteractionsForm ui_int{};
  Ui::RawDataSysSettingsDisplayForm ui_display{};
  VP1CollectionWidget * collWidget = nullptr;

  VP1Interval last_cutAllowedEta;
  QList<VP1Interval> last_cutAllowedPhi;//All off: empty list. All on: list with one entry: ]-inf,inf[
  bool last_showVolumeOutLines = false;
  bool last_useSpecialTRTHTMaterial = false;
  bool last_useSpecialBCMHighAttMaterial = false;
  VP1RawDataFlags::InDetPartsFlags last_enabledInDetParts;
  bool last_trtRequireHT = false;
  VP1Interval last_trtAllowedToT;
  unsigned last_lucidMinQDC = 0U;
  bool last_mdtExcludeMasked = false;
  bool last_stgcShowPads = true;
  bool last_stgcShowStrips = true;
  bool last_stgcShowWires = true;
  bool last_stgcExcludeDead = false;
  bool last_stgcExcludePileup = false;

  VP1Interval last_mdtAllowedADC;
  VP1Interval last_mdtAllowedTDC;
  VP1Interval last_mmAllowedCharge;
  VP1Interval last_mmAllowedTime;
  VP1Interval last_stgcAllowedCharge;
  VP1Interval last_stgcAllowedTime;

};

//____________________________________________________________________
VP1RawDataSysController::VP1RawDataSysController(IVP1System * sys)
  : VP1Controller(sys,"VP1RawDataSysController"), m_d(new Imp)
{
  m_d->ui.setupUi(this);

  m_d->collWidget = new VP1CollectionWidget;
  setupCollWidgetInScrollArea(m_d->ui.collWidgetScrollArea,m_d->collWidget);

  initDialog(m_d->ui_display, m_d->ui.pushButton_settings_display);
  initDialog(m_d->ui_int, m_d->ui.pushButton_settings_interactions);
  // Keep long cut forms usable on small screens and with large desktop fonts.
  auto cutsDialog = std::make_unique<QDialog>(nullptr, Qt::WindowStaysOnTopHint);
  m_d->ui_cuts.setupUi(cutsDialog.get());
  cutsDialog->setSizeGripEnabled(true);
  cutsDialog->setMinimumSize(320, 240);
  m_d->ui_cuts.cutsContentLayout->setAlignment(Qt::AlignTop);
  QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
  if (!screen)
    screen = QGuiApplication::primaryScreen();
  const QSize preferredSize(520, 640);
  cutsDialog->resize(screen ? preferredSize.boundedTo(screen->availableGeometry().size() * 0.85)
                            : preferredSize);
  initDialog(std::move(cutsDialog), m_d->ui.pushButton_settings_cuts,
             m_d->ui_cuts.pushButton_close);

  m_d->ui_display.widget_drawOptions->setLineWidths(1.5);
  m_d->ui_display.widget_drawOptions->setPointSizes(2.5);
  m_d->ui_display.widget_drawOptions->setComplexityDisabled();

  m_d->ui_cuts.etaPhiCutWidget->setEtaCutEnabled(false);

  m_d->ui_display.matButton_trtht->setMaterial(VP1MaterialButton::createMaterial(1.0,0.0,0.0));
  m_d->ui_display.matButton_bcmhighatt->setMaterial(VP1MaterialButton::createMaterial(1.0,0.0,0.0));


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //  Setup connections which monitor changes in the controller so that we may emit signals as appropriate:  //
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // -> cutAllowedEta:
  addUpdateSlot(SLOT(possibleChange_cutAllowedEta()));
  connectToLastUpdateSlot(m_d->ui_cuts.etaPhiCutWidget,SIGNAL(allowedEtaChanged(const VP1Interval&)));

  // -> cutAllowedPhi:
  addUpdateSlot(SLOT(possibleChange_cutAllowedPhi()));
  connectToLastUpdateSlot(m_d->ui_cuts.etaPhiCutWidget,SIGNAL(allowedPhiChanged(const QList<VP1Interval>&)));

  // -> showVolumeOutLines
  addUpdateSlot(SLOT(possibleChange_showVolumeOutLines()));
  connectToLastUpdateSlot(m_d->ui_display.checkBox_showVolumeOutLines);

  // -> useSpecialTRTHTMaterial
  addUpdateSlot(SLOT(possibleChange_useSpecialTRTHTMaterial()));
  connectToLastUpdateSlot(m_d->ui_display.checkBox_trtHTUseSpecialMat);

  // -> useSpecialBCMHighAttMaterial
  addUpdateSlot(SLOT(possibleChange_useSpecialBCMHighAttMaterial()));
  connectToLastUpdateSlot(m_d->ui_display.checkBox_bcmHighAttUseSpecialMat);

  // -> enabledInDetParts
  addUpdateSlot(SLOT(possibleChange_enabledInDetParts()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_cut_indet_barrelA);
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_cut_indet_barrelC);
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_cut_indet_endcapA);
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_cut_indet_endcapC);

  // -> trtRequireHT
  addUpdateSlot(SLOT(possibleChange_trtRequireHT()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkbox_cut_trt_needHT);

  // -> trtAllowedToT
  addUpdateSlot(SLOT(possibleChange_trtAllowedToT()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkbox_cut_trt_TotMin);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_cut_trt_ToTMin);
  connectToLastUpdateSlot(m_d->ui_cuts.checkbox_cut_trt_TotMax);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_cut_trt_ToTMax);

  // -> lucidMinQDC
  addUpdateSlot(SLOT(possibleChange_lucidMinQDC()));
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_lucid_qdc_min);

  addUpdateSlot(SLOT(possibleChange_mdtAllowedADC()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_mdtAllowedADCMin);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_mdtAllowedADCMin);
  connect(m_d->ui_cuts.checkBox_mdtAllowedADCMin, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_mdtAllowedADCMin, &QDoubleSpinBox::setEnabled);
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_mdtAllowedADCMax);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_mdtAllowedADCMax);
  connect(m_d->ui_cuts.checkBox_mdtAllowedADCMax, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_mdtAllowedADCMax, &QDoubleSpinBox::setEnabled);
  addUpdateSlot(SLOT(possibleChange_mdtAllowedTDC()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_mdtAllowedTDCMin);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_mdtAllowedTDCMin);
  connect(m_d->ui_cuts.checkBox_mdtAllowedTDCMin, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_mdtAllowedTDCMin, &QDoubleSpinBox::setEnabled);
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_mdtAllowedTDCMax);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_mdtAllowedTDCMax);
  connect(m_d->ui_cuts.checkBox_mdtAllowedTDCMax, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_mdtAllowedTDCMax, &QDoubleSpinBox::setEnabled);
  addUpdateSlot(SLOT(possibleChange_mmAllowedCharge()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_mmAllowedChargeMin);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_mmAllowedChargeMin);
  connect(m_d->ui_cuts.checkBox_mmAllowedChargeMin, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_mmAllowedChargeMin, &QDoubleSpinBox::setEnabled);
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_mmAllowedChargeMax);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_mmAllowedChargeMax);
  connect(m_d->ui_cuts.checkBox_mmAllowedChargeMax, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_mmAllowedChargeMax, &QDoubleSpinBox::setEnabled);
  addUpdateSlot(SLOT(possibleChange_mmAllowedTime()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_mmAllowedTimeMin);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_mmAllowedTimeMin);
  connect(m_d->ui_cuts.checkBox_mmAllowedTimeMin, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_mmAllowedTimeMin, &QDoubleSpinBox::setEnabled);
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_mmAllowedTimeMax);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_mmAllowedTimeMax);
  connect(m_d->ui_cuts.checkBox_mmAllowedTimeMax, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_mmAllowedTimeMax, &QDoubleSpinBox::setEnabled);
  addUpdateSlot(SLOT(possibleChange_stgcAllowedCharge()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_stgcAllowedChargeMin);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_stgcAllowedChargeMin);
  connect(m_d->ui_cuts.checkBox_stgcAllowedChargeMin, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_stgcAllowedChargeMin, &QDoubleSpinBox::setEnabled);
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_stgcAllowedChargeMax);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_stgcAllowedChargeMax);
  connect(m_d->ui_cuts.checkBox_stgcAllowedChargeMax, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_stgcAllowedChargeMax, &QDoubleSpinBox::setEnabled);
  addUpdateSlot(SLOT(possibleChange_stgcAllowedTime()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_stgcAllowedTimeMin);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_stgcAllowedTimeMin);
  connect(m_d->ui_cuts.checkBox_stgcAllowedTimeMin, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_stgcAllowedTimeMin, &QDoubleSpinBox::setEnabled);
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_stgcAllowedTimeMax);
  connectToLastUpdateSlot(m_d->ui_cuts.spinBox_stgcAllowedTimeMax);
  connect(m_d->ui_cuts.checkBox_stgcAllowedTimeMax, &QCheckBox::toggled,
          m_d->ui_cuts.spinBox_stgcAllowedTimeMax, &QDoubleSpinBox::setEnabled);

  addUpdateSlot(SLOT(possibleChange_mdtExcludeMasked()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_mdtExcludeMasked);

  addUpdateSlot(SLOT(possibleChange_stgcShowPads()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_stgcShowPads);
  addUpdateSlot(SLOT(possibleChange_stgcShowStrips()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_stgcShowStrips);
  addUpdateSlot(SLOT(possibleChange_stgcShowWires()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_stgcShowWires);
  addUpdateSlot(SLOT(possibleChange_stgcExcludeDead()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_stgcExcludeDead);
  addUpdateSlot(SLOT(possibleChange_stgcExcludePileup()));
  connectToLastUpdateSlot(m_d->ui_cuts.checkBox_stgcExcludePileup);

  initLastVars();
}

//____________________________________________________________________
VP1RawDataSysController::~VP1RawDataSysController()
{
  delete m_d;
}


//____________________________________________________________________
int VP1RawDataSysController::currentSettingsVersion() const
{
  return 4;
}

//____________________________________________________________________
void VP1RawDataSysController::actualSaveSettings(VP1Serialise&s) const
{
  s.save(m_d->ui_cuts.etaPhiCutWidget);
  s.save(m_d->ui_cuts.checkBox_cut_indet_barrelA);
  s.save(m_d->ui_cuts.checkBox_cut_indet_barrelC);
  s.save(m_d->ui_cuts.checkBox_cut_indet_endcapA);
  s.save(m_d->ui_cuts.checkBox_cut_indet_endcapC);
  s.save(m_d->ui_cuts.checkbox_cut_trt_needHT);
  s.save(m_d->ui_cuts.checkbox_cut_trt_TotMin);
  s.save(m_d->ui_cuts.spinBox_cut_trt_ToTMin);
  s.save(m_d->ui_cuts.checkbox_cut_trt_TotMax);
  s.save(m_d->ui_cuts.spinBox_cut_trt_ToTMax);
  s.save(m_d->ui_display.checkBox_showVolumeOutLines);
  s.save(m_d->ui_display.widget_drawOptions);
  s.save(m_d->ui_display.checkBox_trtHTUseSpecialMat);
  s.save(m_d->ui_display.checkBox_bcmHighAttUseSpecialMat);
  s.save(m_d->ui_display.matButton_trtht);
  s.save(m_d->ui_display.matButton_bcmhighatt);
  s.save(m_d->ui_int.checkBox_printinfo);
  s.save(m_d->ui_int.checkBox_printinfo_verbose);
  s.save(m_d->ui_int.checkBox_zoom);
  s.save(m_d->ui_cuts.spinBox_lucid_qdc_min);//Version 1+
  // Version 2: muon digit ranges.
  s.save(m_d->ui_cuts.checkBox_mdtAllowedADCMin);
  s.save(m_d->ui_cuts.spinBox_mdtAllowedADCMin);
  s.save(m_d->ui_cuts.checkBox_mdtAllowedADCMax);
  s.save(m_d->ui_cuts.spinBox_mdtAllowedADCMax);
  s.save(m_d->ui_cuts.checkBox_mdtAllowedTDCMin);
  s.save(m_d->ui_cuts.spinBox_mdtAllowedTDCMin);
  s.save(m_d->ui_cuts.checkBox_mdtAllowedTDCMax);
  s.save(m_d->ui_cuts.spinBox_mdtAllowedTDCMax);
  s.save(m_d->ui_cuts.checkBox_mmAllowedChargeMin);
  s.save(m_d->ui_cuts.spinBox_mmAllowedChargeMin);
  s.save(m_d->ui_cuts.checkBox_mmAllowedChargeMax);
  s.save(m_d->ui_cuts.spinBox_mmAllowedChargeMax);
  s.save(m_d->ui_cuts.checkBox_mmAllowedTimeMin);
  s.save(m_d->ui_cuts.spinBox_mmAllowedTimeMin);
  s.save(m_d->ui_cuts.checkBox_mmAllowedTimeMax);
  s.save(m_d->ui_cuts.spinBox_mmAllowedTimeMax);
  s.save(m_d->ui_cuts.checkBox_stgcAllowedChargeMin);
  s.save(m_d->ui_cuts.spinBox_stgcAllowedChargeMin);
  s.save(m_d->ui_cuts.checkBox_stgcAllowedChargeMax);
  s.save(m_d->ui_cuts.spinBox_stgcAllowedChargeMax);
  s.save(m_d->ui_cuts.checkBox_stgcAllowedTimeMin);
  s.save(m_d->ui_cuts.spinBox_stgcAllowedTimeMin);
  s.save(m_d->ui_cuts.checkBox_stgcAllowedTimeMax);
  s.save(m_d->ui_cuts.spinBox_stgcAllowedTimeMax);
  s.save(m_d->ui_cuts.checkBox_mdtExcludeMasked);//Version 3+
  // Version 4: sTGC channel types and status.
  s.save(m_d->ui_cuts.checkBox_stgcShowPads);
  s.save(m_d->ui_cuts.checkBox_stgcShowStrips);
  s.save(m_d->ui_cuts.checkBox_stgcShowWires);
  s.save(m_d->ui_cuts.checkBox_stgcExcludeDead);
  s.save(m_d->ui_cuts.checkBox_stgcExcludePileup);


}

//____________________________________________________________________
void VP1RawDataSysController::actualRestoreSettings(VP1Deserialise& s)
{
  if (s.version()<0||s.version()>4) {
    message("Warning: State data in .vp1 file has unsupported version ("+str(s.version())+")");
    return;
  }
  s.restore(m_d->ui_cuts.etaPhiCutWidget);
  s.restore(m_d->ui_cuts.checkBox_cut_indet_barrelA);
  s.restore(m_d->ui_cuts.checkBox_cut_indet_barrelC);
  s.restore(m_d->ui_cuts.checkBox_cut_indet_endcapA);
  s.restore(m_d->ui_cuts.checkBox_cut_indet_endcapC);
  s.restore(m_d->ui_cuts.checkbox_cut_trt_needHT);
  s.restore(m_d->ui_cuts.checkbox_cut_trt_TotMin);
  s.restore(m_d->ui_cuts.spinBox_cut_trt_ToTMin);
  s.restore(m_d->ui_cuts.checkbox_cut_trt_TotMax);
  s.restore(m_d->ui_cuts.spinBox_cut_trt_ToTMax);
  s.restore(m_d->ui_display.checkBox_showVolumeOutLines);
  s.restore(m_d->ui_display.widget_drawOptions);
  s.restore(m_d->ui_display.checkBox_trtHTUseSpecialMat);
  s.restore(m_d->ui_display.checkBox_bcmHighAttUseSpecialMat);
  s.restore(m_d->ui_display.matButton_trtht);
  s.restore(m_d->ui_display.matButton_bcmhighatt);
  s.restore(m_d->ui_int.checkBox_printinfo);
  s.restore(m_d->ui_int.checkBox_printinfo_verbose);
  s.restore(m_d->ui_int.checkBox_zoom);
  if (s.version()>=1)
    s.restore(m_d->ui_cuts.spinBox_lucid_qdc_min);
  if (s.version() >= 2) {
    s.restore(m_d->ui_cuts.checkBox_mdtAllowedADCMin);
    s.restore(m_d->ui_cuts.spinBox_mdtAllowedADCMin);
    s.restore(m_d->ui_cuts.checkBox_mdtAllowedADCMax);
    s.restore(m_d->ui_cuts.spinBox_mdtAllowedADCMax);
    s.restore(m_d->ui_cuts.checkBox_mdtAllowedTDCMin);
    s.restore(m_d->ui_cuts.spinBox_mdtAllowedTDCMin);
    s.restore(m_d->ui_cuts.checkBox_mdtAllowedTDCMax);
    s.restore(m_d->ui_cuts.spinBox_mdtAllowedTDCMax);
    s.restore(m_d->ui_cuts.checkBox_mmAllowedChargeMin);
    s.restore(m_d->ui_cuts.spinBox_mmAllowedChargeMin);
    s.restore(m_d->ui_cuts.checkBox_mmAllowedChargeMax);
    s.restore(m_d->ui_cuts.spinBox_mmAllowedChargeMax);
    s.restore(m_d->ui_cuts.checkBox_mmAllowedTimeMin);
    s.restore(m_d->ui_cuts.spinBox_mmAllowedTimeMin);
    s.restore(m_d->ui_cuts.checkBox_mmAllowedTimeMax);
    s.restore(m_d->ui_cuts.spinBox_mmAllowedTimeMax);
    s.restore(m_d->ui_cuts.checkBox_stgcAllowedChargeMin);
    s.restore(m_d->ui_cuts.spinBox_stgcAllowedChargeMin);
    s.restore(m_d->ui_cuts.checkBox_stgcAllowedChargeMax);
    s.restore(m_d->ui_cuts.spinBox_stgcAllowedChargeMax);
    s.restore(m_d->ui_cuts.checkBox_stgcAllowedTimeMin);
    s.restore(m_d->ui_cuts.spinBox_stgcAllowedTimeMin);
    s.restore(m_d->ui_cuts.checkBox_stgcAllowedTimeMax);
    s.restore(m_d->ui_cuts.spinBox_stgcAllowedTimeMax);
  } else {
    // Old settings must also clear any cuts active in this session.
    m_d->ui_cuts.checkBox_mdtAllowedADCMin->setChecked(false);
    m_d->ui_cuts.checkBox_mdtAllowedADCMax->setChecked(false);
    m_d->ui_cuts.checkBox_mdtAllowedTDCMin->setChecked(false);
    m_d->ui_cuts.checkBox_mdtAllowedTDCMax->setChecked(false);
    m_d->ui_cuts.checkBox_mmAllowedChargeMin->setChecked(false);
    m_d->ui_cuts.checkBox_mmAllowedChargeMax->setChecked(false);
    m_d->ui_cuts.checkBox_mmAllowedTimeMin->setChecked(false);
    m_d->ui_cuts.checkBox_mmAllowedTimeMax->setChecked(false);
    m_d->ui_cuts.checkBox_stgcAllowedChargeMin->setChecked(false);
    m_d->ui_cuts.checkBox_stgcAllowedChargeMax->setChecked(false);
    m_d->ui_cuts.checkBox_stgcAllowedTimeMin->setChecked(false);
    m_d->ui_cuts.checkBox_stgcAllowedTimeMax->setChecked(false);
  }
  if (s.version() >= 3)
    s.restore(m_d->ui_cuts.checkBox_mdtExcludeMasked);
  else
    m_d->ui_cuts.checkBox_mdtExcludeMasked->setChecked(false);

  if (s.version() >= 4) {
    s.restore(m_d->ui_cuts.checkBox_stgcShowPads);
    s.restore(m_d->ui_cuts.checkBox_stgcShowStrips);
    s.restore(m_d->ui_cuts.checkBox_stgcShowWires);
    s.restore(m_d->ui_cuts.checkBox_stgcExcludeDead);
    s.restore(m_d->ui_cuts.checkBox_stgcExcludePileup);
  } else {
    m_d->ui_cuts.checkBox_stgcShowPads->setChecked(true);
    m_d->ui_cuts.checkBox_stgcShowStrips->setChecked(true);
    m_d->ui_cuts.checkBox_stgcShowWires->setChecked(true);
    m_d->ui_cuts.checkBox_stgcExcludeDead->setChecked(false);
    m_d->ui_cuts.checkBox_stgcExcludePileup->setChecked(false);
  }

  // Deserialisation blocks checkbox signals, so synchronize the fields explicitly.
  m_d->ui_cuts.spinBox_mdtAllowedADCMin->setEnabled(m_d->ui_cuts.checkBox_mdtAllowedADCMin->isChecked());
  m_d->ui_cuts.spinBox_mdtAllowedADCMax->setEnabled(m_d->ui_cuts.checkBox_mdtAllowedADCMax->isChecked());
  m_d->ui_cuts.spinBox_mdtAllowedTDCMin->setEnabled(m_d->ui_cuts.checkBox_mdtAllowedTDCMin->isChecked());
  m_d->ui_cuts.spinBox_mdtAllowedTDCMax->setEnabled(m_d->ui_cuts.checkBox_mdtAllowedTDCMax->isChecked());
  m_d->ui_cuts.spinBox_mmAllowedChargeMin->setEnabled(m_d->ui_cuts.checkBox_mmAllowedChargeMin->isChecked());
  m_d->ui_cuts.spinBox_mmAllowedChargeMax->setEnabled(m_d->ui_cuts.checkBox_mmAllowedChargeMax->isChecked());
  m_d->ui_cuts.spinBox_mmAllowedTimeMin->setEnabled(m_d->ui_cuts.checkBox_mmAllowedTimeMin->isChecked());
  m_d->ui_cuts.spinBox_mmAllowedTimeMax->setEnabled(m_d->ui_cuts.checkBox_mmAllowedTimeMax->isChecked());
  m_d->ui_cuts.spinBox_stgcAllowedChargeMin->setEnabled(m_d->ui_cuts.checkBox_stgcAllowedChargeMin->isChecked());
  m_d->ui_cuts.spinBox_stgcAllowedChargeMax->setEnabled(m_d->ui_cuts.checkBox_stgcAllowedChargeMax->isChecked());
  m_d->ui_cuts.spinBox_stgcAllowedTimeMin->setEnabled(m_d->ui_cuts.checkBox_stgcAllowedTimeMin->isChecked());
  m_d->ui_cuts.spinBox_stgcAllowedTimeMax->setEnabled(m_d->ui_cuts.checkBox_stgcAllowedTimeMax->isChecked());

}

//____________________________________________________________________
VP1CollectionWidget * VP1RawDataSysController::collWidget() const
{
  return m_d->collWidget;
}

//____________________________________________________________________
SoGroup * VP1RawDataSysController::drawOptions() const
{
  return m_d->ui_display.widget_drawOptions->drawOptionsGroup();
}

//____________________________________________________________________
bool VP1RawDataSysController::printInfoOnClick() const { return m_d->ui_int.checkBox_printinfo->isChecked(); }
bool VP1RawDataSysController::printVerboseInfoOnClick() const { return m_d->ui_int.checkBox_printinfo_verbose->isChecked(); }
bool VP1RawDataSysController::zoomOnClick() const { return m_d->ui_int.checkBox_zoom->isChecked(); }

//____________________________________________________________________
VP1Interval VP1RawDataSysController::cutAllowedEta() const
{
  return m_d->ui_cuts.etaPhiCutWidget->allowedEta();
}

//____________________________________________________________________
QList<VP1Interval> VP1RawDataSysController::cutAllowedPhi() const
{
  return m_d->ui_cuts.etaPhiCutWidget->allowedPhi();
}

//____________________________________________________________________
bool VP1RawDataSysController::showVolumeOutLines() const
{
  return m_d->ui_display.checkBox_showVolumeOutLines->isChecked();
}

//____________________________________________________________________
bool VP1RawDataSysController::useSpecialTRTHTMaterial() const
{
  return m_d->ui_display.checkBox_trtHTUseSpecialMat->isChecked();
}

//____________________________________________________________________
bool VP1RawDataSysController::useSpecialBCMHighAttMaterial() const
{
  return m_d->ui_display.checkBox_bcmHighAttUseSpecialMat->isChecked();
}

//____________________________________________________________________
VP1RawDataFlags::InDetPartsFlags VP1RawDataSysController::enabledInDetParts() const
{
  VP1RawDataFlags::InDetPartsFlags f;
  if (m_d->ui_cuts.checkBox_cut_indet_barrelA->isChecked())
    f |= VP1RawDataFlags::BarrelPositive;
  if (m_d->ui_cuts.checkBox_cut_indet_barrelC->isChecked())
    f |= VP1RawDataFlags::BarrelNegative;
  if (m_d->ui_cuts.checkBox_cut_indet_endcapA->isChecked())
    f |= VP1RawDataFlags::EndCapPositive;
  if (m_d->ui_cuts.checkBox_cut_indet_endcapC->isChecked())
    f |= VP1RawDataFlags::EndCapNegative;
  return f;
}

//____________________________________________________________________
bool VP1RawDataSysController::trtRequireHT() const
{
  return m_d->ui_cuts.checkbox_cut_trt_needHT->isChecked();
}

//____________________________________________________________________
VP1Interval VP1RawDataSysController::trtAllowedToT() const
{
  VP1Interval i( (m_d->ui_cuts.checkbox_cut_trt_TotMin->isChecked() ? (m_d->ui_cuts.spinBox_cut_trt_ToTMin->value()-0.5)*3.125*Gaudi::Units::nanosecond : -VP1Interval::inf() ),
		 (m_d->ui_cuts.checkbox_cut_trt_TotMax->isChecked() ? (m_d->ui_cuts.spinBox_cut_trt_ToTMax->value()+0.5)*3.125*Gaudi::Units::nanosecond : VP1Interval::inf() ) );
  return i.lower()>i.upper() ? VP1Interval() : i;
}

//____________________________________________________________________
unsigned VP1RawDataSysController::lucidMinQDC() const
{
  return m_d->ui_cuts.spinBox_lucid_qdc_min->value();
}

//____________________________________________________________________
SoMaterial * VP1RawDataSysController::trtHTMaterial() const
{
  return getMaterial(m_d->ui_display.matButton_trtht);
}

//____________________________________________________________________
SoMaterial * VP1RawDataSysController::bcmHighAttMaterial() const
{
  return getMaterial(m_d->ui_display.matButton_bcmhighatt);
}


namespace {
VP1Interval muonDigitRange(const QCheckBox* minimumEnabled, const QDoubleSpinBox* minimum,
                          const QCheckBox* maximumEnabled, const QDoubleSpinBox* maximum)
{
  const double lower = minimumEnabled->isChecked() ? minimum->value() : -VP1Interval::inf();
  const double upper = maximumEnabled->isChecked() ? maximum->value() : VP1Interval::inf();
  return lower > upper ? VP1Interval() : VP1Interval(lower, upper, false, false);
}
}

VP1Interval VP1RawDataSysController::mdtAllowedADC() const
{
  return muonDigitRange(m_d->ui_cuts.checkBox_mdtAllowedADCMin, m_d->ui_cuts.spinBox_mdtAllowedADCMin,
                        m_d->ui_cuts.checkBox_mdtAllowedADCMax, m_d->ui_cuts.spinBox_mdtAllowedADCMax);
}

VP1Interval VP1RawDataSysController::mdtAllowedTDC() const
{
  return muonDigitRange(m_d->ui_cuts.checkBox_mdtAllowedTDCMin, m_d->ui_cuts.spinBox_mdtAllowedTDCMin,
                        m_d->ui_cuts.checkBox_mdtAllowedTDCMax, m_d->ui_cuts.spinBox_mdtAllowedTDCMax);
}

VP1Interval VP1RawDataSysController::mmAllowedCharge() const
{
  return muonDigitRange(m_d->ui_cuts.checkBox_mmAllowedChargeMin, m_d->ui_cuts.spinBox_mmAllowedChargeMin,
                        m_d->ui_cuts.checkBox_mmAllowedChargeMax, m_d->ui_cuts.spinBox_mmAllowedChargeMax);
}

VP1Interval VP1RawDataSysController::mmAllowedTime() const
{
  return muonDigitRange(m_d->ui_cuts.checkBox_mmAllowedTimeMin, m_d->ui_cuts.spinBox_mmAllowedTimeMin,
                        m_d->ui_cuts.checkBox_mmAllowedTimeMax, m_d->ui_cuts.spinBox_mmAllowedTimeMax);
}

VP1Interval VP1RawDataSysController::stgcAllowedCharge() const
{
  return muonDigitRange(m_d->ui_cuts.checkBox_stgcAllowedChargeMin, m_d->ui_cuts.spinBox_stgcAllowedChargeMin,
                        m_d->ui_cuts.checkBox_stgcAllowedChargeMax, m_d->ui_cuts.spinBox_stgcAllowedChargeMax);
}

VP1Interval VP1RawDataSysController::stgcAllowedTime() const
{
  return muonDigitRange(m_d->ui_cuts.checkBox_stgcAllowedTimeMin, m_d->ui_cuts.spinBox_stgcAllowedTimeMin,
                        m_d->ui_cuts.checkBox_stgcAllowedTimeMax, m_d->ui_cuts.spinBox_stgcAllowedTimeMax);
}

///////////////////////////////////////////////////////////////////////////
// Test for possible changes in values and emit signals as appropriate:
// (possibleChange_XXX() slots code provided by macros)
#define VP1CONTROLLERCLASSNAME VP1RawDataSysController
#include "VP1Base/VP1ControllerMacros.h"
POSSIBLECHANGE_IMP(cutAllowedEta)
POSSIBLECHANGE_IMP(cutAllowedPhi)
POSSIBLECHANGE_IMP(showVolumeOutLines)
POSSIBLECHANGE_IMP(useSpecialTRTHTMaterial)
POSSIBLECHANGE_IMP(useSpecialBCMHighAttMaterial)
POSSIBLECHANGE_IMP(enabledInDetParts)
POSSIBLECHANGE_IMP(trtRequireHT)
POSSIBLECHANGE_IMP(trtAllowedToT)
POSSIBLECHANGE_IMP(lucidMinQDC)

POSSIBLECHANGE_IMP(mdtAllowedADC)
POSSIBLECHANGE_IMP(mdtAllowedTDC)
POSSIBLECHANGE_IMP(mmAllowedCharge)
POSSIBLECHANGE_IMP(mmAllowedTime)
POSSIBLECHANGE_IMP(stgcAllowedCharge)
POSSIBLECHANGE_IMP(stgcAllowedTime)

bool VP1RawDataSysController::mdtExcludeMasked() const
{
  return m_d->ui_cuts.checkBox_mdtExcludeMasked->isChecked();
}

POSSIBLECHANGE_IMP(mdtExcludeMasked)

bool VP1RawDataSysController::stgcShowPads() const
{
  return m_d->ui_cuts.checkBox_stgcShowPads->isChecked();
}

POSSIBLECHANGE_IMP(stgcShowPads)

bool VP1RawDataSysController::stgcShowStrips() const
{
  return m_d->ui_cuts.checkBox_stgcShowStrips->isChecked();
}

POSSIBLECHANGE_IMP(stgcShowStrips)

bool VP1RawDataSysController::stgcShowWires() const
{
  return m_d->ui_cuts.checkBox_stgcShowWires->isChecked();
}

POSSIBLECHANGE_IMP(stgcShowWires)

bool VP1RawDataSysController::stgcExcludeDead() const
{
  return m_d->ui_cuts.checkBox_stgcExcludeDead->isChecked();
}

POSSIBLECHANGE_IMP(stgcExcludeDead)

bool VP1RawDataSysController::stgcExcludePileup() const
{
  return m_d->ui_cuts.checkBox_stgcExcludePileup->isChecked();
}

POSSIBLECHANGE_IMP(stgcExcludePileup)
