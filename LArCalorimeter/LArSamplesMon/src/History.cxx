/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArSamplesMon/History.h"

#include "LArCafJobs/CellInfo.h"
#include "LArCafJobs/ShapeInfo.h"

#include "LArSamplesMon/AbsShapeErrorGetter.h"
#include "LArSamplesMon/Data.h"
#include "LArCafJobs/SimpleShape.h"
#include "LArSamplesMon/GraphShape.h"
#include "LArSamplesMon/ShapeDrawer.h"
#include "LArSamplesMon/ShapeFitter.h"
#include "LArSamplesMon/DataTweaker.h"
#include "LArSamplesMon/OFC.h"
#include "LArCafJobs/Definitions.h"
#include "LArSamplesMon/ScaledShiftedShape.h"
#include "LArSamplesMon/Averager.h"
#include "LArSamplesMon/ShapeErrorData.h"
#include "LArSamplesMon/ScaledErrorData.h"
#include "LArSamplesMon/UniformShapeErrorGetter.h"
#include "LArSamplesMon/CombinedShapeErrorGetter.h"
#include "LArSamplesMon/Residual.h"
#include "LArCafJobs/Geometry.h"
#include "LArSamplesMon/Interface.h"
#include "LArSamplesMon/ClassCounts.h"
#include "LArCafJobs/HistoryContainer.h"

#include "TString.h"
#include "TMath.h"
#include <iostream>
#include <memory>

using std::cout;
using std::endl;

using namespace LArSamples;


History::History(const HistoryContainer& container,
                 std::vector<std::unique_ptr<const EventData> >&& eventData, unsigned hash,
                 const AbsShapeErrorGetter* shapeErrorGetter)
  : m_cellInfo(*container.cellInfo()),
    m_eventData(std::move(eventData)),
    m_hash(hash), m_shapeErrorGetter(shapeErrorGetter)
{
  ClassCounts::incrementInstanceCount("History");
  if (container.nDataContainers() != m_eventData.size()) return;
  for (unsigned int i = 0; i < container.nDataContainers(); i++) 
    m_data.push_back(std::make_unique<Data>(*container.dataContainer(i), *eventData[i], this, i));
}


History::History(std::vector<std::unique_ptr<const Data> >&& data,
                 const CellInfo& info,
		 std::vector<std::unique_ptr<const EventData> >&& eventData,
		 unsigned int hash, const AbsShapeErrorGetter* shapeErrorGetter)
  : m_data(std::move(data)), m_cellInfo(info), m_eventData(std::move(eventData)), m_hash(hash),
    m_shapeErrorGetter(shapeErrorGetter)
{
  ClassCounts::incrementInstanceCount("History");
  unsigned int i = 0;
  for (std::unique_ptr<const Data>& pdata : m_data)
    pdata->setCallBacks(this, i);
}
      

History::~History()
{
  ClassCounts::decrementInstanceCount("History");
}


std::unique_ptr<HistoryContainer> History::dissolve()
{
  auto histCont = std::make_unique<HistoryContainer>(new CellInfo(*cellInfo()));
  for (unsigned int k = 0; k < nData(); k++) {
    Data* newData = new Data(*data(k));
    histCont->add(newData->dissolve());
  }
  delete this;
  return histCont;
}


const Data* History::data(unsigned int i) const 
{ 
  if (i >= nData()) return nullptr;
  return m_data[i].get();
}


const Data* History::data_for_event(int event, int run) const
{
  for (unsigned int k = 0; k < nData(); k++) {
    if (data(k)->event() != event) continue;
    if (run > 0 && data(k)->run() != run) continue;
    return data(k);
  }
  return nullptr;
}


const Data* History::data_for_event(const EventData& eventData) const
{
  // turn off caching

  //if (m_dataForEvent.size() == 0) 
  //  for (unsigned int k = 0; k < nData(); k++)
  //    m_dataForEvent[data(k)->event()] = std::make_pair(data(k)->run(), data(k));

  //std::unordered_map<int, std::pair<int, const Data*> >::const_iterator
  //  data = m_dataForEvent.find(eventData.event());
  //if (data == m_dataForEvent.end()) return 0;
  //if (data->second.first != eventData.run()) return data_for_event(eventData.event(), eventData.run()); // wrong run: revert to pedestrian method...
  //return data->second.second;

  return data_for_event(eventData.event(), eventData.run());
}


