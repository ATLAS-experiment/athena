/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "JetH5Writer/IParticleWriter.h"
#include "JetH5Writer/IParticleWriterConfig.h"

#include "PrimitiveHelpers.h"

#include "xAODBase/IParticle.h"
#include "HDF5Utils/Writer.h"

// needed concrete classes for ElementLink access
#include "xAODBTagging/BTaggingContainer.h"

namespace {
  template <unsigned int N>
  using Writer_t = H5Utils::Writer<N, const xAOD::IParticle*>;
  using In_t = Writer_t<1>::input_type;
  using Consumer_t = Writer_t<1>::consumer_type;
  using CountWriter_t = H5Utils::Writer<0, unsigned char>;

  template <typename A=detail::defaultAccessor_t<Consumer_t>>
  void addCustomType(Consumer_t& c,
                     const Primitive& p,
                     A a=detail::defaultAccessor<Consumer_t>) {
    using I = In_t;

    if (!detail::isCustom(p)) {
      throw std::logic_error("called addCustomType on non-custom type");
    }
    bool force_precision = p.type == Primitive::Type::PRECISION_CUSTOM;
    const H5Utils::Compression half = force_precision ?
      H5Utils::Compression::STANDARD :
      H5Utils::Compression::HALF_PRECISION;

    const std::string s = p.source;
    const std::string t = p.target;

    // check for match
    bool m = false;

    auto add = [&c, a, t](auto f, auto compression, float mult = 1.0) {
      c.add(
        t,
        [a, f, mult](I in) -> float {
          const auto* associated = a(in);
          if (!associated) return NAN;
          return f(associated)*mult;
        },
        NAN, compression);
    };

    // these do most of the matching work
    auto match = [&add, s, &m, half](const std::string& n, auto func) {
      if (s == n) {
        add(func, half);
        m = true;
      }
    };
    // This matches two possible strings: the original string and one
    // with a "GeV" suffix. The purpose of the suffixed version is to
    // allow us to store things at half precision (since most things
    // stored in MeV would overflow half precision floats).
    auto matchGeV = [&add, s, &m, half](const std::string& n, auto func) {
      if (s == n) {
        // Sort of convoluted logic here: if we force higher precision
        // we set "half" to be full precision. But if someone asks for
        // full precision in a variable that must be stored that way
        // anyway, we throw an exception.
        if (half == H5Utils::Compression::STANDARD) {
          throw std::logic_error(
            "asked for a full precision version of a variable that can"
            " not be stored at half precision: " + s);
        }
        add(func, H5Utils::Compression::STANDARD);
        m = true;
      } else if (s == n + "GeV") {
        add(func, half, 0.001);
        m = true;
      }
    };

    // match cases
    matchGeV("pt", [](auto in) {return in->pt(); });
    match("eta", [](auto in) {return in->eta(); });
    match("phi", [](auto in) {return in->phi(); });
    matchGeV("px", [](auto in) {return in->p4().Px(); });
    matchGeV("py", [](auto in) {return in->p4().Py(); });
    matchGeV("pz", [](auto in) {return in->p4().Pz(); });
    matchGeV("mass", [](auto in) {return in->m(); });

    if (m) return;

    if (s == "valid") {
      c.add(s, [](I) {return true; }, false);
      return;
    }
    throw std::logic_error("unknow known custom primitive: " + s);
  }


  // stuff to retrieve associated links
  template <typename T, typename R=SG::AuxElement>
  class LinkGetter
  {
  public:
    LinkGetter(std::string name);
    const R* operator()(In_t in) const;
  private:
    using LinkAccessor = SG::AuxElement::ConstAccessor<ElementLink<T>>;
    const LinkAccessor m_accessor;
    const std::string m_linkName;
  };
  template <typename T, typename R>
  LinkGetter<T,R>::LinkGetter(std::string name):
    m_accessor(name),
    m_linkName(name)
  {}
  template <typename T, typename R>
  const R* LinkGetter<T,R>::operator()(In_t in) const {
    auto elink = m_accessor(*in);
    // isDefault should generally indicate that the element link has
    // been created but not set to point to any specific place. This
    // is distinct from being created and pointing to a particle that
    // has been thinned or slimmed away. So it's safe to use as a "not
    // set" code.
    if (elink.isDefault()) {
      return nullptr;
    }
    // If the link is _not_ default, but is also invalid, then we're
    // trying to access something that has been removed. We treat this
    // as an error, because the behavior becomes dependent on the
    // format.
    if (!elink.isValid()) {
      throw std::runtime_error("invalid link " + m_linkName);
    }
    return *elink;
  }

  CountWriter_t::consumer_type getOffsetConsumer()
  {
    using CountIn_t = CountWriter_t::input_type;
    CountWriter_t::consumer_type c;
    c.add("count", [](CountIn_t i) { return i; });
    return c;
  }

}


