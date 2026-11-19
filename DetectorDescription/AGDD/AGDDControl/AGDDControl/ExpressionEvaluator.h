/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EXPRESSION_EVALUATOR_H
#define EXPRESSION_EVALUATOR_H

#include "CLHEP/Evaluator/Evaluator.h"

#include <string>
#include <string_view>
#include <vector>
#include <map>

namespace AGDD {

typedef std::map< std::string, double> ConstantsTable;
typedef std::map< std::string, double> PhysicalConstantsTable;
typedef std::map< std::string, std::string> ExpressionsTable;

class ExpressionEvaluator
{
public:
	ExpressionEvaluator();
  	~ExpressionEvaluator();

  	bool RegisterConstant( std::string& c, double v );
  	bool RegisterPhysConstant( std::string&, const std::string&, const std::string&  );
  	bool RegisterExpression( std::string& c, const std::string& v  );
  	bool RegisterArray( std::string& c, const std::vector<double>& v );
  	bool RegisterVariable( std::string var_name, double value);
  
  	double EvaluateString(std::string_view str);
  	static bool is_delimiter(char c) noexcept;
  	bool is_real_variable(std::string_view var_name) const noexcept;
  	double Eval( std::string_view expr );
	
	static std::vector<std::string> tokenize(std::string_view,std::string_view); 
  
  	void setFileCurrentlyParsed(std::string set)
  	{
   		m_fileCurrentlyParsed = std::move(set);
  	}

private:
  	HepTool::Evaluator     m_calc;
  	ConstantsTable         m_CTable;
  	PhysicalConstantsTable m_PCTable;
  	std::string m_fileCurrentlyParsed;
  	std::vector<std::string> m_real_vars;
};

} // namespace AGDD

#endif // EXPRESSION_EVALUATOR_H