bool History::sum(std::unique_ptr<SimpleShape>& sum,
                  std::unique_ptr<SimpleShape>& reference) const
{
  reference.reset();
  sum.reset();
  if (nData() == 0) return false;
  
  sum = std::make_unique<SimpleShape>(m_data[0]->nSamples());

  for (unsigned int j = 0; j < nData(); j++) {
    if (sum->nPoints() !=  m_data[j]->nSamples()) return false;
    for (unsigned int k = 0; k < m_data[j]->nSamples(); k++)
      sum->add(k, m_data[j]->pedestalSubtractedSample(k), m_data[j]->error(k));
    std::unique_ptr<SimpleShape> thisRef = referenceShape(j);
    if (!thisRef) return false;
    if (reference) {
      reference->add(*thisRef);
    }
    else reference = std::move(thisRef);
  }
    
  return true;
}


bool History::isValid() const
{
  if (!m_cellInfo.isValid()) return false;
  if (nData() == 0) return false;
  
  for (const std::unique_ptr<const Data>& data : m_data)
    if (!data->isValid()) return false;
  
  return true;
}


double History::chi2(int i, int lwb, int upb, int chi2Params, ShapeErrorType shapeErrorType, unsigned int* nDof) const
{
  std::unique_ptr<SimpleShape> reference = referenceShape(i);
  if (!reference) return -1;
  Chi2Calc c2c(chi2Params);
  if ( m_data[i]->isDisconnected()) return -1;
  std::unique_ptr<const ScaledErrorData> sea = scaledErrorData(i, -1, Definitions::none, shapeErrorType);
  if (!sea && shapeErrorType != NoShapeError && shapeErrorType != BestShapeError) return -1;
  double chi2Value = c2c.chi2(*m_data[i], *reference, sea.get(), lwb, upb);
  if (nDof) *nDof = c2c.nDof();
  return chi2Value;
}


double History::chi2_k(int i, double k, int lwb, int upb, int chi2Params) const
{
  const AbsShapeErrorGetter* oldGetter = shapeErrorGetter();
  UniformShapeErrorGetter uniGetter(k);
  CombinedShapeErrorGetter combGetter;
  if (oldGetter) combGetter.add(*oldGetter);
  combGetter.add(uniGetter);
  setShapeErrorGetter(&combGetter);
  double chi2Value = chi2(i, lwb, upb, chi2Params);
  setShapeErrorGetter(oldGetter);
  return chi2Value;
}


double History::maxChi2(int lwb, int upb, int chi2Params) const
{
  double maxChi2 = -2;  
  for (unsigned int i = 0; i < nData(); i++) {
    double chi2Value = chi2(i, lwb, upb, chi2Params);
    if (chi2Value > maxChi2) maxChi2 = chi2Value;
  }
  return maxChi2;
}


std::unique_ptr<OFC>
History::ofc(unsigned int k, int lwb, int upb, double time, bool withAutoCorr) const
{
  if (k >= nData()) return nullptr;
  if (Definitions::isNone(time)) {
    time = data(k)->ofcTime();
    //cout << "Using reference time = " << time << endl;
  }
  std::unique_ptr<SimpleShape> reference = referenceShape(k, 1, time); // ADC=1 : we need a normalized shape
  if (!reference) return nullptr;
  std::unique_ptr<const ShapeErrorData> sed = shapeErrorData(CaloGain::LARHIGHGAIN);
  auto result = std::make_unique<OFC>(*reference, *m_data[k], lwb, upb, sed.get(), withAutoCorr); // FixMe
  if (result->g().GetNrows() == 0) result.reset();
  return result;
}


bool History::refVal(unsigned int k, unsigned int sample, double& val, double& err) const
{
  if (k >= nData()) return false;
  std::unique_ptr<SimpleShape> reference = referenceShape(k);
  if (!reference) return false;  
  if (reference->interpolate(m_data[k]->time(sample), val, err) != 0) return false;
  return true;
}


std::unique_ptr<History> History::refit(Chi2Params pars) const
{
  std::vector<std::unique_ptr<const Data> > datas;
  DataTweaker tw;
  tw.setRefit(true);
  tw.setFitParams(pars);

  for (unsigned int j = 0; j < nData(); j++) {
    std::unique_ptr<const Data> refitData (tw.tweak(*data(j)));
    if (!refitData) {
      return nullptr;
    }
    datas.push_back(std::move(refitData));
  }

  std::vector<std::unique_ptr<const EventData> > eventData;
  for (const std::unique_ptr<const EventData>& event : m_eventData)
    eventData.push_back(std::make_unique<EventData>(*event));

  return std::make_unique<History>(std::move(datas), *cellInfo(), std::move(eventData), hash(), shapeErrorGetter());
}


