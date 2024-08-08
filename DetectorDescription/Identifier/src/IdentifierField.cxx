/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "Identifier/IdentifierField.h"
#include <limits>
#include <algorithm>
#include <iostream>
#include <bit>



//------------------------------------------------------------------
ExpandedIdentifier::size_type IdentifierField::get_bits() const{
  size_t indices = get_indices ();
  if (--indices) return  std::bit_width(indices);
  return 1;
}

 
//----------------------------------------------- 
ExpandedIdentifier::size_type 
IdentifierField::get_value_index(element_type value) const{
    // Only both_bounded and enumerated are valid to calculate the
    // index.
    // both_bounded if the more frequent case and so comes first.
    if (both_bounded == m_mode) {
      return (value - m_minimum); 
    }
    else if (enumerated == m_mode) {
      if (not m_indexes.empty()) {
          // Table has been created, do simple lookup
          assert (value >= m_minimum && value - m_minimum < (int)m_indexes.size());
          return (m_indexes.at(value - m_minimum));
      } else {
        auto it = std::ranges::find(m_values, value);
        if (it != m_values.end()) return std::distance(m_values.begin(), it);
      }
    }
 
    return (0); 
}

//----------------------------------------------- 
bool 
IdentifierField::match(element_type value) const {  
    if (both_bounded == m_mode) {
      return ((value >= m_minimum) && (value <= m_maximum)); 
    } else if (enumerated == m_mode) {
      return (std::ranges::find(m_values, value) != m_values.end());
    } else if (unbounded == m_mode) {
      return (true); 
    }  else if (high_bounded == m_mode) {
      return (value <= m_maximum); 
    } else if (low_bounded == m_mode) {
      return (value >= m_minimum); 
    }
    return (false);
} 


 
//----------------------------------------------- 
IdentifierField::IdentifierField (element_type value) 
    :
    m_minimum(value),
    m_maximum(value),
    m_mode(both_bounded){
  //    
} 
 
//----------------------------------------------- 
IdentifierField::IdentifierField (element_type minimum, element_type maximum) 
   :
    m_indices(0),
    m_previous(0),
    m_next(0),
    m_mode(both_bounded),
    m_continuation_mode(none){ 
  set (minimum, maximum); 
  set_indices();    
} 
 
 
//----------------------------------------------- 
bool 
IdentifierField::get_previous (element_type current, element_type& previous) const{
  switch (m_mode) 
    { 
    case unbounded: 
      previous = current - 1; 
      return (current != std::numeric_limits<element_type>::min());
      break; 
    case low_bounded: 
      if (current == m_minimum) {
        previous = current;
        return (false);
      }
      previous = current - 1; 
      return (true); 
      break; 
    case high_bounded: 
      previous = current - 1; 
      return (current != std::numeric_limits<element_type>::min());
      break; 
    case both_bounded: 
      if (current == m_minimum) {
        if (has_wrap_around == m_continuation_mode) {
            previous = m_maximum;
            return (true); 
        }
        if (has_previous == m_continuation_mode) {
            previous = m_previous;
            return (true); 
        }
        previous = current;
        return (false);
      }
      previous = current - 1; 
      return (true); 
      break; 
    case enumerated: 
      size_type index = get_value_index(current);
      if (index == 0) {
        if (has_wrap_around == m_continuation_mode && !m_values.empty()) {
            previous = m_values.back();
            return (true); 
        }
        if (has_previous == m_continuation_mode) {
            previous = m_previous;
            return (true); 
        }
        previous = current;
        return (false);
      }
      --index;
      previous = m_values[index];
      return (true);
      break; 
    } 
  return (false); 
}


//----------------------------------------------- 
bool 
IdentifierField::get_next(element_type current, element_type& next) const{
  switch (m_mode) 
    { 
    case unbounded: 
      next = current + 1; 
      return (current != std::numeric_limits<element_type>::max());
      break; 
    case low_bounded: 
      next = current + 1; 
      return (current != std::numeric_limits<element_type>::max());
      break; 
    case high_bounded: 
      if (current == m_maximum) {
        next = current;
        return (false);
      }
      next = current + 1; 
      return (true); 
      break; 
    case both_bounded: 
      if (current == m_maximum) {
        if (has_wrap_around == m_continuation_mode) {
            next = m_minimum;
            return (true); 
        }
        if (has_next == m_continuation_mode) {
            next = m_next;
            return (true); 
        }
        next = current;
        return (false);
      }
      next = current + 1; 
      return (true); 
      break; 
    case enumerated: 
      size_type index = get_value_index(current);
      if ((index == m_values.size() - 1) || (index == 0 && current != m_values.front())) {
        if (has_wrap_around == m_continuation_mode) {
            next = m_values.front();
            return (true); 
        }
        if (has_next == m_continuation_mode) {
            next = m_next;
            return (true); 
        }
        next = current;
        return (false);
      }
      ++index;
      next = m_values[index];
      return (true);
      break; 
    } 
 
  return (false); 
}

 
 
