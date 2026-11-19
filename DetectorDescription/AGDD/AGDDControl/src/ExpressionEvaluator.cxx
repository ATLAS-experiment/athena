/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "AGDDControl/ExpressionEvaluator.h"

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <CxxUtils/StringUtils.h>
#include <CxxUtils/StringUtilsTemplates.h>

using namespace AGDD;

ExpressionEvaluator::ExpressionEvaluator()
{
  m_calc.clear();
  m_calc.setStdMath();                 // set standard constants and functions
  // Set Geant4 system of units
  m_calc.setSystemOfUnits(1.e+3, 1./1.60217733e-25, 1.e+9, 1./1.60217733e-10,1.0, 1.0, 1.0);
  m_fileCurrentlyParsed = "";
}

ExpressionEvaluator::~ExpressionEvaluator()
{
  m_CTable.clear();
  m_PCTable.clear();
  m_calc.clear();
}

bool ExpressionEvaluator::RegisterConstant( std::string& c, double v )
{
  double value = v;

  if( m_calc.status() != HepTool::Evaluator::OK )
  {
    std::cerr << "Expression evaluator:: Error registering constant " << c << std::endl;
    m_calc.print_error();
    std::cout << std::endl;
    return false;
  }

  RegisterVariable( c, value );
  return true;
}

bool ExpressionEvaluator::RegisterVariable( std::string var_name, double value)
{
 m_calc.setVariable( var_name.c_str(), value );
 m_real_vars.push_back(std::move(var_name));
 return true;
}


bool ExpressionEvaluator::RegisterArray(std::string& c, const std::vector<double>& v) {
    char buf[32];
    for (size_t i = 0; i < v.size(); ++i) {
        double value = v[i];
        auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), i);
        std::string_view index(buf, ptr - buf);
        std::string name = CxxUtils::strcat(c, '_', index, '_');
        RegisterVariable(std::move(name), value);
    }
    return true;
}

bool ExpressionEvaluator::is_real_variable(std::string_view var_name) const noexcept {
    return std::ranges::contains(m_real_vars, var_name);
}

bool ExpressionEvaluator::is_delimiter(char ch) noexcept {
    unsigned char c = static_cast<unsigned char>(ch);
    return std::isspace(c) || c == '\0' || 
           std::string_view(" ()|&=!><+-%*/^").find(ch) != std::string_view::npos;
}

double ExpressionEvaluator::EvaluateString(std::string_view str) {
    std::string str_mod{str};
    std::vector<int> variable_ends;
    size_t cur_variable_end = 0;
    
    auto c_str_view = str;
    size_t idx = 0;
    
    while (idx < c_str_view.size()) {
        char c = c_str_view[idx];
        if (is_delimiter(c) || std::isdigit(static_cast<unsigned char>(c))) {
            idx++;
            cur_variable_end++;
        } else if (std::isalpha(static_cast<unsigned char>(c))) {
            size_t start_token = idx;
            while (idx < c_str_view.size() && !is_delimiter(c_str_view[idx])) {
                idx++;
                cur_variable_end++;
            }
            std::string_view variable_to_guard = c_str_view.substr(start_token, idx - start_token);
            if (is_real_variable(std::string(variable_to_guard))) {
                variable_ends.push_back(static_cast<int>(cur_variable_end));
            }
        } else {
            idx++;
            cur_variable_end++;
        }
    }

    size_t shift = 0;
    size_t ns_length = m_fileCurrentlyParsed.size();
    for (int var_end : variable_ends) {
        str_mod.insert(shift + var_end, m_fileCurrentlyParsed);
        shift += ns_length;
    }
    
    return m_calc.evaluate(str_mod.c_str());
}

/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/


bool ExpressionEvaluator::RegisterPhysConstant( std::string& c, const std::string& value, const std::string& unit  )
{
  std::string expr = CxxUtils::strcat(value, "*(", unit, ')');
  double dvalue      = EvaluateString( expr );

  if( m_calc.status() != HepTool::Evaluator::OK )
  {
    std::cerr << "Expression evaluator:: Error registering quantity "
              << c << std::endl;
    m_calc.print_error();
    std::cout << std::endl;
    return false;
  }

  RegisterVariable( c, dvalue );
  return true;
}

bool ExpressionEvaluator::RegisterExpression( std::string& name, const std::string& text )
{
  std::string expr = CxxUtils::strcat('(',text,')');
  double value = EvaluateString( expr );

  if( m_calc.status() != HepTool::Evaluator::OK )
  {
    std::cerr << "Expression evaluator:: Error registering expression " << name << std::endl;
    m_calc.print_error();
    std::cout << std::endl;
    return false;
  }

  RegisterVariable( name, value );
  return true;
}

double ExpressionEvaluator::Eval( std::string_view expr_mod )
{
  
  std::string expr {expr_mod};
  std::string::size_type start_index = 0;
  std::string::size_type end_index = 0;
  while(true)
  {
   start_index = expr.find('[', start_index); 
   if(start_index == std::string::npos) break;
   std::string::size_type boundary_index = expr.find(']', start_index);
   expr.replace(start_index,1,1,'_');
   end_index = expr.find(',', start_index);
   if(end_index != std::string::npos && end_index < boundary_index)
   {
    start_index++;
    std::string var1 = expr.substr(start_index, end_index-start_index);
    double eval1 = EvaluateString( var1 );
    std::stringstream ss1;
    std::string str1;
    ss1 << eval1;
    ss1 >> str1;
    expr.replace(start_index, end_index-start_index, str1, 0, str1.size());
   }
   else
   {
    end_index = boundary_index;
    if(end_index != std::string::npos)
    {
     start_index++;
     std::string var1 = expr.substr(start_index, end_index-start_index);
     double eval1 = EvaluateString( var1 );
     std::stringstream ss1;
     std::string str1;
     ss1 << eval1;
     ss1 >> str1;
     expr.replace(start_index, end_index-start_index, str1, 0, str1.size());
    }
   }
  }
  start_index = 0;
  end_index = 0;
  while(true)
  {
   start_index = expr.find(',', start_index);
   if(start_index == std::string::npos) break;
   expr.replace(start_index,1,1,'_');
   end_index = expr.find(']', start_index);
   start_index++;
   std::string var2 = expr.substr(start_index, end_index-start_index);
   double eval2 = EvaluateString( var2 );
   std::stringstream ss2;
   std::string str2;
   ss2 << eval2;
   ss2 >> str2;
   expr.replace(start_index, end_index-start_index, str2, 0, str2.size());
  }
  start_index = 0;
  end_index = 0;
  while(true)
  {
   start_index = expr.find(']', start_index);
   if(start_index == std::string::npos) break;
   expr.replace(start_index,1,1,'_');
  }
  double result = EvaluateString( expr );
  if( m_calc.status() != HepTool::Evaluator::OK )
  {
    std::cerr << expr << std::endl;
    for (int i=0; i<m_calc.error_position(); i++)
    {
      std::cerr << "-";
    }
    std::cerr << "^\a" << std::endl;
    m_calc.print_error();
    std::cerr << std::endl;
  }
  return result;
}


    
std::vector<std::string> ExpressionEvaluator::tokenize(std::string_view sep, std::string_view expr) {
    auto tokens = CxxUtils::tokenize<std::string>(expr, sep);
    
    for (auto& token : tokens) {
        token = CxxUtils::trimWhiteSpaces(token);
    }
    
    return tokens;
}
