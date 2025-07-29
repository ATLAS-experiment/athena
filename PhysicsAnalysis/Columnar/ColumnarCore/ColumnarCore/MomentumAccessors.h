/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_MOMENTUM_ACCESSORS_H
#define COLUMNAR_CORE_MOMENTUM_ACCESSORS_H

#include <ColumnarCore/ColumnAccessor.h>
#include <TruthUtils/ParticleConstants.h>
#include <Math/Vector4D.h>

namespace columnar
{
  /// @file a generic interface for accessing the momentum of columnar
  /// objects
  ///
  /// There are a couple of challenges with accessing the momentum of
  /// columnar objects:
  /// * the way the momentum is stored will depend on the type of object,
  ///   e.g. for jets it is stored as pt, eta, phi, m; while for muons the
  ///   mass is hard-coded; and for truth particles it is stored as px, py,
  ///   pz, e
  /// * there are various derived quantities that are derived from the
  ///   primary variables from the file, and the calculation will change
  ///   depending on what the primary variables are
  /// * we don't have an ability to attach virtual methods to the columnar
  ///   objects, as we can for regular xAOD objects, all this functionality
  ///   has to be implemented externally in an accessor class
  /// * there are often multiple different momentum definitions for the
  ///   same object, e.g. for jets you can access both the regular
  ///   momentum and the "dressed" momentum. in xAOD code there is only
  ///   the primary momentum definition and the rest has to be done with
  ///   individual accessors, but since we anyways have to do external
  ///   accessors it would be useful to treat them in a uniform way.
  /// * the implementation of the accessors will still have to be
  ///   specialized for the different container IDs columnar modes, as
  ///   that's the way columnar accessors work
  ///
  /// In general momentum accessors aren't expected to be used that much
  /// in columnar tools, as the "stereotypical" columnar tool is
  /// something like a pt-eta lookup tool which can take its input
  /// variables directly from PHYSLITE, without having to calculate any
  /// other momentum variables based on them. However some tools do need
  /// full momentum support, e.g. MET and some of the calibration tools.
  /// Should it become more widely used, we may consider cleaning up the
  /// underlying implementation.
  ///
  /// The general design is that there is a `MomentumAccessors` class
  /// that can interface with the different underlying momentum
  /// definitions. I had also played around with some versions in which
  /// the user could also instantiate momentum accessors that were
  /// statically configured to a specific type of particle, but it
  /// didn't seem to be worth the effort. It added quite a bit of
  /// complexity, it was a bit awkward to select the right
  /// implementation class, and for my test cases it didn't seem to do a
  /// lot for performance (i.e. a few ns/call).
  ///
  /// The underlying implementation of `MomentumAccessors` is a
  /// type-erased implementation that can be reset to different particle
  /// types at configuration time. This seems to work well/fast in my
  /// tests, makes it easy to add extra momentum representations, and
  /// keeps the implementation relatively clean. It also makes it easier
  /// to check how an accessor without a virtual redirect would perform.
  ///
  /// Note that in columnar mode an alternate approach would be to
  /// precalculate all the relevant momentum variables and feed them in
  /// as extra input columns. However, in xAOD mode that is not
  /// practical, and we have no infrastructure to support such split
  /// command paths.
  ///
  /// There is a fair chance that this accessor will request more
  /// columns than it actually uses. That would be very hard to avoid in
  /// the current ecosystem, and there is a fair chance the user (or
  /// other tools) will request those columns anyway. So I don't try to
  /// optimize that away.

  namespace Detail
  {
    /// columnar momentum accessors that redirect to the @ref
    /// xAOD::IParticle interface internally
    ///
    /// This is mostly to allow a fallback version of columnar accessors
    /// that need to be configured at runtime, but haven't been
    /// configured (yet). Should this ever be used in actual columnar
    /// code, it will throw an exception when called.

    template<ContainerId CIVal, typename CMVal = ColumnarModeDefault>
    class MomentumAccessorsIParticle
    {
    public:
      static constexpr ContainerId CI = CIVal;
      using CM = CMVal;

