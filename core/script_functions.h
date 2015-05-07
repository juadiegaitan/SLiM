//
//  script_functions.h
//  SLiM
//
//  Created by Ben Haller on 4/6/15.
//  Copyright (c) 2015 Philipp Messer.  All rights reserved.
//	A product of the Messer Lab, http://messerlab.org/software/
//

//	This file is part of SLiM.
//
//	SLiM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
//	SLiM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with SLiM.  If not, see <http://www.gnu.org/licenses/>.

/*
 
 This file contains most of the code for processing function calls in the SLiMScript interpreter.
 
 */

#ifndef __SLiM__script_functions__
#define __SLiM__script_functions__

#include "script_interpreter.h"
#include "script_value.h"

#include <iostream>


// Utility functions usable by everybody
ScriptValue *ConcatenateScriptValues(std::string p_function_name, std::vector<ScriptValue*> p_arguments);


// A numeric identifier for a function once its name has been looked up; just for efficiency, to allow switch()
enum class FunctionIdentifier {
	kNoFunction = 0,
	kDelegatedFunction,		// implemented through a delegate, such as SLiM
	
	// data construction functions
	repFunction,
	repEachFunction,
	seqFunction,
	seqAlongFunction,
	cFunction,
	floatFunction,
	integerFunction,
	logicalFunction,
	stringFunction,
	objectFunction,
	
	// data inspection/manipulation functions
	printFunction,
	catFunction,
	sizeFunction,
	strFunction,
	sumFunction,
	productFunction,
	rangeFunction,
	minFunction,
	maxFunction,
	whichMinFunction,
	whichMaxFunction,
	whichFunction,
	meanFunction,
	sdFunction,
	revFunction,
	sortFunction,
	anyFunction,
	allFunction,
	ifelseFunction,
	strsplitFunction,
	pasteFunction,
	
	// data class testing/coercion functions
	classFunction,
	isLogicalFunction,
	isStringFunction,
	isIntegerFunction,
	isFloatFunction,
	isObjectFunction,
	asLogicalFunction,
	asStringFunction,
	asIntegerFunction,
	asFloatFunction,
	isFiniteFunction,
	isNaNFunction,
	isNULLFunction,
	
	// math functions, all implemented using the standard C++ function of the same name
	acosFunction,
	asinFunction,
	atanFunction,
	atan2Function,
	cosFunction,
	sinFunction,
	tanFunction,
	expFunction,
	logFunction,
	log10Function,
	log2Function,
	sqrtFunction,
	absFunction,
	ceilFunction,
	floorFunction,
	roundFunction,
	truncFunction,
	
	// bookkeeping functions
	stopFunction,
	versionFunction,
	licenseFunction,
	helpFunction,
	lsFunction,
	rmFunction,
	functionFunction,
	dateFunction,
	timeFunction,
	
	// object instantiation
	PathFunction
};

// Functions that are built into SLiMscript are handled internally.  However, it is also possible for external agents to register their own
// functions with SLiMscript; this is how SLiM gets its functions into SLiMscript.  This registration is done with a delegate object and
// a delegate function pointer (NOT a pointer to method).  This is the prototype for a function implementation delegate.  The function
// pointed to will be called, and passed the delegate object along with other parameters for the function call.
typedef ScriptValue *(*SLiMDelegateFunctionPtr)(void *delegate, std::string const &p_function_name, std::vector<ScriptValue*> const &p_arguments, std::ostream &p_output_stream, ScriptInterpreter &p_interpreter);

// FunctionSignature is used to represent the return type and argument types of a function or method, for shared runtime type checking
class FunctionSignature
{
public:
	std::string function_name_;
	FunctionIdentifier function_id_;
	ScriptValueMask return_mask_;					// a mask specifying the exact return type; the singleton flag is used, the optional flag is not
	std::vector<ScriptValueMask> arg_masks_;		// the expected types for each argument, as a mask
	bool has_optional_args_ = false;				// if true, at least one optional argument has been added
	bool has_ellipsis_ = false;						// if true, the function accepts arbitrary varargs after the specified arguments
	
	// ivars related to delegated function implementations
	SLiMDelegateFunctionPtr delegate_function_ = nullptr;
	void *delegate_object_ = nullptr;
	std::string delegate_name_;
	
	FunctionSignature(const FunctionSignature&) = delete;					// no copying
	FunctionSignature& operator=(const FunctionSignature&) = delete;		// no copying
	FunctionSignature(void) = delete;										// no null construction
	
	FunctionSignature(std::string p_function_name, FunctionIdentifier p_function_id, ScriptValueMask p_return_mask);
	FunctionSignature(std::string p_function_name, FunctionIdentifier p_function_id, ScriptValueMask p_return_mask, SLiMDelegateFunctionPtr p_delegate_function, void *p_delegate_object, std::string p_delegate_name);
	
	FunctionSignature *AddArg(ScriptValueMask p_arg_mask);
	FunctionSignature *AddEllipsis();
	
	// vanilla type-specified arguments
	FunctionSignature *AddLogical();
	FunctionSignature *AddInt();
	FunctionSignature *AddFloat();
	FunctionSignature *AddString();
	FunctionSignature *AddObject();
	FunctionSignature *AddNumeric();
	FunctionSignature *AddLogicalEquiv();
	FunctionSignature *AddAnyBase();
	FunctionSignature *AddAny();
	
	// optional arguments
	FunctionSignature *AddLogical_O();
	FunctionSignature *AddInt_O();
	FunctionSignature *AddFloat_O();
	FunctionSignature *AddString_O();
	FunctionSignature *AddObject_O();
	FunctionSignature *AddNumeric_O();
	FunctionSignature *AddLogicalEquiv_O();
	FunctionSignature *AddAnyBase_O();
	FunctionSignature *AddAny_O();
	
	// singleton arguments (i.e. required to have a size of exactly 1)
	FunctionSignature *AddLogical_S();
	FunctionSignature *AddInt_S();
	FunctionSignature *AddFloat_S();
	FunctionSignature *AddString_S();
	FunctionSignature *AddObject_S();
	FunctionSignature *AddNumeric_S();
	FunctionSignature *AddLogicalEquiv_S();
	FunctionSignature *AddAnyBase_S();
	FunctionSignature *AddAny_S();
	
	// optional singleton arguments
	FunctionSignature *AddLogical_OS();
	FunctionSignature *AddInt_OS();
	FunctionSignature *AddFloat_OS();
	FunctionSignature *AddString_OS();
	FunctionSignature *AddObject_OS();
	FunctionSignature *AddNumeric_OS();
	FunctionSignature *AddLogicalEquiv_OS();
	FunctionSignature *AddAnyBase_OS();
	FunctionSignature *AddAny_OS();
	
	// check an argument list; p_call_type should be "function" or "method", for error output only
	void CheckArguments(std::string const &p_call_type, std::vector<ScriptValue*> const &p_arguments) const;
	
	// check a return value; p_call_type should be "function" or "method", for error output only
	void CheckReturn(std::string const &p_call_type, ScriptValue *p_result) const;
};

std::ostream &operator<<(std::ostream &p_outstream, const FunctionSignature &p_signature);


#endif /* defined(__SLiM__script_functions__) */





























