// IParticleWriter implementaitons
namespace details {
  class IParticleWriterBase
  {
  public:
    virtual ~IParticleWriterBase() = default;
    virtual void fill(const std::vector<const xAOD::IParticle*>& info) = 0;
    virtual void flush() = 0;
  };
}
namespace {
  // implementation for writer for 2d arrays
  class IParticle2dWriter: public details::IParticleWriterBase
  {
  private:
    Writer_t<1> m_writer;
  public:
    IParticle2dWriter(H5::Group& group,
                      const std::string& n,
                      Consumer_t c,
                      long long unsigned size):
      m_writer(group, n, c, {{size}}) {}
    ~IParticle2dWriter() = default;

    void fill(const std::vector<In_t>& v) override {
      m_writer.fill(v);
    }
    void flush() override {
      m_writer.flush();
    }
  };

  // implementation for writer for awkward arrays
  class IParticleAwkwardWriter: public details::IParticleWriterBase
  {
  private:
    H5::Group m_group;
    Writer_t<0> m_writer;
    CountWriter_t m_counts;
  public:
    IParticleAwkwardWriter(H5::Group& parent,
                           const std::string& n,
                           Consumer_t c):
      m_group(parent.createGroup(n)),
      m_writer(m_group, "raw", c),
      m_counts(m_group, "counts", getOffsetConsumer())
      {}
    ~IParticleAwkwardWriter() = default;

    void fill(const std::vector<In_t>& v) override {
      using Count_t = CountWriter_t::input_type;
      constexpr auto max = std::numeric_limits<Count_t>::max();
      auto n_entries = v.size();
      if (n_entries > max) {
        throw std::overflow_error(
          "number of entries exceeds maximum for this datatype "
          "[" + std::to_string(n_entries) + " > " + std::to_string(max) + "]"
          );
      }
      for (const auto& e: v) m_writer.fill(e);
      m_counts.fill(n_entries);
    }
    void flush() override {
      m_writer.flush();
      m_counts.flush();
    }
  };


  // implementation for writer for awkward arrays
  class IParticleFlatWriter: public details::IParticleWriterBase
  {
  private:
    H5::Group m_group;
    Writer_t<0> m_writer;
  public:
    IParticleFlatWriter(H5::Group& parent,
                           const std::string& n,
                           Consumer_t c):
      m_writer(parent, n, c)
      {}
    ~IParticleFlatWriter() = default;

    void fill(const std::vector<In_t>& v) override {
      for (const auto& e: v) m_writer.fill(e);
    }
    void flush() override {
      m_writer.flush();
    }
  };

  std::unique_ptr<details::IParticleWriterBase> getWriter(
    H5::Group& g,
    const IParticleWriterConfig& cfg,
    Consumer_t c) {
    using af = IParticleWriterConfig::ArrayFormat;
    switch (cfg.format) {
      case af::AWKWARD: {
        if (cfg.maximum_size != 0) {
          throw std::domain_error(
            "no maximum_size should be specified for awkward arrays");
        }
        return std::make_unique<IParticleAwkwardWriter>(g, cfg.name, c);
      }
      case af::FLAT: {
        if (cfg.maximum_size != 0) {
          throw std::domain_error(
            "no maximum_size should be specified for flat arrays");
        }
        return std::make_unique<IParticleFlatWriter>(g, cfg.name, c);
      }
      case af::PADDED: {
        if (cfg.maximum_size == 0) {
          throw std::domain_error(
            "maximum_size should be specified for padded 2d arrays");
        }
        return std::make_unique<IParticle2dWriter>(
          g, cfg.name, c, cfg.maximum_size);
      }
      default: {
        throw std::domain_error("unknown array format");
      }
    }
  }
}


IParticleWriter::IParticleWriter(
  H5::Group& group,
  const IParticleWriterConfig& cfg)
{
  using IPC = xAOD::IParticleContainer;
  using IP = xAOD::IParticle;
  Consumer_t c;
  for (const auto& input: cfg.inputs) {
    if (input.link_name.empty()) {
      const auto& primitive = input.input;
      if (detail::isCustom(primitive.type)) {
        addCustomType(c, primitive);
      } else {
        detail::addInput(c, primitive);
      }
    } else {
      // else we have some association to follow
      std::string n = input.link_name;
      // unfortunately we need a special case for b-tagging
      if (n == "btaggingLink") {
        LinkGetter<xAOD::BTaggingContainer> getter(n);
        detail::addInput(c, input.input, getter);
      } else {
        // everything else is an IParticle
        LinkGetter<IPC,IP> getter(n);
        if (detail::isCustom(input.input.type)) {
          addCustomType(c, input.input, getter);
        } else {
          detail::addInput(c, input.input, getter);
        }
      }
    }
  }
  m_writer = getWriter(group, cfg, c);
}


IParticleWriter::~IParticleWriter() = default;


void IParticleWriter::fill(const std::vector<In_t>& info) {
  m_writer->fill(info);
}


void IParticleWriter::flush() {
  m_writer->flush();
}