      double pt (ObjectId<CI,CM> object) const {
        return object.getXAODObject().pt();}
      double eta (ObjectId<CI,CM> object) const {
        return object.getXAODObject().eta();}
      double phi (ObjectId<CI,CM> object) const {
        return object.getXAODObject().phi();}
      double m (ObjectId<CI,CM> object) const {
        return object.getXAODObject().m();}
      double e (ObjectId<CI,CM> object) const {
        return object.getXAODObject().e();}
      double rapidity (ObjectId<CI,CM> object) const {
        return object.getXAODObject().rapidity();}
    };

    /// generic columnar momentum accessors that use a pt, eta, phi, m
    /// representation underneath
    ///
    /// This takes a template parameter for the implementation of a class
    /// that provides the accessors for pt, eta, phi, and m. That then
    /// allows to specialize it for different particle types which may
    /// have different implementations of the pt, eta, phi, and m
    /// accessors. Particularly m is different for each particle, but some
    /// particles may also vary other implementations, e.g. calorimeter
    /// eta for electrons and photons, or dressed properties for jets.

    template<typename CoreAccessors>
    class FullMomentumAccessorsPtEtaPhiM : public CoreAccessors
    {
    public:
      static constexpr ContainerId CI = CoreAccessors::CI;
      using CM = typename CoreAccessors::CM;

      using CoreAccessors::CoreAccessors;

      double e (ObjectId<CI,CM> object) const
      {
        const double myPt = this->pt (object);
        const double myEta = this->eta (object);
        const double myM = this->m (object);

        // not reading phi, not part of the energy calculation
        const ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>> myP4 {myPt, myEta, 0, myM};
        return myP4.energy();
      }

      double rapidity (ObjectId<CI,CM> object) const
      {
        const double myEta = this->eta(object);
        const double myM = this->m(object);
        if (myM == 0)
          return myEta; // for massless particles, rapidity is the same as eta
        const double myPt = this->pt(object);

        // not reading phi, not part of the rapidity calculation
        const ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>> myP4 {myPt, myEta, 0, myM};
        return myP4.Rapidity();
      }
    };



    /// a core momentum accessor that reads pt, eta, phi from the file
    template<ContainerId CIVal,typename CMVal> struct CoreMomentumAccessorsPtEtaPhi
    {
      static constexpr ContainerId CI = CIVal;
      using CM = CMVal;

      ColumnAccessor<CI,RetypeColumn<double,float>,CM> pt;
      ColumnAccessor<CI,RetypeColumn<double,float>,CM> eta;
      ColumnAccessor<CI,RetypeColumn<double,float>,CM> phi;

      CoreMomentumAccessorsPtEtaPhi (ColumnarTool<CM>& columnarTool)
        : pt (columnarTool, "pt"), eta (columnarTool, "eta"), phi (columnarTool, "phi")
      {}

      /// a constructor that uses a prefix for the variables (used for jets)
      CoreMomentumAccessorsPtEtaPhi (ColumnarTool<CM>& columnarTool, const std::string& prefix)
        : pt (columnarTool, prefix + "pt"), eta (columnarTool, prefix + "eta"), phi (columnarTool, prefix + "phi")
      {}
    };

    /// a core momentum accessor that reads pt, eta, phi, and m from the file
    template<ContainerId CI, typename CM> struct CoreMomentumAccessorsPtEtaPhiReadM
      : CoreMomentumAccessorsPtEtaPhi<CI,CM>
    {
      ColumnAccessor<CI,RetypeColumn<double,float>,CM> m;

      CoreMomentumAccessorsPtEtaPhiReadM (ColumnarTool<CM>& columnarTool)
        : CoreMomentumAccessorsPtEtaPhi<CI,CM> (columnarTool), m (columnarTool, "m")
      {}

      CoreMomentumAccessorsPtEtaPhiReadM (ColumnarTool<CM>& columnarTool, const std::string& prefix)
        : CoreMomentumAccessorsPtEtaPhi<CI,CM> (columnarTool, prefix), m (columnarTool, prefix + "m")
      {}
    };

    /// a core momentum accessor that reads pt, eta, phi from the file, but
    /// uses a fixed value for m
    template<ContainerId CI, typename CM>
    struct CoreMomentumAccessorsPtEtaPhiFixedM : CoreMomentumAccessorsPtEtaPhi<CI,CM>
    {
      double mValue = 0;

      double m (ObjectId<CI,CM> /*object*/) const noexcept {
        return mValue; }

