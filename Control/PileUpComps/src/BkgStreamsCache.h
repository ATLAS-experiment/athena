/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PILEUPTOOLS_BKGSTREAMSCACHE_H
# define PILEUPTOOLS_BKGSTREAMSCACHE_H
/** @file BkgStreamsCache.h
 * @brief In memory cache for pileup events
 *
 * @author Paolo Calafiura - ATLAS Collaboration
 */

#include <string>
#include <vector>
#include <functional>

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"
#include "PileUpTools/PileUpStream.h"
#include "PileUpTools/IBkgStreamsCache.h"

class IEvtSelector;
class IBeamIntensity;
namespace CLHEP {
  class RandFlat;
  class RandPoisson;
}


/** @class BkgStreamsCache
 * @brief In-memory cache for pileup events
 */
class BkgStreamsCache :
  public extends<AthAlgTool, IBkgStreamsCache>
{
public:
  BkgStreamsCache( const std::string&, const std::string&, const IInterface*);
  virtual ~BkgStreamsCache();

  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;
  /**
     @param nXings bunch Xings to be processed
     @param firstStore id of first store in cache
  */
  virtual StatusCode setup(int firstXing,
                           unsigned int nXings,
                           unsigned int firstStore,
                           IBeamIntensity*) override final;
  /// inform cache that we start overlaying a new event
  virtual void newEvent() override final;
  /// reset scale factor at new run/lumiblk
  virtual void resetEvtsPerXingScaleFactor(float sf) override final;
  /**
     @brief Read input events in bkg stores and link them to overlay store
     @param iXing         offset to first xing number (=0 first Xing, =nXings for last xing)
     @param overlaidEvent reference to resulting overlaid event
     @param t0BinCenter   time wrto t0 of current bin center in ns
  */
  virtual StatusCode addSubEvts(unsigned int iXing,
                                xAOD::EventInfo* overlaidEvent,
                                int t0BinCenter) override final;
  /**
     @brief Read input events in bkg stores and link them to overlay store
     @param iXing         offset to first xing number (=0 first Xing, =nXings for last xing)
     @param overlaidEvent reference to resulting overlaid event
     @param t0BinCenter   time wrto t0 of current bin center in ns
     @param loadEventProxies should we load the event proxies or not.
     @param BCID          bunch-crossing ID of signal bunch crossing
  */
  virtual StatusCode addSubEvts(unsigned int iXing,
                                xAOD::EventInfo* overEvent,
                                int t0BinCenter, bool loadEventProxies, unsigned int /*BCID*/) override final;
  /// how many stores in this cache
  virtual unsigned int nStores() const override final { return m_nStores; }

  /// meant to be used (mainly) via m_f_collDistr
  long collXing() { return m_collXing; }
  long collXingPoisson();
  /// meant to be used via m_f_numberOfBackgroundForBunchCrossing
  unsigned int numberOfBkgForBunchCrossingIgnoringBeamIntensity(unsigned int iXing) const;
  unsigned int numberOfBkgForBunchCrossingDefaultImpl(unsigned int iXing) const;
  unsigned int numberOfCavernBkgForBunchCrossing(unsigned int iXing) const;
private:
  /// get next bkg event from cache
  const xAOD::EventInfo* nextEvent(bool isCentralBunchCrossing);
  /// as nextEvent except don't actually load anything
  StatusCode nextEvent_passive(bool isCentralBunchCrossing);
  /// get current (last asked) stream
  PileUpStream* current();

  unsigned int setNEvtsXing(unsigned int iXing);
  unsigned int nEvtsXing(unsigned int iXing) const;

  typedef std::vector<PileUpStream> StreamVector;
  bool alreadyInUse(StreamVector::size_type iStream);
  StreamVector::iterator m_cursor{};
  StreamVector m_streams;
  std::vector<bool> m_usedStreams;
  unsigned int m_nXings{0};
  unsigned int m_nStores{0};
  std::vector<unsigned int> m_nEvtsXing;

  ServiceHandle<IEvtSelector> m_selecName{this, "EventSelector", "FakeEventSelector"};

  ServiceHandle<IAtRndmGenSvc> m_atRndmSvc{this, "RndmGenSvc", "AtRndmGenSvc",
    "IAtRndmGenSvc controlling the distribution of bkg events/xing"};

  Gaudi::Property<float> m_collXing{this, "CollPerXing", 23.0,
    "(average) number of collisions per beam crossing"};

  Gaudi::Property<float> m_occupationFraction{this, "OccupationFraction", 1.0,
    "The maximum fraction of bunch-crossings which will be occupied."};

  Gaudi::Property<std::string> m_collDistrName{this, "CollDistribution", "Poisson",
    "nEvts/Xings can be either Fixed at CollPerXing or Poisson with average CollPerXing"};

  Gaudi::Property<float> m_readDownscale{this, "ReadDownscaleFactor", 150,
    "read one event every downscaleFactor accesses (asymptotically -> number of times "
    "an event in the cache will be reused)"};

  Gaudi::Property<std::string> m_randomStreamName{this, "RndmStreamName", "PileUpCollXingStream",
    "IAtRndmGenSvc stream used as engine for our various random distributions, including the CollPerXing one "};

  Gaudi::CheckedProperty<unsigned short> m_pileUpEventTypeProp{this, "PileUpEventType", 0,
    &BkgStreamsCache::PileUpEventTypeHandler,
    "Type of the pileup events in this cache: 0:Signal, 1:MinimumBias, 2:Cavern, 3:HaloGas, "
    "4:ZeroBias. Default=0 (Signal, Invalid)"};
  void PileUpEventTypeHandler(Gaudi::Details::PropertyBase&);

  Gaudi::Property<unsigned short> m_subtractBC0{this, "SubtractBC0", 0,
    "reduce the number of events at bunch xing t=0 by m_subtractBC0. Default=0, set to 1 when "
    "using the same type of events (e.g. minbias) for original and background streams"};

  Gaudi::Property<bool> m_ignoreBM{this, "IgnoreBeamInt", false,
    "Default=False, set to True to ignore the PileUpEventLoopMgr beam intensity "
    "tool in setting the number of events per xing."};

  Gaudi::Property<bool> m_ignoreSF{this, "IgnoreBeamLumi", false,
    "Default=False, set to True to ignore the PileUpEventLoopMgr beam luminosity "
    "tool in setting the number of events per xing."};

  Gaudi::Property<bool> m_forceReadForBC0{this, "ForceReadForBC0", true,
    "Force events used in the central bunch crossing to be refreshed"};

  /// the type of events in this cache
  xAOD::EventInfo::PileUpType m_pileUpEventType;
  /// read a new event every downscaleFactor accesses
  CLHEP::RandFlat* m_readEventRand{nullptr};
  /// pickup an event store at random from the cache
  CLHEP::RandFlat* m_chooseEventRand{nullptr};
  /// set number of collisions per bunch crossing (if Poisson distribution chosen)
  CLHEP::RandPoisson* m_collXingPoisson{nullptr};
  /// function returning the number of collisions per bunch crossing
  /// before bunch structure modulation
  std::function< long() > m_f_collDistr;
  /// function returning the number of bkg events per bunch crossing
  /// after bunch structure modulation
  std::function< unsigned int(unsigned int) > m_f_numberOfBackgroundForBunchCrossing;
  /// float scaling number of collisions per bunch crossing
  float m_collXingSF{1.0};
  /// offset of BC=0 xing
  int m_zeroXing{-1};
  /// pointer to the IBeamIntensity distribution tool
  IBeamIntensity* m_beamInt{nullptr};

};

#endif // PILEUPTOOLS_BKGSTREAMSCACHE_H
