/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./eEmSelector.h"
#include <sstream>

namespace GlobalSim {

  class lt: public ICutter {
  public:
    lt(const ulong& c): m_cut(c) {}
    virtual bool cut(const ulong& v) const override {
      return v < m_cut;  
    };

    virtual std::string to_string() const override {
      std::stringstream ss;
      ss << "cut: " << m_cut << " op: < ";
      return ss.str();
    }
    
  private:
    ulong m_cut;
  };

  class leq: public ICutter {
  public:
    leq(const ulong& c): m_cut(c) {}
    virtual bool cut(const ulong& v) const override {
      return v <= m_cut;  
    };

    virtual std::string to_string() const override {
      std::stringstream ss;
      ss << "cut: " << m_cut << " op: <= ";
      return ss.str();
    }

  private:
    ulong m_cut;
  };

  class gt: public ICutter {
  public:
    gt(const ulong& c): m_cut(c) {}
    virtual bool cut(const ulong& v) const override {
      return v > m_cut;  
    };
    
    virtual std::string to_string() const override {
      std::stringstream ss;
      ss << "cut: " << m_cut << " op: > ";
      return ss.str();
    }

  private:
    ulong m_cut;
  };

  
  class geq: public ICutter {
  public:
    geq(const ulong& c): m_cut(c) {}
    virtual bool cut(const ulong& v) const override {
      return v >= m_cut;  
    };

    virtual std::string to_string() const override {
      std::stringstream ss;
      ss << "cut: " << m_cut << " op: >= ";
      return ss.str();
    }

  private:
    ulong m_cut;
  };


  

  std::unique_ptr<ICutter> make_cutter(const ulong& cut,
					const std::string& op) {
    
    auto cutter = std::unique_ptr<ICutter>(nullptr);

    if (op == ">"){
      cutter.reset(new gt(cut));
    } else if (op == ">="){
      cutter.reset(new geq(cut));
    } else if (op == "<"){
      cutter.reset(new lt(cut));
    } else if (op == "<="){
      cutter.reset(new leq(cut));
    } else {
      throw std::invalid_argument("unown operator " + op);
    }

   

    return cutter;
  }

  using namespace GlobalSim::IOBitwise;
  
  eEmSelector::eEmSelector(ulong rhad_cut,
			   const std::string& rhad_op,
			   ulong reta_cut,
			   const std::string& reta_op,
			   ulong wstot_cut,
			   const std::string& wstot_op) :
    m_rhad_cutter{make_cutter(rhad_cut, rhad_op)},
    m_reta_cutter{make_cutter(reta_cut, reta_op)},
    m_wstot_cutter{make_cutter(wstot_cut, wstot_op)}{
  }


  bool eEmSelector::select(const IeEmTOB& tob) const {

    if(!m_rhad_cutter->cut(tob.RHad_bits().to_ulong())) {return false;}
    if(!m_reta_cutter->cut(tob.REta_bits().to_ulong())) {return false;}
    if(!m_wstot_cutter->cut(tob.WsTot_bits().to_ulong())) {return false;}

    return true;
  };


  std::string eEmSelector::to_string() const {
    
    auto ss = std::stringstream();
    ss << "rhad cutter: " << m_rhad_cutter->to_string() << ' '
       << "reta cutter: " << m_reta_cutter->to_string() << ' '
       << "wstot cutter: " << m_wstot_cutter->to_string();
    return ss.str();
  };


}