std::unique_ptr<History> History::adjust() const
{
  std::vector<std::unique_ptr<const Data> > datas;
  DataTweaker tw;
  tw.setAdjust(true);

  for (unsigned int j = 0; j < nData(); j++) {
    std::unique_ptr<const Data> newData (tw.tweak(*data(j)));
    if (!newData) {
      return nullptr;
    }
    datas.push_back(std::move(newData));
  }

  std::vector<std::unique_ptr<const EventData> > eventData;
  for (const std::unique_ptr<const EventData>& event : m_eventData)
    eventData.push_back(std::make_unique<EventData>(*event));

  return std::make_unique<History>(std::move(datas), *cellInfo(), std::move(eventData), hash(), shapeErrorGetter());
}


std::unique_ptr<History> History::filter(const TString& cuts) const
{
  FilterParams f;
  if (!f.set(cuts)) return nullptr;
  
  std::vector<std::unique_ptr<const Data> > datas;

  for (unsigned int j = 0; j < nData(); j++) {
    if (!f.pass(hash(), *this, j)) continue;
    datas.push_back(std::make_unique<Data>(*data(j)));
  }

  std::vector<std::unique_ptr<const EventData> > eventData;
  for (const std::unique_ptr<const EventData>& event : m_eventData)
    eventData.push_back(std::make_unique<EventData>(*event));

  return std::make_unique<History>(std::move(datas), *cellInfo(), std::move(eventData), hash(), shapeErrorGetter());
}


std::unique_ptr<const ShapeErrorData>
History::shapeErrorData(CaloGain::CaloGain gain, ShapeErrorType shapeErrorType, const Residual* res) const
{
  if (shapeErrorType == NoShapeError || !shapeErrorGetter()) return nullptr;
  if (shapeErrorType == BestShapeError) {
    for (unsigned int i = 0; i < NShapeErrorTypes; i++) {
      std::unique_ptr<const ShapeErrorData> sed = shapeErrorData(gain, (ShapeErrorType)i, res);
      if (sed) return sed;
    }
    return nullptr;
  }
  
  if (shapeErrorType == CellShapeError) {
    std::unique_ptr<ShapeErrorData> sed = shapeErrorGetter()->shapeErrorData(hash(), gain, res);
    if (!sed) return nullptr;
    sed->setShapeErrorType(CellShapeError);
    return sed;
  }
  
  if (shapeErrorType == LowGainCellShapeError || shapeErrorType == MedGainCellShapeError || shapeErrorType == HighGainCellShapeError) {
    CaloGain::CaloGain fbGain = (shapeErrorType == LowGainCellShapeError ? CaloGain::LARLOWGAIN :
                                 (shapeErrorType == MedGainCellShapeError ? CaloGain::LARMEDIUMGAIN :CaloGain::LARHIGHGAIN));
    std::unique_ptr<ShapeErrorData> sed = shapeErrorGetter()->shapeErrorData(hash(), fbGain, res);
    if (!sed) return nullptr;
    sed->setShapeErrorType(shapeErrorType);
    return sed;
  }
  
  if (shapeErrorType == RingShapeError) {
    std::unique_ptr<ShapeErrorData> sed = shapeErrorGetter()->phiSymShapeErrorData(cellInfo()->globalPhiRing(), gain, res);
    if (!sed) return nullptr;
    sed->setShapeErrorType(RingShapeError);
    return sed;
  }

  if (shapeErrorType == LowGainRingShapeError || shapeErrorType == MedGainRingShapeError || shapeErrorType == HighGainRingShapeError) {
    CaloGain::CaloGain fbGain = (shapeErrorType == LowGainRingShapeError ? CaloGain::LARLOWGAIN :
                                 (shapeErrorType == MedGainRingShapeError ? CaloGain::LARMEDIUMGAIN :CaloGain::LARHIGHGAIN));
    std::unique_ptr<ShapeErrorData> sed = shapeErrorGetter()->phiSymShapeErrorData(cellInfo()->globalPhiRing(), fbGain, res);
    if (!sed) return nullptr;
    sed->setShapeErrorType(shapeErrorType);
    return sed;
  }
 
  return nullptr;
}


