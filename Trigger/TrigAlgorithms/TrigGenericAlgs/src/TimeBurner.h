/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGENERICALGS_TimeBurner_h
#define TRIGGENERICALGS_TimeBurner_h

#include "AthenaKernel/IAthRNGSvc.h"
#include "DecisionHandling/HypoBase.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/ICPUCrunchSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include <atomic>
#include <string>

/**
 *  @class TimeBurner
 *  @brief Hypo algorithm that burns/sleeps for some time per event and
 *         optionally accepts events with a configurable probability.
 *
 *  Per-event duration:
 *    - Selected by @p TimeDistribution:
 *        - "fixed":  use the constant @p SleepTimeMillisec.
 *        - "landau": sample from a Landau distribution with parameters
 *                    @p LandauMPV and @p LandauSigma (in ms), clipped at 0.
 *    - If @p MaxTimeMs > 0 (default 450000), the duration is capped at
 *      that value to avoid HLT timeouts.
 *
 *  How time is consumed:
 *    - If @p BurnCPU is true, busy-wait via Gaudi::CPUCrunchSvc (consumes CPU).
 *    - Otherwise, std::this_thread::sleep_for (CPU idle).
 *
 *  Per-event accept decision:
 *    - Event is accepted with probability @p AcceptFraction (default 0,
 *      i.e. reject all).
 **/
class TimeBurner : public HypoBase {
  public:
    /// Standard constructor
    TimeBurner(const std::string& name, ISvcLocator* svcLoc);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& eventContext) const override;
    virtual StatusCode finalize() override;

  private:
    // Internal enum mapped from the TimeDistribution string property.
    enum class TimeDist { Fixed, Landau };
    TimeDist m_timeDist{TimeDist::Fixed};

    Gaudi::Property<std::string> m_timeDistribution {
      this, "TimeDistribution", "fixed",
      "Distribution used to sample the per-event time. Allowed values: "
      "\"fixed\" (use SleepTimeMillisec), \"landau\" (use LandauMPV/LandauSigma)."
    };

    Gaudi::Property<unsigned int> m_sleepTimeMillisec {
      this, "SleepTimeMillisec", 0,
      "Fixed per-event time [ms]. Used when TimeDistribution == \"fixed\"."
    };

    Gaudi::Property<double> m_landauMPV{
      this, "LandauMPV", 105.0,
      "MPV of the Landau distribution [ms]. "
      "Used when TimeDistribution == \"landau\"."
    };

    Gaudi::Property<double> m_landauSigma{
      this, "LandauSigma", 66.0,
      "Sigma of the Landau distribution [ms]. "
      "Used when TimeDistribution == \"landau\"."
    };

    Gaudi::Property<double> m_acceptFraction{
      this, "AcceptFraction", 0.0,
      "Probability in [0,1] of accepting an event. Default 0 = reject all."
    };

    Gaudi::Property<bool> m_burnCPU{
      this, "BurnCPU", false,
      "If true, busy-wait (via Gaudi::CPUCrunchSvc) instead of sleeping."
    };

    Gaudi::Property<double> m_maxTimeMs{
      this, "MaxTimeMs", 450000.0,
      "Upper bound on the per-event time [ms]. Set to <= 0 to disable the cap (e.g. to deliberately generate HLT timeouts)."
    };

    // Gaudi service that does a calibrated CPU-burning loop.
    ServiceHandle<ICPUCrunchSvc> m_cpuCrunchSvc{
      this, "CPUCrunchSvc", "CPUCrunchSvc",
      "Handle to Gaudi CPUCrunchSvc (used when BurnCPU is true)."
    };

    // Slot-local random engines (thread-safe by construction).
    ServiceHandle<IAthRNGSvc> m_rngSvc{
      this, "AthRNGSvc", "AthRNGSvc",
      "Handle to the slot-local random number service."
    };

    // Unused dummy property to pass duck-test of HypoAlgs
    ToolHandleArray<IAlgTool> m_hypoTools{this, "HypoTools", {}};

    mutable std::atomic<unsigned long> m_nSeen{0};
    mutable std::atomic<unsigned long> m_nAccepted{0};
    mutable std::atomic<unsigned long> m_nRejected{0};
};

#endif // TRIGGENERICALGS_TimeBurner_h
