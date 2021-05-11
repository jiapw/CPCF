#pragma once

/**
 * @file exprtk.h
 * @author JP Wang (wangjiaping@idea.edu.cn)
 * @brief 
 * @version 1.0
 * @date 2021-05-08
 * 
 * @copyright  
 * Cross-Platform Core Foundation (CPCF)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *      * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials provided
 *        with the distribution.
 *      * Neither the name of CPCF.  nor the names of its
 *        contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   
 */
#include "../../rt/small_math.h"
#include "../../rt/string_type_ops.h"
#include "./exprtk/exprtk.hpp"


namespace rt
{
/** \addtogroup exprtk
 * @ingroup ext
 *  @{
 */
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
	/**
	 * @brief Set the Bind Variable As object
	 * all existing binding will be removed
	 * @param e 
	 * @return INLFUNC 
	 */
	INLFUNC void SetBindVariableAs(const Expression& e)	
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


/** @}*/
} // namespace rt