std::unique_ptr<const ScaledErrorData>
History::scaledErrorData(unsigned int k, double adcMax, double time,
                         ShapeErrorType shapeErrorType) const
{
  if (shapeErrorType == NoShapeError || !shapeErrorGetter()) return nullptr;
  if (k >= nData() || data(k)->adcMax() <= 0) return nullptr;

  std::unique_ptr<Residual> res = residual(k, false);
  std::unique_ptr<const ShapeErrorData> sed = shapeErrorData(data(k)->gain(), shapeErrorType, res.get());
  if (!sed) return nullptr;

  double sf = (adcMax < 0 ? data(k)->adcMax() : adcMax);
  double ts = (Definitions::isNone(time) ? data(k)->ofcTime() : time);  
  return std::make_unique<ScaledErrorData>(*sed, sf, ts);
}


bool History::delta(unsigned int k, unsigned int sample, double& del) const
{
  if (k >= nData()) return 0;
  TVectorD dv = deltas(k, sample, sample);
  if (dv.GetNrows() == 0) return false;
  del = dv[sample];
  return true;
}


TVectorD History::deltas(unsigned int k, int lwb, int upb, bool correct) const
{
  if (k >= nData()) return TVectorD();
  std::unique_ptr<SimpleShape> reference = referenceShape(k);
  if (!reference) return TVectorD();  
  Chi2Calc c2c;
  CovMatrix errors;
  std::unique_ptr<const ScaledErrorData> sea = (correct ? scaledErrorData(k) : nullptr);
  TVectorD dv = c2c.deltas(*data(k), *reference, errors, sea.get(), lwb, upb);
  return dv;
}


bool History::residualOffset(unsigned int k, short sample, double& offset, double adcMax, double time) const
{
  std::unique_ptr<const ScaledErrorData> sea = scaledErrorData(k, adcMax, time);
  if (!sea) return false;
  if (!sea->isInRange(sample)) return false;
  offset = sea->offsets()(sample);
  return true;
}


bool History::residualError(unsigned int k, short sample1, short sample2, double& error, double adcMax, double time) const
{
  std::unique_ptr<const ScaledErrorData> sea = scaledErrorData(k, adcMax, time);
  if (!sea) return false;
  if (!sea->isInRange(sample1) || !sea->isInRange(sample2)) return false;
  error = sea->errors()(sample1, sample2);
  return true;
}

      
bool History::allShape(std::unique_ptr<GraphShape>& allData,
                       std::unique_ptr<SimpleShape>& allRef) const
{
  allData.reset();
  allRef.reset();
  
  for (unsigned int j = 0; j < nData(); j++) {
    if ( m_data[j]->isDisconnected()) continue;
    if (m_data[j]->adcMax() < 1) continue;
    auto thisData = std::make_unique<GraphShape>(*m_data[j], 1/m_data[j]->adcMax(), -m_data[j]->ofcTime());
    if (allData) {
      allData->add(*thisData);
    }
    else allData = std::move(thisData);
    std::unique_ptr<SimpleShape> thisRef = referenceShape(j, 1); // normalized to 1, like the data
    if (allRef) {
      allRef->add(*thisRef);
    }
    else allRef = std::move(thisRef);
  }

  return true;
}


double History::allChi2(Chi2Params pars) const
{
  Chi2Calc c2c(pars);
  
  std::unique_ptr<GraphShape> allData;
  std::unique_ptr<SimpleShape> allRef;
  if (!allShape(allData, allRef)) return -1;
  double chi2Value = c2c.chi2(*allData, *allRef);
  return chi2Value;
}


bool History::drawWithReference(int k, const TString& atlasTitle) const
{
  if ((unsigned int)k >= nData()) return false;
  std::unique_ptr<SimpleShape> refShape = referenceShape(k);
  std::unique_ptr<SimpleShape> smpShape = referenceShape(k, -1, Definitions::none, true);
  
  if (!refShape || !smpShape) return false;
  int pars = DataFirst | Legend;
  TString title = "";
  if (atlasTitle != "") { // Make sure AtlasStyle is set in ROOT, otherwise will not be as pretty...
    pars |= AtlasStyle;
    title = atlasTitle;
  }
  else 
    title = Form("%s, run %d, event %d", cellInfo()->location(1).Data(), m_data[k]->run(), m_data[k]->event());
  ShapeDrawer drawer(pars);
  bool result = drawer.draw(title, m_data[k].get(), refShape.get(), smpShape.get());
  return result;
}


