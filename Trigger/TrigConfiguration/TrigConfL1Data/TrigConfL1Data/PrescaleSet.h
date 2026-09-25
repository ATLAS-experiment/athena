// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigConf_PrescaleSet
#define TrigConf_PrescaleSet

#include "TrigConfL1Data/L1DataBaseclass.h"

#include <vector>
#include <string>
#include <stdint.h>

/** @brief class PrescaleSet contains prescales of a Level 1 partition
 *
 *  In run 2 the CTP uses a new prescaling mechanism: for each item a 24-bit pseudo-random number generator is used.
 *  If the resulting (thrown) number is 
 **/
namespace TrigConf {

   class PrescaleSet : public L1DataBaseclass {
   public:

      static const int32_t maxPrescaleCut;

      static int32_t getCutFromPrescale(double prescale);

      static double getPrescaleFromCut(int32_t cut);

      PrescaleSet() = default;
      virtual ~PrescaleSet() override = default;

      // Accessors
      bool                         isNull() const { return m_null; }
      const std::string&           type() const { return m_Type; }
      unsigned int                 partition() const { return m_Partition; }
      const std::vector<int64_t>&  prescales_ctp() const { return m_Prescales_ctp; }
      const std::vector<float>&    prescales_float() const { return m_Prescales_float; }
      const std::vector<int32_t>&  cuts() const { return m_Cuts; }

      // Setters
      void setType(const std::string& type)        { m_Type = type; }
      void setPartition(unsigned int partition)    { m_Partition = partition; }

      // setting of prescales
      void resize(size_t size);
      void setPrescale(unsigned int num, float prescaleValue);
      void setCuts(const std::vector<int32_t>& cuts);
      void setCut(unsigned int num, int32_t cut);

      // reset all prescales to -1
      void reset();

      virtual void print(const std::string& indent="", unsigned int detail=1) const override;

      void writeXML(std::ostream & xmlfile, int indentLevel = 0, int indentWidth = 2) const;

   private:
      bool m_null{true}; // false: indicates if the prescale set has been filled

      std::string           m_Type{"Physics"}; // the type of the prescale set, e.g. Physics, Standby, Cosmics, HighRate, VdM (case insensitive)
      unsigned int          m_Partition{0};
      std::vector<int64_t>  m_Prescales_ctp; // the 64bit representation.
      std::vector<float>    m_Prescales_float; // the float representation.
      std::vector<int32_t>  m_Cuts; // the cut 
      
   };
   
}

#endif