//----------------------------------------------- 
bool IdentifierField::overlaps_with (const IdentifierField& other) const { 
  enum check_type{ 
      done,  
      min_max,  
      max_min,  
      both, 
      both_and_enum, 
      enum_and_both, 
      all_values 
    } ; 
 
  static constexpr check_type check_needed[5][5] = 
  { 
    {done, done,    done,    done,          done}, 
    {done, done,    min_max, min_max,       min_max}, 
    {done, max_min, done,    max_min,       max_min}, 
    {done, max_min, min_max, both,          both_and_enum}, 
    {done, max_min, min_max, enum_and_both, all_values} 
  }; 
 
  mode other_mode = other.get_mode (); 
 
  switch (check_needed [m_mode][other_mode]) { 
    case done: 
      return (true); 
    case min_max: 
      if (m_minimum <= other.get_maximum ()) return (true); 
      break; 
    case max_min: 
      if (m_maximum >= other.get_minimum ()) return (true); 
      break; 
    case both: 
      if ((m_minimum <= other.get_maximum ()) && 
          (m_maximum >= other.get_minimum ())) { 
          return (true); 
        } 
      break; 
    case both_and_enum: // this is both_bounded while other is enumerated 
      if ((m_minimum <= other.get_maximum ()) && 
          (m_maximum >= other.get_minimum ())) { 
          // Check if this(bb) is entirely within other(enum). 
          if ((m_minimum > other.get_minimum ()) && 
              (m_maximum < other.get_maximum ())) { 
              const element_vector& ev = other.get_values (); 
              for (const auto & v: ev) { 
                  if ((v < m_minimum) || (v > m_maximum)) return (false); 
                } 
            } 
          return (true); 
        } 
      break; 
    case enum_and_both: // this is enumerated while other is both_bounded 
      if ((m_minimum <= other.get_maximum ()) && 
          (m_maximum >= other.get_minimum ()))  { 
          // Check if other(bb) is entirely within this(enum). 
          if ((other.get_minimum () > m_minimum) && 
              (other.get_maximum () < m_maximum)) { 
              const element_vector& ev = get_values (); 
              for (const auto & v: ev){ 
                  if ((v < other.get_minimum ()) || (v > other.get_maximum ())) return (false); 
                } 
            } 
          return (true); 
        } 
      break; 
    case all_values: 
      // Both fields are enumerated only if there is possibility of overlap 
      if ((m_minimum <= other.get_maximum ()) && 
        (m_maximum >= other.get_minimum ()))  { 
        const element_vector& ev = other.get_values (); 
        for (const auto & thisValue:m_values)  { 
          for (const auto & otherValue: ev) { 
            if (thisValue == otherValue) return (true); 
          } 
        } 
      } 
      break; 
    }
  return (false); 
} 
 

//----------------------------------------------- 
void 
IdentifierField::clear () { 
  m_minimum  = 0; 
  m_maximum  = 0; 
  m_previous = 0;
  m_next     = 0;
  m_continuation_mode = none;
  m_mode = unbounded; 
  m_values.clear (); 
} 
 
//----------------------------------------------- 
void 
IdentifierField::set (element_type minimum, element_type maximum)  {
  m_values.clear ();
  if (minimum == maximum) {
      add_value (minimum);
  } else {
    m_minimum = std::min(minimum,maximum); 
    m_maximum = std::max(maximum, minimum); 
    m_mode = both_bounded; 
  }
  set_indices();
} 
 
//----------------------------------------------- 
void 
IdentifierField::set_minimum (element_type value){ 
  if (m_mode == unbounded) { 
      m_mode = low_bounded; 
      m_minimum = value; 
  } else if ((m_mode == high_bounded) ||  
         (m_mode == both_bounded) ||  
         (m_mode == enumerated)) { 
    set (value, get_maximum ()); 
  }  else  { 
    m_minimum = value; 
  } 
  set_indices();
} 
 
//----------------------------------------------- 
void 
IdentifierField::set_maximum (element_type value) { 
  if (m_mode == unbounded) { 
      m_mode = high_bounded; 
      m_maximum = value; 
    }  else if ((m_mode == low_bounded) ||  
           (m_mode == both_bounded) || 
           (m_mode == enumerated)) { 
      set (get_minimum (), value); 
    } else  { 
      m_maximum = value; 
    } 

  set_indices();
} 
 