bool History::drawSumWithReference() const
{
  std::unique_ptr<SimpleShape> dataShape;
  std::unique_ptr<SimpleShape> refShape;
  if (!sum(dataShape, refShape)) return false;
    
  ShapeDrawer drawer(DataFirst | Legend);
  return drawer.drawAndDelete("", std::move(dataShape), std::move(refShape));
}


bool History::drawAllWithReference(bool doRefit) const
{  
  if (!nData()) {
    cout << "No data" << endl;
    return false;
  }
  
  if (doRefit) {
    std::unique_ptr<History> refitted_history = refit(DefaultChi2);
    if (!refitted_history) return false;
    bool result = refitted_history->drawAllWithReference(false);
    return result;
  }
  
  // Use the shape of the gain of the first pulse...
  std::unique_ptr<SimpleShape> refShape = referenceShape(0, 1000, 0);
  std::unique_ptr<SimpleShape> smpShape = referenceShape(0, 1000, 0, true);
  std::vector<std::unique_ptr<const AbsShape> > shapes;
  
  for (unsigned int i = 0; i < nData(); i++) {
    if (m_data[i]->adcMax() < 1) continue;
    auto shape = std::make_unique<SimpleShape>(*m_data[i], 1000/m_data[i]->adcMax(), -m_data[i]->ofcTime());
    std::unique_ptr<const ScaledErrorData> sed = scaledErrorData(i, 1000);
    if (sed) {
      for (unsigned int k = 0; k < shape->nPoints(); k++)
        if (sed->isInRange(k))
          shape->set(k, shape->value(k) - sed->offsets()(k));
    }
    shapes.push_back(std::move(shape));
  }
  
  ShapeDrawer drawer(Legend);
  return drawer.drawAndDelete(Form("%s (Normalized Shape, max at 1000)", cellInfo()->location(2).Data()), std::move(shapes), std::move(refShape), std::move(smpShape));
}


bool History::drawResiduals(int k, bool errors, bool rescale) const
{  
  std::vector<std::unique_ptr<const AbsShape> > shapes;
  for (unsigned int i = 0; i < nData(); i++) {
    if (m_data[i]->adcMax() < 1) continue;
    if (k >= 0 && k != (int)i) continue;
    std::unique_ptr<SimpleShape> shape = deltaShape(i);
    if (!shape) continue;
    if (!errors) 
      for (unsigned int idx = 0; idx < shape->nPoints(); idx++) shape->setError(idx, 0);
    if (rescale) {
      auto scaled = std::make_unique<SimpleShape>(*shape, 1/m_data[i]->adcMax(), 9.0*i/nData() - m_data[i]->ofcTime());
      shape = std::move(scaled);
    }
    shapes.push_back(std::move(shape));
  }
  ShapeDrawer drawer(DataFirst);
  TString title = (rescale ? "Normalized " : "") + TString("residuals for %s");
  return drawer.drawAndDelete(Form(title.Data(), cellInfo()->location(2).Data()), std::move(shapes));
}


std::unique_ptr<SimpleShape> History::referenceShape(unsigned int k, double adcMax, double time,
                                                     bool samplesOnly) const
{
  if (!cellInfo()->shape(m_data[k]->gain())) return nullptr;
  if (adcMax < 0) adcMax = m_data[k]->adcMax();
  if (Definitions::isNone(time)) time = m_data[k]->ofcTime();
  return std::make_unique<SimpleShape>(*cellInfo()->shape(m_data[k]->gain()), adcMax, time, samplesOnly);
}


std::unique_ptr<SimpleShape> History::deltaShape(unsigned int k, int lwb, int upb) const
{
  if (k >= nData()) return nullptr;
  std::unique_ptr<SimpleShape> reference = referenceShape(k);
  if (!reference) return nullptr;  
  Chi2Calc c2c;
  CovMatrix errors;
  std::unique_ptr<const ScaledErrorData> sea = scaledErrorData(k);
  TVectorD dv = c2c.deltas(*data(k), *reference, errors, sea.get(), lwb, upb);
  if (dv.GetNrows() == 0) return nullptr;
  std::unique_ptr<SimpleShape> shape = std::make_unique<SimpleShape>(dv.GetNrows(), Definitions::samplingInterval, data(k)->time(c2c.lwb()) + data(k)->ofcTime());
  for (int l = c2c.lwb(); l <= c2c.upb(); l++) shape->set(l - c2c.lwb(), dv(l), TMath::Sqrt(errors(l, l)));
  return shape;
}