      CoreMomentumAccessorsPtEtaPhiFixedM (ColumnarTool<CM>& columnarTool, double val_mValue)
        : CoreMomentumAccessorsPtEtaPhi<CI,CM> (columnarTool), mValue (val_mValue)
      {}
    };


    /// a virtual interface for columnar momentum accessors
    ///
    /// This needs to be specialized for the different container IDs,
    /// because the columnar accessors need to be specialized for a
    /// specific container. I may put in some effort to avoid
    /// specializing this for each container ID, but that would need a
    /// serious extension of the underlying infrastructure. So for now
    /// this is the way to go.
    template<ContainerId CI, typename CM = ColumnarModeDefault>
    class IMomentumAccessors
    {
    public:
      virtual ~IMomentumAccessors () = default;

      virtual double pt (ObjectId<CI,CM> object) const = 0;
      virtual double eta (ObjectId<CI,CM> object) const = 0;
      virtual double phi (ObjectId<CI,CM> object) const = 0;
      virtual double m (ObjectId<CI,CM> object) const = 0;
      virtual double e (ObjectId<CI,CM> object) const = 0;
      virtual double rapidity (ObjectId<CI,CM> object) const = 0;
    };

    /// the implementation of @ref IMomentumAccessors that wraps a static
    /// accessor class
    template<typename CoreAccessors>
    class MomentumAccessorsImp : public IMomentumAccessors<CoreAccessors::CI,typename CoreAccessors::CM>
    {
    public:
      static constexpr ContainerId CI = CoreAccessors::CI;
      using CM = typename CoreAccessors::CM;

      template<typename... Args>
      MomentumAccessorsImp (Args&&... args)
        : m_coreAccessors (std::forward<Args> (args)...) {}

      virtual double pt (ObjectId<CI,CM> object) const override {
        return m_coreAccessors.pt (object); }
      virtual double eta (ObjectId<CI,CM> object) const override {
        return m_coreAccessors.eta (object); }
      virtual double phi (ObjectId<CI,CM> object) const override {
        return m_coreAccessors.phi (object); }
      virtual double m (ObjectId<CI,CM> object) const override  {
        return m_coreAccessors.m (object); }
      virtual double e (ObjectId<CI,CM> object) const override {
        return m_coreAccessors.e (object); }
      virtual double rapidity (ObjectId<CI,CM> object) const override {
        return m_coreAccessors.rapidity (object); }

    private:
      CoreAccessors m_coreAccessors;
    };
  }

  /// a handle to hold a @ref IMomentumAccessors object
  ///
  /// In principle the user could also hold a (smart) pointer to the
  /// underlying `IMomentumAccessors`, but having it in a class like
  /// this allows some refactoring in the future.  Currently the
  /// expectation is that momentum accessors will not be widely used,
  /// but if it is I can then refactor this class without affecting the
  /// users.
  ///
  /// The implementation of this is essentially this is a variation on a
  /// type-erased interface. There are a number of variations and
  /// optimizations that could be done here. Most notably if one tried
  /// hard enough one could probably avoid the `ContainerId` template
  /// parameter for the virtual interface, which would cut down on the
  /// code bloat. However that would need some infrastructure support
  /// which is not available (as of 24 Jul 25).
  ///
  /// I'm not quite clear whether it would be a good idea to hide the
  /// underlying momentum accessors 
  ///
  /// Note that this is also always guaranteed to be valid, defaulting
  /// to the `IParticle` implementation (which is not available in
  /// columnar mode).

  template<ContainerId CI, typename CM = ColumnarModeDefault>
  class MomentumAccessors final
  {
  public:

    MomentumAccessors () noexcept
    {
      reset (std::in_place_type<Detail::MomentumAccessorsIParticle<CI,CM>>);
    }

    template<typename MyMomentumAccessors,typename... Args>
    MomentumAccessors (std::in_place_type_t<MyMomentumAccessors>, Args&&... args)
    {
      reset (std::in_place_type<MyMomentumAccessors>, std::forward<Args> (args)...);
    }

    template<typename MyMomentumAccessors,typename... Args>
    void reset (std::in_place_type_t<MyMomentumAccessors>, Args&&... args)
    {
      m_accessors = std::make_shared<Detail::MomentumAccessorsImp<MyMomentumAccessors>> (std::forward<Args> (args)...);
    }

