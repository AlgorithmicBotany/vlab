/************************************************************************************
*	Project:		L-Studio														*
*	Module:			warningset.h													*
*	Author:			RadekK															*
*	Created:		07-Oct-97														*
*	Description:																	*
*																					*
*		Individual settings for VC++ warnings										*
*																					*
************************************************************************************/


#ifndef __WARNINGSET_H__
#define __WARNINGSET_H__


#pragma warning ( error   : 4005 ) /* 'identifier' : macro redefinition                                                                                             */
#if _MSC_VER > 1200
#pragma warning ( error   : 4018 ) /* 'expression' : signed/unsigned mismatch                                                                                       */
#endif
#pragma warning ( error   : 4019 ) /* empty statement at global scope                                                                                               */
#pragma warning ( error   : 4032 ) /* formal parameter 'number' has different type when promoted                                                                    */
#pragma warning ( error   : 4057 ) /* 'operator' : 'identifier1' indirection to slightly different base types from 'identifier2'                                    */
#pragma warning ( error   : 4061 ) /* enumerate 'identifier' in switch of enum 'identifier' is not explicitly handled by a case label                               */
#pragma warning ( error   : 4092 ) /* sizeof returns 'unsigned long'                                                                                                */
#if _MSC_VER > 1200
#pragma warning ( error   : 4100 ) /* 'identifier' : unreferenced formal parameter                                                                                  */
#endif
#pragma warning ( error   : 4101 ) /* 'identifier' : unreferenced local variable                                                                                    */
#pragma warning ( error   : 4121 ) /* 'symbol' : alignment of a member was sensitive to packing                                                                     */
#pragma warning ( error   : 4125 ) /* decimal digit terminates octal escape sequence                                                                                */
#pragma warning ( error   : 4127 ) /* conditional expression is constant                                                                                            */
#pragma warning ( error   : 4128 ) /* storage-class specifier after type                                                                                            */
#pragma warning ( error   : 4129 ) /* 'character' : unrecognized character escape sequence                                                                          */
#pragma warning ( error   : 4130 ) /* 'operator ' : logical operation on address of string constant                                                                 */
#pragma warning ( error   : 4131 ) /* 'function' : uses old-style declarator                                                                                        */
#pragma warning ( error   : 4132 ) /* 'object' : const object should be initialized                                                                                 */
#pragma warning ( error   : 4134 ) /* conversion between pointers to members of same class                                                                          */
#pragma warning ( error   : 4152 ) /* non standard extension, function/data ptr conversion in expression                                                            */
#pragma warning ( error   : 4172 ) /* returning address of local variable or temporary                                                                              */
#pragma warning ( error   : 4189 ) /* 'identifier' : local variable is initialized but not referenced"                                                              */
#pragma warning ( error   : 4200 ) /* nonstandard extension used : zero-sized array in struct/union                                                                 */
#pragma warning ( disable : 4201 ) /* nonstandard extension used : nameless struct/union                                                                            */
#pragma warning ( error   : 4202 ) /* nonstandard extension used : '...': prototype parameter in name list illegal                                                  */
#pragma warning ( error   : 4206 ) /* nonstandard extension used : translation unit is empty                                                                        */
#pragma warning ( error   : 4207 ) /* nonstandard extension used : extended initializer form                                                                        */
#pragma warning ( error   : 4208 ) /* nonstandard extension used : delete [exp] - exp evaluated but ignored                                                         */
#pragma warning ( error   : 4209 ) /* nonstandard extension used : benign typedef redefinition                                                                      */
#pragma warning ( error   : 4210 ) /* nonstandard extension used : function given file scope                                                                        */
#pragma warning ( error   : 4211 ) /* nonstandard extension used : redefined extern to static                                                                       */
#pragma warning ( error   : 4212 ) /* nonstandard extension used : function declaration used ellipsis                                                               */
#pragma warning ( error   : 4213 ) /* nonstandard extension used : cast on l-value                                                                                  */
#pragma warning ( error   : 4214 ) /* nonstandard extension used : bit field types other than int                                                                   */
#pragma warning ( error   : 4220 ) /* varargs matches remaining parameters                                                                                          */
#pragma warning ( error   : 4221 ) /* nonstandard extension used : 'identifier' : cannot be initialized using address of automatic variable                         */
#pragma warning ( error   : 4222 ) /* nonstandard extension used : 'identifier' : 'static' should not be used on member functions defined at file scope             */
#pragma warning ( error   : 4223 ) /* nonstandard extension used : non-lvalue array converted to pointer                                                            */
#pragma warning ( error   : 4232 ) /* nonstandard extension used : 'identifier' : address of dllimport 'dllimport' is not static, identity not guaranteed           */
#pragma warning ( error   : 4233 ) /* nonstandard extension used : 'keyword' keyword only supported in C++, not C                                                   */
#pragma warning ( error   : 4234 ) /* nonstandard extension used: 'keyword' keyword reserved for future use                                                         */
#pragma warning ( error   : 4235 ) /* nonstandard extension used : 'keyword' keyword not supported in this product                                                  */
#pragma warning ( error   : 4236 ) /* nonstandard extension used : 'keyword' is an obsolete keyword, see documentation for __declspec(dllexport)                    */
#pragma warning ( error   : 4238 ) /* nonstandard extension used : class rvalue used as lvalue                                                                      */
#pragma warning ( error   : 4239 ) /* nonstandard extension used : 'token' : conversion from 'type' to 'type'                                                       */
#pragma warning ( error   : 4244 ) /* 'conversion' conversion from 'type1' to 'type2', possible loss of data                                                        */
#pragma warning ( error   : 4245 ) /* 'conversion' : conversion from 'type1' to 'type2', signed/unsigned mismatch                                                   */
#pragma warning ( error   : 4268 ) /* 'identifier' : 'const' static/global data initialized with compiler generated default constructor fills the object with zeros */
#pragma warning ( error   : 4296 ) /* 'operator' : expression is always false/true                                                                                  */
#pragma warning ( error   : 4305 ) /* 'identifier' : truncation from 'type1' to 'type2'                                                                             */
#pragma warning ( disable : 4355 ) /* 'this' : used in base member initializer list                                                                                 */
#pragma warning ( error   : 4504 ) /* type still ambiguous after parsing 'number' tokens, assuming declaration                                                      */
#pragma warning ( error   : 4505 ) /* 'function' : unreferenced local function has been removed                                                                     */
#pragma warning ( error   : 4507 ) /* explicit linkage specified after default linkage was used                                                                     */
#pragma warning ( disable : 4512 ) /* 'class' : assignment operator could not be generated                                                                          */
#pragma warning ( disable : 4514 ) /* 'function' : unreferenced inline function has been removed                                                                    */
#pragma warning ( error   : 4515 ) /* 'namespace' : namespace uses itself                                                                                           */
#pragma warning ( error   : 4516 ) /* 'class::symbol' : access-declarations are deprecated; member using-declarations provide a better alternative                  */
#pragma warning ( error   : 4517 ) /* access-declarations are deprecated; member using-declarations provice a better alternative                                    */
#pragma warning ( error   : 4551 ) /* function call missing argument list                                                                                           */
#pragma warning ( error   : 4611 ) /* interaction between '_setjmp' and C++ object destruction is non-portable                                                      */
#if _MSC_VER > 1200
#pragma warning ( error   : 4663 ) /* C++ language change: to explicitly specialize class template 'identifier' use the following syntax:                           */
#endif
#pragma warning ( error   : 4665 ) /* C++ language change: assuming 'declaration' is an explicit specialization of a function template                              */
#pragma warning ( error   : 4670 ) /* 'identifier' : this base class is inaccessible                                                                                */
#pragma warning ( error   : 4671 ) /* 'identifier' : the copy constructor is inaccessible                                                                           */
#pragma warning ( error   : 4672 ) /* 'identifier1' : ambiguous. First seen as 'identifier2'                                                                        */
#pragma warning ( error   : 4673 ) /* throwing 'identifier' the following types will not be considered at the catch site                                            */
#pragma warning ( error   : 4674 ) /* 'identifier' : the destructor is inaccessible                                                                                 */
#pragma warning ( error   : 4701 ) /* local variable 'name' may be used without having been initialized                                                             */
#pragma warning ( error   : 4705 ) /* statement has no effect                                                                                                       */
#pragma warning ( error   : 4706 ) /* assignment within conditional expression                                                                                      */
#pragma warning ( error   : 4709 ) /* comma operator within array index expression                                                                                  */
#pragma warning ( disable : 4710 ) /* 'function' : not expanded                                                                                                     */
#pragma warning ( error   : 4715 ) /* 'function' : not all control paths return a value                                                                             */
#pragma warning ( error   : 4727 ) /* conditional expression is constant                                                                                            */
#pragma warning ( disable : 4786 ) /* 'identifier' : identifier was truncated to 'number' characters in the debug information                                       */

#ifndef __cplusplus
#pragma warning ( disable : 4214 ) /* nonstandard extension used : bit field types other than int                                                                   */
#pragma warning ( error   : 4013 ) /* 'function' undefined; assuming extern returning int                                                                           */
#pragma warning ( error   : 4029 ) /* declared formal parameter list different from definition                                                                      */
#pragma warning ( error   : 4047 ) /* 'identifier1' : 'operator' : different levels of indirection from 'identifier2'                                               */
#endif


#endif


