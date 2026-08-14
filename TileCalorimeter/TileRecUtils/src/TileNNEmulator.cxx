/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TileNNEmulator.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <stdexcept>

int64_t TileNNEmulator::rshiftRoundHalfEven(int64_t v, int k) {
  if (k <= 0) return v;
  int64_t q = v >> k; 
  int64_t r = v - (q << k);
  int64_t half = int64_t(1) << (k - 1);
  if (r > half || (r == half && (q & 1))) ++q;

  return q;
}

int64_t TileNNEmulator::castTo(int64_t v, int fracFrom, const FixSpec& spec) {
  int d = fracFrom - spec.frac();
  if (d > 0) {
    v = (spec.round == Round::RND_CONV) ? rshiftRoundHalfEven(v, d) : (v >> d);
  } else if (d < 0) {
    v <<= -d;
  }

  int64_t lim = int64_t(1) << (spec.w - 1);
  switch (spec.sat) {
    case Sat::SAT:
      if (v < -lim) v = -lim;
      else if (v >= lim) v = lim - 1;
      break;
    case Sat::SAT_SYM:
      if (v <= -lim) v = -lim + 1;
      else if (v >= lim) v = lim - 1;
      break;
    case Sat::WRAP:
      v = ((v + lim) & ((int64_t(1) << spec.w) - 1)) - lim;
      break;
  }

  return v;
}

float TileNNEmulator::sValue(float hg, float lg) const {
  return static_cast<float>((hg + m_lgScale * lg) / m_denominator);
}

int64_t TileNNEmulator::run(const float* s) const {
  std::vector<int64_t> x(m_nSamples);

  for (int k = 0; k < m_nSamples; ++k) {
    // float -> in_t: ldexp scales by 2^frac exactly, llrint rounds the real
    // value half-to-even (FE_TONEAREST), i.e, AP_RND_CONV.
    int64_t c = std::llrint(std::ldexp(static_cast<double>(s[k]), m_inT.frac()));
    c = castTo(c, m_inT.frac(), m_inT);
    x[k] = castTo(c, m_inT.frac(), m_castT);
  }

  int frac = m_castT.frac();
  std::vector<int64_t> y;

  for (const Layer& l : m_layers) {
    if (l.op == Op::Dense) {
      y.assign(l.nOut, 0);

      for (int i = 0; i < l.nIn; ++i) {
        for (int j = 0; j < l.nOut; ++j) {
          int64_t p = x[i] * l.w[i * l.nOut + j];
          y[j] += castTo(p, frac + l.weightT.frac(), l.accumT);
        }
      }

      int up = l.biasT.frac() - l.accumT.frac();

      for (int j = 0; j < l.nOut; ++j) {
        y[j] = castTo((y[j] << up) + l.b[j], l.biasT.frac(), l.outT);
      }
    } else { 
      y.resize(l.nOut);

      for (int j = 0; j < l.nOut; ++j) {
        // Negative branch: multiplying by alpha = 2^-alphaShift is a
        // reinterpretation of the code at alphaShift more fractional bits.
        int from = (x[j] > 0) ? frac : frac + l.alphaShift;
        y[j] = castTo(x[j], from, l.outT);
      }
    }
    
    frac = l.outT.frac();
    x.swap(y);
  }

  return x[0];
}

bool TileNNEmulator::load(const std::string& jsonText, std::string& error) {
  using nlohmann::json;

  auto fix = [](const json& j) {
    FixSpec t;
    t.w = j.at("w").get<int>();
    t.i = j.at("i").get<int>();
    std::string r = j.at("round").get<std::string>();
    std::string s = j.at("sat").get<std::string>();
    if (r == "RND_CONV") t.round = Round::RND_CONV;
    else if (r == "TRN") t.round = Round::TRN;
    else throw std::runtime_error("unknown rounding mode '" + r + "'");
    if (s == "SAT") t.sat = Sat::SAT;
    else if (s == "SAT_SYM") t.sat = Sat::SAT_SYM;
    else if (s == "WRAP") t.sat = Sat::WRAP;
    else throw std::runtime_error("unknown saturation mode '" + s + "'");

    return t;
  };

  try {
    json cfg = json::parse(jsonText);
    if (cfg.at("format_version").get<int>() != 1) {
      throw std::runtime_error("unsupported format_version");
    }

    const json& in = cfg.at("input");
    m_nSamples = in.at("n_samples").get<int>();
    m_lgScale = in.at("lg_scale").get<double>();
    m_denominator = in.at("denominator").get<double>();
    m_inT = fix(in.at("in_t"));
    m_castT = fix(in.at("cast_t"));
    m_ampScale = cfg.at("output").at("amplitude_scale").get<double>();
    m_layers.clear();
    int width = m_nSamples;
    
    for (const json& jl : cfg.at("layers")) {
      Layer l;
      std::string op = jl.at("op").get<std::string>();
      if (op == "dense") {
        l.op = Op::Dense;
        l.nIn = jl.at("n_in").get<int>();
        l.nOut = jl.at("n_out").get<int>();
        l.weightT = fix(jl.at("weight_t"));
        l.accumT = fix(jl.at("accum_t"));
        l.biasT = fix(jl.at("bias_t"));
        l.outT = fix(jl.at("out_t"));
        l.w = jl.at("weights").get<std::vector<int64_t>>();
        l.b = jl.at("bias").get<std::vector<int64_t>>();
        if (l.w.size() != size_t(l.nIn) * l.nOut || l.b.size() != size_t(l.nOut)) {
          throw std::runtime_error("dense layer weight/bias size mismatch");
        }

        if (l.biasT.frac() < l.accumT.frac()) {
          throw std::runtime_error("bias grid coarser than accumulator grid");
        }
      } else if (op == "leaky_relu") {
        l.op = Op::LeakyRelu;
        l.nIn = l.nOut = jl.at("n").get<int>();
        l.alphaShift = jl.at("alpha_shift").get<int>();
        l.outT = fix(jl.at("out_t"));
      } else {
        throw std::runtime_error("unknown op '" + op + "'");
      }

      if (l.nIn != width) {
        throw std::runtime_error("layer input width " + std::to_string(l.nIn)
                                 + " does not chain from " + std::to_string(width));
      }

      width = l.nOut;
      m_layers.push_back(std::move(l));
    }

    if (m_layers.empty() || width != 1) {
      throw std::runtime_error("network must end in a single output");
    }
  } catch (const std::exception& e) {
    error = e.what();
    m_layers.clear();

    return false;
  }

  return true;
}