//----------------------------------------------- 
void 
IdentifierField::add_value (element_type value) { 
  if (m_mode == low_bounded) { 
    m_values.clear (); 
    m_values.push_back (m_minimum); 
    m_maximum = m_minimum; 
    m_mode = enumerated; 
  }  else if (m_mode != enumerated)  { 
    m_values.clear (); 
    m_mode = enumerated; 
  } 
  //value already exists in the enumeration vector
  if (std::ranges::find(m_values, value) !=  m_values.end()) return;
 
  m_values.push_back (value); 
  std::sort (m_values.begin (), m_values.end()); 
  m_minimum = m_values.front(); 
  m_maximum = m_values.back(); 
  set_indices();
} 
 
//----------------------------------------------- 
void 
IdentifierField::set (const std::vector <element_type>& values) { 
  if (values.empty())  { 
      clear (); 
      return; 
    } 
 
  for (size_type i = 0; i < values.size (); ++i) { 
      add_value (values[i]); 
    } 

  set_indices();
} 
 
//----------------------------------------------- 
void 
IdentifierField::set (bool wraparound) { 
  if (wraparound) {
    m_continuation_mode = has_wrap_around;
  }
} 
 
//----------------------------------------------- 
void 
IdentifierField::set_next (int next) {
  if (has_previous == m_continuation_mode) {
    m_continuation_mode = has_both;
  } else {
    m_continuation_mode = has_next;
  }
  m_next = next;
}


//----------------------------------------------- 
void 
IdentifierField::set_previous (int previous) {
    if (has_next == m_continuation_mode) {
      m_continuation_mode = has_both;
    } else {
      m_continuation_mode = has_previous;
    }
    m_previous = previous;
}


//----------------------------------------------- 
void 
IdentifierField::operator |= (const IdentifierField& other) {
  mode other_mode = other.get_mode ();

  if (m_mode == other_mode) {
        /*
          x . . . .
          . x . . .
          . . x . .
          . . . x .
          . . . . x
        */
      switch (m_mode) { 
          case unbounded: 
            break; 
          case high_bounded: 
            if (other.get_maximum () > m_maximum) m_maximum = other.get_maximum ();
            break; 
          case low_bounded: 
            if (other.get_minimum () < m_minimum) m_minimum = other.get_minimum ();
            break; 
          case enumerated:
          {
            const element_vector& ev = other.get_values ();

            for (size_t i = 0; i < ev.size (); ++i) 
              { 
                add_value (ev[i]);
              } 
          }
            break; 
          default:  // both_bounded 
              /**
               *  If there is no overlap we should build a multi-segment specification.
               *  The current algorithm is only correct if the overlap in not empty !!
               *   A multi-segment specification might also be implemented as an 
               *  expanded enumerated set (not very optimized !!)
               */
            if (other.get_maximum () > m_maximum) m_maximum = other.get_maximum ();
            if (other.get_minimum () < m_minimum) m_minimum = other.get_minimum ();

            break; 
        } 
    }
  else if ((m_mode == unbounded) || (other_mode == unbounded))
    {
        /*
          o x x x x
          x o . . .
          x . o . .
          x . . o .
          x . . . o
         */
      clear ();
    } else if ((m_mode == low_bounded) && (other_mode == high_bounded)) {
        /**
         *  If there is no overlap we should build a multi-segment specification.
         *  The current algorithm is only correct if the overlap in not empty !!
         *
         *   (in addition, the expanded solution - to enumerated - is not possible
         *    due to the unbounded nature of this mode)
         */


        /*
          o o o o o
          o o x . .
          o . o . .
          o . . o .
          o . . . o
        */
      clear ();
    } else if ((m_mode == high_bounded) && (other_mode == low_bounded)) {
        /**
         *  If there is no overlap we should build a multi-segment specification.
         *  The current algorithm is only correct if the overlap in not empty !!
         *
         *   (in addition, the expanded solution - to enumerated - is not possible
         *    due to the unbounded nature of this mode)
         */


        /*
          o o o o o
          o o o . .
          o x o . .
          o . . o .
          o . . . o
         */
      clear ();
    } else {
        // all other cases...

      if (has_minimum () && other.has_minimum ()) {
            /*
              o o o o o
              o o o x x
              o o o . .
              o x . o x
              o x . x o
            */

          if (other.get_minimum () < m_minimum) set_minimum (other.get_minimum ());
        }

      if (has_maximum () && other.has_maximum ()) {
            /*
              o o o o o
              o o o . .
              o o o x x
              o . x o x
              o . x x o
            */

          if (other.get_maximum () > m_maximum) set_maximum (other.get_maximum ());
        }
    }

  set_indices();
}

//----------------------------------------------- 

