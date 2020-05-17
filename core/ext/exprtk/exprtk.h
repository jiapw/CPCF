#pragma once
#include "../../rt/small_math.h"
#include "../../rt/string_type_ops.h"
#include "./exprtk/exprtk.hpp"


namespace rt
{

namespace _details
{	
	void _translate_code(const rt::String_Ref& code, std::string& out);
	void _translate_symbol(const rt::String_Ref& code, std::string& out);
};

template<typename T>
class Expression
{
	exprtk::expression<T>		_Expr;
	exprtk::symbol_table<T>		_Vars;
	rt::String					_LastErrMsg;
public:
	typedef T t_Value;
	Expression()
	{	_Expr.register_symbol_table(_Vars);
	}
	bool Compile(const rt::String_Ref& code)
	{	std::string translated;
		_details::_translate_code(code, translated);

		_LastErrMsg.Empty();
		exprtk::parser<T> parser;
		if(parser.compile(translated,_Expr))return true;

		rt::String estr = translated.c_str();
		int padding = 0;

		for (std::size_t i = 0; i < parser.error_count(); ++i)
		{
			exprtk::parser_error::type error = parser.get_error(i);
			_LastErrMsg += rt::SS("ExprTk ERR: ") + (int)error.token.position + 
						   rt::SS(" [") + exprtk::parser_error::to_str(error.mode).c_str() + rt::SS("] ") + error.diagnostic.c_str() + '\n';
			if(error.token.position != -1)
			{	estr.Insert(error.token.position + padding, rt::SS(" [ERR->]"));
				padding += 3;
			}
		}
		_LastErrMsg += rt::SS("\nTranslated:\n") +  estr;

		return false;
	}
	INLFUNC void ClearBinding(){ _Vars.clear();	}
	INLFUNC void SetBindVariableAs(const Expression& e)	// all existing binding will be removed
	{	_Vars = e._Vars;
	}
	INLFUNC void BindVariable(const rt::String_Ref& symbol, T& v)
	{	std::string out;
		rt::_details::_translate_symbol(symbol, out);
		_Vars.add_variable(out, v, false);
	}
	INLFUNC void BindVariable(const rt::String_Ref& symbol, T* p, SIZE_T len)
	{	std::string out;
		rt::_details::_translate_symbol(symbol, out);
		_Vars.add_vector(out, p, len);
	}
	INLFUNC T Compute(){ return _Expr.value(); }
	const rt::String& GetLastError() const { return _LastErrMsg; }
};



} // namespace rt