TString History::description(unsigned int verbosity) const
{
  TString desc = "";
  for (unsigned int i = 0; i < nData(); i++)
    desc += Form("  #%-2d : ", i) + data(i)->description(verbosity) + "\n";
  if (desc == "") return desc;
  return cellInfo()->location(2) + "\n" + desc;
}


std::unique_ptr<Averager> History::calculatePedestal(int i) const
{
  auto avg = std::make_unique<Averager>(1);
  for (unsigned int k = 0; k < nData(); k++) {
    if (i >= 0 && i != (int)k) continue;
    std::unique_ptr<SimpleShape> reference = referenceShape(k);
    if (!reference) continue;  
    double v,e;
    for (unsigned int l = 0; l < data(k)->nPoints(); l++)
      if (reference->interpolate(data(k)->time(l), v, e) == -1) { // we're before the ref shapeError
        TVectorD ped(1);
        ped(0) = data(k)->value(l);
        avg->fill(ped);
      }
  }
  return avg;
}


std::unique_ptr<Residual>
History::residual(unsigned int k, bool correct, bool zeroTime) const
{
  if (k >= nData()) return nullptr;
  TVectorD del = deltas(k, -1, -1, correct);
  return std::make_unique<Residual>(del, data(k)->run(), data(k)->event(), data(k)->adcMax(),
                                    (zeroTime ? 0 : data(k)->ofcTime() /*- Definitions::samplingTime(del.GetLwb()) */));
}


std::unique_ptr<Residuals>
History::residuals(CaloGain::CaloGain gain, double absResCut, bool correct, bool zeroTime) const
{
  Chi2Calc c2c;
  CovMatrix dummyErrors;
  auto residuals = std::make_unique<Residuals>();
  for (unsigned int k = 0; k < nData(); k++) {
    if (gain != CaloGain::LARNGAIN && data(k)->gain() != gain) continue;
    auto res = std::unique_ptr<Residual>(residual(k, correct, zeroTime));
    if (!res) { cout << "Error calculating residual for hash = " << m_hash << ", index = " << k << endl; return nullptr; }
    if (residuals->size() > 0 && !residuals->hasSameRange(*res)) {
      cout << "Warning for hash = " << m_hash << ", index = " << k << " : index interval changed from [" 
           << residuals->lwb() << ", " << residuals->upb() << "] to " << res->rangeStr() << endl;      
      return nullptr;
    }
    bool pass = true;
    for (int i = res->lwb(); i < res->upb(); i++) {
      if (absResCut > 0 && TMath::Abs(res->scaledDelta(i)) > absResCut) {
        pass = false;
        break;
      }
    }
    if (pass) residuals->add(*res);
  }
  return residuals;
}


double History::upstreamEnergy(unsigned int k) const
{
  if (!m_interface || !cellInfo()) return -1;
  if (k >= nData()) return -1;
  std::vector<unsigned int> upstreamNeighbors;
  if (!m_interface->firstNeighbors(hash(), upstreamNeighbors, cellInfo()->layer() - 1)) return -1;
  if (upstreamNeighbors.empty()) return -1;
  std::vector<std::unique_ptr<const Data> > unData;
  if (!m_interface->data(upstreamNeighbors, data(k)->eventData(), unData)) return -1;
  double upstreamE = 0;
  for (std::unique_ptr<const Data>& data : unData) {
    upstreamE += data->energy();
  }
  return upstreamE;
}


double History::chi2Anomaly(double chi2Cut, unsigned int nDof) const
{
  if (nData() == 0 || chi2Cut <= 0) return -1;
  double nBadChi2 = 0;
  for (unsigned int k = 0; k < nData(); k++) if (chi2(k) > chi2Cut) nBadChi2++;
  double effBad = nBadChi2/nData();
  double effRef = TMath::Prob(chi2Cut, nDof);
  double dEff = sqrt(effRef*(1 - effRef))/sqrt(nData());
  if (dEff == 0) dEff = 1/sqrt(nData());
  return (effBad - effRef)/dEff;
}
