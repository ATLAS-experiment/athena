//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_EXTRATAGDEFINITIONS_H
#define CALORECGPU_EXTRATAGDEFINITIONS_H

#include <cstdint>
#include "CaloRecGPU/TagDefinitions.h"

//This holds several possible definitions for tags,
//used throughout development (in particular of the splitter).
//The current implementation only really uses Tag_1_1_12_32_18,
//but it could be relevant to keep the others here too
//for possible future changes to the algorithms...


namespace CaloRecGPU
{
  
  /*! @class GenericTagBase
  
      A base class to implement the constructor, destructor and conversion operators.
  */
  struct GenericTagBase
  {
   public:

    using carrier = CaloRecGPU::TagBase::carrier;
    //uint64_t is unsigned long long int in CUDA.

    carrier value;

    constexpr operator carrier () const
    {
      return value;
    }

    constexpr GenericTagBase (const carrier v): value(v)
    {
    }

    constexpr GenericTagBase & operator = (const carrier v)
    {
      value = v;
      return (*this);
    }
    
    [[nodiscard]] static constexpr carrier make_invalid_tag()
    {
      return 0ULL;
    }
  };
  
  /*! @class Tag_1_30_18_15
  
      A tag with a bit flag, then 30 bit (intended for a truncated float),
      18 bit (for a cell index) and 15 bit (for a cluster index).
  */
  struct Tag_1_30_18_15 : public GenericTagBase
  {
   public:

    using GenericTagBase::GenericTagBase;
    
   protected:

    constexpr static carrier s_one_bit_mask = 0x8000000000000000ULL;
    constexpr static carrier s_30_bit_mask  = 0x7FFFFFFE00000000ULL;
    constexpr static carrier s_18_bit_mask  = 0x00000001FFFF8000ULL;
    constexpr static carrier s_15_bit_mask  = 0x0000000000007FFFULL;
        
   public:

    [[nodiscard]] constexpr bool get_flag() const
    {
      return value & s_one_bit_mask;
    }
    
    [[nodiscard]] constexpr uint32_t get_30_bits() const
    {
      return (value & s_30_bit_mask) >> 33;
    }

    [[nodiscard]] constexpr uint32_t get_18_bits() const
    {
      return (value & s_18_bit_mask) >> 15;
    }
    
    [[nodiscard]] constexpr uint32_t get_15_bits() const
    {
      return (value & s_15_bit_mask);
    }
    
    [[nodiscard]] static constexpr carrier make_generic_tag(const uint16_t bits_15, const uint32_t bits_18, const uint32_t bits_30, const bool flag)
    {
      constexpr uint32_t thirty_bits_mask   = 0x3FFFFFFFU;
      constexpr uint32_t eighteen_bits_mask =    0x3FFFF ;
      constexpr uint16_t fifteen_bits_mask  =     0x7FFFU;
      
      carrier ret = bits_30 & thirty_bits_mask;
      
      ret = (ret << 18) | (bits_18 & eighteen_bits_mask);
      
      ret = (ret << 15) | (bits_15 & fifteen_bits_mask);

      ret = ret | (s_one_bit_mask * flag);

      return ret;
    }

  };
  
  
  /*! @class Tag_1_1_12_16_18_16
  
      A tag with two bit flags, then 12 bits (for a counter),
      16 bits (for a half-precision float), 18 bits (for a cell index)
      and 16 bits (for a cluster index)
      
  */
  struct Tag_1_1_12_16_18_16 : public GenericTagBase
  {
   public:

    using GenericTagBase::GenericTagBase;

   protected:

    constexpr static carrier s_first_flag_mask    = 0x8000000000000000ULL;
    constexpr static carrier s_second_flag_mask   = 0x4000000000000000ULL;
    constexpr static carrier s_12_bit_mask        = 0x3FFC000000000000ULL;
    constexpr static carrier s_first_16_bit_mask  = 0x0003FFFC00000000ULL;
    constexpr static carrier s_18_bit_mask        = 0x00000003FFFF0000ULL;
    constexpr static carrier s_second_16_bit_mask = 0x000000000000FFFFULL;
    
    constexpr static unsigned int s_second_16_bit_offset = 0;
    constexpr static unsigned int s_18_bit_offset        = s_second_16_bit_offset + 16;
    constexpr static unsigned int s_first_16_bit_offset  = s_18_bit_offset + 18;
    constexpr static unsigned int s_12_bit_offset        = s_first_16_bit_offset + 16;
        
   public:

    [[nodiscard]] constexpr bool get_first_flag() const
    {
      return value & s_first_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier set_first_flag() const
    {
      return value | s_first_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier unset_first_flag() const
    {
      return value & (~s_first_flag_mask);
    }
    
    [[nodiscard]] constexpr bool get_second_flag() const
    {
      return value & s_second_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier set_second_flag() const
    {
      return value | s_second_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier unset_second_flag() const
    {
      return value & (~s_second_flag_mask);
    }
    
    [[nodiscard]] constexpr uint32_t get_12_bits() const
    {
      return (value & s_12_bit_mask) >> s_12_bit_offset;
    }
    
    [[nodiscard]] constexpr uint32_t get_first_16_bits() const
    {
      return (value & s_first_16_bit_mask) >> s_first_16_bit_offset;
    }

    [[nodiscard]] constexpr uint32_t get_18_bits() const
    {
      return (value & s_18_bit_mask) >> s_18_bit_offset;
    }
    
    [[nodiscard]] constexpr uint32_t get_second_16_bits() const
    {
      return (value & s_second_16_bit_mask) >> s_second_16_bit_offset;
    }
    
    [[nodiscard]] static constexpr carrier make_generic_tag(const uint16_t second_bits_16, const uint32_t bits_18,
                                              const uint16_t first_bits_16, const uint16_t bits_12,
                                              const bool flag_2, const bool flag_1            )
    {
      constexpr uint16_t twelve_bits_mask   =   0xFFFU;
      constexpr uint32_t eighteen_bits_mask = 0x3FFFFU;
      
      carrier ret = bits_12 & twelve_bits_mask;
      
      ret = (ret << 16) | first_bits_16;
      
      ret = (ret << 18) | (bits_18 & eighteen_bits_mask);
      
      ret = (ret << 16) | second_bits_16;

      ret = ret | (s_second_flag_mask * flag_2) | (s_first_flag_mask * flag_1);

      return ret;
    }

  };
  
  
  /*! @class Tag_1_1_7_31_8_16
  
      A tag with two bit flags, then 7 bits (for a counter),
      31 bits (for a float with one bit shaved off), 8 bits (for an extra factor)
      and 16 bits (for a cluster index)
      
  */
  struct Tag_1_1_7_31_8_16 : public GenericTagBase
  {
   public:

    using GenericTagBase::GenericTagBase;

   protected:

    constexpr static carrier s_first_flag_mask   = 0x8000000000000000ULL;
    constexpr static carrier s_second_flag_mask  = 0x4000000000000000ULL;
    constexpr static carrier s_7_bit_mask        = 0x3F80000000000000ULL;
    constexpr static carrier s_31_bit_mask       = 0x007FFFFFFF000000ULL;
    constexpr static carrier s_8_bit_mask        = 0x0000000000FF0000ULL;
    constexpr static carrier s_16_bit_mask       = 0x000000000000FFFFULL;
    
    constexpr static unsigned int s_16_bit_offset = 0;
    constexpr static unsigned int s_8_bit_offset  = s_16_bit_offset + 16;
    constexpr static unsigned int s_31_bit_offset =  s_8_bit_offset + 8;
    constexpr static unsigned int s_7_bit_offset  = s_31_bit_offset + 31;
        
   public:

    [[nodiscard]] constexpr bool get_first_flag() const
    {
      return value & s_first_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier set_first_flag() const
    {
      return value | s_first_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier unset_first_flag() const
    {
      return value & (~s_first_flag_mask);
    }
    
    [[nodiscard]] constexpr bool get_second_flag() const
    {
      return value & s_second_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier set_second_flag() const
    {
      return value | s_second_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier make_first_flag(const bool flag) const
    {
      return (value & (~(s_first_flag_mask * (!flag)))) | (s_first_flag_mask * flag);
    }
    
    [[nodiscard]] constexpr carrier make_second_flag(const bool flag) const
    {
      return (value & (~(s_second_flag_mask * (!flag)))) | (s_second_flag_mask * flag);
    }
    
    [[nodiscard]] constexpr carrier unset_second_flag() const
    {
      return value & (~s_second_flag_mask);
    }
    
    [[nodiscard]] constexpr uint32_t get_7_bits() const
    {
      return (value & s_7_bit_mask) >> s_7_bit_offset;
    }
    
    [[nodiscard]] constexpr uint32_t get_31_bits() const
    {
      return (value & s_31_bit_mask) >> s_31_bit_offset;
    }

    [[nodiscard]] constexpr uint32_t get_8_bits() const
    {
      return (value & s_8_bit_mask) >> s_8_bit_offset;
    }
    
    [[nodiscard]] constexpr uint32_t get_16_bits() const
    {
      return (value & s_16_bit_mask) >> s_16_bit_offset;
    }
    
    [[nodiscard]] static constexpr carrier make_generic_tag(const uint16_t bits_16, const uint8_t bits_8,
                                              const uint32_t bits_31, const uint8_t bits_7,
                                              const bool flag_2, const bool flag_1            )
    {
      constexpr uint32_t bits_31_mask = 0x7FFFFFFFU;
      constexpr uint8_t bits_7_mask  = 0x7FU;
      
      carrier ret = bits_7 & bits_7_mask;
      
      ret = (ret << 31) | (bits_31 & bits_31_mask);
      
      ret = (ret << 8) | bits_8;
      
      ret = (ret << 16) | bits_16;

      ret = ret | (s_second_flag_mask * flag_2) | (s_first_flag_mask * flag_1);

      return ret;
    }

  };
  
  
  /*! @class Tag_1_7_1_31_8_16
  
      A tag with a bit flag, then 7 bits (for a counter), then another bit flag,
      31 bits (for a float with one bit shaved off), 8 bits (for an extra factor)
      and 16 bits (for a cluster index)
      
  */
  struct Tag_1_7_1_31_8_16 : public GenericTagBase
  {
   public:

    using GenericTagBase::GenericTagBase;

   protected:

    constexpr static carrier s_first_flag_mask   = 0x8000000000000000ULL;
    constexpr static carrier s_7_bit_mask        = 0x7F00000000000000ULL;
    constexpr static carrier s_second_flag_mask  = 0x0080000000000000ULL;
    constexpr static carrier s_31_bit_mask       = 0x007FFFFFFF000000ULL;
    constexpr static carrier s_8_bit_mask        = 0x0000000000FF0000ULL;
    constexpr static carrier s_16_bit_mask       = 0x000000000000FFFFULL;
    
    constexpr static unsigned int s_16_bit_offset = 0;
    constexpr static unsigned int s_8_bit_offset  = s_16_bit_offset + 16;
    constexpr static unsigned int s_31_bit_offset =  s_8_bit_offset + 8;
    constexpr static unsigned int s_7_bit_offset  = s_31_bit_offset + 31 + 1;
        
   public:

    [[nodiscard]] constexpr bool get_first_flag() const
    {
      return value & s_first_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier set_first_flag() const
    {
      return value | s_first_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier unset_first_flag() const
    {
      return value & (~s_first_flag_mask);
    }
    
    [[nodiscard]] constexpr bool get_second_flag() const
    {
      return value & s_second_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier set_second_flag() const
    {
      return value | s_second_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier unset_second_flag() const
    {
      return value & (~s_second_flag_mask);
    }
    
    [[nodiscard]] constexpr carrier make_first_flag(const bool flag) const
    {
      return (value & (~(s_first_flag_mask * (!flag)))) | (s_first_flag_mask * flag);
    }
    
    [[nodiscard]] constexpr carrier make_second_flag(const bool flag) const
    {
      return (value & (~(s_second_flag_mask * (!flag)))) | (s_second_flag_mask * flag);
    }
    
    [[nodiscard]] constexpr uint32_t get_7_bits() const
    {
      return (value & s_7_bit_mask) >> s_7_bit_offset;
    }
    
    [[nodiscard]] constexpr uint32_t get_31_bits() const
    {
      return (value & s_31_bit_mask) >> s_31_bit_offset;
    }

    [[nodiscard]] constexpr uint32_t get_8_bits() const
    {
      return (value & s_8_bit_mask) >> s_8_bit_offset;
    }
    
    [[nodiscard]] constexpr uint32_t get_16_bits() const
    {
      return (value & s_16_bit_mask) >> s_16_bit_offset;
    }
    
    [[nodiscard]] static constexpr carrier make_generic_tag(const uint16_t bits_16, const uint8_t bits_8,
                                              const uint32_t bits_31, const uint8_t bits_7,
                                              const bool flag_2, const bool flag_1            )
    {
      constexpr uint32_t bits_31_mask = 0x7FFFFFFFU;
      constexpr uint8_t bits_7_mask  = 0x7FU;
      
      carrier ret = bits_7 & bits_7_mask;
      
      ret = (ret << 32) | (bits_31 & bits_31_mask);
      
      ret = (ret << 8) | bits_8;
      
      ret = (ret << 16) | bits_16;

      ret = ret | (s_second_flag_mask * flag_2) | (s_first_flag_mask * flag_1);

      return ret;
    }

  };
  
  
  /*! @class Tag_1_1_12_32_18
  
      A tag with two bit flags, then 12 bits (for a counter),
      32 bits (for a float) and 18 bits (for a cell index)
      
  */
  struct Tag_1_1_12_32_18 : public GenericTagBase
  {
   public:

    using GenericTagBase::GenericTagBase;

   protected:

    constexpr static carrier s_first_flag_mask   = 0x8000000000000000ULL;
    constexpr static carrier s_second_flag_mask  = 0x4000000000000000ULL;
    constexpr static carrier s_12_bit_mask       = 0x3FFC000000000000ULL;
    constexpr static carrier s_32_bit_mask       = 0x0003FFFFFFFC0000ULL;
    constexpr static carrier s_18_bit_mask       = 0x000000000003FFFFULL;
    
    constexpr static unsigned int s_18_bit_offset = 0;
    constexpr static unsigned int s_32_bit_offset =  s_18_bit_offset + 18;
    constexpr static unsigned int s_12_bit_offset  = s_32_bit_offset + 32;
        
   public:

    [[nodiscard]] constexpr bool get_first_flag() const
    {
      return value & s_first_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier set_first_flag() const
    {
      return value | s_first_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier unset_first_flag() const
    {
      return value & (~s_first_flag_mask);
    }
    
    [[nodiscard]] constexpr bool get_second_flag() const
    {
      return value & s_second_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier set_second_flag() const
    {
      return value | s_second_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier make_first_flag(const bool flag) const
    {
      return (value & (~(s_first_flag_mask * (!flag)))) | (s_first_flag_mask * flag);
    }
    
    [[nodiscard]] constexpr carrier make_second_flag(const bool flag) const
    {
      return (value & (~(s_second_flag_mask * (!flag)))) | (s_second_flag_mask * flag);
    }
    
    [[nodiscard]] constexpr carrier unset_second_flag() const
    {
      return value & (~s_second_flag_mask);
    }
    
    [[nodiscard]] constexpr uint32_t get_12_bits() const
    {
      return (value & s_12_bit_mask) >> s_12_bit_offset;
    }
    
    [[nodiscard]] constexpr uint32_t get_32_bits() const
    {
      return (value & s_32_bit_mask) >> s_32_bit_offset;
    }

    [[nodiscard]] constexpr uint32_t get_18_bits() const
    {
      return (value & s_18_bit_mask) >> s_18_bit_offset;
    }
    
    [[nodiscard]] static constexpr carrier make_generic_tag(const uint32_t bits_18, const uint32_t bits_32,
                                              const uint32_t bits_12, const bool flag_2, const bool flag_1  )
    {
      constexpr uint32_t bits_18_mask = 0x0003FFFFU;
      constexpr uint32_t bits_12_mask = 0x00000FFFU;
      
      carrier ret = bits_12 & bits_12_mask;
      
      ret = (ret << 32) | bits_32;
      
      ret = (ret << 18) | (bits_18 & bits_18_mask);
      
      ret = ret | (s_second_flag_mask * flag_2) | (s_first_flag_mask * flag_1);

      return ret;
    }

  };
  
  /*! @class Tag_1_12_1_32_18
  
      A tag with a bit flag, then 12 bits (for a counter),
      then another bit flag, then 32 bits (for a float)
      and finally 18 bits (for a cell index)
      
  */
  struct Tag_1_12_1_32_18 : public GenericTagBase
  {
   public:

    using GenericTagBase::GenericTagBase;

   protected:

    constexpr static carrier s_first_flag_mask   = 0x8000000000000000ULL;
    constexpr static carrier s_12_bit_mask       = 0x7FF8000000000000ULL;
    constexpr static carrier s_second_flag_mask  = 0x0004000000000000ULL;
    constexpr static carrier s_32_bit_mask       = 0x0003FFFFFFFC0000ULL;
    constexpr static carrier s_18_bit_mask       = 0x000000000003FFFFULL;
    
    constexpr static unsigned int s_18_bit_offset = 0;
    constexpr static unsigned int s_32_bit_offset =  s_18_bit_offset + 18;
    constexpr static unsigned int s_12_bit_offset  = s_32_bit_offset + 32 + 1;
        
   public:

    [[nodiscard]] constexpr bool get_first_flag() const
    {
      return value & s_first_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier set_first_flag() const
    {
      return value | s_first_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier unset_first_flag() const
    {
      return value & (~s_first_flag_mask);
    }
    
    [[nodiscard]] constexpr bool get_second_flag() const
    {
      return value & s_second_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier set_second_flag() const
    {
      return value | s_second_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier make_first_flag(const bool flag) const
    {
      return (value & (~(s_first_flag_mask * (!flag)))) | (s_first_flag_mask * flag);
    }
    
    [[nodiscard]] constexpr carrier make_second_flag(const bool flag) const
    {
      return (value & (~(s_second_flag_mask * (!flag)))) | (s_second_flag_mask * flag);
    }
    
    [[nodiscard]] constexpr carrier unset_second_flag() const
    {
      return value & (~s_second_flag_mask);
    }
    
    [[nodiscard]] constexpr uint32_t get_12_bits() const
    {
      return (value & s_12_bit_mask) >> s_12_bit_offset;
    }
    
    [[nodiscard]] constexpr uint32_t get_32_bits() const
    {
      return (value & s_32_bit_mask) >> s_32_bit_offset;
    }

    [[nodiscard]] constexpr uint32_t get_18_bits() const
    {
      return (value & s_18_bit_mask) >> s_18_bit_offset;
    }
    
    [[nodiscard]] static constexpr carrier make_generic_tag(const uint32_t bits_18, const uint32_t bits_32,
                                              const uint32_t bits_12, const bool flag_2, const bool flag_1  )
    {
      constexpr uint32_t bits_18_mask = 0x0003FFFFU;
      constexpr uint32_t bits_12_mask = 0x00000FFFU;
      
      carrier ret = bits_12 & bits_12_mask;
      
      ret = (ret << 33) | bits_32;
      
      ret = (ret << 18) | (bits_18 & bits_18_mask);
      
      ret = ret | (s_second_flag_mask * flag_2) | (s_first_flag_mask * flag_1);

      return ret;
    }

  };
  
  /*! @class Tag_1_13_32_18
  
      A tag with a bit flag, then 13 bits (for a counter),
      then 32 bits (for a float) and finally 18 bits (for a cell index)
  */
  struct Tag_1_13_32_18 : public GenericTagBase
  {
   public:

    using GenericTagBase::GenericTagBase;

   protected:

    constexpr static carrier s_flag_mask          = 0x8000000000000000ULL;
    constexpr static carrier s_13_bit_mask        = 0x7FFC000000000000ULL;
    constexpr static carrier s_32_bit_mask        = 0x0003FFFFFFFC0000ULL;
    constexpr static carrier s_18_bit_mask        = 0x000000000003FFFFULL;
    
    constexpr static unsigned int s_18_bit_offset = 0;
    constexpr static unsigned int s_32_bit_offset = s_18_bit_offset + 18;
    constexpr static unsigned int s_13_bit_offset = s_32_bit_offset + 32;
        
   public:

    [[nodiscard]] constexpr bool get_flag() const
    {
      return value & s_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier set_flag() const
    {
      return value | s_flag_mask;
    }
    
    [[nodiscard]] constexpr carrier unset_flag() const
    {
      return value & (~s_flag_mask);
    }
        
    [[nodiscard]] constexpr carrier make_flag(const bool flag) const
    {
      return (value & (~(s_flag_mask * (!flag)))) | (s_flag_mask * flag);
    }
        
    [[nodiscard]] constexpr uint32_t get_13_bits() const
    {
      return (value & s_13_bit_mask) >> s_13_bit_offset;
    }
    
    [[nodiscard]] constexpr uint32_t get_32_bits() const
    {
      return (value & s_32_bit_mask) >> s_32_bit_offset;
    }

    [[nodiscard]] constexpr uint32_t get_18_bits() const
    {
      return (value & s_18_bit_mask) >> s_18_bit_offset;
    }
    
    [[nodiscard]] static constexpr carrier make_generic_tag(const uint32_t bits_18, const uint32_t bits_32,
                                                            const uint32_t bits_13, const bool flag)
    {
      constexpr uint32_t bits_18_mask = 0x0003FFFFU;
      constexpr uint32_t bits_13_mask = 0x00001FFFU;
      
      carrier ret = bits_13 & bits_13_mask;
      
      ret = (ret << 32) | bits_32;
      
      ret = (ret << 18) | (bits_18 & bits_18_mask);
      
      ret = ret | (s_flag_mask * flag);

      return ret;
    }

  };
}

#endif