    /// the various momentum accessors
    [[nodiscard]] double pt (ObjectId<CI,CM> object) const {
      return m_accessors->pt (object); }
    [[nodiscard]] double eta (ObjectId<CI,CM> object) const {
      return m_accessors->eta (object); }
    [[nodiscard]] double phi (ObjectId<CI,CM> object) const {
      return m_accessors->phi (object); }
    [[nodiscard]] double m (ObjectId<CI,CM> object) const {
      return m_accessors->m (object); }
    [[nodiscard]] double e (ObjectId<CI,CM> object) const {
      return m_accessors->e (object); }
    [[nodiscard]] double rapidity (ObjectId<CI,CM> object) const {
      return m_accessors->rapidity (object); }

  private:

    /// the underlying accessors, which default to the `xAOD::IParticle`
    /// accessors in case they are not overridden
    std::shared_ptr<const Detail::IMomentumAccessors<CI,CM>> m_accessors;
  };

  /// reset the dynamic momentum accessors to various default implementations
  ///
  /// These are convenience functions to avoid the user having to know
  /// how a specific particle type is implemented. I decided to make
  /// them standalone functions, as that makes it seamless to define
  /// further specializations if needed in other packages. In particular
  /// tracking momentum accessors need some enums from xAODTracking for
  /// the xAOD hypothesis, which I don't want to include here.
  template<ContainerId CI, typename CM>
  void resetIParticle (MomentumAccessors<CI,CM>& accessors) {
    accessors.reset (std::in_place_type<Detail::MomentumAccessorsIParticle<CI,CM>>); }
  template<ContainerId CI, typename CM>
  void resetPtEtaPhiReadM (MomentumAccessors<CI,CM>& accessors, ColumnarTool<CM>& columnarTool) {
    accessors.reset (std::in_place_type<Detail::FullMomentumAccessorsPtEtaPhiM<Detail::CoreMomentumAccessorsPtEtaPhiReadM<CI,CM>>>, columnarTool); }
  template<ContainerId CI, typename CM>
  void resetPtEtaPhiFixedM (MomentumAccessors<CI,CM>& accessors, ColumnarTool<CM>& columnarTool, double mValue) {
    accessors.reset (std::in_place_type<Detail::FullMomentumAccessorsPtEtaPhiM<Detail::CoreMomentumAccessorsPtEtaPhiFixedM<CI,CM>>>, columnarTool, mValue); }
  template<ContainerId CI, typename CM>
  void resetJet (MomentumAccessors<CI,CM>& accessors, ColumnarTool<CM>& columnarTool) {
    resetPtEtaPhiReadM (accessors, columnarTool); }
  template<ContainerId CI, typename CM>
  void resetJetConstituentScale (MomentumAccessors<CI,CM>& accessors, ColumnarTool<CM>& columnarTool, const std::string& prefix = "JetConstitScaleMomentum_") {
    accessors.reset (std::in_place_type<Detail::FullMomentumAccessorsPtEtaPhiM<Detail::CoreMomentumAccessorsPtEtaPhiReadM<CI,CM>>>, columnarTool, prefix); }
  template<ContainerId CI, typename CM>
  void resetElectron (MomentumAccessors<CI,CM>& accessors, ColumnarTool<CM>& columnarTool) {
    resetPtEtaPhiFixedM (accessors, columnarTool, ParticleConstants::electronMassInMeV); }
  template<ContainerId CI, typename CM>
  void resetPhoton (MomentumAccessors<CI,CM>& accessors, ColumnarTool<CM>& columnarTool) {
    resetPtEtaPhiFixedM (accessors, columnarTool, ParticleConstants::photonMassInMeV); }
  template<ContainerId CI, typename CM>
  void resetEgamma (MomentumAccessors<CI,CM>& accessors, ColumnarTool<CM>& columnarTool) {
    resetPtEtaPhiReadM (accessors, columnarTool); }
  template<ContainerId CI, typename CM>
  void resetMuon (MomentumAccessors<CI,CM>& accessors, ColumnarTool<CM>& columnarTool) {
    resetPtEtaPhiFixedM (accessors, columnarTool, ParticleConstants::muonMassInMeV); }
  template<ContainerId CI, typename CM>
  void resetTau (MomentumAccessors<CI,CM>& accessors, ColumnarTool<CM>& columnarTool) {
    resetPtEtaPhiReadM (accessors, columnarTool); }
}

#endif