//----------------------------------------------- 
IdentifierField::operator std::string () const { 
  std::string result; 
  char temp[20]; 

  if (!is_valued ()) { 
      result += "*"; 
    }  else  { 
      element_type minimum = get_minimum (); 
      element_type maximum = get_maximum (); 
 
      if (!has_maximum ()) { 
          sprintf (temp, "%d", minimum); 
          result += temp; 
          result += ":"; 
        }  else if (!has_minimum ())  { 
          sprintf (temp, "%d", maximum); 
          result += ":"; 
          result += temp; 
        } else if (minimum == maximum)  { 
          sprintf (temp, "%d", minimum); 
          result += temp; 
        }  else  { 
          if (get_mode () == IdentifierField::enumerated)  { 
              for (size_type i = 0; i < get_indices (); ++i)  { 
                  if (i > 0) result += ","; 
                  sprintf (temp, "%d", get_value_at (i)); 
                  result += temp; 
                } 
            } else  { 
              sprintf (temp, "%d", minimum); 
              result += temp; 
              sprintf (temp, "%d", maximum); 
              result += ":"; 
                  result += temp; 
            } 
        } 
    } 
 
  return (result); 
} 

//-----------------------------------------------
bool 
IdentifierField::operator == (const IdentifierField& other) const {
    if (m_mode != other.m_mode)   return false;
    if (m_minimum != other.m_minimum)   return false;
    if (m_maximum != other.m_maximum)   return false;
    if (m_values  != other.m_values)  return false;
    return (true);
}

//----------------------------------------------- 
bool 
IdentifierField::operator != (const IdentifierField& other) const {
    return (!((*this) == other));
}
 

//----------------------------------------------- 
void 
IdentifierField::show() const {
    
  std::cout << "min/max " << m_minimum << " " << m_maximum << " "; 
  std::cout << "values  ";
  for (const auto v: m_values) {
      std::cout << v << " ";
  }
  std::cout << "indexes  ";
  for (const auto idx: m_indexes) {
      std::cout << idx << " ";
  }
  std::cout << "indices  " << m_indices << " ";
  std::cout << "prev  " << m_previous << " ";
  std::cout << "next  " << m_next << " ";

  std::cout << "mode  ";
  switch (m_mode) { 
  case IdentifierField::unbounded: 
      std::cout << "unbounded  ";
      break; 
  case IdentifierField::low_bounded: 
      std::cout << "low_bounded  ";
      break; 
  case IdentifierField::high_bounded: 
      std::cout << "high_bounded  ";
      break; 
  case IdentifierField::both_bounded: 
      std::cout << "both_bounded  ";
      break; 
  case IdentifierField::enumerated: 
      std::cout << "enumerated  ";
      break; 
  } 

  std::cout << "cont mode  ";
  switch (m_continuation_mode) { 
  case IdentifierField::none: 
      std::cout << "none  ";
      break; 
  case IdentifierField::has_next: 
      std::cout << "has_next  ";
      break; 
  case IdentifierField::has_previous: 
      std::cout << "has_previous  ";
      break; 
  case IdentifierField::has_both:
      std::cout << "has_both  ";
      break; 
  case IdentifierField::has_wrap_around:
      std::cout << "has_wrap_around  ";
      break; 
  }
  std::cout << std::endl;
}



//----------------------------------------------- 
void 
IdentifierField::optimize() {
    /// Check mode - switch from enumerated to both_bounded if possible
    check_for_both_bounded();
    /// Create index table from value table
    create_index_table();
}

//----------------------------------------------- 
void 
IdentifierField::set_indices() {
    /// Set the number of indices
    m_indices = 1;
    if (m_mode == both_bounded) { 
      m_indices = m_maximum - m_minimum + 1; 
    }  else if (m_mode == enumerated) { 
      m_indices = m_values.size (); 
    } 
} 

//----------------------------------------------- 
void 
IdentifierField::check_for_both_bounded() {
  if (m_mode == enumerated && !m_values.empty()) {
    element_type last = m_values.front(); 
    for (const auto & thisValue: m_values) { 
        if (thisValue > last + 1) return;
        last = thisValue;
    }
    // Is both bounded - switch mode
    m_minimum = m_values.front();
    m_maximum = m_values.back();
    m_mode = both_bounded; 
    m_values.clear (); 
  }
}

//----------------------------------------------- 
void 
IdentifierField::create_index_table() {
  /// Create index table from value table
  if (m_mode == enumerated && !m_values.empty()) {
    size_type size = m_maximum - m_minimum + 1;
    // return if we are over the maximum desired vector table size  
    if (size > m_maxNumberOfIndices) {
        m_indexes.clear();
        return;
    }
    // Set up vectors for decoding
    m_indexes = std::vector<size_type>(size, 0);
    size_type index{};
    int i{};
    for (const auto & thisValue: m_values) {
      if (const auto idx=(thisValue - m_minimum); idx < (int)size) {
        m_indexes[idx] = index;
        index++;
      } else {
        std::cout << "size, value, index, i " 
        << size << " " << thisValue << " "
        << index << " " << i++ << "  min, max " 
        << m_minimum << " " 
        << m_maximum 
        << std::endl;
      }
    }
  }
}