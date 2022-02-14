/*-----------------------------------------------------------------------------+
 * Copyright 2018 SAP SE or an SAP affiliate company.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *                                                                                
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * 
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * While not a requirement of the license, if you do modify this file, we
 * would appreciate hearing about it. Please email sqlany_interfaces@sap.com
 *-----------------------------------------------------------------------------+
 */

#if defined( WINNT ) && !POINTERS_ARE_64BITS
#define _USE_32BIT_TIME_T 1
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stddef.h>
#include <string.h>


#include "php.h"
#include "php_ini.h"
#include "php_sqlanywhere.h"
#include "php_sqlany_ver.h"
#include "php_globals.h"
#include "ext/standard/info.h"

#define _min( x, y ) ( x < y ? x : y )
#if HAVE_SQLANYWHERE 

#include "sqlany_dbg.h"

#define _SACAPI_VERSION 2
#include "sacapi.h"
#include "sacapidll.h"

ZEND_DECLARE_MODULE_GLOBALS(sqlanywhere)

#if defined( WIN32 )
   #define DBCAPI_PREFIX   ""
   #define DBCAPI_SUFFIX   ".dll"

   #if defined( ZTS )
       #define DBCAPI_ZTS	   ""
   #else
       #define DBCAPI_ZTS	   ""
   #endif

   #define DLL_PATH	"PATH"
#else

   #define DBCAPI_PREFIX   "lib"
   #define DBCAPI_SUFFIX   ".so"

   #if defined( _REENTRANT )
       #define DBCAPI_ZTS	   "_r"
   #else
       #define DBCAPI_ZTS	   ""
   #endif

   #define DLL_PATH	"LD_LIBRARY_PATH"
#endif

#ifndef Z_ADDREF_P
#define Z_ADDREF_P(x)	(((x)->refcount)++)
#define Z_DELREF_P(x)	(((x)->refcount)--)
#define Z_SET_REFCOUNT_P(x,v)   ((x)->refcount = v)
#define Z_REFCOUNT_P(x)   ((x)->refcount)
#endif


#define DBCAPI_NAME DBCAPI_PREFIX "dbcapi" DBCAPI_ZTS DBCAPI_SUFFIX

//#define DBCAPI_NOT_FOUND_ERROR "Could not load " DBCAPI_NAME ". Please ensure that " DBCAPI_NAME " can be found in your " DLL_PATH " environment variable."
#define DBCAPI_NOT_FOUND_ERROR "The SQLAnywhere client libraries could not be loaded. Please ensure that " DBCAPI_NAME " can be found in your " DLL_PATH " environment variable."


#if PHP_MAJOR_VERSION >= 8
#define TSRMLS_CC
#define TSRMLS_DC
#endif

#if (PHP_MAJOR_VERSION < 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION <= 5)

ZEND_BEGIN_ARG_INFO(arginfo_sasql_stmt_bind_param_ex, 0)
  ZEND_ARG_PASS_INFO(0)
  ZEND_ARG_PASS_INFO(0)
  ZEND_ARG_PASS_INFO(1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_sasql_stmt_bind_param, 1)
  ZEND_ARG_PASS_INFO(0)
  ZEND_ARG_PASS_INFO(0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_sasql_stmt_bind_result, 1)
  ZEND_ARG_PASS_INFO(0)
ZEND_END_ARG_INFO()

#elseif (PHP_MAJOR_VERSION < 8)

ZEND_BEGIN_ARG_INFO(arginfo_sasql_stmt_bind_param_ex, 0)
  ZEND_ARG_INFO(0, stmt)
  ZEND_ARG_INFO(0, types)
  ZEND_ARG_INFO(1, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_sasql_stmt_bind_param, 0)
  ZEND_ARG_INFO(0, stmt)
  ZEND_ARG_INFO(0, types)
  ZEND_ARG_VARIADIC_INFO(1, vars)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_sasql_stmt_bind_result, 0)
  ZEND_ARG_INFO(0, stmt)
  ZEND_ARG_VARIADIC_INFO(1, vars)
ZEND_END_ARG_INFO()

#else

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_connect, 0, 0, 1)
  ZEND_ARG_INFO(0, con_str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_pconnect, 0, 0, 1)
  ZEND_ARG_INFO(0, con_str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_pconnect_from_sqlca, 0, 0, 1)
  ZEND_ARG_INFO(0, sqlca)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_close, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_commit, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_rollback, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_query, 0, 0, 2)
  ZEND_ARG_INFO(0, conn)
  ZEND_ARG_INFO(0, sql_str)
  ZEND_ARG_INFO(0, result_mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_real_query, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_store_result, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_use_result, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_next_result, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_set_option, 0, 0, 3)
  ZEND_ARG_INFO(0, conn)
  ZEND_ARG_INFO(0, option)
  ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_affected_rows, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_data_seek, 0, 0, 2)
  ZEND_ARG_INFO(0, result)
  ZEND_ARG_INFO(0, row_num)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_sqlstate, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_fetch_array, 0, 0, 1)
  ZEND_ARG_INFO(0, result)
  ZEND_ARG_INFO(0, result_type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_fetch_assoc, 0, 0, 1)
  ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_fetch_field, 0, 0, 1)
  ZEND_ARG_INFO(0, result)
  ZEND_ARG_INFO(0, field_offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_fetch_object, 0, 0, 1)
  ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_fetch_row, 0, 0, 1)
  ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_field_seek, 0, 0, 1)
  ZEND_ARG_INFO(0, result)
  ZEND_ARG_INFO(0, field_offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_free_result, 0, 0, 1)
  ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_num_rows, 0, 0, 1)
  ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_result_all, 0, 0, 1)
  ZEND_ARG_INFO(0, result)
  ZEND_ARG_INFO(0, html_table_format_string)
  ZEND_ARG_INFO(0, html_table_header_format_string)
  ZEND_ARG_INFO(0, html_table_row_format_string)
  ZEND_ARG_INFO(0, html_table_cell_format_string)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_error, 0, 0, 0)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_errorcode, 0, 0, 0)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_insert_id, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_message, 0, 0, 2)
  ZEND_ARG_INFO(0, conn)
  ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_prepare, 0, 0, 2)
  ZEND_ARG_INFO(0, conn)
  ZEND_ARG_INFO(0, sql_str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_field_count, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_num_fields, 0, 0, 1)
  ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_param_count, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_bind_param_ex, 0, 0, 4)
  ZEND_ARG_INFO(0, smt)
  ZEND_ARG_INFO(0, param_number)
  ZEND_ARG_INFO(1, var)
  ZEND_ARG_INFO(0, type)
  ZEND_ARG_INFO(0, is_null)
  ZEND_ARG_INFO(0, direction)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_bind_param, 0, 0, 3)
  ZEND_ARG_INFO(0, stmt)
  ZEND_ARG_INFO(0, types)
  ZEND_ARG_VARIADIC_INFO(1, vars)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_execute, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_close, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_result_metadata, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_affected_rows, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_num_rows, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_insert_id, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_bind_result, 0, 0, 2)
  ZEND_ARG_INFO(0, stmt)
  ZEND_ARG_VARIADIC_INFO(1, vars)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_send_long_data, 0, 0, 3)
  ZEND_ARG_INFO(0, smt)
  ZEND_ARG_INFO(0, param_number)
  ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_store_result, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_free_result, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_reset, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_fetch, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_field_count, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_data_seek, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
  ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_next_result, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_errno, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_error, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_stmt_sqlstate, 0, 0, 1)
  ZEND_ARG_INFO(0, stmt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_real_escape_string, 0, 0, 2)
  ZEND_ARG_INFO(0, conn)
  ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_get_client_info, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_disconnect, 0, 0, 1)
  ZEND_ARG_INFO(0, conn)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_escape_string, 0, 0, 2)
  ZEND_ARG_INFO(0, conn)
  ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sasql_multi_query, 0, 0, 2)
  ZEND_ARG_INFO(0, conn)
  ZEND_ARG_INFO(0, sql_str)
ZEND_END_ARG_INFO()

#endif


PHP_MINIT_FUNCTION(sqlanywhere);
PHP_MSHUTDOWN_FUNCTION(sqlanywhere);
PHP_RINIT_FUNCTION(sqlanywhere);
PHP_RSHUTDOWN_FUNCTION(sqlanywhere);
PHP_MINFO_FUNCTION(sqlanywhere);

PHP_FUNCTION(sasql_affected_rows);
PHP_FUNCTION(sasql_close);
PHP_FUNCTION(sasql_commit);
PHP_FUNCTION(sasql_connect);
/* sasql_disconnect */
PHP_FUNCTION(sasql_data_seek);
PHP_FUNCTION(sasql_error);
PHP_FUNCTION(sasql_errorcode);
/* sasql_escape_string */
PHP_FUNCTION(sasql_fetch_array);
PHP_FUNCTION(sasql_fetch_assoc);
PHP_FUNCTION(sasql_fetch_field);
PHP_FUNCTION(sasql_fetch_object);
PHP_FUNCTION(sasql_fetch_row);
PHP_FUNCTION(sasql_field_count);
PHP_FUNCTION(sasql_field_seek);
PHP_FUNCTION(sasql_free_result);
PHP_FUNCTION(sasql_get_client_info);
PHP_FUNCTION(sasql_insert_id);
PHP_FUNCTION(sasql_message);
PHP_FUNCTION(sasql_next_result);
PHP_FUNCTION(sasql_num_fields);
PHP_FUNCTION(sasql_num_rows);
PHP_FUNCTION(sasql_pconnect);
PHP_FUNCTION(sasql_pconnect_from_sqlca);
PHP_FUNCTION(sasql_prepare);
PHP_FUNCTION(sasql_query);
PHP_FUNCTION(sasql_real_escape_string);
PHP_FUNCTION(sasql_real_query);
PHP_FUNCTION(sasql_result_all);
PHP_FUNCTION(sasql_rollback);
PHP_FUNCTION(sasql_set_option);
PHP_FUNCTION(sasql_sqlstate);
PHP_FUNCTION(sasql_stmt_affected_rows);
PHP_FUNCTION(sasql_stmt_bind_param);
PHP_FUNCTION(sasql_stmt_bind_param_ex);
PHP_FUNCTION(sasql_stmt_bind_result);
PHP_FUNCTION(sasql_stmt_close);
PHP_FUNCTION(sasql_stmt_data_seek);
PHP_FUNCTION(sasql_stmt_execute);
PHP_FUNCTION(sasql_stmt_errno);
PHP_FUNCTION(sasql_stmt_error);
PHP_FUNCTION(sasql_stmt_fetch);
PHP_FUNCTION(sasql_stmt_field_count);
PHP_FUNCTION(sasql_stmt_free_result);
PHP_FUNCTION(sasql_stmt_insert_id);
PHP_FUNCTION(sasql_stmt_next_result);
PHP_FUNCTION(sasql_stmt_num_rows);
PHP_FUNCTION(sasql_stmt_param_count);
PHP_FUNCTION(sasql_stmt_reset);
PHP_FUNCTION(sasql_stmt_result_metadata);
PHP_FUNCTION(sasql_stmt_send_long_data);
PHP_FUNCTION(sasql_stmt_store_result);
PHP_FUNCTION(sasql_stmt_sqlstate);
PHP_FUNCTION(sasql_store_result);
PHP_FUNCTION(sasql_use_result);


/*
 * Every user visible function must have an entry in sasql_functions[].
 */
zend_function_entry sqlanywhere_functions[] = {
    	/* connection functions */
	PHP_FE(sasql_connect,				arginfo_sasql_connect)
	PHP_FE(sasql_pconnect,				arginfo_sasql_pconnect)
	PHP_FE(sasql_pconnect_from_sqlca,	arginfo_sasql_pconnect_from_sqlca)
	PHP_FE(sasql_close,				    arginfo_sasql_close)
	PHP_FE(sasql_query,				    arginfo_sasql_query)
	PHP_FE(sasql_real_query,			arginfo_sasql_real_query)
	PHP_FE(sasql_store_result,			arginfo_sasql_store_result)
	PHP_FE(sasql_use_result,			arginfo_sasql_use_result)
	PHP_FE(sasql_next_result,			arginfo_sasql_next_result)
	PHP_FE(sasql_set_option,			arginfo_sasql_set_option)
	PHP_FE(sasql_affected_rows,			arginfo_sasql_affected_rows)
	PHP_FE(sasql_commit,				arginfo_sasql_commit)
	PHP_FE(sasql_rollback,				arginfo_sasql_rollback)
	PHP_FE(sasql_sqlstate,				arginfo_sasql_sqlstate)

	/* result functions */
/*	PHP_FE(sasql_field_tell,			NULL) */
	PHP_FE(sasql_data_seek,				arginfo_sasql_data_seek)
	PHP_FE(sasql_fetch_array,			arginfo_sasql_fetch_array)
	PHP_FE(sasql_fetch_assoc,			arginfo_sasql_fetch_assoc)
	PHP_FE(sasql_fetch_field,			arginfo_sasql_fetch_field)
	PHP_FE(sasql_fetch_object,			arginfo_sasql_fetch_object)
	PHP_FE(sasql_fetch_row,				arginfo_sasql_fetch_row)
	PHP_FE(sasql_field_seek,			arginfo_sasql_field_seek)
	PHP_FE(sasql_free_result,			arginfo_sasql_free_result)
	PHP_FE(sasql_num_rows,				arginfo_sasql_num_rows)
	PHP_FE(sasql_result_all,			arginfo_sasql_result_all)

	/* connection functions */
	PHP_FE(sasql_error,				    arginfo_sasql_error)
	PHP_FE(sasql_errorcode,				arginfo_sasql_errorcode)
	PHP_FE(sasql_insert_id,				arginfo_sasql_insert_id)
	PHP_FE(sasql_message,				arginfo_sasql_message)
	PHP_FE(sasql_prepare, 				arginfo_sasql_prepare)
	PHP_FE(sasql_field_count,			arginfo_sasql_field_count)
	PHP_FE(sasql_num_fields,			arginfo_sasql_num_fields)

	/* statement functions */
	PHP_FE(sasql_stmt_param_count,			arginfo_sasql_stmt_param_count)
	PHP_FE(sasql_stmt_bind_param_ex,		arginfo_sasql_stmt_bind_param_ex)
	PHP_FE(sasql_stmt_bind_param,			arginfo_sasql_stmt_bind_param)
	PHP_FE(sasql_stmt_execute,			    arginfo_sasql_stmt_execute)
	PHP_FE(sasql_stmt_close,			    arginfo_sasql_stmt_close)
	PHP_FE(sasql_stmt_result_metadata,		arginfo_sasql_stmt_result_metadata)
	PHP_FE(sasql_stmt_affected_rows,		arginfo_sasql_stmt_affected_rows)
	PHP_FE(sasql_stmt_num_rows,			    arginfo_sasql_stmt_num_rows)
	PHP_FE(sasql_stmt_insert_id,			arginfo_sasql_stmt_insert_id)
	PHP_FE(sasql_stmt_bind_result,			arginfo_sasql_stmt_bind_result)
	PHP_FE(sasql_stmt_send_long_data,		arginfo_sasql_stmt_send_long_data)
	PHP_FE(sasql_stmt_store_result,			arginfo_sasql_stmt_store_result)
	PHP_FE(sasql_stmt_free_result,			arginfo_sasql_stmt_free_result)
	PHP_FE(sasql_stmt_reset,			    arginfo_sasql_stmt_reset)
	PHP_FE(sasql_stmt_fetch,			    arginfo_sasql_stmt_fetch)
	PHP_FE(sasql_stmt_field_count,			arginfo_sasql_stmt_field_count)
	PHP_FE(sasql_stmt_data_seek,			arginfo_sasql_stmt_data_seek)
	PHP_FE(sasql_stmt_next_result,          arginfo_sasql_stmt_next_result)
	PHP_FE(sasql_stmt_errno, 			    arginfo_sasql_stmt_errno)
	PHP_FE(sasql_stmt_error, 			    arginfo_sasql_stmt_error)
	PHP_FE(sasql_stmt_sqlstate, 			arginfo_sasql_stmt_sqlstate)

	/* misc functions */
	PHP_FE(sasql_real_escape_string, 		arginfo_sasql_real_escape_string)
	PHP_FE(sasql_get_client_info,			arginfo_sasql_get_client_info)


	/* Aliases */
	PHP_FALIAS(sasql_disconnect, 			sasql_close,			    arginfo_sasql_close)
	PHP_FALIAS(sasql_escape_string, 		sasql_real_escape_string,	arginfo_sasql_real_escape_string)
	PHP_FALIAS(sasql_multi_query, 			sasql_real_query,		    arginfo_sasql_real_query)
	{NULL, NULL, NULL}
};

zend_module_entry sqlanywhere_module_entry = {
    STANDARD_MODULE_HEADER,
    "sqlanywhere",
    sqlanywhere_functions,
    PHP_MINIT(sqlanywhere),
    PHP_MSHUTDOWN(sqlanywhere),
    PHP_RINIT(sqlanywhere),	/* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(sqlanywhere),	/* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(sqlanywhere),
    PHP_SQLANYWHERE_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SQLANYWHERE
ZEND_GET_MODULE(sqlanywhere)
#endif

static int le_result;
static int le_conn;
static int le_pconn;
static int le_stmt;
static SQLAnywhereInterface api;

#define sasql_HASH_STR	"sasql___"

/* types of fetch */
#define FETCH_ROW	(0x01)
#define FETCH_OBJ	(0x02)

#if PHP_MAJOR_VERSION >= 7
    #define ASA_UpdateInt       OnUpdateLong
    #define zend_rsrc_list_entry zend_resource
#elif PHP_MAJOR_VERSION >= 5
#define ASA_UpdateInt	OnUpdateLong
  #if PHP_MINOR_VERSION < 4
    #define zend_rsrc_list_entry list_entry
  #endif
#else
#define ASA_UpdateInt	OnUpdateInt
#endif

/*******************************************************************************
 * Macros to support version differences
 *******************************************************************************/
#if PHP_MAJOR_VERSION >= 7
#define ZEND_VERIFY_RESOURCE(rsrc) \
	if (!rsrc) {               \
		RETURN_FALSE;      \
	}

#define ZEND_FETCH_RESOURCE(rsrc, rsrc_type, passed_id, default_id, resource_type_name, resource_type)	\
	rsrc = (rsrc_type) zend_fetch_resource_ex(&passed_id TSRMLS_CC, resource_type_name, resource_type);	\
	ZEND_VERIFY_RESOURCE(rsrc);

#define ZEND_FETCH_RESOURCE2(rsrc, rsrc_type, passed_id, default_id, resource_type_name, resource_type1, resource_type2)	\
	rsrc = (rsrc_type) zend_fetch_resource2_ex(&passed_id TSRMLS_CC, resource_type_name, resource_type1, resource_type2);	\
	ZEND_VERIFY_RESOURCE(rsrc);
#endif

#if PHP_MAJOR_VERSION >= 7
    #define sqlany_convert_to_string_ex(__zval) convert_to_string_ex(&__zval)
    #define sqlany_convert_to_long_ex(__zval) convert_to_long_ex(&__zval)
    #define sqlany_convert_to_double_ex(__zval) convert_to_double_ex(&__zval)
    #define sqlany_convert_to_boolean_ex(__zval) convert_to_boolean_ex(&__zval)
#else
    #define sqlany_convert_to_string_ex(__ppzv) convert_to_string_ex(__ppzv)
    #define sqlany_convert_to_long_ex(__ppzv) convert_to_long_ex(__ppzv)
    #define sqlany_convert_to_double_ex(__ppzv) convert_to_double_ex(__ppzv)
    #define sqlany_convert_to_boolean_ex(__ppzv) convert_to_boolean_ex(__ppzv)
#endif

#if PHP_MAJOR_VERSION >= 7
    #define DECLARE_ZVAL_ARG1 \
        zval    arg1
#else
    #define DECLARE_ZVAL_ARG1 \
        zval ** arg1
#endif

typedef struct sqlany_connection_t sqlany_connection_t;
typedef struct sqlany_result_t 	   sqlany_result_t;
typedef struct sqlany_stmt_t 	   sqlany_stmt_t;
typedef struct sqlany_stmt_param_t sqlany_stmt_param_t;
typedef struct php_stmt_param_t	   php_stmt_param_t;
typedef struct sqlany_result_row_t sqlany_result_row_t;
typedef struct sqlany_result_col_t sqlany_result_col_t;

struct sqlany_connection_t
{
#if PHP_MAJOR_VERSION >= 7
    zend_resource		*res;
#else
    int				id;
#endif
    a_sqlany_connection		*handle;
    int				auto_commit;
    int				verbose_errors;
    int				persistent;
    sqlany_result_t 		*result_list;
    sqlany_stmt_t		*stmt_list;
    a_sqlany_stmt		*last_stmt;
    int				ext_env_conn;
    char			error[SACAPI_ERROR_SIZE];
    int				errorcode;
    char			sqlstate[6];
    int				len;
};

struct sqlany_result_row_t
{
    int				row_id;
    a_sqlany_data_value		*values;
    sqlany_result_row_t		*next;
};

struct sqlany_result_t
{
#if PHP_MAJOR_VERSION >= 7
    zend_resource		*res;
#else
    int				id;
#endif
    a_sqlany_stmt		*stmt_handle;
    sqlany_connection_t		*sqlany_conn;
    sqlany_result_t		*next;
    sqlany_stmt_t		*sqlany_stmt;

    sqlany_result_row_t		*curr_row;
    int				fetch_pos;
    int				num_rows;

    a_sqlany_column_info * 	column_info;
    int				num_cols;
    int				curr_col;
    int				result_cached;
};

struct sqlany_stmt_param_t
{
    a_sqlany_data_value		in;
    int				in_is_null;
    size_t			in_len;
    int				using_send_data;
    a_sqlany_data_value 	out;
    int				out_is_null;
    size_t			out_len;
};

struct php_stmt_param_t
{
#if PHP_MAJOR_VERSION >= 7
    zval	 		in;
    zval	 		out;
#else
    zval	*		in;
    zval	*		out;
#endif
};

struct sqlany_stmt_t
{
#if PHP_MAJOR_VERSION >= 7
    zend_resource		*res;
#else
    int				id;
#endif
    a_sqlany_stmt		*handle;
    sqlany_stmt_t		*next;
    sqlany_connection_t		*sqlany_conn;
    sqlany_result_t		*sqlany_result;
    int				num_params;
    php_stmt_param_t		*php_params;
    sqlany_stmt_param_t		*sqlany_params;
#if PHP_MAJOR_VERSION >= 7
    zval 			*php_result_vars;
#else
    zval*			*php_result_vars;
#endif
    int				num_result_vars;
    char			error[SACAPI_ERROR_SIZE];
    int				errorcode;
    char			sqlstate[6];
    int				len;
};

static void SQLAnywhereConnect( INTERNAL_FUNCTION_PARAMETERS, int persistent );

/* {{{ PHP_INI */
PHP_INI_BEGIN()
    STD_PHP_INI_BOOLEAN(  "sqlanywhere.allow_persistent", "1", PHP_INI_SYSTEM, ASA_UpdateInt,
	    allow_persistent, zend_sqlanywhere_globals, sqlanywhere_globals )
    STD_PHP_INI_ENTRY_EX( "sqlanywhere.max_persistent_connections", "-1", PHP_INI_SYSTEM,
	    ASA_UpdateInt, max_pconns, zend_sqlanywhere_globals, sqlanywhere_globals,
	    display_link_numbers )
    STD_PHP_INI_ENTRY_EX( "sqlanywhere.max_connections", "-1", PHP_INI_SYSTEM, ASA_UpdateInt,
	    max_conns, zend_sqlanywhere_globals, sqlanywhere_globals, display_link_numbers )
    STD_PHP_INI_BOOLEAN(  "sqlanywhere.auto_commit", "1", PHP_INI_SYSTEM, ASA_UpdateInt,
	    auto_commit, zend_sqlanywhere_globals, sqlanywhere_globals )
    STD_PHP_INI_BOOLEAN(  "sqlanywhere.verbose_errors", "1", PHP_INI_SYSTEM, ASA_UpdateInt,
	    verbose_errors, zend_sqlanywhere_globals, sqlanywhere_globals )
PHP_INI_END()
/* }}} */

#if PHP_MAJOR_VERSION >= 5
#undef ASA_UpdateInt
#endif

static
void save_conn_error( sqlany_connection_t * sqlany_conn )
/********************************************************/
{
    sqlany_conn->errorcode = api.sqlany_error( sqlany_conn->handle,
	    sqlany_conn->error, sizeof(sqlany_conn->error) );
    sqlany_conn->len = api.sqlany_sqlstate( sqlany_conn->handle,
	    sqlany_conn->sqlstate, sizeof(sqlany_conn->sqlstate) - 1 );
}

static
void clear_conn_error( sqlany_connection_t * sqlany_conn )
/*********************************************************/
{
    sqlany_conn->errorcode = 0;
    memset( sqlany_conn->error, 0, sizeof(sqlany_conn->error) );
    strcpy( sqlany_conn->sqlstate, "00000" );
}

static
void save_stmt_error( sqlany_stmt_t * sqlany_stmt )
/**************************************************/
{
    sqlany_stmt->errorcode = api.sqlany_error( sqlany_stmt->sqlany_conn->handle,
	    sqlany_stmt->error, sizeof(sqlany_stmt->error) );
    sqlany_stmt->len = api.sqlany_sqlstate( sqlany_stmt->sqlany_conn->handle,
	    sqlany_stmt->sqlstate, sizeof(sqlany_stmt->sqlstate) - 1);
}

static
void clear_stmt_error( sqlany_stmt_t * sqlany_stmt )
/**************************************************/
{
    sqlany_stmt->errorcode = 0;
    memset( sqlany_stmt->error, 0, sizeof(sqlany_stmt->error) );
    strcpy( sqlany_stmt->sqlstate, "00000" );
}

static
void free_cached_result( sqlany_result_t * sqlany_result )
/*********************************************************/
{
    int i;

    if( sqlany_result->column_info != NULL ) {
	for( i=0; i < sqlany_result->num_cols; i++ ) {
	    efree( sqlany_result->column_info[i].name );
	}
	efree( sqlany_result->column_info );
	sqlany_result->column_info = NULL;
    }

    while( sqlany_result->curr_row != NULL ) {
	sqlany_result_row_t * curr_row = sqlany_result->curr_row->next;

	if( curr_row->next == curr_row ) {
	    sqlany_result->curr_row = NULL;
	} else {
	    sqlany_result->curr_row->next = curr_row->next;
	}
	curr_row->next = NULL;

	for( i = 0; i < sqlany_result->num_cols; i++ ) {
	    if( (*curr_row->values[i].is_null) == 0 ) {
		efree( curr_row->values[i].buffer );
		efree( curr_row->values[i].length );
	    }
	    efree( curr_row->values[i].is_null );
	}
	efree( curr_row->values );
	efree( curr_row );
	sqlany_result->num_rows--;
    }
#if PHP_MAJOR_VERSION >= 7
    assert( sqlany_result->curr_row == NULL || sqlany_result->num_rows == 0 );
#else
    assert( sqlany_result->num_rows == 0 );
    assert( sqlany_result->curr_row == NULL );
#endif
}

static
int cache_result( sqlany_result_t * sqlany_result )
/**************************************************/
{
    sqlany_result_row_t * curr_row;
    int			  i;

    sqlany_result->num_rows = 0;
    sqlany_result->fetch_pos = 0;
    sqlany_result->num_cols = api.sqlany_num_cols( sqlany_result->stmt_handle );
    sqlany_result->column_info = (a_sqlany_column_info *)
	ecalloc( sqlany_result->num_cols, sizeof(a_sqlany_column_info));
    sqlany_result->result_cached = 1;

    // first cache column information
    for( i = 0; i < sqlany_result->num_cols; i++ ) {
	a_sqlany_column_info cinfo;

	api.sqlany_get_column_info( sqlany_result->stmt_handle, i, &cinfo );

	sqlany_result->column_info[i] = cinfo;
	sqlany_result->column_info[i].name = estrdup( cinfo.name );
    }

    // second cache row information
    while( api.sqlany_fetch_next( sqlany_result->stmt_handle ) ) {
	curr_row = ecalloc( 1, sizeof(sqlany_result_row_t) );
	curr_row->row_id = sqlany_result->num_rows;
	curr_row->values = ecalloc( sqlany_result->num_cols, sizeof(a_sqlany_data_value));

	for( i = 0; i < sqlany_result->num_cols; i++ ) {
	    a_sqlany_data_value	fetched_value;

	    api.sqlany_get_column( sqlany_result->stmt_handle, i, &fetched_value );

	    curr_row->values[i].type = fetched_value.type;
	    curr_row->values[i].is_null = (int *)emalloc( sizeof(int) );
	    *(curr_row->values[i].is_null) = *(fetched_value.is_null);
	    if( *(fetched_value.is_null) ) {
		curr_row->values[i].buffer = NULL;
		curr_row->values[i].length = NULL;
		continue;
	    }
	    switch( fetched_value.type ) {
		case A_BINARY:
		case A_STRING:
		    curr_row->values[i].buffer = (char *)emalloc( *(fetched_value.length) + 1 );
		    memcpy( curr_row->values[i].buffer, fetched_value.buffer,
			    *(fetched_value.length) );
		    curr_row->values[i].buffer[*(fetched_value.length)] = '\0';
		    curr_row->values[i].length = (size_t*)emalloc( sizeof(size_t) );
		    *(curr_row->values[i].length) = *(fetched_value.length);
		    break;

		case A_DOUBLE:
		case A_VAL64:
		case A_UVAL64:
		    curr_row->values[i].buffer = emalloc( 8 );
		    memcpy( curr_row->values[i].buffer, fetched_value.buffer, 8 );
		    curr_row->values[i].length = (size_t*)emalloc( sizeof(size_t) );
		    *(curr_row->values[i].length) = sizeof(double);
		    break;

		case A_VAL32:
		case A_UVAL32:
		    curr_row->values[i].buffer = emalloc( 4 );
		    memcpy( curr_row->values[i].buffer, fetched_value.buffer, 4 );
		    curr_row->values[i].length = (size_t*)emalloc( sizeof(size_t) );
		    *(curr_row->values[i].length) = sizeof(int);
		    break;

		case A_VAL16:
		case A_UVAL16:
		    curr_row->values[i].buffer = emalloc( 2 );
		    memcpy( curr_row->values[i].buffer, fetched_value.buffer, 2 );
		    curr_row->values[i].length = (size_t*)emalloc( sizeof(size_t) );
		    *(curr_row->values[i].length) = 2;
		    break;

		case A_VAL8:
		case A_UVAL8:
		    curr_row->values[i].buffer = emalloc( 1 );
		    memcpy( curr_row->values[i].buffer, fetched_value.buffer, 1 );
		    curr_row->values[i].length = (size_t*)emalloc( sizeof(size_t) );
		    *(curr_row->values[i].length) = 1;
		    break;
		default:
		    break;
	    }
	}

	if( sqlany_result->curr_row == NULL ) {
	    curr_row->next = curr_row;
	} else {
	    curr_row->next = sqlany_result->curr_row->next;
	    sqlany_result->curr_row->next = curr_row;
	}
	sqlany_result->curr_row = curr_row;
	sqlany_result->num_rows++;
    }

    return api.sqlany_error( sqlany_result->sqlany_conn->handle, NULL, 0 );
}

static
int result_fetch_next( sqlany_result_t * sqlany_result, int is_connection )
/*************************************************************************/
{
    if( sqlany_result->result_cached ) {
	if( sqlany_result->fetch_pos >= sqlany_result->num_rows ) {
	    if( (is_connection && sqlany_result->sqlany_conn->errorcode == 0) ||
		 (!is_connection && sqlany_result->sqlany_stmt->errorcode == 0) ) {
		return 0;
	    } else {
		return -1;
	    }
	}
	sqlany_result->curr_row = sqlany_result->curr_row->next;
	sqlany_result->fetch_pos++;
    } else {
	if( !api.sqlany_fetch_next( sqlany_result->stmt_handle ) ) {
	    if( api.sqlany_error( sqlany_result->sqlany_conn->handle, NULL, 0 ) == 100 ) {
		if( is_connection ) {
		    clear_conn_error( sqlany_result->sqlany_conn );
		} else {
		    clear_stmt_error( sqlany_result->sqlany_stmt );
		}
		return 0;
	    } else {
		if( is_connection ) {
		    save_conn_error( sqlany_result->sqlany_conn );
		} else {
		    save_stmt_error( sqlany_result->sqlany_stmt );
		}
		return -1;
	    }
	}
	sqlany_result->fetch_pos++;
    }
    return 1;
}

static
int result_data_seek( sqlany_result_t * sqlany_result, int offset, int is_connection )
/*************************************************************************************/
{
    if( sqlany_result->result_cached ) {
	int row_id;
	if( offset < 0 ) {
	    offset += sqlany_result->num_rows;
	}
	if( offset < 0 || offset >= sqlany_result->num_rows ) {
	    return 0;
	}
	if( offset == 0 ) {
	    row_id = sqlany_result->num_rows - 1;
	} else {
	    row_id = offset - 1;
	}
	while( sqlany_result->curr_row->row_id != row_id ) {
	    sqlany_result->curr_row = sqlany_result->curr_row->next;
	}
	sqlany_result->fetch_pos = sqlany_result->curr_row->next->row_id;
	return 1;
    } else {
	int ok;
	if( offset <= 0 ) {
	    api.sqlany_fetch_absolute( sqlany_result->stmt_handle, offset );
	    ok = 1;
	} else {
	    ok = api.sqlany_fetch_absolute( sqlany_result->stmt_handle, offset );
	    if( !ok ) {
		if( is_connection ) {
		    save_conn_error( sqlany_result->sqlany_conn );
		} else {
		    save_stmt_error( sqlany_result->sqlany_stmt );
		}
	    }
	}
	return ok;
    }
}

int result_get_column_data( sqlany_result_t * sqlany_result, int col_num, a_sqlany_data_value * dvalue )
/*******************************************************************************************************/
{
    if( sqlany_result->curr_row != NULL ) {
	(*dvalue) = sqlany_result->curr_row->values[col_num];
    } else {
	api.sqlany_get_column( sqlany_result->stmt_handle, col_num, dvalue );
    }
    return 1;
}

int result_get_column_info( sqlany_result_t * sqlany_result, int col_num, a_sqlany_column_info * cinfo )
/*******************************************************************************************************/
{
    if( sqlany_result->column_info != NULL ) {
	(*cinfo) = sqlany_result->column_info[col_num];
    } else {
	api.sqlany_get_column_info( sqlany_result->stmt_handle, col_num, cinfo );
	save_conn_error( sqlany_result->sqlany_conn );
	if( sqlany_result->sqlany_conn->errorcode != 0 ) {
	    return 0;
	}
    }
    return 1;
}

static
void VerboseErrors( sqlany_connection_t * sqlany_conn TSRMLS_DC )
/***************************************************************/
{
    if( sqlany_conn->verbose_errors ) {
	php_error_docref( NULL TSRMLS_CC, E_WARNING, "SQLAnywhere: [%d] %s", 
		(int)sqlany_conn->errorcode, sqlany_conn->error );
    }
}


static
ZEND_RSRC_DTOR_FUNC( sqlany_close_connection )
/*********************************************/
{
#if PHP_MAJOR_VERSION >= 7
    sqlany_connection_t * sqlany_conn = (sqlany_connection_t *)res->ptr;
#else
    sqlany_connection_t * sqlany_conn = (sqlany_connection_t *)rsrc->ptr;
#endif
    int 		  persistent = sqlany_conn->persistent;
#if PHP_MAJOR_VERSION >= 7
    zend_resource	  *id = sqlany_conn->res;
#else
    int			  id = sqlany_conn->id;
#endif

    if( !sqlany_conn->ext_env_conn ) {
	while( sqlany_conn->result_list != NULL ) {
#if PHP_MAJOR_VERSION >= 7
	    zend_list_close( sqlany_conn->result_list->res );
#else
	    zend_list_delete( sqlany_conn->result_list->id );
#endif
	}
	while( sqlany_conn->stmt_list != NULL ) {
#if PHP_MAJOR_VERSION >= 7
	    zend_list_close( sqlany_conn->stmt_list->res );
#else
	    zend_list_delete( sqlany_conn->stmt_list->id );
#endif
	}
	if( sqlany_conn->last_stmt != NULL ) {
	    api.sqlany_free_stmt( sqlany_conn->last_stmt );
	    sqlany_conn->last_stmt = NULL;
	}
#if PHP_MAJOR_VERSION >= 7
	_debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_connection persist=%d id=%p ",
		    persistent, res ) );
#else
	_debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_connection persist=%d id=%d ",
		    persistent, id ) );
#endif
	if( sqlany_conn->auto_commit ) {
	    api.sqlany_commit( sqlany_conn->handle );
	}
	api.sqlany_disconnect( sqlany_conn->handle );
	api.sqlany_free_connection( sqlany_conn->handle );
    }
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_connection persist=%d id=%p done",
		persistent, res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_connection persist=%d id=%d done",
		persistent, id ) );
#endif
    if( persistent ) {
	SAG(num_pconns)--;
	free( sqlany_conn );
    } else {
	efree( sqlany_conn );
    }
    SAG(num_conns)--;
}

static
ZEND_RSRC_DTOR_FUNC( sqlany_close_result )
/******************************************/
{
#if PHP_MAJOR_VERSION >= 7
    sqlany_result_t * sqlany_result = (sqlany_result_t *)res->ptr;
#else
    sqlany_result_t * sqlany_result = (sqlany_result_t *)rsrc->ptr;
#endif

    if( sqlany_result ) {
	sqlany_connection_t 	*sqlany_conn = sqlany_result->sqlany_conn;
	sqlany_result_t 	**curr;
#if PHP_MAJOR_VERSION >= 7
	zend_resource		*res;
#else
	int			id;
#endif

	// We only need to auto commit if the result set is *not* cached and auto_commit is 'on'
	if( sqlany_result->curr_row == NULL && sqlany_conn->auto_commit ) {
	    api.sqlany_commit( sqlany_conn->handle );
	}

	for( curr = &(sqlany_conn->result_list); *curr != NULL ; curr = &(*curr)->next ) {
	    if( (*curr) == sqlany_result ) {
		(*curr) = (*curr)->next;
		break;
	    }
	}
	sqlany_result->sqlany_conn = NULL;

	free_cached_result( sqlany_result );

	if( sqlany_result->sqlany_stmt ) {
	    sqlany_result->sqlany_stmt->sqlany_result = NULL;
	    sqlany_result->sqlany_stmt = NULL;
	    sqlany_result->stmt_handle = NULL;
	} else if( sqlany_result->stmt_handle != NULL ) {
	    // if the result is referencing the current statement don't free it
	    // as the user might want to do a sasql_next_result()
	    if( sqlany_result->stmt_handle != sqlany_conn->last_stmt ) {
		api.sqlany_free_stmt( sqlany_result->stmt_handle );
	    }
	    sqlany_result->stmt_handle = NULL;
	}
#if PHP_MAJOR_VERSION >= 7
	res = sqlany_result->res;
	_debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_result res=%p", res ) );
#else
	id = sqlany_result->id;
	_debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_result id=%d", id ) );
#endif
	efree( sqlany_result );
#if PHP_MAJOR_VERSION >= 7
	_debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_result res=%p done", res ) );
#else
	_debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_result id=%d done", id ) );
#endif
    }
}

static
ZEND_RSRC_DTOR_FUNC( sqlany_close_stmt )
/******************************************/
{
#if PHP_MAJOR_VERSION >= 7
    sqlany_stmt_t * sqlany_stmt = (sqlany_stmt_t *)res->ptr;
#else
    sqlany_stmt_t * sqlany_stmt = (sqlany_stmt_t *)rsrc->ptr;
#endif

    if( sqlany_stmt ) {
	sqlany_connection_t 	*sqlany_conn = sqlany_stmt->sqlany_conn;
	sqlany_stmt_t 		**curr;

	for( curr = &(sqlany_conn->stmt_list); *curr != NULL ; curr = &(*curr)->next ) {
	    if( (*curr) == sqlany_stmt ) {
		(*curr) = (*curr)->next;
		break;
	    }
	}

#if PHP_MAJOR_VERSION >= 7
	_debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_stmt res=%p", sqlany_stmt->res ) );
#else
	_debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_stmt id=%d", sqlany_stmt->id ) );
#endif

	if( sqlany_stmt->sqlany_params ) {
	    efree( sqlany_stmt->sqlany_params );
	    sqlany_stmt->sqlany_params = NULL;
	}

	if( sqlany_stmt->php_params ) {
	    int i;
	    for( i = 0; i < sqlany_stmt->num_params; i++ ) {
#if PHP_MAJOR_VERSION >= 7
                // Do nothing.
#else
		if( sqlany_stmt->php_params[i].in ) {
		    Z_DELREF_P( sqlany_stmt->php_params[i].in );
		}
		if( sqlany_stmt->php_params[i].out ) {
		    Z_DELREF_P( sqlany_stmt->php_params[i].out );
		}
#endif
	    }
	    efree( sqlany_stmt->php_params );
	    sqlany_stmt->php_params = NULL;
	}

	if( sqlany_stmt->php_result_vars ) {
	    int i;
	    for( i = 0; i < sqlany_stmt->num_result_vars; i++ ) {
#if PHP_MAJOR_VERSION >= 7
                // Do nothing.
#else
		if( sqlany_stmt->php_result_vars[i] != NULL ) {
		    Z_DELREF_P( sqlany_stmt->php_result_vars[i] );
		}
#endif
	    }
	    efree( sqlany_stmt->php_result_vars );
	    sqlany_stmt->php_result_vars = NULL;
	    sqlany_stmt->num_result_vars = 0;
	}

	if( sqlany_stmt->sqlany_result ) {
	    free_cached_result( sqlany_stmt->sqlany_result );
#if PHP_MAJOR_VERSION >= 7
	    zend_list_close( sqlany_stmt->sqlany_result->res );
#else
	    zend_list_delete( sqlany_stmt->sqlany_result->id );
#endif
	    sqlany_stmt->sqlany_result = NULL;
	}

	if( sqlany_stmt->handle != NULL ) {
	    api.sqlany_free_stmt( sqlany_stmt->handle );
	}
#if PHP_MAJOR_VERSION >= 7
	_debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_stmt res=%p done",
		    sqlany_stmt->res ) );
#else
	_debug_enabled( SQLAnyDebug( _LOCATION_, "  sqlany_close_stmt id=%d done",
		    sqlany_stmt->id ) );
#endif
	efree( sqlany_stmt );
    }
}

/* {{{ php_sqlany_init_globals
*/
static
void php_sqlany_init_globals( zend_sqlanywhere_globals * sqlany_globals TSRMLS_DC )
/*********************************************************************************/
{
    memset( sqlany_globals, 0, sizeof(*sqlany_globals));
    sqlany_globals->debug_trace = ( getenv( "SA_PHP_DEBUG" ) == NULL ? 0 : 1 );
}
/* }}} */

PHP_MINIT_FUNCTION(sqlanywhere)
/******************************/
{
#ifdef ZTS
    ts_allocate_id( &sqlanywhere_globals_id, sizeof(zend_sqlanywhere_globals),
	    php_sqlany_init_globals, NULL);
#else
    php_sqlany_init_globals( &sqlanywhere_globals TSRMLS_CC );
#endif
    REGISTER_INI_ENTRIES();

    _debug_enabled( SQLAnyDebug( _LOCATION_, "BEGIN MINIT " ) );

    if( !sqlany_initialize_interface( &api, NULL ) ) {
	_debug_enabled( SQLAnyDebug( _LOCATION_, "Failed to load interface" ) );
	php_error_docref(NULL TSRMLS_CC, E_WARNING, DBCAPI_NOT_FOUND_ERROR );
    }
    le_conn  = zend_register_list_destructors_ex( sqlany_close_connection,
	    NULL,
	    "SQLAnywhere connection", module_number );

    le_pconn = zend_register_list_destructors_ex( NULL,
	    sqlany_close_connection,
	    "SQLAnywhere persistent connection ", module_number );

    le_result = zend_register_list_destructors_ex( sqlany_close_result,
	    NULL,
	    "SQLAnywhere result", module_number );

    le_stmt = zend_register_list_destructors_ex( sqlany_close_stmt,
	    NULL,
	    "SQLAnywhere statement", module_number );

    REGISTER_LONG_CONSTANT("SASQL_D_INPUT", 1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SASQL_D_OUTPUT", 2, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SASQL_D_INPUT_OUTPUT", 3, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("SASQL_USE_RESULT", 0, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SASQL_STORE_RESULT", 1, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("SASQL_NUM", 1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SASQL_ASSOC", 2, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SASQL_BOTH", 3, CONST_CS | CONST_PERSISTENT);
    _debug_enabled( SQLAnyDebug( _LOCATION_, "FINI  MINIT " ) );
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(sqlanywhere)
/**********************************/
{
    _debug_enabled( SQLAnyDebug( _LOCATION_, "BEGIN MSHUTDOWN " ) );
    UNREGISTER_INI_ENTRIES();
    if( !api.initialized ) {
	_debug_enabled( SQLAnyDebug( _LOCATION_, "FINI  MSHUTDOWN" ) );
	return SUCCESS;
    }
    if( SAG(sqlany_init_called) == 1 ) {
	if( SAG(context) ) {
	    api.sqlany_fini_ex( SAG(context) );
	    _debug_enabled( SQLAnyDebug( _LOCATION_, "sqlany_fini_ex called" ) );
	} else {
	    api.sqlany_fini();
	    _debug_enabled( SQLAnyDebug( _LOCATION_, "sqlany_fini called" ) );
	}
	SAG(sqlany_init_called) = 0;
    }
    sqlany_finalize_interface( &api );
    _debug_enabled( SQLAnyDebug( _LOCATION_, "FINI MSHUTDOWN" ) );
    return SUCCESS;
}

/* Remove if there's nothing to do at request start */
PHP_RINIT_FUNCTION(sqlanywhere)
/******************************/
{
    _debug_enabled( SQLAnyDebug( _LOCATION_, "BEGIN RINIT" ) );
    _debug_enabled( SQLAnyDebug( _LOCATION_, "api.initialized=%d", api.initialized) );
    if( api.initialized && SAG(sqlany_init_called) == 0 ) {
	if( api.sqlany_init_ex != NULL ) {
	    SAG(context) = api.sqlany_init_ex( "PHP", 2, NULL );
	    if( SAG(context) != NULL ) {
		SAG(sqlany_init_called) = 1;
	    }
	}
	if( SAG(sqlany_init_called) == 0 && !api.sqlany_init( "PHP", 1, NULL ) ) {
	    _debug_enabled( SQLAnyDebug( _LOCATION_, "Failed in sqlany_init() " ) );
	    return FAILURE;
	}
	SAG(sqlany_init_called) = 1;
	_debug_enabled( SQLAnyDebug( _LOCATION_, "sqlany_init() called successfully" ) );
    }
    _debug_enabled( SQLAnyDebug( _LOCATION_, "END RINIT" ) );
    return SUCCESS;
}

/* Remove if there's nothing to do at request end */
PHP_RSHUTDOWN_FUNCTION(sqlanywhere)
/**********************************/
{
    _debug_enabled( SQLAnyDebug( _LOCATION_, "BEGIN RSHUTDOWN" ) );
    _debug_enabled( SQLAnyDebug( _LOCATION_, "END RSHUTDOWN" ) );
    return SUCCESS;
}

PHP_MINFO_FUNCTION(sqlanywhere)
/******************************/
{
    char maxp[32],maxl[32],client_version[32];

    if ( SAG(max_pconns) == -1 ) {
	snprintf(maxp, 31, "%ld/unlimited", SAG(num_pconns) );
    } else {
	snprintf(maxp, 31, "%ld/%ld", SAG(num_pconns), SAG(max_pconns));
    }
    maxp[31]=0;

    if ( SAG(max_conns) == -1) {
	snprintf(maxl, 31, "%ld/unlimited", SAG(num_conns) );
    } else {
	snprintf(maxl, 31, "%ld/%ld", SAG(num_conns), SAG(max_conns) );
    }
    maxl[31]=0;

    php_info_print_table_start();
    php_info_print_table_header(2, "SQLAnywhere support", "enabled");
    php_info_print_table_row(2, "Allow Persistent Connections", (SAG(allow_persistent)?"Yes":"No") );
    php_info_print_table_row(2, "Persistent Connections", maxp);
    php_info_print_table_row(2, "Total Connections", maxl);
    php_info_print_table_row(2, "PHP SQLAnywhere driver version", PHP_SQLANYWHERE_VERSION );
    if( api.initialized ) {
	int rc;
	if( SAG(context) ) {
	    rc = api.sqlany_client_version_ex( SAG(context), client_version, sizeof(client_version) );
	} else {
	    rc = api.sqlany_client_version( client_version, sizeof(client_version) );
	}
	if( !rc ) {
	    php_sprintf( client_version, "Uknown" );
	}
	php_info_print_table_row(2, "SQLAnywhere client version", client_version);
    } else {
	php_info_print_table_row(2, "SQLAnywhere client version", DBCAPI_NOT_FOUND_ERROR );
    }
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

/* {{{ proto sasql_conn sasql_connect( string $conn_str )
   Open a SQLAnywhere server connection using arg as a connection string and returns a connection id */
PHP_FUNCTION(sasql_connect)
/**************************/
{
    SQLAnywhereConnect( INTERNAL_FUNCTION_PARAM_PASSTHRU, 0 );
}
/* }}} */

/* {{{ proto sasql_conn sasql_pconnect( string $conn_str )
   Open a persistant SQLAnywhere server connection using arg as a connection string and returns a connection id */
PHP_FUNCTION(sasql_pconnect)
/***************************/
{
    SQLAnywhereConnect( INTERNAL_FUNCTION_PARAM_PASSTHRU, 1 );
}
/* }}} */

static
void SQLAnywhereConnect( INTERNAL_FUNCTION_PARAMETERS, int persistent )
/**********************************************************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    arg;
    zend_string * hashed_details;
    char *  key_string;
    int     key_length;
    zend_string * key_str;
#else
    zval ** arg;
    char * hashed_details;
#endif
    int    hashed_details_length;
    sqlany_connection_t * sqlany_conn;

    if( !api.initialized ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, DBCAPI_NOT_FOUND_ERROR );
	RETURN_FALSE;
    }

    if( ( ZEND_NUM_ARGS() != 1) || (zend_get_parameters_array_ex(1, &arg) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }

    sqlany_convert_to_string_ex( arg );

#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect( '%s', persistance=%d )",
		Z_STRVAL(arg), persistent ) );

    key_length = strlen( sasql_HASH_STR ) + Z_STRLEN(arg);
    key_string = (char *)emalloc( key_length + 1 );
    php_sprintf( key_string, "%s%s", sasql_HASH_STR, Z_STRVAL(arg) );
    hashed_details = zend_string_init(key_string, key_length, 0);
    hashed_details_length = key_length;
    efree( key_string );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect( '%s', persistance=%d )",
		Z_STRVAL_PP(arg), persistent ) );

    hashed_details_length = strlen( sasql_HASH_STR ) + Z_STRLEN_PP(arg);
    hashed_details = (char *)emalloc( hashed_details_length + 1 );
    php_sprintf( hashed_details, "%s%s", sasql_HASH_STR, Z_STRVAL_PP(arg) );
#endif

    if( ! SAG(allow_persistent) ) {
	persistent = 0;
    }

try_again:
    /* always check the persistent list when no connection parms are given */
    if( persistent ||
	    hashed_details_length == strlen( sasql_HASH_STR ) + 1 ) {

	zend_rsrc_list_entry * le = NULL;

	/* try to find if we already have this link in our persistent list */
#if PHP_MAJOR_VERSION >= 7
        if ((le = zend_hash_find_ptr(&EG(persistent_list), hashed_details)) == NULL) {
#else
        if (zend_hash_find(&EG(persistent_list), hashed_details, hashed_details_length+1,
                                        (void **) &le)==FAILURE) {
#endif
	    /* we don't */
	    zend_rsrc_list_entry new_le;

	    if( SAG(max_conns) != -1 &&
		    SAG(num_conns) >= SAG(max_conns)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, 
			"SQLAnywhere: Too many open connections (%ld)",SAG(num_conns));
#if PHP_MAJOR_VERSION >= 7
                zend_string_release( hashed_details );
#else
		efree( hashed_details );
#endif
		_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect=>FALSE" ) );
		RETURN_FALSE;
	    }
	    if( SAG(max_pconns) != -1 &&
		    SAG(num_pconns) >= SAG(max_pconns) ) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"SQLAnywherel:  Too many open persistent connections (%ld)",
			SAG(num_pconns) );
#if PHP_MAJOR_VERSION >= 7
                zend_string_release( hashed_details );
#else
		efree( hashed_details );
#endif
		_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect=>FALSE" ) );
		RETURN_FALSE;
	    }

	    sqlany_conn = calloc( 1, sizeof(sqlany_connection_t) );
	    if( sqlany_conn == NULL ) {
#if PHP_MAJOR_VERSION >= 7
                zend_string_release( hashed_details );
#else
		efree( hashed_details );
#endif
		RETURN_FALSE;
	    }
	    clear_conn_error( sqlany_conn );

	    if( SAG(context) ) {
		sqlany_conn->handle = api.sqlany_new_connection_ex( SAG(context) );
	    } else {
		sqlany_conn->handle = api.sqlany_new_connection();
	    }
	    if( sqlany_conn->handle == NULL ) {
		free( sqlany_conn );
#if PHP_MAJOR_VERSION >= 7
                zend_string_release( hashed_details );
#else
		efree( hashed_details );
#endif
		RETURN_FALSE;
	    }

	    sqlany_conn->persistent = persistent;
	    sqlany_conn->auto_commit = SAG(auto_commit);
	    sqlany_conn->verbose_errors = SAG(verbose_errors);

#if PHP_MAJOR_VERSION >= 7
	    if( !api.sqlany_connect( sqlany_conn->handle, Z_STRVAL(arg) ) ) {
#else
	    if( !api.sqlany_connect( sqlany_conn->handle, Z_STRVAL_PP(arg) ) ) {
#endif
		save_conn_error( sqlany_conn );
		VerboseErrors( sqlany_conn TSRMLS_CC );
		SAG(error_code) = sqlany_conn->errorcode;
		memcpy( SAG(error), sqlany_conn->error, SACAPI_ERROR_SIZE );
		strcpy( SAG(sqlstate), sqlany_conn->sqlstate );
		api.sqlany_free_connection( sqlany_conn->handle );
		free( sqlany_conn );
#if PHP_MAJOR_VERSION >= 7
                zend_string_release( hashed_details );
#else
		efree( hashed_details );
#endif
		_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect=>FALSE [%s]",
			    SAG(error) ) );
		RETURN_FALSE;
	    }

	    new_le.type = le_pconn;
	    new_le.ptr  = sqlany_conn;
#if PHP_MAJOR_VERSION >= 7
	    if( zend_hash_update_mem( &EG(persistent_list), hashed_details,
			(void *)&new_le, sizeof(zend_rsrc_list_entry) ) == NULL ) {
#else
	    if( zend_hash_update(&EG(persistent_list), hashed_details,
			hashed_details_length+1, (void *) &new_le,
			sizeof(zend_rsrc_list_entry),NULL)==FAILURE) {
#endif
		api.sqlany_disconnect( sqlany_conn->handle );
		api.sqlany_free_connection( sqlany_conn->handle );
#if PHP_MAJOR_VERSION >= 7
                zend_string_release( hashed_details );
#else
		efree( hashed_details );
#endif
		free( sqlany_conn );
		_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect=>FALSE" ) );
		RETURN_FALSE;
	    }
	    SAG(num_pconns)++;
	    SAG(num_conns)++;
	} else { /* we do */
	    if( le->type != le_pconn ) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, 
			"SQLAnywhere:  Hashed persistent link is not a SQLAnywhere link!");
		_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect=>FALSE" ) );
		RETURN_FALSE;
	    }

	    sqlany_conn = (sqlany_connection_t  *) le->ptr;

	    // test the connection
	    if( !api.sqlany_execute_immediate( sqlany_conn->handle, "select 1 from dummy" ) ) {
		// connection is broken, destroy it and create a new one.
#if PHP_MAJOR_VERSION >= 7
		zend_hash_del(&EG(persistent_list), hashed_details );
#else
		zend_hash_del(&EG(persistent_list), hashed_details, hashed_details_length+1 );
#endif
		goto try_again;
	    }
	}
#if PHP_MAJOR_VERSION >= 7
	sqlany_conn->res = Z_RES_P(zend_list_insert( sqlany_conn, le_pconn ));
#else
	sqlany_conn->id = zend_list_insert(sqlany_conn, le_pconn TSRMLS_CC);
#endif
    } else {
	/* ! persistent */
	zend_rsrc_list_entry * index_ptr, new_index_ptr;
#if PHP_MAJOR_VERSION >= 7
        if ((index_ptr = zend_hash_find_ptr(&EG(regular_list), hashed_details)) != NULL) {
#else
	if (zend_hash_find(&EG(regular_list),hashed_details,hashed_details_length+1,
		    (void **) &index_ptr)==SUCCESS) {
#endif
	    int type;
	    long id;
	    void * ptr;

	    if( index_ptr->type != le_index_ptr ) {
		RETURN_FALSE;
	    }

#if PHP_MAJOR_VERSION >= 7
            id = index_ptr->handle;
            type = index_ptr->type;
            ptr = index_ptr->ptr;
#else
	    id = (long)index_ptr->ptr;
	    ptr = (void *)zend_list_find( id, &type ); /* check if the link is still there */
#endif
	    if( ptr && (type == le_conn || type == le_pconn ) ) {
#if PHP_MAJOR_VERSION >= 7
                zend_string_release( hashed_details );
#else
		efree( hashed_details );
#endif

		ZVAL_LONG( return_value, id );
		return;
	    } else {
#if PHP_MAJOR_VERSION >= 7
		zend_hash_del( &EG(regular_list), hashed_details );
#else
		zend_hash_del( &EG(regular_list), hashed_details, hashed_details_length+1 );
#endif
	    }
	}
	if( SAG(max_conns) != -1 &&
		SAG(num_conns) >= SAG(max_conns) ) {
	    php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		    "SQLAnywhere: Too many open connections (%ld)",SAG(num_conns));
#if PHP_MAJOR_VERSION >= 7
            zend_string_release( hashed_details );
#else
	    efree( hashed_details );
#endif
	    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect=>FALSE" ) );
	    RETURN_FALSE;
	}

	/* actually establish the connection */
	sqlany_conn = ecalloc( 1, sizeof(sqlany_connection_t));
	if( sqlany_conn == NULL ) {
#if PHP_MAJOR_VERSION >= 7
            zend_string_release( hashed_details );
#else
	    efree( hashed_details );
#endif
	    RETURN_FALSE;
	}
	clear_conn_error( sqlany_conn );

	if( SAG(context) ) {
	    sqlany_conn->handle = api.sqlany_new_connection_ex( SAG(context) );
	} else {
	    sqlany_conn->handle = api.sqlany_new_connection();
	}
	if( sqlany_conn->handle == NULL ) {
	    efree( sqlany_conn );
#if PHP_MAJOR_VERSION >= 7
            zend_string_release( hashed_details );
#else
	    efree( hashed_details );
#endif
	    RETURN_FALSE;
	}
	sqlany_conn->auto_commit = SAG(auto_commit);
	sqlany_conn->verbose_errors = SAG(verbose_errors);
	sqlany_conn->persistent = persistent;

#if PHP_MAJOR_VERSION >= 7
	if( !api.sqlany_connect( sqlany_conn->handle, Z_STRVAL(arg) ) ) {
#else
	if( !api.sqlany_connect( sqlany_conn->handle, Z_STRVAL_PP(arg) ) ) {
#endif
	    save_conn_error( sqlany_conn );
	    VerboseErrors( sqlany_conn TSRMLS_CC );
	    SAG(error_code) = sqlany_conn->errorcode;
	    memcpy( SAG(error), sqlany_conn->error, SACAPI_ERROR_SIZE );
	    strcpy( SAG(sqlstate), sqlany_conn->sqlstate );
	    api.sqlany_free_connection( sqlany_conn->handle );
	    efree( sqlany_conn );
#if PHP_MAJOR_VERSION >= 7
            zend_string_release( hashed_details );
#else
	    efree( hashed_details );
#endif
	    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect=>FALSE[%s]", SAG(error) ) );
	    RETURN_FALSE;
	}
#if PHP_MAJOR_VERSION >= 7
	sqlany_conn->res = Z_RES_P(zend_list_insert( sqlany_conn, le_conn ));
#else
	sqlany_conn->id = zend_list_insert( sqlany_conn,le_conn TSRMLS_CC);
#endif

	new_index_ptr.ptr  = (void *) Z_LVAL_P( return_value );
	new_index_ptr.type = le_index_ptr;
#if PHP_MAJOR_VERSION >= 7
	if( zend_hash_update_mem( &EG(regular_list),hashed_details,
		    (void *) &new_index_ptr, sizeof(zend_rsrc_list_entry) ) == NULL ) {
            zend_string_release( hashed_details );
#else
	if (zend_hash_update(&EG(regular_list),hashed_details,hashed_details_length+1,
		    (void *) &new_index_ptr, sizeof(zend_rsrc_list_entry),NULL)==FAILURE) {
	    efree(hashed_details);
#endif
	    api.sqlany_disconnect( sqlany_conn->handle );
	    api.sqlany_free_connection( sqlany_conn->handle );
	    efree( sqlany_conn );
	    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect=>FALSE" ) );
	    RETURN_FALSE;
	}
	SAG(num_conns)++;
    }

#if PHP_MAJOR_VERSION >= 7
    zend_string_release( hashed_details );
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect=>%p", sqlany_conn->res ) );
    /* default to return return_value->value.lval */
    RETURN_RES( sqlany_conn->res );
#else
    efree(hashed_details);
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_connect=>%d", sqlany_conn->id ) );
    /* default to return return_value->value.lval */
    RETURN_RESOURCE( sqlany_conn->id );
#endif
}

/* {{{ proto sasql_conn sasql_pconnect_from_sqlca( void* sqlca)
   Create a persistent SQLAnywhere server connection using the given sqlca
   It can be accessed later by using sasql_pconnect( '' )
   Only one of thse may exist at a time.
   This should only be used by the SQLAnywhere PHP External Environment driver
*/
PHP_FUNCTION(sasql_pconnect_from_sqlca)
/**************************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    zv_sqlca;
#else
    zval ** zv_sqlca;
#endif
    void *sqlca;
    sqlany_connection_t * sqlany_conn;
#if PHP_MAJOR_VERSION >= 7
    zend_string * hashed_details = NULL;
#else
    char * hashed_details = sasql_HASH_STR;
#endif
    zend_rsrc_list_entry new_le;
    zend_rsrc_list_entry * le = NULL;

    if( !api.initialized ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, DBCAPI_NOT_FOUND_ERROR );
	RETURN_FALSE;
    }

#if PHP_MAJOR_VERSION >= 7
    hashed_details = zend_string_init(sasql_HASH_STR, strlen(sasql_HASH_STR), 0);
    if( ( ZEND_NUM_ARGS() != 1 ) ||
        ( zend_get_parameters_array_ex( 1, &zv_sqlca ) != SUCCESS ) ) {
        zend_string_release( hashed_details );
        WRONG_PARAM_COUNT;
    }
#else
    if( ( ZEND_NUM_ARGS() != 1 ) ||
        ( zend_get_parameters_array_ex( 1, &zv_sqlca ) != SUCCESS ) ) {
        WRONG_PARAM_COUNT;
    }
#endif

    /* we convert to long because we expect the zval struct to have type long
     * but the value should be stored in str.val (because it could be a 64-bit
     * pointer as opposed to a long int)
     */
    sqlany_convert_to_long_ex( zv_sqlca );
#if PHP_MAJOR_VERSION >= 7
    sqlca = (void*)Z_STRVAL(zv_sqlca);
#else
    sqlca = (void*)Z_STRVAL_PP(zv_sqlca);
#endif

    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_pconnect_from_sqlca( sqlca = %p )",
		sqlca ) );

    /* try to find if we already have this link in our persistent list */
#if PHP_MAJOR_VERSION >= 7
    if ( (le = zend_hash_find_ptr( &EG(persistent_list), hashed_details )) != NULL ) {
        zend_string_release( hashed_details );
#else
    if ( zend_hash_find( &EG(persistent_list), hashed_details,
                         strlen( hashed_details ) + 1,
                         (void **) &le ) != FAILURE ) {
#endif
        /* we already have this connection, so we fail
         * this seems to be better than silently returning the connection
         * that uses a different sqlca from what was passed in
         * which would cause confusion
         */
	_debug_enabled( SQLAnyDebug( _LOCATION_,
		    " sasql_pconnect_from_sqlca return FALSE" ) );
        RETURN_FALSE;
    }

    /* actually create the connection */
    sqlany_conn = calloc( 1, sizeof(sqlany_connection_t));
    if( sqlany_conn == NULL ) {
#if PHP_MAJOR_VERSION >= 7
        zend_string_release( hashed_details );
#endif
        RETURN_FALSE;
    }
    if( SAG(context) ) {
	sqlany_conn->handle = api.sqlany_make_connection_ex( SAG(context), sqlca );
    } else {
	sqlany_conn->handle = api.sqlany_make_connection( sqlca );
    }
    if( sqlany_conn->handle == NULL ) {
#if PHP_MAJOR_VERSION >= 7
        zend_string_release( hashed_details );
#endif
        RETURN_FALSE;
    }

    sqlany_conn->persistent = 1;
    sqlany_conn->auto_commit = SAG(auto_commit);
    sqlany_conn->verbose_errors = SAG(verbose_errors);
#if PHP_MAJOR_VERSION >= 7
    sqlany_conn->res = NULL;
#else
    sqlany_conn->id = 0;
#endif
    sqlany_conn->result_list = NULL;
    sqlany_conn->ext_env_conn = 1;

    new_le.type = le_pconn;
    new_le.ptr  = sqlany_conn;
#if PHP_MAJOR_VERSION >= 7
    if( zend_hash_update_mem( &EG( persistent_list ), hashed_details,
                          (void *) &new_le, sizeof( zend_rsrc_list_entry ) ) == NULL ) {
        zend_string_release( hashed_details );
#else
    if( zend_hash_update( &EG( persistent_list ), hashed_details,
                          strlen( hashed_details ) + 1, (void *) &new_le,
                          sizeof( zend_rsrc_list_entry ), NULL ) == FAILURE ) {
#endif
        /* we don't want to close the connection because it
         * belongs to someone else */
        api.sqlany_free_connection( sqlany_conn->handle );
        efree( sqlany_conn );
        RETURN_FALSE;
    }

#if PHP_MAJOR_VERSION >= 7
    sqlany_conn->res = Z_RES_P(zend_list_insert( sqlany_conn, le_pconn ));
#else
    sqlany_conn->id = zend_list_insert(sqlany_conn, le_pconn TSRMLS_CC);
#endif

    SAG( num_pconns )++;
    SAG( num_conns )++;
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_pconnect_from_sqlca return %p",
		sqlany_conn->res ) );
    RETURN_RES( sqlany_conn->res );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_pconnect_from_sqlca return %d",
		sqlany_conn->id ) );
    RETURN_LONG( sqlany_conn->id );
#endif
}
/* }}} */

/* {{{ proto bool sasql_close( sasql_conn $conn )
   Closes a connection */
PHP_FUNCTION(sasql_close)
/*****************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    arg;
#else
    zval ** arg;
#endif
    sqlany_connection_t * sqlany_conn;
    zend_rsrc_list_entry * le;

    if( ( ZEND_NUM_ARGS() != 1) || (zend_get_parameters_array_ex(1,&arg) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }

    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, arg, -1, "SQLAnywhere connection",
	    le_conn, le_pconn );

#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_discconnect( %p )", sqlany_conn->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_discconnect( %d )", sqlany_conn->id ) );
#endif

#if PHP_MAJOR_VERSION >= 7
    zend_string * hashed_details = NULL;
    hashed_details = zend_string_init(sasql_HASH_STR, strlen(sasql_HASH_STR), 0);

    if ( (le = zend_hash_find_ptr( &EG(persistent_list), hashed_details )) != NULL ) {
        zend_string_release( hashed_details );
#else
    if ( zend_hash_find( &EG(persistent_list),  sasql_HASH_STR,
		strlen( sasql_HASH_STR ) + 1,
		(void **) &le ) != FAILURE  &&
	    (sqlany_connection_t *)(le->ptr) == sqlany_conn ) {
#endif
#if PHP_MAJOR_VERSION >= 7
	php_error_docref(NULL TSRMLS_CC, E_WARNING,
		"Supplied argument 1 (%ld) refers to the default connection, which cannot be closed"
		, Z_LVAL(arg) );
#else
	php_error_docref(NULL TSRMLS_CC, E_WARNING,
		"Supplied argument 1 (%ld) refers to the default connection, which cannot be closed"
		, Z_LVAL_PP(arg) );
#endif
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_close=>FALSE" ) );
	RETURN_FALSE;
    }

#if PHP_MAJOR_VERSION >= 7
    zend_string_release( hashed_details );
#endif

#if PHP_MAJOR_VERSION >= 7
    zend_list_close( Z_RES_P(&arg) );
#else
    zend_list_delete( Z_LVAL_PP(arg) );
#endif
    RETURN_TRUE;
}
/* }}} */

static
int sasql_real_query( sqlany_connection_t * sqlany_conn, char * query TSRMLS_DC )
/********************************************************************************/
{
    if( sqlany_conn->last_stmt != NULL ) {
	/* Before we open a new statement, make sure we close the previous default
	 * statement if and only if it's associated result set has been freed.
	 * When the result set gets closed and it's statement is not the default
	 * statement then the statement will be closed then. We need to keep the
	 * default statement around in case the user wants to call sasql_next_result()
	 * on the connection.
	 */
	sqlany_result_t * curr = sqlany_conn->result_list;
	int		  found = 0;
	while( curr != NULL ) {
	    if( curr->stmt_handle == sqlany_conn->last_stmt ) {
		found = 1;
		break;
	    }
	    curr = curr->next;
	}
	if( !found ) {
	    api.sqlany_free_stmt( sqlany_conn->last_stmt );
	}
    }
    sqlany_conn->last_stmt = api.sqlany_execute_direct( sqlany_conn->handle, query );
    save_conn_error( sqlany_conn );
    if( sqlany_conn->last_stmt == NULL ) {
	VerboseErrors( sqlany_conn TSRMLS_CC );
	_debug_enabled( SQLAnyDebug( _LOCATION_, " ssasql_real_query=>FALSE [%s]",
		    sqlany_conn->error ) );
	return 0;
    }

    if( api.sqlany_num_cols( sqlany_conn->last_stmt ) == 0 &&
	    sqlany_conn->auto_commit ) {
	api.sqlany_commit( sqlany_conn->handle );
    }
    return 1;
}

/* {{{ proto bool sasql_real_query( sasql_conn $conn, string $sql_str )
   Execute a query */
PHP_FUNCTION(sasql_real_query)
/****************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[2];
#else
    zval ** args[2];
#endif
    sqlany_connection_t * sqlany_conn;

    switch( ZEND_NUM_ARGS() ) {
	case 2:
	    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), args ) == FAILURE ) {
		WRONG_PARAM_COUNT;
	    }
	    sqlany_convert_to_string_ex( args[1] );
	    break;
	default:
	    WRONG_PARAM_COUNT;
	    break;
    }

    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, args[0], -1,
	    "SQLAnywhere connection", le_conn, le_pconn );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_real_query( conn=%p, sql=%s )",
		sqlany_conn->res, Z_STRVAL(args[1]) ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_real_query( conn=%d, sql=%s )",
		sqlany_conn->id, Z_STRVAL_PP(args[1]) ) );
#endif

#if PHP_MAJOR_VERSION >= 7
    if( sasql_real_query( sqlany_conn, Z_STRVAL(args[1]) TSRMLS_CC ) ) {
#else
    if( sasql_real_query( sqlany_conn, Z_STRVAL_PP(args[1]) TSRMLS_CC ) ) {
#endif
	RETURN_TRUE;
    } else {
	RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto mixed sasql_query(int conn, char * sql_str [, int $result_mode ] )
   Execute a query */
PHP_FUNCTION(sasql_query)
/************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval   		  args[3];
#else
    zval ** 		  args[3];
#endif
    sqlany_connection_t * sqlany_conn;
    sqlany_result_t * 	  sqlany_result;
    int			  store_result;
    int 		  rc;

    switch( ZEND_NUM_ARGS() ) {
	case 3:
	    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), args ) == FAILURE ) {
		WRONG_PARAM_COUNT;
	    }
	    sqlany_convert_to_long_ex( args[2] );
	    sqlany_convert_to_string_ex( args[1] );
#if PHP_MAJOR_VERSION >= 7
	    store_result = Z_LVAL(args[2]);
#else
	    store_result = Z_LVAL_PP(args[2]);
#endif
	    break;
	case 2:
#if PHP_MAJOR_VERSION < 7
	    args[2] = NULL;
#endif
	    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), args ) == FAILURE ) {
		WRONG_PARAM_COUNT;
	    }
	    store_result = 1;
	    sqlany_convert_to_string_ex( args[1] );
	    break;
	default:
	    WRONG_PARAM_COUNT;
	    break;
    }

    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, args[0], -1,
	    "SQLAnywhere connection", le_conn, le_pconn );

#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_query( conn=%p, sql=%s )",
		sqlany_conn->res, Z_STRVAL(args[1]) ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_query( conn=%d, sql=%s )",
		sqlany_conn->id, Z_STRVAL_PP(args[1]) ) );
#endif

#if PHP_MAJOR_VERSION >= 7
    if( !sasql_real_query( sqlany_conn, Z_STRVAL(args[1]) TSRMLS_CC ) ) {
#else
    if( !sasql_real_query( sqlany_conn, Z_STRVAL_PP(args[1]) TSRMLS_CC ) ) {
#endif
	RETURN_FALSE;
    }

    if( api.sqlany_num_cols( sqlany_conn->last_stmt ) == 0 ) {
	RETURN_TRUE;
    }

    // otherwise, create a result object
    sqlany_result = (sqlany_result_t *)ecalloc( 1, sizeof(sqlany_result_t) );
#if PHP_MAJOR_VERSION >= 7
    sqlany_result->res = Z_RES_P(zend_list_insert( sqlany_result, le_result ));
#else
    sqlany_result->id = zend_list_insert( sqlany_result, le_result TSRMLS_CC); 
#endif
    sqlany_result->sqlany_conn = sqlany_conn;
    sqlany_result->stmt_handle = sqlany_conn->last_stmt;
    sqlany_result->num_cols = api.sqlany_num_cols( sqlany_result->stmt_handle );
    sqlany_result->num_rows = api.sqlany_num_rows( sqlany_result->stmt_handle );
    sqlany_result->next = sqlany_conn->result_list;
    sqlany_conn->result_list = sqlany_result;

    if( !store_result ) {
#if PHP_MAJOR_VERSION >= 7
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_query=>%p use_result",
		    sqlany_result->res ) );
	RETURN_RES( sqlany_result->res );
#else
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_query=>%d use_result",
		    sqlany_result->id ) );
	RETURN_RESOURCE( sqlany_result->id );
#endif
    }

    rc = cache_result( sqlany_result );
    if( rc == 100 || rc == 0 ) {
	rc = 0;
	clear_conn_error( sqlany_conn );
    } else {
	save_conn_error( sqlany_conn );
    }
    if( rc >= 0 ) {
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_query:err=%d", rc ) );
	if( sqlany_conn->auto_commit ) {
	    api.sqlany_commit( sqlany_conn->handle );
#if PHP_MAJOR_VERSION >= 7
	    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_query:auto commit", sqlany_result->res ) );
#else
	    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_query:auto commit", sqlany_result->id ) );
#endif
	}
    } else {
	if( sqlany_conn->auto_commit ) {
	    api.sqlany_rollback( sqlany_conn->handle );
#if PHP_MAJOR_VERSION >= 7
	    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_query:auto commit", sqlany_result->res ) );
#else
	    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_query:auto commit", sqlany_result->id ) );
#endif
	}
    }
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_query=>%p stored_result, num_rows=%d num_cols=%d",
			    sqlany_result->res, sqlany_result->num_rows, sqlany_result->num_cols ) );
    RETURN_RES( sqlany_result->res );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_query=>%d stored_result, num_rows=%d num_cols=%d",
			    sqlany_result->id, sqlany_result->num_rows, sqlany_result->num_cols ) );
    RETURN_RESOURCE( sqlany_result->id );
#endif
}
/* }}} */

/* {{{ proto mixed sasql_store_result( sasql_conn $conn )
   Execute a query */
PHP_FUNCTION(sasql_store_result)
/******************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[1];
#else
    zval ** args[1];
#endif
    sqlany_connection_t * sqlany_conn;
    sqlany_result_t * 	  sqlany_result;
    int			  rc;

    switch( ZEND_NUM_ARGS() ) {
	case 1:
	    if( zend_get_parameters_array_ex( 1, args ) == FAILURE ) {
		WRONG_PARAM_COUNT;
	    }
	    break;
	default:
	    WRONG_PARAM_COUNT;
	    break;
    }

    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, args[0], -1,
	    "SQLAnywhere connection", le_conn, le_pconn );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_store_result( conn=%p, sql=%s )",
		sqlany_conn->res, Z_STRVAL(args[0]) ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_store_result( conn=%d, sql=%s )",
		sqlany_conn->id, Z_STRVAL_PP(args[0]) ) );
#endif

    if( sqlany_conn->last_stmt == NULL ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "No previous result to store" );
	RETURN_FALSE;
    }

    if( api.sqlany_num_cols( sqlany_conn->last_stmt ) == 0 ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Previous query does not return a result" );
	RETURN_FALSE;
    }

    // otherwise, create a result object
    sqlany_result = (sqlany_result_t *)ecalloc( 1, sizeof(sqlany_result_t) );
#if PHP_MAJOR_VERSION >= 7
    sqlany_result->res = Z_RES_P(zend_list_insert( sqlany_result, le_result ));
#else
    sqlany_result->id = zend_list_insert( sqlany_result, le_result TSRMLS_CC); 
#endif
    sqlany_result->sqlany_conn = sqlany_conn;
    sqlany_result->stmt_handle = sqlany_conn->last_stmt;
    sqlany_result->num_cols = api.sqlany_num_cols( sqlany_result->stmt_handle );
    sqlany_result->next = sqlany_conn->result_list;
    sqlany_conn->result_list = sqlany_result;

    rc = cache_result( sqlany_result );
    if( rc == 100 || rc == 0 ) {
	rc = 0;
	clear_conn_error( sqlany_conn );
    } else {
	save_conn_error( sqlany_conn );
    }

    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_store_result:err=%d", rc ) );
    if( rc >= 0 ) {
	if( sqlany_conn->auto_commit ) {
	    api.sqlany_commit( sqlany_conn->handle );
#if PHP_MAJOR_VERSION >= 7
	    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_store_result:auto commit", sqlany_result->res ) );
#else
	    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_store_result:auto commit", sqlany_result->id ) );
#endif
	}
    } else {
	if( sqlany_conn->auto_commit ) {
	    api.sqlany_rollback( sqlany_conn->handle );
#if PHP_MAJOR_VERSION >= 7
	    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_store_result:auto rolling back", sqlany_result->res ) );
#else
	    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_store_result:auto rolling back", sqlany_result->id ) );
#endif
	}
    }
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_store_result=>%p", sqlany_result->res ) );
    RETURN_RES( sqlany_result->res );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_store_result=>%d", sqlany_result->id ) );
    RETURN_RESOURCE( sqlany_result->id );
#endif
}
/* }}} */

/* {{{ proto sasql_result sasql_use_result( sasql_conn $conn )
   */
PHP_FUNCTION(sasql_use_result)
/******************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[1];
#else
    zval ** args[1];
#endif
    sqlany_connection_t * sqlany_conn;
    sqlany_result_t * 	  sqlany_result;

    switch( ZEND_NUM_ARGS() ) {
	case 1:
	    if( zend_get_parameters_array_ex( 1, args ) == FAILURE ) {
		WRONG_PARAM_COUNT;
	    }
	    break;
	default:
	    WRONG_PARAM_COUNT;
	    break;
    }
    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, args[0], -1,
	    "SQLAnywhere connection", le_conn, le_pconn );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_use_result( conn=%p, sql=%s )",
		sqlany_conn->res, Z_STRVAL(args[0]) ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_use_result( conn=%d, sql=%s )",
		sqlany_conn->id, Z_STRVAL_PP(args[0]) ) );
#endif

    if( sqlany_conn->last_stmt == NULL ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "No previous result to store" );
	RETURN_FALSE;
    }

    if( api.sqlany_num_cols( sqlany_conn->last_stmt ) == 0 ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Previous query does not return a result" );
	RETURN_FALSE;
    }

    // otherwise, create a result object
    sqlany_result = (sqlany_result_t *)ecalloc( 1, sizeof(sqlany_result_t) );
#if PHP_MAJOR_VERSION >= 7
    sqlany_result->res = Z_RES_P(zend_list_insert( sqlany_result, le_result ));
#else
    sqlany_result->id = zend_list_insert( sqlany_result, le_result TSRMLS_CC); 
#endif
    sqlany_result->sqlany_conn = sqlany_conn;
    sqlany_result->stmt_handle = sqlany_conn->last_stmt;
    sqlany_result->num_cols = api.sqlany_num_cols( sqlany_result->stmt_handle );
    sqlany_result->num_rows = api.sqlany_num_rows( sqlany_result->stmt_handle );
    sqlany_result->next = sqlany_conn->result_list;
    sqlany_conn->result_list = sqlany_result;

#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_use_result=>%p", sqlany_result->res ) );
    RETURN_RES( sqlany_result->res );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_use_result=>%d", sqlany_result->id ) );
    RETURN_RESOURCE( sqlany_result->id );
#endif
}
/* }}} */

/* {{{ proto bool sasql_next_result( sasql_conn $conn )
   Retrieves the next result set */
PHP_FUNCTION(sasql_next_result)
/******************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[1];
#else
    zval ** args[1];
#endif
    sqlany_connection_t * sqlany_conn;
    int     ok;

    switch( ZEND_NUM_ARGS() ) {
	case 1:
	    if( zend_get_parameters_array_ex( 1, args ) == FAILURE ) {
		WRONG_PARAM_COUNT;
	    }
	    break;
	default:
	    WRONG_PARAM_COUNT;
	    break;
    }
    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, args[0], -1,
	    "SQLAnywhere connection", le_conn, le_pconn );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_next_result( conn=%p, sql=%s )",
		sqlany_conn->res, Z_STRVAL(args[0]) ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_next_result( conn=%d, sql=%s )",
		sqlany_conn->id, Z_STRVAL_PP(args[0]) ) );
#endif

    if( sqlany_conn->last_stmt == NULL ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "No previous result to store" );
	RETURN_FALSE;
    }

    ok = api.sqlany_get_next_result( sqlany_conn->last_stmt );
    if( api.sqlany_error( sqlany_conn->handle, NULL, 0 ) == 105 ) {
	clear_conn_error( sqlany_conn );
    } else {
	save_conn_error( sqlany_conn );
    }

    if( ok ) {
	sqlany_result_t * sqlany_result = (sqlany_result_t *)ecalloc( 1, sizeof(sqlany_result_t) );
	sqlany_result->num_cols = api.sqlany_num_cols( sqlany_conn->last_stmt );
	sqlany_result->num_rows = api.sqlany_num_rows( sqlany_conn->last_stmt );
#if PHP_MAJOR_VERSION >= 7
        sqlany_result->res = Z_RES_P(zend_list_insert( sqlany_result, le_result ));
#else
	sqlany_result->id = zend_list_insert( sqlany_result, le_result TSRMLS_CC); 
#endif
	sqlany_result->sqlany_conn = sqlany_conn;
	sqlany_result->next = sqlany_conn->result_list;
	sqlany_result->sqlany_stmt = NULL;
	sqlany_result->stmt_handle = sqlany_conn->last_stmt;
	sqlany_conn->result_list = sqlany_result;
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_next_result=>TRUE" ) );
	RETURN_TRUE;
    } else {
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_next_result=>FALSE" ) );
	RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto bool sasql_free_result( sasql_result $result )
   Free result memory */
PHP_FUNCTION(sasql_free_result)
/******************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_result_t	* sqlany_result;

#if PHP_MAJOR_VERSION >= 7
    if( ( ZEND_NUM_ARGS() != 1) || (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
#else
    if( ( ZEND_NUM_ARGS() != 1) || (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
#endif

    ZEND_FETCH_RESOURCE( sqlany_result, sqlany_result_t*, arg1, -1, "SQLAnywhere result",
	    le_result );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_free_result( %p )", sqlany_result->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_free_result( %d )", sqlany_result->id ) );
#endif

#if PHP_MAJOR_VERSION >= 7
    zend_list_close( sqlany_result->res );
#else
    zend_list_delete( sqlany_result->id );
#endif
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_free_result=>SUCCESS" ) );
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool sasql_data_seek(int result, int row_num)
   Move internal row pointer */
PHP_FUNCTION(sasql_data_seek)
/****************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[2];
#else
    zval ** args[2];
#endif
    sqlany_result_t	* sqlany_result;

    if( ( ZEND_NUM_ARGS() != 2) || (zend_get_parameters_array_ex(2,args) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
    sqlany_convert_to_long_ex( args[1] );

    ZEND_FETCH_RESOURCE( sqlany_result, sqlany_result_t*, args[0], -1,
	    "SQLAnywhere result", le_result );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_data_seek( %p, %d )",
		sqlany_result->res, Z_LVAL( args[1] ) ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_data_seek( %d, %d )",
		sqlany_result->id, Z_LVAL_PP( args[1] ) ) );
#endif

    if( !sqlany_result->result_cached ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"sasql_data_seek() cannot be used with a result object returned from sasql_use_result()" );
	RETURN_FALSE;
    }

#if PHP_MAJOR_VERSION >= 7
    if( result_data_seek( sqlany_result, Z_LVAL(args[1]), 1 ) ) {
#else
    if( result_data_seek( sqlany_result, Z_LVAL_PP(args[1]), 1 ) ) {
#endif
	RETURN_TRUE;
    }
    RETURN_FALSE;
}
/* }}} */


/* {{{ proto int sasql_num_rows( sasql_result $result )
   Get number of rows in result, FALSE if the result was not stored! */
PHP_FUNCTION(sasql_num_rows)
/********************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_result_t	* sqlany_result;

    if( ( ZEND_NUM_ARGS() != 1) || (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_result, sqlany_result_t*, arg1, -1, "SQLAnywhere result",
	    le_result );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_num_rows( %p )", sqlany_result->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_num_rows( %d )", sqlany_result->id ) );
#endif
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_num_rows=>%d", sqlany_result->num_rows ) );
    RETURN_LONG( sqlany_result->num_rows );
}
/* }}} */

/* {{{ proto int sasql_affected_rows( sasql_conn $conn )
   Get number of rows affected by the SQL statement. */
PHP_FUNCTION(sasql_affected_rows)
/********************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_connection_t	* sqlany_conn;

    if( ( ZEND_NUM_ARGS() != 1) || (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, arg1, -1,
	    "SQLAnywhere connection", le_conn, le_pconn );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_affected_rows( %p )", sqlany_conn->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_affected_rows( %d )", sqlany_conn->id ) );
#endif

    if( sqlany_conn->last_stmt != NULL ) {
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_affected_rows=>%d",
		    api.sqlany_affected_rows( sqlany_conn->last_stmt ) ) );
	RETURN_LONG( api.sqlany_affected_rows( sqlany_conn->last_stmt ) );
    } else {
	RETURN_LONG( 0 );
    }
}
/* }}} */

/* {{{ proto int sasql_field_count( sasql_conn $conn )
   Get number of columns from the last statement */
PHP_FUNCTION(sasql_field_count)
/********************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_connection_t	* sqlany_conn;

    if( ( ZEND_NUM_ARGS() != 1) || (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, arg1, -1,
	    "SQLAnywhere connection", le_conn, le_pconn );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_field_count( %p )", sqlany_conn->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_field_count( %d )", sqlany_conn->id ) );
#endif

    if( sqlany_conn->last_stmt != NULL ) {
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_field_count=>%d",
		    api.sqlany_affected_rows( sqlany_conn->last_stmt ) ) );
	RETURN_LONG( api.sqlany_num_cols( sqlany_conn->last_stmt ) );
    } else {
	RETURN_LONG( 0 );
    }
}
/* }}} */

/* {{{ proto bool sasql_field_seek(sasql result, int field_offset )
   Set result pointer to a specified field offset */
PHP_FUNCTION(sasql_field_seek)
/******************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[2];
#else
    zval ** args[2];
#endif
    int col_num = 0;
    sqlany_result_t *sqlany_result;

    if(ZEND_NUM_ARGS() != 2 ) {
	WRONG_PARAM_COUNT;
    }
    if (zend_get_parameters_array_ex(2, args) != SUCCESS) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_result, sqlany_result_t*, args[0], -1, "SQLAnywhere result", le_result );
    sqlany_convert_to_long_ex( args[1] );

#if PHP_MAJOR_VERSION >= 7
    col_num = Z_LVAL(args[1]);
#else
    col_num = Z_LVAL_PP(args[1]);
#endif

    if( col_num < 0 || col_num >= sqlany_result->num_cols ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"SQLAnywhere: Bad column offset (%d) min = 0, max = %d", col_num, sqlany_result->num_cols -1 );
	RETURN_FALSE;
    }

    sqlany_result->curr_col = col_num;
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto int sasql_fetch_field(int result [, field_offset] )
   Get field information */
PHP_FUNCTION(sasql_fetch_field)
/******************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval   args[2];
#else
    zval **args[2];
#endif
    int col_num = 0;
    sqlany_result_t *sqlany_result;
    a_sqlany_column_info cinfo;

    switch (ZEND_NUM_ARGS()) {
	case 2:
	    if (zend_get_parameters_array_ex(2, args) != SUCCESS) {
		WRONG_PARAM_COUNT;
	    }
	    sqlany_convert_to_long_ex(args[1]);
#if PHP_MAJOR_VERSION >= 7
	    col_num = Z_LVAL(args[1]) ;
#else
	    col_num = Z_LVAL_PP(args[1]) ;
#endif
	    break;

	case 1:
	    if (zend_get_parameters_array_ex(1, args) != SUCCESS) {
		WRONG_PARAM_COUNT;
	    }
	    break;

	default:
	    WRONG_PARAM_COUNT;
	    break;
    }
    ZEND_FETCH_RESOURCE( sqlany_result, sqlany_result_t*, args[0], -1, "SQLAnywhere result", le_result );

    switch( ZEND_NUM_ARGS() ) {
	case 1:
	    col_num = sqlany_result->curr_col;
	    if( col_num >= sqlany_result->num_cols ) {
		RETURN_FALSE;
	    }
	    sqlany_result->curr_col++;
	    break;

	case 2:
	    if( col_num < 0 || col_num >= sqlany_result->num_cols ) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, 
			"SQLAnywhere: Bad column offset (%d) min = 0, max = %d", col_num, sqlany_result->num_cols -1 );
		RETURN_FALSE;
	    }
	    sqlany_result->curr_col = col_num + 1;
	    break;

	default:
	    WRONG_PARAM_COUNT;
	    break;
    }

    if( !result_get_column_info( sqlany_result, col_num, &cinfo ) ) {
	RETURN_FALSE;
    }

#if (PHP_MAJOR_VERSION < 8)
    if( object_init( return_value ) == FAILURE ) {
	RETURN_FALSE;
    }
#else
    object_init( return_value );
#endif

#if PHP_MAJOR_VERSION >= 7
    #define sqlany_add_property_string_dup(__arg, __key, __str) \
            add_property_string(__arg, __key, __str ) // always created
#else
    #define sqlany_add_property_string_dup(__arg, __key, __str) \
            add_property_string(__arg, __key, __str, 1)
#endif
    add_property_long( return_value, "id", col_num );
    sqlany_add_property_string_dup( return_value, "name", cinfo.name );
    switch( cinfo.type ) {
	case A_STRING:
	    add_property_long( return_value, "numeric", 0 );
	    sqlany_add_property_string_dup( return_value, "type", "string" );
	    break;
	case A_BINARY:
	    add_property_long( return_value, "numeric", 0 );
	    sqlany_add_property_string_dup( return_value, "type", "blob" );
	    break;
	case A_DOUBLE:
	    add_property_long( return_value, "numeric", 1 );
	    sqlany_add_property_string_dup( return_value, "type", "double" );
	    break;
	case A_VAL64:
	case A_UVAL64:
	case A_VAL32:
	case A_UVAL32:
	case A_VAL16:
	case A_UVAL16:
	case A_VAL8:
	case A_UVAL8:
	    add_property_long( return_value, "numeric", 1 );
	    sqlany_add_property_string_dup( return_value, "type", "int" );
	    break;
	default:
	    add_property_long( return_value, "numeric", 0 );
	    sqlany_add_property_string_dup( return_value, "type", "unknown" );
    }
    switch( cinfo.native_type ) {
	case DT_NOTYPE:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_NOTYPE" );
	    break;
	case DT_DATE:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_DATE" );
	    break;
	case DT_TIME:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_TIME" );
	    break;
	case DT_TIMESTAMP:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_TIMESTAMP" );
	    break;
	case DT_VARCHAR:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_VARCHAR" );
	    break;
	case DT_FIXCHAR:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_FIXCHAR" );
	    break;
	case DT_LONGVARCHAR:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_LONGVARCHAR" );
	    break;
	case DT_STRING:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_STRING" );
	    break;
	case DT_DOUBLE:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_DOUBLE" );
	    break;
	case DT_FLOAT:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_FLOAT" );
	    break;
	case DT_DECIMAL:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_DECIMAL" );
	    add_property_long( return_value, "precision", cinfo.precision );
	    add_property_long( return_value, "scale", cinfo.scale );
	    break;
	case DT_INT:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_INT" );
	    break;
	case DT_SMALLINT:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_SMALLINT" );
	    break;
	case DT_BINARY:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_BINARY" );
	    break;
	case DT_LONGBINARY:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_LONGBINARY" );
	    break;
	case DT_TINYINT:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_TINYINT" );
	    break;
	case DT_BIGINT:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_BIGINT" );
	    break;
	case DT_UNSINT:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_UNSINT" );
	    break;
	case DT_UNSSMALLINT:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_UNSSMALLINT" );
	    break;
	case DT_UNSBIGINT:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_UNSBIGINT" );
	    break;
	case DT_BIT:
	    sqlany_add_property_string_dup( return_value, "native_type", "DT_BIT" );
	    break;
	default:
	    sqlany_add_property_string_dup( return_value, "native_type", "unknown" );
	    break;
    }
    /* FIXME: do something about the length and type fields, now they return numbers
       corresponding to database data type which is not meaningfull in php scripts */
    add_property_long( return_value, "length", cinfo.max_size );
}
/* }}} */

static
void assign_from_sqlany_value( zval * tmp, a_sqlany_data_value * sqlany_value )
/****************************************************************************/
{
    char buffer[128];

#if PHP_MAJOR_VERSION >= 7
    if( Z_ISREF_P( tmp ) ) {
        tmp = Z_REFVAL_P( tmp );
    }
#endif

    if( *(sqlany_value->is_null) ) {
#if PHP_MAJOR_VERSION >= 7
        ZVAL_NULL(tmp);
#else
	tmp->type = IS_NULL;
#endif
	return;
    }

    switch( sqlany_value->type ) {
	case A_STRING:
	case A_BINARY:
#if PHP_MAJOR_VERSION >= 7
            ZVAL_STRINGL( tmp, sqlany_value->buffer, *(sqlany_value->length) );
#else
	    tmp->value.str.len = *(sqlany_value->length);
	    tmp->value.str.val = emalloc( tmp->value.str.len + 1 );
	    memcpy( tmp->value.str.val, sqlany_value->buffer, tmp->value.str.len );
	    tmp->value.str.val[tmp->value.str.len] = '\0';
	    tmp->type = IS_STRING;
#endif
	    break;

	case A_VAL64:
	    if( (*(sasql_int64 *)sqlany_value->buffer) >= -INT_MAX &&
		    (*(sasql_int64 *)sqlany_value->buffer) <= INT_MAX ) {
#if PHP_MAJOR_VERSION >= 7
                ZVAL_LONG( tmp, *(sasql_int64 *)sqlany_value->buffer );
#else
		tmp->value.lval = *(sasql_int64 *)sqlany_value->buffer;
		tmp->type = IS_LONG;
#endif
	    } else {
		php_sprintf( buffer, "%"SA_FMT64"d", *(sasql_int64*)sqlany_value->buffer );
#if PHP_MAJOR_VERSION >= 7
                ZVAL_STRINGL( tmp, buffer, strlen( buffer ) );
#else
		tmp->value.str.len = strlen( buffer );
		tmp->value.str.val = emalloc( tmp->value.str.len + 1 );
		memcpy( tmp->value.str.val, buffer, tmp->value.str.len );
		tmp->value.str.val[tmp->value.str.len] = '\0';
		tmp->type = IS_STRING;
#endif
	    }
	    break;

	case A_UVAL64:
	    if( (*(sasql_uint64 *)sqlany_value->buffer) <= INT_MAX ) {
#if PHP_MAJOR_VERSION >= 7
                ZVAL_LONG( tmp, *(sasql_uint64 *)sqlany_value->buffer );
#else
		tmp->value.lval = *(sasql_uint64 *)sqlany_value->buffer;
		tmp->type = IS_LONG;
#endif
	    } else {
		php_sprintf( buffer, "%"SA_FMT64"u", *(sasql_uint64*)sqlany_value->buffer );
#if PHP_MAJOR_VERSION >= 7
                ZVAL_STRINGL( tmp, buffer, strlen( buffer ) );
#else
		tmp->value.str.len = strlen( buffer );
		tmp->value.str.val = emalloc( tmp->value.str.len + 1 );
		memcpy( tmp->value.str.val, buffer, tmp->value.str.len );
		tmp->value.str.val[tmp->value.str.len] = '\0';
		tmp->type = IS_STRING;
#endif
	    }
	    break;

	case A_DOUBLE:
#if PHP_MAJOR_VERSION >= 7
            ZVAL_DOUBLE( tmp, *(double *)sqlany_value->buffer );
#else
	    tmp->value.dval = *(double *)sqlany_value->buffer;
	    tmp->type = IS_DOUBLE;
#endif
	    break;

	case A_VAL32:
#if PHP_MAJOR_VERSION >= 7
            ZVAL_LONG( tmp, *(int *)sqlany_value->buffer );
#else
	    tmp->value.lval = *(int *)sqlany_value->buffer;
	    tmp->type = IS_LONG;
#endif
	    break;
	case A_UVAL32:
	    if( (*(unsigned int*)sqlany_value->buffer) <= INT_MAX ) {
#if PHP_MAJOR_VERSION >= 7
                ZVAL_LONG( tmp, *(unsigned int *)sqlany_value->buffer );
#else
		tmp->value.lval = *(unsigned int *)sqlany_value->buffer;
		tmp->type = IS_LONG;
#endif
	    } else {
		php_sprintf( buffer, "%u", *(unsigned int*)sqlany_value->buffer );
#if PHP_MAJOR_VERSION >= 7
                ZVAL_STRINGL( tmp, buffer, strlen( buffer ) );
#else
		tmp->value.str.val = estrdup( buffer );
		tmp->value.str.len = strlen( buffer );
		tmp->type = IS_STRING;
#endif
	    }
	    break;
	case A_VAL16:
#if PHP_MAJOR_VERSION >= 7
            ZVAL_LONG( tmp, *(short *)sqlany_value->buffer );
#else
	    tmp->value.lval = *(short *)sqlany_value->buffer;
	    tmp->type = IS_LONG;
#endif
	    break;
	case A_UVAL16:
#if PHP_MAJOR_VERSION >= 7
            ZVAL_LONG( tmp, *(unsigned short *)sqlany_value->buffer );
#else
	    tmp->value.lval = *(unsigned short *)sqlany_value->buffer;
	    tmp->type = IS_LONG;
#endif
	    break;
	case A_VAL8:
#if PHP_MAJOR_VERSION >= 7
            ZVAL_LONG( tmp, *(char *)sqlany_value->buffer );
#else
	    tmp->value.lval = *(char *)sqlany_value->buffer;
	    tmp->type = IS_LONG;
#endif
	    break;
	case A_UVAL8:
#if PHP_MAJOR_VERSION >= 7
            ZVAL_LONG( tmp, *(unsigned char *)sqlany_value->buffer );
#else
	    tmp->value.lval = *(unsigned char *)sqlany_value->buffer;
	    tmp->type = IS_LONG;
#endif
	    break;
	default:
	    break;
    }
}

static
void sasql_fetch_hash(INTERNAL_FUNCTION_PARAMETERS, int result_type )
/********************************************************************/
{
    DECLARE_ZVAL_ARG1;
#if !(PHP_MAJOR_VERSION >= 7)
    zval *		  tmp;
#endif
    sqlany_result_t	* sqlany_result;
    int 		  i;
    int 		  rc;

    if( zend_get_parameters_array_ex(1,&arg1) != SUCCESS ) {
	WRONG_PARAM_COUNT;
    }

    ZEND_FETCH_RESOURCE( sqlany_result, sqlany_result_t*, arg1, -1, "SQLAnywhere result",
	    le_result );

#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_fetch_row result_id=%p ", sqlany_result->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_fetch_row result_id=%d ", sqlany_result->id ) );
#endif

    rc = result_fetch_next( sqlany_result, 1 );
    if( rc == 0 || rc == -1 ) {
	RETURN_FALSE;
    }

#if PHP_MAJOR_VERSION < 6 || (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION <= 2 )
    if(
#endif
	// As of PHP 7.3, array_init does not return a return code and does not
        // fail.
        array_init( return_value )
#if PHP_MAJOR_VERSION < 6 || (PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION <= 2 )
        == FAILURE ) {
	RETURN_FALSE;
    }
#else
    ;
#endif

    for( i = 0; i < sqlany_result->num_cols; i++ ) {
#if PHP_MAJOR_VERSION >= 7
        zval  tmp;
	ZVAL_UNDEF( &tmp );
	a_sqlany_data_value dvalue;
	result_get_column_data( sqlany_result, i, &dvalue );

	assign_from_sqlany_value( &tmp, &dvalue );
#else
	a_sqlany_data_value dvalue;
	result_get_column_data( sqlany_result, i, &dvalue );

	ALLOC_INIT_ZVAL( tmp );
	Z_SET_REFCOUNT_P( tmp, 0 );
	assign_from_sqlany_value( tmp, &dvalue );
#endif
	if( result_type & FETCH_ROW ) {
#if PHP_MAJOR_VERSION >= 7
	    zend_hash_index_update( Z_ARRVAL_P(return_value),
		    i,
		    (void *)&tmp );
#else
	    Z_ADDREF_P( tmp );
	    zend_hash_index_update( Z_ARRVAL_P(return_value),
		    i,
		    (void *)&tmp, sizeof(zval*), NULL );
#endif
	}

	if( result_type & FETCH_OBJ ) {
	    a_sqlany_column_info cinfo;

	    result_get_column_info( sqlany_result, i, &cinfo );

#if PHP_MAJOR_VERSION >= 7
            zend_hash_str_update( Z_ARRVAL_P(return_value), cinfo.name, strlen( cinfo.name ), &tmp );
#else
	    Z_ADDREF_P( tmp );
	    zend_hash_update( Z_ARRVAL_P(return_value),
		    cinfo.name,
		    strlen( cinfo.name ) + 1,
		    (void *)&tmp, sizeof(zval*), NULL );
#endif
	}
#if PHP_MAJOR_VERSION >= 7
        // NOTE: if string is used for both, make refcount to 2
	if( (result_type & FETCH_ROW) && (result_type & FETCH_OBJ) ) {
            if( Z_TYPE(tmp) == IS_STRING ) {
                zend_string_addref( Z_STR(tmp) );
            }
        }
#endif
    }
}

/* {{{ proto array sasql_fetch_array( sasql_result $result )
   Fetch result row as an associative array */
PHP_FUNCTION(sasql_fetch_array)
/******************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[2];
#else
    zval ** args[2];
#endif
    int result_type = (FETCH_ROW|FETCH_OBJ);
    if( ZEND_NUM_ARGS() > 2 ) {
	WRONG_PARAM_COUNT;
    }
    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), args) == FAILURE ) {
	WRONG_PARAM_COUNT
    }
    if( ZEND_NUM_ARGS() == 2 ) {
	sqlany_convert_to_long_ex( args[1] );
#if PHP_MAJOR_VERSION >= 7
	result_type = Z_LVAL(args[1]);
#else
	result_type = Z_LVAL_PP(args[1]);
#endif
    }
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_fetch_array started " ) );
    sasql_fetch_hash( INTERNAL_FUNCTION_PARAM_PASSTHRU, result_type );
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_fetch_array finish " ) );
}
/* }}} */

/* {{{ proto array sasql_fetch_assoc( sasql_result $result )
   Fetch result row as an associative array */
PHP_FUNCTION(sasql_fetch_assoc)
/******************************/
{
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_fetch_assoc started " ) );
    sasql_fetch_hash( INTERNAL_FUNCTION_PARAM_PASSTHRU, FETCH_OBJ );
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_fetch_assoc finish " ) );
}
/* }}} */

/* {{{ proto object sasql_fetch_object( sasql_result $result )
   Get row as object */
PHP_FUNCTION(sasql_fetch_object)
/*******************************/
{
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_fetch_object started " ) );
    sasql_fetch_hash( INTERNAL_FUNCTION_PARAM_PASSTHRU, FETCH_OBJ );
    if(Z_TYPE_P(return_value)==IS_ARRAY) {
	object_and_properties_init( return_value, ZEND_STANDARD_CLASS_DEF_PTR,
		Z_ARRVAL_P(return_value));
    }
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_fetch_object finish " ) );
}
/* }}} */

/* {{{ proto array sasql_fetch_row( sasql_result $result )
   Get row as enumerated array */
PHP_FUNCTION(sasql_fetch_row)
/****************************/
{
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_fetch_row started " ) );
    sasql_fetch_hash( INTERNAL_FUNCTION_PARAM_PASSTHRU, FETCH_ROW );
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_fetch_row finished " ) );
}
/* }}} */

/* {{{ proto bool sasql_commit( sasql_conn $conn )
   commit transaction on a onnection */
PHP_FUNCTION(sasql_commit)
/*************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_connection_t * sqlany_conn;
    int ok;

    if( ( ZEND_NUM_ARGS() != 1) || (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }

    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, arg1, -1,
	    "SQLAnywhere connection resource", le_conn, le_pconn );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_commit( %p )", sqlany_conn->res ));
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_commit( %d )", sqlany_conn->id ));
#endif

    ok = api.sqlany_commit( sqlany_conn->handle );
    save_conn_error( sqlany_conn );
    if( ok ) {
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_commit=>SUCCESS" ) );
	RETURN_TRUE;
    } else {
	VerboseErrors( sqlany_conn TSRMLS_CC );
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_commit=>FALSE" ) );
	RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto bool sasql_rollback(int conn)
   rollback a transaction on a onnection */
PHP_FUNCTION(sasql_rollback)
/***************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_connection_t * sqlany_conn;
    int ok;

    if( ( ZEND_NUM_ARGS() != 1) || (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }

    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, arg1, -1, "SQLAnywhere connection resource", le_conn, le_pconn );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_rollback( %p )", sqlany_conn->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_rollback( %d )", sqlany_conn->id ) );
#endif

    ok = api.sqlany_rollback( sqlany_conn->handle );
    save_conn_error( sqlany_conn );
    if( ok ) {
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_rollback=>SUCCESS" ) );
	RETURN_TRUE;
    } else {
	VerboseErrors( sqlany_conn TSRMLS_CC );
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_rollback=>FALSE" ) );
	RETURN_FALSE;
    }
}
/* }}} */

typedef struct {
    int	   option_code;
    char * option_name;
    int	   option_type;
} option_setting;

#define OPT_NO_TYPE	0x0
#define OPT_TRUEFALSE	0x00000001
#define OPT_ONOFF	0x00000002
#define OPT_NUMBER	0x00000004
#define OPT_STRING	0x00000008

#define OPC_AUTO_COMMIT		1
#define OPC_VERBOSE_ERRORS 	2
#define OPC_ROW_COUNTS		3

option_setting sqlany_php_options[] =
{
    { OPC_AUTO_COMMIT, 		"auto_commit",    OPT_TRUEFALSE|OPT_ONOFF },
    { OPC_VERBOSE_ERRORS, 	"verbose_errors", OPT_TRUEFALSE|OPT_ONOFF },
    { OPC_ROW_COUNTS, 		"row_counts", 	  OPT_TRUEFALSE|OPT_ONOFF },
    { 0, NULL, 0 }
};

/* {{{ proto bool sasql_set_option(conn, option, value)
   set an option to a specific value */
PHP_FUNCTION(sasql_set_option)
/*****************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[3];
#else
    zval ** args[3];
#endif
    int i;
    sqlany_connection_t * sqlany_conn;

    if(( ZEND_NUM_ARGS() <= 2) ||
	    ( zend_get_parameters_array_ex(3,args) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
    sqlany_convert_to_string_ex( args[1] );

    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, args[0], -1,
	    "SQLAnywhere connection", le_conn, le_pconn );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_set_option( %p, %s )",
		sqlany_conn->res, Z_STRVAL( args[1] ) ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_set_option( %d, %s )",
		sqlany_conn->id, Z_STRVAL_PP( args[1] ) ) );
#endif

    i = 0;
    while( sqlany_php_options[i].option_name != NULL )
    {
#if PHP_MAJOR_VERSION >= 7
	if( zend_binary_strcasecmp( Z_STRVAL(args[1]), Z_STRLEN(args[1]),
#else
	if( zend_binary_strcasecmp( Z_STRVAL_PP(args[1]), Z_STRLEN_PP(args[1]),
#endif
		    sqlany_php_options[i].option_name,
		    strlen(sqlany_php_options[i].option_name) ) == 0 )
	{
	    int option_type = sqlany_php_options[i].option_type;
	    int option_code = sqlany_php_options[i].option_code;
	    zval option_value;


	    if( option_type & OPT_ONOFF ) {
		sqlany_convert_to_string_ex( args[2] );
#if PHP_MAJOR_VERSION >= 7
		if( zend_binary_strcasecmp( Z_STRVAL(args[2]), Z_STRLEN(args[2]),
			    "on", strlen( "on" ) ) == 0 ) {
#else
		if( zend_binary_strcasecmp( Z_STRVAL_PP(args[2]), Z_STRLEN_PP(args[2]),
			    "on", strlen( "on" ) ) == 0 ) {
#endif
		    ZVAL_BOOL( &option_value, 1 );
		    option_type  = 0;
		}
#if PHP_MAJOR_VERSION >= 7
		if( zend_binary_strcasecmp( Z_STRVAL(args[2]), Z_STRLEN(args[2]),
			    "off", strlen( "off" ) ) == 0 ) {
#else
		if( zend_binary_strcasecmp( Z_STRVAL_PP(args[2]), Z_STRLEN_PP(args[2]),
			    "off", strlen( "off" ) ) == 0 ) {
#endif
		    ZVAL_BOOL( &option_value, 0 );
		    option_type  = 0;
		}
	    }

	    if( option_type & OPT_TRUEFALSE ) {
		sqlany_convert_to_boolean_ex( args[2] );
#if PHP_MAJOR_VERSION >= 7
		ZVAL_BOOL( &option_value, Z_TYPE(args[2]) == IS_TRUE );
#else
		ZVAL_BOOL( &option_value, Z_BVAL_PP(args[2]) );
#endif
		option_type  = 0;
	    }

	    if( option_type & OPT_NUMBER ) {
		sqlany_convert_to_long_ex( args[2] );
#if PHP_MAJOR_VERSION >= 7
		ZVAL_LONG( &option_value, Z_LVAL(args[2]) );
#else
		ZVAL_LONG( &option_value, Z_LVAL_PP(args[2]) );
#endif
		option_type  = 0;
	    }

	    if( option_type & OPT_STRING ) {
		sqlany_convert_to_string_ex( args[2] );
#if PHP_MAJOR_VERSION >= 7
		ZVAL_STRING( &option_value, Z_STRVAL(args[2]));
#else
		ZVAL_STRING( &option_value, Z_STRVAL_PP(args[2]), 0);
#endif
		option_type  = 0;
	    }

	    if( option_type == 0 )
		switch( option_code )
		{
		    case OPC_ROW_COUNTS :
			{
			    char sql[256];
#if PHP_MAJOR_VERSION >= 7
			    if( Z_TYPE(option_value) == IS_TRUE ) {
#else
			    if( Z_BVAL_P(&option_value) ) {
#endif
				php_sprintf( sql, "SET OPTION ROW_COUNTS = 'on'" );
			    } else {
				php_sprintf( sql, "SET OPTION ROW_COUNTS = 'off'" );
			    }
			    if( api.sqlany_execute_immediate( sqlany_conn->handle, sql ) ) {
				RETURN_TRUE;
			    } else {
				VerboseErrors( sqlany_conn TSRMLS_CC );
				RETURN_FALSE;
			    }
			}
			break;

		    case OPC_VERBOSE_ERRORS :
#if PHP_MAJOR_VERSION >= 7
			sqlany_conn->verbose_errors = ( Z_TYPE(option_value) == IS_TRUE ? 1 : 0 );
#else
			sqlany_conn->verbose_errors = ( Z_BVAL_P(&option_value) ? 1 : 0 );
#endif
			RETURN_TRUE;
			break;

		    case OPC_AUTO_COMMIT :
#if PHP_MAJOR_VERSION >= 7
			sqlany_conn->auto_commit = ( Z_TYPE(option_value) == IS_TRUE ? 1 : 0 );
#else
			sqlany_conn->auto_commit = ( Z_BVAL_P(&option_value) ? 1 : 0 );
#endif
			RETURN_TRUE;
			break;

		    default:
			RETURN_FALSE;
		}
	}
	i++;
    }

#if PHP_MAJOR_VERSION >= 7
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown option (%s)", Z_STRVAL(args[1]) );
#else
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown option (%s)", Z_STRVAL_PP(args[1]) );
#endif
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_set_option=>FALSE" ) );
    RETURN_FALSE;
}
/* }}} */

/* {{{ proto int sasql_errorcode([int conn])
   returns the error code on the specified connection or the global error code */
PHP_FUNCTION(sasql_errorcode)
/****************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_connection_t * sqlany_conn;

    if( !api.initialized ) {
	RETURN_FALSE;
    }

    if( ZEND_NUM_ARGS() == 1 ) {
	if( (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	    WRONG_PARAM_COUNT;
	}
	ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, arg1, -1, "SQLAnywhere connection", le_conn, le_pconn );
	RETURN_LONG( sqlany_conn->errorcode );
    } else {
	RETURN_LONG( SAG(error_code) );
    }
}
/* }}} */

/* {{{ proto string sasql_error([int conn])
   returns the error msg on the specified connection or the global error code */
PHP_FUNCTION(sasql_error)
/************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_connection_t * sqlany_conn;
    char * msg;

    if( !api.initialized ) {
#if PHP_MAJOR_VERSION >= 7
	RETURN_STRING( DBCAPI_NOT_FOUND_ERROR );
#else
	RETURN_STRING( DBCAPI_NOT_FOUND_ERROR, 1 );
#endif
    }

    if( ZEND_NUM_ARGS() == 1 ) {
	if( (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	    WRONG_PARAM_COUNT;
	}
	ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, arg1, -1, "SQLAnywhere connection", le_conn, le_pconn );
#if PHP_MAJOR_VERSION >= 7
	RETURN_STRING( sqlany_conn->error );
#else
	RETURN_STRING( sqlany_conn->error, 1 );
#endif
    } else {
#if PHP_MAJOR_VERSION >= 7
	RETURN_STRING( SAG(error) );
#else
	RETURN_STRING( SAG(error), 1 );
#endif
    }
}
/* }}} */

/* {{{ proto string sasql_sqlstate( sasql_conn $conn )
   returns the sqlstate of the speicifed connection */
PHP_FUNCTION(sasql_sqlstate)
/****************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_connection_t * sqlany_conn;

    if( !api.initialized ) {
#if PHP_MAJOR_VERSION >= 7
	RETURN_STRING( DBCAPI_NOT_FOUND_ERROR );
#else
	RETURN_STRING( DBCAPI_NOT_FOUND_ERROR, 1 );
#endif
    }

    if( ZEND_NUM_ARGS() == 1 ) {
	if( (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	    WRONG_PARAM_COUNT;
	}
	ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, arg1, -1,
		"SQLAnywhere connection", le_conn, le_pconn );
#if PHP_MAJOR_VERSION >= 7
	RETURN_STRINGL(sqlany_conn->sqlstate, sqlany_conn->len);
#else
	RETURN_STRINGL(sqlany_conn->sqlstate, sqlany_conn->len, 1);
#endif
    } else {
#if PHP_MAJOR_VERSION >= 7
	RETURN_STRING( SAG(sqlstate) );
#else
	RETURN_STRING( SAG(sqlstate), 1 );
#endif
    }
}
/* }}} */

static
int get_identity( sqlany_connection_t * sqlany_conn, zval * value )
/******************************************************************/
{
    a_sqlany_stmt * 	stmt_handle;
    a_sqlany_data_value dvalue;

    stmt_handle = api.sqlany_execute_direct( sqlany_conn->handle, "select @@identity" );
    if( stmt_handle == NULL ) {
	return 0;
    }
    if( !api.sqlany_fetch_next( stmt_handle ) ) {
	api.sqlany_free_stmt( stmt_handle );
	return 0;
    }
    if( !api.sqlany_get_column( stmt_handle, 0, &dvalue ) ) {
	api.sqlany_free_stmt( stmt_handle );
	return 0;
    }

    assign_from_sqlany_value( value, &dvalue );
    api.sqlany_free_stmt( stmt_handle );

    return 1;
}

static
void SQLAnywhereIdentity( INTERNAL_FUNCTION_PARAMETERS )
/*******************************************************/
{
// TODO: retore this
    DECLARE_ZVAL_ARG1;
    sqlany_connection_t * sqlany_conn;

    if( (ZEND_NUM_ARGS() != 1) ||
	    (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, arg1, -1, "SQLAnywhere connection", le_conn, le_pconn );

    if( !get_identity( sqlany_conn, return_value ) ) {
	save_conn_error( sqlany_conn );
	RETURN_FALSE;
    }
}

/* {{{ proto int sasql_insert_id( sasql_conn $conn )
   returns the id generated from the previous insert operation */
PHP_FUNCTION(sasql_insert_id)
/****************************/
{
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_insert_id started " ) );
    SQLAnywhereIdentity( INTERNAL_FUNCTION_PARAM_PASSTHRU );
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_insert_id finished " ) );
}
/* }}} */

/* {{{ proto bool sasql_message(conn, message)
   send a message to the console */
PHP_FUNCTION(sasql_message)
/**************************/
{
    sqlany_connection_t * sqlany_conn;
    char		* sql;
    int			  rc;
#if PHP_MAJOR_VERSION >= 7
    zval	          args[2];
#else
    zval	       ** args[2];
#endif

    if( (ZEND_NUM_ARGS() != 2) ||
	    (zend_get_parameters_array_ex(2,args) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, args[0], -1, "SQLAnywhere connection", le_conn, le_pconn );

    sqlany_convert_to_string_ex( args[1] );

#if PHP_MAJOR_VERSION >= 7
    sql = emalloc( Z_STRLEN(args[1]) + 10 );
    php_sprintf( sql, "message %s", Z_STRVAL(args[1]) );
#else
    sql = emalloc( Z_STRLEN_PP(args[1]) + 10 );
    php_sprintf( sql, "message %s", Z_STRVAL_PP(args[1]) );
#endif
    rc = api.sqlany_execute_immediate( sqlany_conn->handle, sql );
    save_conn_error( sqlany_conn );
    efree( sql );
    if( rc ) {
	RETURN_TRUE;
    } else {
	VerboseErrors( sqlany_conn TSRMLS_CC );
	RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto sasql_stmt sasql_prepare( saconn $conn, string $sql )
   prepares the supplied query and returns a stmt handle */
PHP_FUNCTION(sasql_prepare)
/**************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[2];
#else
    zval ** args[2];
#endif
    sqlany_connection_t * sqlany_conn;
    sqlany_stmt_t	* sqlany_stmt;

    if( ( ZEND_NUM_ARGS() != 2) || (zend_get_parameters_array_ex(2, args) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, args[0], -1,
	    "SQLAnywhere connection", le_conn, le_pconn );

    sqlany_convert_to_string_ex( args[1] );

#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_prepare( conn=%d, sql=%s )",
		Z_LVAL( args[0] ), Z_STRVAL(args[1]) ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_prepare( conn=%d, sql=%s )",
		Z_LVAL_PP( args[0] ), Z_STRVAL_PP(args[1]) ) );
#endif

    sqlany_stmt = (sqlany_stmt_t*)emalloc( sizeof(sqlany_stmt_t) );
    memset( sqlany_stmt, 0, sizeof(sqlany_stmt_t));
    clear_stmt_error( sqlany_stmt );

#if PHP_MAJOR_VERSION >= 7
    sqlany_stmt->handle = api.sqlany_prepare( sqlany_conn->handle, Z_STRVAL(args[1]) );
#else
    sqlany_stmt->handle = api.sqlany_prepare( sqlany_conn->handle, Z_STRVAL_PP(args[1]) );
#endif
    save_conn_error( sqlany_conn );
    if( sqlany_stmt->handle == NULL ) {
	/* failed to prepare */
	VerboseErrors( sqlany_conn TSRMLS_CC );
	efree( sqlany_stmt );
	_debug_enabled( SQLAnyDebug( _LOCATION_,
		    " sasql_query=>FALSE [%s]", sqlany_conn->error ); )
	RETURN_FALSE;
    }

#if PHP_MAJOR_VERSION >= 7
    sqlany_stmt->res = Z_RES_P(zend_list_insert( sqlany_stmt, le_stmt ));
#else
    sqlany_stmt->id = zend_list_insert( sqlany_stmt, le_stmt TSRMLS_CC);
#endif
    sqlany_stmt->sqlany_conn = sqlany_conn;
    sqlany_stmt->num_params = api.sqlany_num_params( sqlany_stmt->handle );
    if( sqlany_stmt->num_params > 0 ) {
	sqlany_stmt->sqlany_params = (sqlany_stmt_param_t*)
	    ecalloc( sqlany_stmt->num_params, sizeof(sqlany_stmt_param_t) );
	sqlany_stmt->php_params = (php_stmt_param_t*)ecalloc( sqlany_stmt->num_params, sizeof(php_stmt_param_t) );
    }
    sqlany_stmt->next = sqlany_conn->stmt_list;
    sqlany_conn->stmt_list = sqlany_stmt;
#if PHP_MAJOR_VERSION >= 7
    RETURN_RES( sqlany_stmt->res );
#else
    RETURN_RESOURCE( sqlany_stmt->id );
#endif
}
/* }}} */

/* {{{ proto int sasql_stmt_param_count( sasql $stmt )
   returns the number of parameters expected for the prepared statement stmt */
PHP_FUNCTION(sasql_stmt_param_count)
/***********************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;

    if( ( ZEND_NUM_ARGS() != 1) || (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );

#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_param_count( stmt=%p )",
		sqlany_stmt->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_param_count( stmt=%d )",
		sqlany_stmt->id ) );
#endif

    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_param_count=>[%d]",
		sqlany_stmt->num_params ) );

    RETURN_LONG( sqlany_stmt->num_params );
}
/* }}} */

/* {{{ proto bool sasql_stmt_bind_param( stmt, types, &$var1 [, &$var2 ... ] );
   bind parameter to be used by prepared statement */
/* stmt statement handle
 * type is string of d,i,b,s
 * variables
 */
PHP_FUNCTION(sasql_stmt_bind_param)
/**********************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval   * args;
#else
    zval *** args;
#endif
    sqlany_stmt_t	* sqlany_stmt;
    int			  i;
    int			  param_number;
    int			  ok;

    if( ZEND_NUM_ARGS() < 3 ) {
	WRONG_PARAM_COUNT;
    }

#if PHP_MAJOR_VERSION >= 7
    args = (zval *)safe_emalloc( ZEND_NUM_ARGS(), sizeof(zval), 0 );
#else
    args = (zval ***)safe_emalloc( ZEND_NUM_ARGS(), sizeof(zval **), 0 );
#endif

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), args) == FAILURE ) {
	efree( args );
	WRONG_PARAM_COUNT;
    }
    sqlany_convert_to_string_ex( args[1] );
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, args[0], -1, "SQLAnywhere statement", le_stmt );

#if PHP_MAJOR_VERSION >= 7
    if( ZEND_NUM_ARGS() - 2 != Z_STRLEN( args[1] ) ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"The num of type characters supplied (%d) does not match the number of parameters supplied (%d)",
		Z_STRLEN(args[1]), ZEND_NUM_ARGS() - 2 );
#else
    if( ZEND_NUM_ARGS() - 2 != Z_STRLEN_PP(args[1] ) ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"The num of type characters supplied (%d) does not match the number of parameters supplied (%d)",
		Z_STRLEN_PP( args[1]), ZEND_NUM_ARGS() - 2 );
#endif
	efree( args );
	RETURN_FALSE;
    }

    if( (ZEND_NUM_ARGS() -2) > sqlany_stmt->num_params ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"More parameters supplied (%d) than expected (%d)", ZEND_NUM_ARGS() - 2, sqlany_stmt->num_params );
	efree( args );
	RETURN_FALSE;
    }

    // do a sanity check that the field types is valid.
#if PHP_MAJOR_VERSION >= 7
    for( i = 0; i < Z_STRLEN(args[1]); i++ ) {
	char field_type = Z_STRVAL(args[1])[i];
#else
    for( i = 0; i < Z_STRLEN_PP(args[1]); i++ ) {
	char field_type = Z_STRVAL_PP(args[1])[i];
#endif
	switch( field_type ) {
	    case 'd': case 'i': case 'b': case 's':
		break;
	    default:
		php_error_docref(NULL TSRMLS_CC, E_WARNING, 
			"Undefined fieldtype %c (parameter %d)", field_type, i+1 );
		efree( args );
		RETURN_FALSE;
	}
    }

    param_number = 0;
    for( i = 2; i < ZEND_NUM_ARGS(); i++ ) {
	a_sqlany_bind_param	param;
#if PHP_MAJOR_VERSION >= 7
	char 			field_type = Z_STRVAL(args[1])[i-2];
#else
	char 			field_type = Z_STRVAL_PP(args[1])[i-2];
#endif

	ok = api.sqlany_describe_bind_param( sqlany_stmt->handle, param_number, &param );
	save_stmt_error( sqlany_stmt );
	if( !ok ) {
	    efree( args );
	    RETURN_FALSE;
	}

#if PHP_MAJOR_VERSION >= 7
        zval * param_zval;
        if( Z_ISREF( args[i] ) ) {
            param_zval = Z_REFVAL( args[i] );
        } else {
            param_zval = &args[i];
        }
#endif

	if( param.direction & DD_OUTPUT ) {
	    a_sqlany_bind_param output;

	    output.direction = DD_OUTPUT;
	    output.name = NULL;
	    output.value.is_null = &(sqlany_stmt->sqlany_params[param_number].out_is_null);
	    output.value.length  = &(sqlany_stmt->sqlany_params[param_number].out_len);
	    switch( field_type ) {
		case 'd':
#if PHP_MAJOR_VERSION >= 7
		    convert_to_double_ex( param_zval );
		    output.value.buffer = (char *)&Z_DVAL_P(param_zval);
#else
		    convert_to_double_ex( args[i] );
		    output.value.buffer = (char *)&Z_DVAL_PP(args[i]);
#endif
		    output.value.buffer_size = 4;
		    output.value.type = A_DOUBLE;
		    break;
		case 'i':
#if PHP_MAJOR_VERSION >= 7
                    convert_to_long_ex( param_zval );
		    output.value.buffer = (char *)&Z_LVAL_P(param_zval);
		    if( sizeof(Z_LVAL_P(param_zval)) == 4 ) {
#else
            convert_to_long_ex( args[i] );
		    output.value.buffer = (char *)&Z_LVAL_PP(args[i]);
		    if( sizeof(Z_LVAL_PP(args[i])) == 4 ) {
#endif
			output.value.type = A_VAL32;
			output.value.buffer_size = 4;
		    } else {
			output.value.type = A_VAL64;
			output.value.buffer_size = 8;
		    }
		    break;
		case 's':
		case 'b':
#if PHP_MAJOR_VERSION >= 7
		    {
                        convert_to_string_ex( param_zval ); // convert IS_NULL type as well
                        zend_string * buffer = zend_string_alloc( param.value.buffer_size + 1, 0 );
                        memset( ZSTR_VAL(buffer), 0x00, param.value.buffer_size + 1 );
                        size_t num_bytes_to_copy = _min( param.value.buffer_size, Z_STRLEN_P(param_zval) );
                        memcpy( ZSTR_VAL(buffer), Z_STRVAL_P(param_zval), num_bytes_to_copy );
                        ZSTR_LEN(buffer) = num_bytes_to_copy;

                        zend_string * orig_string = Z_STR_P( param_zval );
                        zend_string_release( orig_string );

                        // NOTE: this will mimic ZVAL_STRING building.
                        // Setting IS_STRING_EX makes zval to be IS_TYPE_REFCOUNTED so that
                        // zend_vm_execute.h:execute_ex process the refcount.
                        Z_STR_P(param_zval) = buffer;
                        Z_TYPE_INFO_P(param_zval) = IS_STRING_EX;
		    }
		    output.value.type = ( field_type == 's' ? A_STRING : A_BINARY );
		    output.value.buffer = (char *)Z_STRVAL_P(param_zval);
		    output.value.buffer_size = param.value.buffer_size;
		    (*output.value.length) = Z_STRLEN_P(param_zval);
#else
		    {
			size_t num_bytes_to_copy;
			// param.value.buffer_size is already set by the describe_bind_param
			char * buffer = emalloc( param.value.buffer_size + 1 );
			if( ZVAL_IS_NULL(*args[i]) ) {
			    memset( buffer, 0, param.value.buffer_size + 1 );
			    Z_STRLEN_PP(args[i]) = 0;
			} else {
			    convert_to_string_ex( args[i] );
			    num_bytes_to_copy = _min( param.value.buffer_size, Z_STRLEN_PP(args[i]) );
			    memcpy( buffer, Z_STRVAL_PP(args[i]), num_bytes_to_copy );
			    buffer[num_bytes_to_copy] = '\0';

			    if( Z_REFCOUNT_P(*(args[i])) > 1 ) {
				Z_DELREF_P(*(args[i]));
			    } else {
			        efree( Z_STRVAL_PP(args[i]) );
			    }
			    Z_STRLEN_PP(args[i]) = num_bytes_to_copy;
			}
			Z_STRVAL_PP(args[i]) = buffer;
			Z_TYPE_PP(args[i]) = IS_STRING;
		    }
		    output.value.type = ( field_type == 's' ? A_STRING : A_BINARY );
		    output.value.buffer = (char *)Z_STRVAL_PP(args[i]);
		    output.value.buffer_size = param.value.buffer_size;
		    (*output.value.length) = Z_STRLEN_PP(args[i]);
#endif
		    break;
	    }
#if PHP_MAJOR_VERSION >= 7
	    ZVAL_COPY_VALUE( &sqlany_stmt->php_params[param_number].out, &args[i] );
	    sqlany_stmt->sqlany_params[param_number].out = output.value;
#else
	    Z_ADDREF_P(*args[i]);
	    sqlany_stmt->php_params[param_number].out = *args[i];
	    sqlany_stmt->sqlany_params[param_number].out = output.value;
#endif

	    ok = api.sqlany_bind_param( sqlany_stmt->handle, param_number, &output );
	    save_stmt_error( sqlany_stmt );
	    if( !ok ) {
		efree( args );
		RETURN_FALSE;
	    }
	}
	if( param.direction & DD_INPUT ) {
	    a_sqlany_bind_param input;

	    input.direction = DD_INPUT;
	    input.name = NULL;
	    input.value.is_null = &(sqlany_stmt->sqlany_params[param_number].in_is_null);
	    input.value.length  = &(sqlany_stmt->sqlany_params[param_number].in_len);
	    input.value.buffer_size = 0;
#if PHP_MAJOR_VERSION >= 7
	    if( ZVAL_IS_NULL(param_zval) ) {
#else
	    if( ZVAL_IS_NULL(*args[i]) ) {
#endif
		(*input.value.is_null) = 1;
		(*input.value.length) = 0;
		input.value.buffer = NULL;
		switch( field_type ) {
		    case 'd': input.value.type = A_DOUBLE; break;
		    case 'i': input.value.type = A_VAL32; break;
		    case 'b': input.value.type = A_BINARY; break;
		    case 's': input.value.type = A_STRING; break;
		}
	    } else {
		(*input.value.is_null) = 0;
		switch( field_type ) {
		    case 'd':
#if PHP_MAJOR_VERSION >= 7
			convert_to_double_ex( param_zval );
			input.value.buffer = (char *)&Z_DVAL_P(param_zval);
#else
			convert_to_double_ex( args[i] );
			input.value.buffer = (char *)&Z_DVAL_PP(args[i]);
#endif
			input.value.type = A_DOUBLE;
			break;
		    case 'i':
#if PHP_MAJOR_VERSION >= 7
			convert_to_long_ex( param_zval );
			input.value.buffer = (char *)&Z_LVAL_P(param_zval);
			if( sizeof(Z_LVAL_P(param_zval)) == 4 ) {
			    input.value.type = A_VAL32;
			} else {
			    input.value.type = A_VAL64;
			}
#else
			convert_to_long_ex( args[i] );
			input.value.buffer = (char *)&Z_LVAL_PP(args[i]);
			if( sizeof(Z_LVAL_PP(args[i])) == 4 ) {
			    input.value.type = A_VAL32;
			} else {
			    input.value.type = A_VAL64;
			}
#endif
			break;
		    case 'b':
		    case 's':
#if PHP_MAJOR_VERSION >= 7
			convert_to_string_ex( param_zval );
			(*input.value.length) = Z_STRLEN_P(param_zval);
			input.value.type = ( field_type == 's' ? A_STRING : A_BINARY );
			input.value.buffer = (char *)Z_STRVAL_P(param_zval);
#else
			convert_to_string_ex( args[i] );
			(*input.value.length) = Z_STRLEN_PP(args[i]);
			input.value.type = ( field_type == 's' ? A_STRING : A_BINARY );
			input.value.buffer = (char *)Z_STRVAL_PP(args[i]);
#endif
			break;
		}
	    }
#if PHP_MAJOR_VERSION >= 7
	    ZVAL_COPY_VALUE( &sqlany_stmt->php_params[param_number].in, &args[i] );
#else
	    Z_ADDREF_P(*args[i]);
	    sqlany_stmt->php_params[param_number].in = *args[i];
#endif
	    sqlany_stmt->sqlany_params[param_number].in = input.value;

	    ok = api.sqlany_bind_param( sqlany_stmt->handle, param_number, &input );
	    save_stmt_error( sqlany_stmt );
	    if( !ok ) {
		efree( args );
		RETURN_FALSE;
	    }
	}
#if 0
	if( ZVAL_IS_NULL(*args[i]) ) {
	    param.value.buffer = NULL;
	    param.value.length = NULL;
	    switch( field_type ) {
		case 'd': param.value.type = A_DOUBLE; break;
		case 'i': param.value.type = A_VAL32; break;
		case 'b': param.value.type = A_BINARY; break;
		case 's': param.value.type = A_STRING; break;
		default:
		    php_error_docref(NULL TSRMLS_CC, E_WARNING, 
			    "Undefined fieldtype %c (parameter %d)", field_type, i+1 );
		    efree( args );
		    RETURN_FALSE;
	    }
	} else {
	    switch( field_type ) {
		case 'd':
		    convert_to_double_ex( args[i] );
		    param.value.buffer = (char *)&Z_DVAL_PP(args[i]);
		    param.value.type = A_DOUBLE;
		    break;
		case 'i':
		    convert_to_long_ex( args[i] );
		    param.value.buffer = (char *)&Z_LVAL_PP(args[i]);
		    if( sizeof(Z_LVAL_PP(args[i])) == 4 ) {
			param.value.type = A_VAL32;
		    } else {
			param.value.type = A_VAL64;
		    }
		    break;
		case 'b':
		case 's':
		    convert_to_string_ex( args[i] );
		    if( param.direction & DD_OUTPUT ) {
			char * buffer = emalloc( param.value.buffer_size + 1 );
			memcpy( buffer, Z_STRVAL_PP(args[i]), Z_STRLEN_PP(args[i]) );
			buffer[Z_STRLEN_PP(args[i])] = '\0';
			efree( Z_STRVAL_PP(args[i]) );
			Z_STRVAL_PP(args[i]) = buffer;
			// param.value.buffer_size is already set by the describe_bind_param
			Z_TYPE_PP(args[i]) = IS_STRING;
		    }
		    sqlany_stmt->sqlany_params[param_number].in_len = Z_STRLEN_PP(args[i]);

		    param.value.type = ( field_type == 's' ? A_STRING : A_BINARY );
		    param.value.buffer = (char *)Z_STRVAL_PP(args[i]);
		    param.value.length = &(sqlany_stmt->sqlany_params[param_number].in_len);
		    break;
		default:
		    php_error_docref(NULL TSRMLS_CC, E_WARNING, 
			    "Undefined fieldtype %c (parameter %d)", field_type, i+1 );
		    efree( args );
		    RETURN_FALSE;
	    }
	}
	if( param.direction & DD_INPUT ) {
	    param.value.is_null = &sqlany_stmt->sqlany_params[param_number].in_is_null;
	    *(param.value.is_null) = ZVAL_IS_NULL(*args[i]);
	}
	if( param.direction & DD_OUTPUT ) {
	    param.value.is_null = &sqlany_stmt->sqlany_params[param_number].out_is_null;
	    *(param.value.is_null) = ZVAL_IS_NULL(*args[i]);
	}

	ok = api.sqlany_bind_param( sqlany_stmt->handle, param_number, &param );
	save_stmt_error( sqlany_stmt );
	if( !ok ) {
	    efree( args );
	    RETURN_FALSE;
	}
	if( param.direction & DD_INPUT ) {
	    Z_ADDREF_P(*args[i]);
	    sqlany_stmt->php_params[param_number].in = *args[i];
	    sqlany_stmt->sqlany_params[param_number].in = param.value;
	}
	if( param.direction & DD_OUTPUT ) {
	    Z_ADDREF_P(*args[i]);
	    sqlany_stmt->php_params[param_number].out = *args[i];
	    sqlany_stmt->sqlany_params[param_number].out = param.value;
	}
#endif
	param_number++;
    }
    efree( args );
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool sasql_stmt_bind_param_ex( sasql $stmt, int $param_id, mixed $var, string $type [ bool is_null [, int $direction]] )
   bind parameter to be used by prepared statement */
/* var is the reference to PHP variable
 * type is d,i,b,s
 * is_null true or false
 * direction is SASQL_D_INPUT, SASQL_D_OUTPUT, or SASQL_D_INPUT_OUTPUT
 */
PHP_FUNCTION(sasql_stmt_bind_param_ex)
/*************************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    		args[6];
#else
    zval ** 		args[6];
#endif
    sqlany_stmt_t	*sqlany_stmt;
    a_sqlany_bind_param	param;
    a_sqlany_bind_param	desc_info;
    int			param_number;
    int			is_null = 0;
    int			ok;

    if( ZEND_NUM_ARGS() < 4 || ZEND_NUM_ARGS() > 6 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), args ) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }

    param.value.is_null = NULL;
    param.direction = DD_INPUT;
    switch( ZEND_NUM_ARGS() ) {
	case 6:
	    // 5 direction
	    sqlany_convert_to_long_ex( args[5] );
#if PHP_MAJOR_VERSION >= 7
	    switch( Z_LVAL(args[5]) ) {
#else
	    switch( Z_LVAL_PP(args[5]) ) {
#endif
		case 1:
		    param.direction = DD_INPUT;
		    break;
		case 2:
		    param.direction = DD_OUTPUT;
		    break;
		case 3:
		    param.direction = DD_INPUT_OUTPUT;
		    break;
		default:
		    php_error_docref(NULL TSRMLS_CC, E_WARNING, 
			    "Argument 6 is not valid. Direction could only be 1,2, or 3"  );
		    RETURN_FALSE;
	    }
	case 5:
	    // 4 is_null
	    sqlany_convert_to_boolean_ex( args[4] );
#if PHP_MAJOR_VERSION >= 7
	    is_null = Z_TYPE(args[4]) == IS_TRUE;
#else
	    is_null = Z_BVAL_PP(args[4]);
#endif
	    break;
	case 4:
	    // 3 type
	    // 2 var
	    // 1 param_num
	    // 0 stmt
	    sqlany_convert_to_string_ex( args[3] );
	    sqlany_convert_to_long_ex( args[1] );
	    break;
	default:
	    WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, args[0], -1, "SQLAnywhere statement", le_stmt );

#if PHP_MAJOR_VERSION >= 7
    param_number = Z_LVAL(args[1]);
#else
    param_number = Z_LVAL_PP(args[1]);
#endif

    if( param_number >= sqlany_stmt->num_params ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"Argument 2 (%d) is not valid. Expecting a number between 0 and %d",
		param_number, sqlany_stmt->num_params - 1 );
	RETURN_FALSE;
    }
#if PHP_MAJOR_VERSION >= 7
    param_number = Z_LVAL(args[1]);
#else
    param_number = Z_LVAL_PP(args[1]);
#endif

    ok = api.sqlany_describe_bind_param( sqlany_stmt->handle, param_number, &desc_info );
    save_stmt_error( sqlany_stmt );
    if( !ok ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"Failed to find information about statement parameters" );
	RETURN_FALSE;
    }

    if( param.direction & DD_INPUT ) {
	param.value.is_null = &sqlany_stmt->sqlany_params[param_number].in_is_null;
    }
    if( param.direction & DD_OUTPUT ) {
	param.value.is_null = &sqlany_stmt->sqlany_params[param_number].out_is_null;
    }
    *(param.value.is_null) = is_null;

#if PHP_MAJOR_VERSION >= 7
        zval * param_zval;
        if( Z_ISREF( args[2] ) ) {
            param_zval = Z_REFVAL( args[2] );
        } else {
            param_zval = &args[2];
        }
#endif

#if PHP_MAJOR_VERSION >= 7
    switch( Z_STRVAL(args[3])[0] ) {
#else
    switch( Z_STRVAL_PP(args[3])[0] ) {
#endif
	case 'd':
#if PHP_MAJOR_VERSION >= 7
            if( !is_null ) {
                convert_to_double_ex( param_zval );
            }
	    param.value.type = A_DOUBLE;
	    param.value.buffer = (char *)&Z_DVAL_P(param_zval);
#else
            if( !is_null ) {
                convert_to_double_ex( args[2] );
            }
	    param.value.type = A_DOUBLE;
	    param.value.buffer = (char *)&Z_DVAL_PP(args[2]);
#endif
	    break;
	case 'i':
#if PHP_MAJOR_VERSION >= 7
            if( !is_null ) {
                convert_to_long_ex( param_zval );
            }
	    param.value.type = A_VAL32;
	    param.value.buffer = (char *)&Z_LVAL_P(param_zval);
#else
            if( !is_null ) {
                convert_to_long_ex( args[2] );
            }
	    param.value.type = A_VAL32;
	    param.value.buffer = (char *)&Z_LVAL_PP(args[2]);
#endif
	    break;
#if PHP_MAJOR_VERSION >= 7
	case 'b':
	case 's':
	    if( param.direction & DD_OUTPUT ) {
                convert_to_string_ex( param_zval ); // convert IS_NULL type as well
                zend_string * buffer = zend_string_alloc( desc_info.value.buffer_size + 1, 0 );
                memset( ZSTR_VAL(buffer), 0x00, desc_info.value.buffer_size + 1 );
                size_t num_bytes_to_copy = _min( desc_info.value.buffer_size, Z_STRLEN_P(param_zval) );
                memcpy( ZSTR_VAL(buffer), Z_STRVAL_P(param_zval), num_bytes_to_copy );
                ZSTR_LEN(buffer) = num_bytes_to_copy;

                zend_string * orig_string = Z_STR_P( param_zval );
                zend_string_release( orig_string );

                // NOTE: this will mimic ZVAL_STRING building.
                // Setting IS_STRING_EX makes zval to be IS_TYPE_REFCOUNTED so that
                // zend_vm_execute.h:execute_ex process the refcount.
                Z_STR_P(param_zval) = buffer;
                Z_TYPE_INFO_P(param_zval) = IS_STRING_EX;

		param.value.buffer_size = (size_t)desc_info.value.buffer_size;
	        param.value.buffer = (char *)Z_STRVAL_P(param_zval);
	        param.value.length = &(sqlany_stmt->sqlany_params[param_number].out_len);
	        *(param.value.length) = Z_STRLEN_P(param_zval);
	    } else {
	        param.value.length = &(sqlany_stmt->sqlany_params[param_number].in_len);
                if( ZVAL_IS_NULL( param_zval ) ) {
                    param.value.buffer_size = 0;
                    param.value.buffer = 0;
	            *(param.value.length) = 0;
                } else {
                    convert_to_string_ex( param_zval );
                    param.value.buffer_size = (size_t)Z_STRLEN_P(param_zval);
	            param.value.buffer = (char *)Z_STRVAL_P(param_zval);
	            *(param.value.length) = Z_STRLEN_P(param_zval);
                }
	    }
            param.value.type = Z_STRVAL(args[3])[0] == 's' ? A_STRING : A_BINARY;
	    break;
#else
	case 'b':
	    if( param.direction & DD_OUTPUT ) {
		char * buffer = emalloc( desc_info.value.buffer_size + 1 );
		memcpy( buffer, Z_STRVAL_PP(args[2]), Z_STRLEN_PP(args[2]) );
		buffer[Z_STRLEN_PP(args[2])] = '\0';
		efree( Z_STRVAL_PP(args[2]) );
		Z_STRVAL_PP(args[2]) = buffer;
		param.value.buffer_size = (size_t)desc_info.value.buffer_size;
	    } else {
		param.value.buffer_size = (size_t)Z_STRLEN_PP(args[2]);
	    }
	    param.value.type = A_BINARY;
	    param.value.buffer = (char *)Z_STRVAL_PP(args[2]);
	    param.value.length = (size_t*)&Z_STRLEN_PP(args[2]);
	    break;
	case 's':
	    if( param.direction & DD_OUTPUT ) {
		char * buffer = emalloc( desc_info.value.buffer_size + 1 );
		memcpy( buffer, Z_STRVAL_PP(args[2]), Z_STRLEN_PP(args[2]) );
		buffer[Z_STRLEN_PP(args[2])] = '\0';
		efree( Z_STRVAL_PP(args[2]) );
		Z_STRVAL_PP(args[2]) = buffer;
		param.value.buffer_size = (size_t)desc_info.value.buffer_size;
	    } else {
		param.value.buffer_size = (size_t)Z_STRLEN_PP(args[2]);
	    }
	    param.value.type = A_STRING;
	    param.value.buffer = (char *)Z_STRVAL_PP(args[2]);
	    param.value.length = (size_t*)&Z_STRLEN_PP(args[2]);
	    break;
#endif
	default:
#if PHP_MAJOR_VERSION >= 7
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"Unrecognized type information '%s'.", Z_STRVAL(args[3]) );
#else
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"Unrecognized type information '%s'.", Z_STRVAL_PP(args[3]) );
#endif
	RETURN_FALSE;
    }

    ok = api.sqlany_bind_param( sqlany_stmt->handle, param_number, &param );
    save_stmt_error( sqlany_stmt );
    if( ok ) {
	if( param.direction & DD_INPUT ) {
#if PHP_MAJOR_VERSION >= 7
	    ZVAL_COPY_VALUE( &sqlany_stmt->php_params[param_number].in, &args[2] );
#else
	    Z_ADDREF_P(*args[2]);
	    sqlany_stmt->php_params[param_number].in = *args[2];
#endif
	    sqlany_stmt->sqlany_params[param_number].in = param.value;
	}
	if( param.direction & DD_OUTPUT ) {
#if PHP_MAJOR_VERSION >= 7
	    ZVAL_COPY_VALUE( &sqlany_stmt->php_params[param_number].out, &args[2] );
#else
	    Z_ADDREF_P(*args[2]);
	    sqlany_stmt->php_params[param_number].out = *args[2];
#endif
	    sqlany_stmt->sqlany_params[param_number].out = param.value;
	}
	RETURN_TRUE;
    } else {
	RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto bool sasql_stmt_execute( sasql $stmt )
   sasql_stmt_execute() is used to execute a prepared statemtn */
PHP_FUNCTION(sasql_stmt_execute)
/*******************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_connection_t * sqlany_conn;
    sqlany_stmt_t	* sqlany_stmt;
    sqlany_result_t	* sqlany_result;
    int			  rc;
    int			  i;

    if( ZEND_NUM_ARGS() != 1 ) {
	WRONG_PARAM_COUNT;
    }
    if( zend_get_parameters_array_ex(1,&arg1) != SUCCESS ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );

    sqlany_conn = sqlany_stmt->sqlany_conn;

    // We have to rebind all A_STRING and A_BINARY input variables because they could change in size.
    for( i = 0; i < sqlany_stmt->num_params; i++ ) {
#if PHP_MAJOR_VERSION >= 7
        zval * php_param_in;
        if( Z_ISREF( sqlany_stmt->php_params[i].in ) ) {
            php_param_in = Z_REFVAL( sqlany_stmt->php_params[i].in );
        } else {
            php_param_in = &sqlany_stmt->php_params[i].in;
        }
        zval * php_param_out;
        if( Z_ISREF( sqlany_stmt->php_params[i].out ) ) {
            php_param_out = Z_REFVAL( sqlany_stmt->php_params[i].out );
        } else {
            php_param_out = &sqlany_stmt->php_params[i].out;
        }

	if( !Z_ISUNDEF_P( php_param_out ) &&
	    (sqlany_stmt->sqlany_params[i].out.type == A_BINARY
             || sqlany_stmt->sqlany_params[i].out.type == A_STRING ) &&
		sqlany_stmt->sqlany_params[i].out.buffer != Z_STRVAL_P(php_param_out) ) {
#else
	if( sqlany_stmt->php_params[i].out != NULL &&
	    (sqlany_stmt->sqlany_params[i].out.type == A_BINARY || sqlany_stmt->sqlany_params[i].out.type == A_STRING ) &&
		sqlany_stmt->sqlany_params[i].out.buffer != Z_STRVAL_P(sqlany_stmt->php_params[i].out) ) {
#endif
	    a_sqlany_bind_param param;
	    a_sqlany_bind_param desc_info;
#if PHP_MAJOR_VERSION >= 7
	    zend_string		*buffer;
#else
	    char		*buffer;
#endif
	    size_t		num_bytes_to_copy;

	    rc = api.sqlany_describe_bind_param( sqlany_stmt->handle, i, &desc_info );
	    save_stmt_error( sqlany_stmt );
	    if( !rc ) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, 
			"Failed to find information about statement parameters" );
		RETURN_FALSE;
	    }

#if PHP_MAJOR_VERSION >= 7
            convert_to_string_ex( php_param_out ); // convert IS_NULL type as well
            buffer = zend_string_alloc( desc_info.value.buffer_size + 1, 0 );
            memset( ZSTR_VAL(buffer), 0x00, desc_info.value.buffer_size + 1 );
            num_bytes_to_copy = _min( desc_info.value.buffer_size, Z_STRLEN_P(php_param_out) );
	    memcpy( ZSTR_VAL(buffer), Z_STRVAL_P(php_param_out), num_bytes_to_copy );
	    ZSTR_LEN(buffer) = num_bytes_to_copy;

            zend_string * orig_string = Z_STR_P( php_param_out );
            zend_string_release( orig_string );

            Z_STR_P(php_param_out) = buffer;
            Z_TYPE_INFO_P(php_param_out) = IS_STRING_EX;

	    sqlany_stmt->sqlany_params[i].out.buffer = (char *)Z_STRVAL_P(php_param_out);
#else
	    buffer = emalloc( desc_info.value.buffer_size + 1 );
	    num_bytes_to_copy = _min( desc_info.value.buffer_size, Z_STRLEN_P(sqlany_stmt->php_params[i].out) );
	    memcpy( buffer, Z_STRVAL_P(sqlany_stmt->php_params[i].out), num_bytes_to_copy );
	    buffer[num_bytes_to_copy] = '\0';

	    if( Z_REFCOUNT_P( sqlany_stmt->php_params[i].out ) > 1 ) {
	        Z_DELREF_P(sqlany_stmt->php_params[i].out);
	    } else {
	    	efree( Z_STRVAL_P(sqlany_stmt->php_params[i].out) );
	    }
	    Z_STRVAL_P(sqlany_stmt->php_params[i].out) = buffer;
	    Z_STRLEN_P(sqlany_stmt->php_params[i].out) = num_bytes_to_copy;

	    // don't change Z_STRLEN_P( of the out pointer )
	    sqlany_stmt->sqlany_params[i].out.buffer = (char *)Z_STRVAL_P(sqlany_stmt->php_params[i].out);
#endif

	    sqlany_stmt->sqlany_params[i].out.buffer_size = (size_t)desc_info.value.buffer_size;
	    sqlany_stmt->sqlany_params[i].out.length = &(sqlany_stmt->sqlany_params[i].out_len);

	    param.direction = DD_OUTPUT;
	    param.value = sqlany_stmt->sqlany_params[i].out;
	    api.sqlany_bind_param( sqlany_stmt->handle, i, &param );
	    save_stmt_error( sqlany_stmt );
	}
#if PHP_MAJOR_VERSION >= 7
	if( !Z_ISUNDEF_P( php_param_in ) &&
#else
	if( sqlany_stmt->php_params[i].in != NULL &&
#endif
		sqlany_stmt->sqlany_params[i].using_send_data == 0 ) {
	    a_sqlany_bind_param param;

#if PHP_MAJOR_VERSION >= 7
	    if( ZVAL_IS_NULL( php_param_in ) ) {
#else
	    if( ZVAL_IS_NULL( sqlany_stmt->php_params[i].in ) ) {
#endif
		*(sqlany_stmt->sqlany_params[i].in.is_null) = 1;
	    } else {
		*(sqlany_stmt->sqlany_params[i].in.is_null) = 0;
		switch( sqlany_stmt->sqlany_params[i].in.type ) {
		    case A_STRING:
		    case A_BINARY:
#if PHP_MAJOR_VERSION >= 7
			if( Z_TYPE_P( php_param_in ) != IS_STRING ) {
			    convert_to_string_ex( php_param_in );
			}
			sqlany_stmt->sqlany_params[i].in_len = Z_STRLEN_P(php_param_in);
			sqlany_stmt->sqlany_params[i].in.buffer = Z_STRVAL_P(php_param_in);
			sqlany_stmt->sqlany_params[i].in.length = &(sqlany_stmt->sqlany_params[i].in_len);
#else
			if( Z_TYPE_P(sqlany_stmt->php_params[i].in) != IS_STRING ) {
			    convert_to_string_ex( &sqlany_stmt->php_params[i].in );
			}
			sqlany_stmt->sqlany_params[i].in_len = Z_STRLEN_P(sqlany_stmt->php_params[i].in);
			sqlany_stmt->sqlany_params[i].in.buffer = Z_STRVAL_P(sqlany_stmt->php_params[i].in);
			sqlany_stmt->sqlany_params[i].in.length = &(sqlany_stmt->sqlany_params[i].in_len);
#endif
			break;
		    case A_VAL32:
		    case A_VAL64:
#if PHP_MAJOR_VERSION >= 7
			sqlany_stmt->sqlany_params[i].in.type =
			    ( sizeof(Z_LVAL_P(php_param_in)) == 4 ?  A_VAL32 : A_VAL64 );
			sqlany_stmt->sqlany_params[i].in.buffer = (char *)&Z_LVAL_P(php_param_in);
#else
			sqlany_stmt->sqlany_params[i].in.type =
			    ( sizeof(Z_LVAL_P(sqlany_stmt->php_params[i].in)) == 4 ?  A_VAL32 : A_VAL64 );
			sqlany_stmt->sqlany_params[i].in.buffer = (char *)&Z_LVAL_P(sqlany_stmt->php_params[i].in);
#endif
			break;
		    case A_DOUBLE:
#if PHP_MAJOR_VERSION >= 7
			sqlany_stmt->sqlany_params[i].in.buffer = (char *)&Z_DVAL_P(php_param_in);
#else
			sqlany_stmt->sqlany_params[i].in.buffer = (char *)&Z_DVAL_P(sqlany_stmt->php_params[i].in);
#endif
			break;
		    default:
			break;
		}
	    }

	    param.direction = DD_INPUT;
	    param.value = sqlany_stmt->sqlany_params[i].in;
	    api.sqlany_bind_param( sqlany_stmt->handle, i, &param );
	    save_stmt_error( sqlany_stmt );
	}
    }
    if( sqlany_stmt->sqlany_result != NULL ) {
#if PHP_MAJOR_VERSION >= 7
	zend_list_close( sqlany_stmt->sqlany_result->res );
#else
	zend_list_delete( sqlany_stmt->sqlany_result->id );
#endif
	assert( sqlany_stmt->sqlany_result == NULL );
    }

    rc = api.sqlany_execute( sqlany_stmt->handle );
    save_stmt_error( sqlany_stmt );
    if( rc == 0 ) {
	if( sqlany_conn->verbose_errors ) {
	    php_error_docref( NULL TSRMLS_CC, E_WARNING, "SQLAnywhere: [%d] %s", 
		    (int)sqlany_stmt->errorcode, sqlany_stmt->error );
	}
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_execute=>FALSE [%s]",
		    sqlany_stmt->error ) );
	RETURN_FALSE;
    }

    if( api.sqlany_num_cols( sqlany_stmt->handle ) != 0 ) {
	sqlany_result = ecalloc( 1, sizeof(sqlany_result_t) );
	sqlany_result->num_cols = api.sqlany_num_cols( sqlany_stmt->handle );
	sqlany_result->num_rows = api.sqlany_num_rows( sqlany_stmt->handle );
#if PHP_MAJOR_VERSION >= 7
	sqlany_result->res = Z_RES_P(zend_list_insert( sqlany_result, le_result ));
#else
	sqlany_result->id = zend_list_insert( sqlany_result, le_result TSRMLS_CC);
#endif
	sqlany_result->sqlany_conn = sqlany_conn;
	sqlany_result->next = sqlany_conn->result_list;
	sqlany_result->sqlany_stmt = sqlany_stmt;
	sqlany_result->stmt_handle = sqlany_stmt->handle;
	sqlany_conn->result_list = sqlany_result;
	sqlany_stmt->sqlany_result = sqlany_result;
    } else {
	for( i = 0; i < sqlany_stmt->num_params; i++ ) {
#if PHP_MAJOR_VERSION >= 7
            zval * php_param_out;
            if( Z_ISREF( sqlany_stmt->php_params[i].out ) ) {
                php_param_out = Z_REFVAL( sqlany_stmt->php_params[i].out );
            } else {
                php_param_out = &sqlany_stmt->php_params[i].out;
            }
	    if( !Z_ISUNDEF_P( php_param_out ) &&
		    (sqlany_stmt->sqlany_params[i].out.type == A_BINARY ||
		     sqlany_stmt->sqlany_params[i].out.type == A_STRING ) ) {
		Z_STRLEN_P(php_param_out) = sqlany_stmt->sqlany_params[i].out_len;
                // Fixed Warning: String is not zero-terminated
		Z_STRVAL_P(php_param_out)[sqlany_stmt->sqlany_params[i].out_len] = '\0';
	    }
#else
	    if( sqlany_stmt->php_params[i].out != NULL &&
		    (sqlany_stmt->sqlany_params[i].out.type == A_BINARY ||
		     sqlany_stmt->sqlany_params[i].out.type == A_STRING ) ) {
		Z_STRLEN_P(sqlany_stmt->php_params[i].out) = sqlany_stmt->sqlany_params[i].out_len;
	    }
#endif
	}
	sqlany_stmt->sqlany_result = NULL;
    }
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool sasql_stmt_close( sasql $stmt )
   sasql_stmt_close() is used to close a prepared statement */
PHP_FUNCTION(sasql_stmt_close)
/*****************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;

    if( ZEND_NUM_ARGS() != 1 || zend_get_parameters_array_ex(1,&arg1) != SUCCESS ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_close( %p )", sqlany_stmt->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_close( %d )", sqlany_stmt->id ) );
#endif

#if PHP_MAJOR_VERSION >= 7
    zend_list_close( sqlany_stmt->res );
#else
    zend_list_delete( sqlany_stmt->id );
#endif
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_close=>SUCCESS" ) );
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto sasql_result sasql_stmt_result_metadata( sasql $stmt )
   sasql_stmt_result_metadata() is used to retrieve the result object associated with the stmt object */
PHP_FUNCTION(sasql_stmt_result_metadata)
/***************************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;

    if( ZEND_NUM_ARGS() != 1 || zend_get_parameters_array_ex(1,&arg1) != SUCCESS ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );

    if( sqlany_stmt->sqlany_result != NULL ) {
#if PHP_MAJOR_VERSION >= 7
	RETURN_RES( sqlany_stmt->sqlany_result->res );
#else
	RETURN_RESOURCE( sqlany_stmt->sqlany_result->id );
#endif
    } else {
	RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto int sasql_stmt_affected_rows( sasql $stmt )
   sasql_stmt_affected_rows() is used to retrieve the number of rows affected by the last sasql_stmt_execute() */
PHP_FUNCTION(sasql_stmt_affected_rows)
/***************************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;
    int			  ok;

    if( ZEND_NUM_ARGS() != 1 || zend_get_parameters_array_ex(1,&arg1) != SUCCESS ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement",
	    le_stmt );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_result_metadata( %p )",
		sqlany_stmt->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_result_metadata( %d )",
		sqlany_stmt->id ) );
#endif
    ok = api.sqlany_affected_rows( sqlany_stmt->handle );
    save_stmt_error( sqlany_stmt );
    RETURN_LONG( ok );
}
/* }}} */

/* {{{ proto int sasql_stmt_num_rows( sasql $stmt )
   sasql_stmt_num_rows() is used to retrieve the number of rows retreived by the last prepared statement */
PHP_FUNCTION(sasql_stmt_num_rows)
/***************************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;

    if( ZEND_NUM_ARGS() != 1 || zend_get_parameters_array_ex(1,&arg1) != SUCCESS ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_result_metadata( %p )",
		sqlany_stmt->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_result_metadata( %d )",
		sqlany_stmt->id ) );
#endif

    if( sqlany_stmt->sqlany_result ) {
	if( sqlany_stmt->sqlany_result->curr_row != NULL ) {
	    RETURN_LONG( sqlany_stmt->sqlany_result->num_rows );
	} else {
	    RETURN_LONG( 0 );
	}
    } else {
        RETURN_LONG( 0 );
    }
}
/* }}} */

/* {{{ proto int sasql_stmt_insert_id( sasql $stmt )
   sasql_stmt_insert_id() is used to retrieve the last inserted identity value */
PHP_FUNCTION(sasql_stmt_insert_id)
/***************************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;

    if( ZEND_NUM_ARGS() != 1 || zend_get_parameters_array_ex(1,&arg1) != SUCCESS ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_result_metadata( %p )",
		sqlany_stmt->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_stmt_result_metadata( %d )",
		sqlany_stmt->id ) );
#endif

    if( !get_identity( sqlany_stmt->sqlany_conn, return_value ) ) {
	save_stmt_error( sqlany_stmt );
	RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto bool sasql_stmt_bind_result( sasql $stmt, mixed &$var1 [, mixed &$var2 ...] )
   sasql_stmt_bind_result() binds user supplied variables to result data */
PHP_FUNCTION(sasql_stmt_bind_result)
/***************************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval   * args;
#else
    zval *** args;
#endif
    sqlany_stmt_t	* sqlany_stmt;
    int			  num_cols;
    int			  i;

    if( ZEND_NUM_ARGS() < 2 ) {
	WRONG_PARAM_COUNT;
    }

#if PHP_MAJOR_VERSION >= 7
    args = (zval *)safe_emalloc( ZEND_NUM_ARGS(), sizeof(zval), 0 );
#else
    args = (zval ***)safe_emalloc( ZEND_NUM_ARGS(), sizeof(zval **), 0 );
#endif

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), args) == FAILURE ) {
	efree( args );
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, args[0], -1, "SQLAnywhere statement", le_stmt );

    num_cols = api.sqlany_num_cols( sqlany_stmt->handle );
    if( num_cols == 0 ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"Supplied statement does not return a result set" );
	efree( args );
	RETURN_FALSE;
    }

    if( (ZEND_NUM_ARGS() -1) > num_cols ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"More variables (%d) supplied than can be bound (%d)", ZEND_NUM_ARGS() -1, num_cols );
	efree( args );
	RETURN_FALSE;
    }

    if( sqlany_stmt->php_result_vars ) {
	for( i = 0; i < sqlany_stmt->num_result_vars; i++ ) {
#if PHP_MAJOR_VERSION >= 7
            // Do nothing.
#else
	    Z_DELREF_P( sqlany_stmt->php_result_vars[i] );
#endif
	}
	efree( sqlany_stmt->php_result_vars );
	sqlany_stmt->num_result_vars = 0;
    }
    sqlany_stmt->num_result_vars = num_cols;
#if PHP_MAJOR_VERSION >= 7
    sqlany_stmt->php_result_vars = ecalloc( num_cols, sizeof(zval) );
#else
    sqlany_stmt->php_result_vars = ecalloc( num_cols, sizeof(zval*) );
#endif

    for( i = 1; i < ZEND_NUM_ARGS(); i++ ) {
#if PHP_MAJOR_VERSION >= 7
	ZVAL_COPY_VALUE( &sqlany_stmt->php_result_vars[i-1], &args[i] );
#else
	Z_ADDREF_P( *args[i] );
	sqlany_stmt->php_result_vars[i-1] = *args[i];
#endif
    }
    efree( args );
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool sasql_stmt_send_long_data( sasql_stmt $stmt, int $param_num, string $data )
   sasql_stmt_send_long_data() allow the user to send the data of a buffer in chunks */
PHP_FUNCTION(sasql_stmt_send_long_data)
/***************************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[3];
#else
    zval ** args[3];
#endif
    sqlany_stmt_t	* sqlany_stmt;
    int			  i;
    int			  ok;

    if( ZEND_NUM_ARGS() != 3 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), args) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }
    sqlany_convert_to_string_ex( args[2] );
    sqlany_convert_to_long_ex( args[1] );
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, args[0], -1, "SQLAnywhere statement", le_stmt );

#if PHP_MAJOR_VERSION >= 7
    i = Z_LVAL(args[1]);
#else
    i = Z_LVAL_PP(args[1]);
#endif

    if( i >= sqlany_stmt->num_params ) {
#if PHP_MAJOR_VERSION >= 7
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"Supplied argument 2 (%ld) is not a valid parameter number", Z_LVAL(args[1]) );
#else
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"Supplied argument 2 (%ld) is not a valid parameter number", Z_LVAL_PP(args[1]) );
#endif
	RETURN_FALSE;
    }

#if PHP_MAJOR_VERSION >= 7
    zval * php_param_in;
    if( Z_ISREF( sqlany_stmt->php_params[i].in ) ) {
        php_param_in = Z_REFVAL( sqlany_stmt->php_params[i].in );
    } else {
        php_param_in = &sqlany_stmt->php_params[i].in;
    }
    if( Z_ISUNDEF_P( php_param_in ) ) {
#else
    if( sqlany_stmt->php_params[i].in == NULL ) {
#endif
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"Parameter number %d must be bound first using sasql_stmt_bind_param", i+1 );
	RETURN_FALSE;
    }
    sqlany_stmt->sqlany_params[i].using_send_data = 1;

#if PHP_MAJOR_VERSION >= 7
    ok = api.sqlany_send_param_data( sqlany_stmt->handle,
	    i, Z_STRVAL(args[2]), Z_STRLEN(args[2]) );
#else
    ok = api.sqlany_send_param_data( sqlany_stmt->handle,
	    i, Z_STRVAL_PP(args[2]), Z_STRLEN_PP(args[2]) );
#endif
    save_stmt_error( sqlany_stmt );
    if( ok ) {
	RETURN_TRUE;
    } else {
	RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto bool sasql_stmt_store_result( sasql_stmt $stmt )
   sasql_stmt_store_result() save the result set in the client */
PHP_FUNCTION(sasql_stmt_store_result)
/************************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;
    int			rc;

    if( ZEND_NUM_ARGS() != 1 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), &arg1) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );

    if( sqlany_stmt->sqlany_result == NULL ) {
	// The query does not return a result set
	RETURN_FALSE;
    }

    rc = cache_result( sqlany_stmt->sqlany_result );
    if( rc == 100 || rc == 0 ) {
	rc = 0;
	clear_stmt_error( sqlany_stmt );
    } else {
	save_stmt_error( sqlany_stmt );
    }
    if( sqlany_stmt->sqlany_result->num_rows > 0 || rc >= 0 ) {
	RETURN_TRUE;
    } else {
	RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto bool sasql_stmt_free_result( sasql_stmt $stmt )
   sasql_stmt_free_result() frees the result set from the client */
PHP_FUNCTION(sasql_stmt_free_result)
/************************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;

    if( ZEND_NUM_ARGS() != 1 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), &arg1) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );

    if( sqlany_stmt->sqlany_result == NULL ) {
	RETURN_TRUE;
    }

    free_cached_result( sqlany_stmt->sqlany_result );
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool sasql_stmt_reset( sasql_stmt $stmt )
   sasql_stmt_reset() resets the statement to it's prepared state */
PHP_FUNCTION(sasql_stmt_reset)
/*****************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;
    int			ok;

    if( ZEND_NUM_ARGS() != 1 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), &arg1) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );

    ok = api.sqlany_reset( sqlany_stmt->handle );
    save_stmt_error( sqlany_stmt );
    if( ok ) {
	//FIXME: free the result object and unbind all PHP zvals
	RETURN_TRUE;
    } else {
	RETURN_FALSE;
    }
}
/* }}} */
/* {{{ proto bool sasql_stmt_fetch( sasql_stmt $stmt )
   sasql_stmt_fetch() fetches the next result set */
PHP_FUNCTION(sasql_stmt_fetch)
/*****************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;
    sqlany_result_t	* sqlany_result;
    int			  i;
    int			  rc;

    if( ZEND_NUM_ARGS() != 1 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), &arg1) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );

    if( sqlany_stmt->sqlany_result == NULL ) {
	RETURN_FALSE;
    }
    sqlany_result = sqlany_stmt->sqlany_result;

    rc = result_fetch_next( sqlany_result, 0 );
    if( rc == 0 || rc == -1 ) {
	RETURN_FALSE;
    }

    if( sqlany_stmt->php_result_vars == NULL ) {
	// No variables were bound
	RETURN_TRUE;
    }

#if PHP_MAJOR_VERSION >= 7
    for( i = 0; i < sqlany_result->num_cols; i++ ) {
	zval * zvalue;
	zvalue = &sqlany_stmt->php_result_vars[i];
        zval * php_result_var;
        if( Z_ISREF( sqlany_stmt->php_result_vars[i] ) ) {
            php_result_var = Z_REFVAL( sqlany_stmt->php_result_vars[i] );
        } else {
            php_result_var = &sqlany_stmt->php_result_vars[i];
        }
	if( !Z_ISUNDEF_P( php_result_var ) ) {
	    a_sqlany_data_value dvalue;

	    result_get_column_data( sqlany_result, i, &dvalue );

	    if( Z_TYPE_P(php_result_var) == IS_STRING ) {
                zend_string * orig_string = Z_STR_P( php_result_var );
                zend_string_release( orig_string );
	    }

	    assign_from_sqlany_value( zvalue, &dvalue );
	}
    }
#else
    for( i = 0; i < sqlany_result->num_cols; i++ ) {
	zval * zvalue;
	if( (zvalue = sqlany_stmt->php_result_vars[i]) != NULL ) {
	    a_sqlany_data_value dvalue;

	    result_get_column_data( sqlany_result, i, &dvalue );

	    if( Z_TYPE_P(zvalue) == IS_STRING ) {
		efree( Z_STRVAL_P(zvalue) );
	    }

	    assign_from_sqlany_value( zvalue, &dvalue );
	}
    }
#endif
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto int sasql_stmt_field_count( sasql_stmt $stmt )
   sasql_stmt_field_count() returns the number of columns */
PHP_FUNCTION(sasql_stmt_field_count)
/**********************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;

    if( ZEND_NUM_ARGS() != 1 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), &arg1) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );

    if( sqlany_stmt->sqlany_result == NULL ) {
	RETURN_LONG( 0 );
    }

    sqlany_stmt->sqlany_result->num_cols = api.sqlany_num_cols( sqlany_stmt->handle );
    RETURN_LONG( sqlany_stmt->sqlany_result->num_cols );
}
/* }}} */

/* {{{ proto bool sasql_stmt_data_seek( sasql_stmt $stmt, int $offset )
   sasql_stmt_data_seek() returns true or false */
PHP_FUNCTION(sasql_stmt_data_seek)
/**********************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[2];
#else
    zval ** args[2];
#endif
    sqlany_stmt_t	* sqlany_stmt;
    sqlany_result_t	* sqlany_result;

    if( ZEND_NUM_ARGS() != 2 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), args) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }
    sqlany_convert_to_long_ex( args[1] );
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, args[0], -1, "SQLAnywhere statement", le_stmt );

    if( sqlany_stmt->sqlany_result == NULL ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"Statement does not generate a result set" );
	RETURN_FALSE;
    }
    if( sqlany_stmt->sqlany_result->curr_row == NULL ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, 
		"Must call sasql_stmt_store_result() before attempting to call sasql_stmt_data_seek()" );
	RETURN_FALSE;
    }

    sqlany_result = sqlany_stmt->sqlany_result;

#if PHP_MAJOR_VERSION >= 7
    if( Z_LVAL(args[1]) >= sqlany_result->num_rows ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid seek offset (%ld)", Z_LVAL(args[1]) );
#else
    if( Z_LVAL_PP(args[1]) >= sqlany_result->num_rows ) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid seek offset (%ld)", Z_LVAL_PP(args[1]) );
#endif
	RETURN_FALSE;
    }

#if PHP_MAJOR_VERSION >= 7
    if( result_data_seek( sqlany_result, Z_LVAL(args[1]), 0 ) ) {
#else
    if( result_data_seek( sqlany_result, Z_LVAL_PP(args[1]), 0 ) ) {
#endif
	RETURN_TRUE;
    }
    RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool sasql_stmt_next_result( sasql $stmt )
   sasql_stmt_next_result() if there is another result it it will advance to next result set and return true, otherwise false */
PHP_FUNCTION(sasql_stmt_next_result)
/**********************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;
    int			ok;

    if( ZEND_NUM_ARGS() != 1 || (zend_get_parameters_array_ex(1,&arg1) != SUCCESS) ) {
	WRONG_PARAM_COUNT;
    }

    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement", le_stmt );

    if( sqlany_stmt->sqlany_result ) {
#if PHP_MAJOR_VERSION >= 7
	zend_list_close( sqlany_stmt->sqlany_result->res );
#else
	zend_list_delete( sqlany_stmt->sqlany_result->id );
#endif
    }
    assert( sqlany_stmt->sqlany_result == NULL );

    ok = api.sqlany_get_next_result( sqlany_stmt->handle );
    save_stmt_error( sqlany_stmt );
    if( ok ) {
	sqlany_result_t * sqlany_result = (sqlany_result_t *)ecalloc( 1, sizeof(sqlany_result_t) );
	sqlany_result->num_cols = api.sqlany_num_cols( sqlany_stmt->handle );
	sqlany_result->num_rows = api.sqlany_num_cols( sqlany_stmt->handle );
#if PHP_MAJOR_VERSION >= 7
	sqlany_result->res = Z_RES_P(zend_list_insert( sqlany_result, le_result ));
#else
	sqlany_result->id = zend_list_insert( sqlany_result, le_result TSRMLS_CC); 
#endif
	sqlany_result->sqlany_conn = sqlany_stmt->sqlany_conn;
	sqlany_result->next = sqlany_stmt->sqlany_conn->result_list;
	sqlany_result->sqlany_stmt = sqlany_stmt;
	sqlany_result->stmt_handle = sqlany_stmt->handle;
	sqlany_stmt->sqlany_conn->result_list = sqlany_result;
	sqlany_stmt->sqlany_result = sqlany_result;
	RETURN_TRUE;
    } else {
	RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto bool sasql_stmt_errno( sasql_stmt $stmt )
   sasql_stmt_errno() retrieves the last errno code stmt */
PHP_FUNCTION(sasql_stmt_errno)
/*****************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;
    int			  i;

    if( ZEND_NUM_ARGS() != 1 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), &arg1) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement",
	    le_stmt );
    RETURN_LONG(sqlany_stmt->errorcode);
}
/* }}} */

/* {{{ proto bool sasql_stmt_error( sasql_stmt $stmt )
   sasql_stmt_error() retrieves the last error message from stmt */
PHP_FUNCTION(sasql_stmt_error)
/*****************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;
    int			  i;

    if( ZEND_NUM_ARGS() != 1 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), &arg1) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement",
	    le_stmt );
#if PHP_MAJOR_VERSION >= 7
    RETURN_STRING( sqlany_stmt->error );
#else
    RETURN_STRING( sqlany_stmt->error, 1 );
#endif
}
/* }}} */

/* {{{ proto bool sasql_stmt_sqlstate( sasql_stmt $stmt )
   sasql_stmt_sqlstate() retrieves the last sqlstate from stmt */
PHP_FUNCTION(sasql_stmt_sqlstate)
/*****************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_stmt_t	* sqlany_stmt;
    int			  i;

    if( ZEND_NUM_ARGS() != 1 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), &arg1) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_stmt, sqlany_stmt_t*, arg1, -1, "SQLAnywhere statement",
	    le_stmt );
#if PHP_MAJOR_VERSION >= 7
    RETURN_STRING( sqlany_stmt->sqlstate );
#else
    RETURN_STRING( sqlany_stmt->sqlstate, 1 );
#endif
}
/* }}} */


/* {{{ proto string sasql_real_escape_string( saconn $conn, string $escapestr )
   sasql_real_escape_string() escape a string for use in a SQL statement */
PHP_FUNCTION(sasql_real_escape_string)
/************************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[2];
#else
    zval ** args[2];
#endif
    sqlany_connection_t	* sqlany_conn;
    int			  i;
    char *		  source;
    char *		  dest;
    int			  dest_len = 0;

    if( ZEND_NUM_ARGS() != 2 ) {
	WRONG_PARAM_COUNT;
    }

    if( zend_get_parameters_array_ex( ZEND_NUM_ARGS(), args) == FAILURE ) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE2( sqlany_conn, sqlany_connection_t*, args[0], -1,
	    "SQLAnywhere connection", le_conn, le_pconn );
    sqlany_convert_to_string_ex( args[1] );

#if PHP_MAJOR_VERSION >= 7
    source = Z_STRVAL(args[1]);
#else
    source = Z_STRVAL_PP(args[1]);
#endif
    dest_len = 0;
#if PHP_MAJOR_VERSION >= 7
    for( i = 0; i < Z_STRLEN(args[1]); i++ ) {
#else
    for( i = 0; i < Z_STRLEN_PP(args[1]); i++ ) {
#endif
	switch( source[i] ) {
	    case '\0':
	    case '\n':
	    case '\r':
	    case 26 /* ^Z */:
	    case ';':
	    case '"':
		dest_len += 4;
		break;
	    case '\'':
	    case '\\':
		dest_len += 2;
		break;
	    default:
		dest_len++;
	}
    }

    dest = (char *)emalloc( dest_len + 1 );
    dest_len = 0;
#if PHP_MAJOR_VERSION >= 7
    for( i = 0; i < Z_STRLEN(args[1]); i++ ) {
#else
    for( i = 0; i < Z_STRLEN_PP(args[1]); i++ ) {
#endif
	switch( source[i] ) {
	    case '\0':
		dest[dest_len++] = '\\';
		dest[dest_len++] = 'x';
		dest[dest_len++] = '0';
		dest[dest_len++] = '0';
		break;
	    case '\n':
		dest[dest_len++] = '\\';
		dest[dest_len++] = 'x';
		dest[dest_len++] = '0';
		dest[dest_len++] = 'a';
		break;
	    case '\r':
		dest[dest_len++] = '\\';
		dest[dest_len++] = 'x';
		dest[dest_len++] = '0';
		dest[dest_len++] = 'd';
		break;
	    case 26 :
		dest[dest_len++] = '\\';
		dest[dest_len++] = 'x';
		dest[dest_len++] = '1';
		dest[dest_len++] = 'a';
		break;
	    case ';':
		dest[dest_len++] = '\\';
		dest[dest_len++] = 'x';
		dest[dest_len++] = '3';
		dest[dest_len++] = 'b';
		break;
	    case '"':
		dest[dest_len++] = '\\';
		dest[dest_len++] = 'x';
		dest[dest_len++] = '2';
		dest[dest_len++] = '2';
		break;
	    case '\'':
		dest[dest_len++] = '\'';
		dest[dest_len++] = '\'';
		break;
	    case '\\':
		dest[dest_len++] = '\\';
		dest[dest_len++] = '\\';
		break;
	    default:
		dest[dest_len++] = source[i];
		break;
	}
    }
    dest[dest_len] = '\0';
#if PHP_MAJOR_VERSION >= 7
    // to avoid memory leak, free the original string
    RETVAL_STRINGL(dest, dest_len); // created new zend_string
    efree(dest);
    return;
#else
    RETURN_STRINGL(dest, dest_len, 0); // no duplicate
#endif
}
/* }}} */

/* {{{ proto string sasql_get_client_info( )
   sasql_get_client_info() returns a string that represents the client version */
PHP_FUNCTION(sasql_get_client_info)
/************************************/
{
    char   buffer[20];
    int	   rc;

    if( !api.initialized ) {
#if PHP_MAJOR_VERSION >= 7
	RETURN_STRING( DBCAPI_NOT_FOUND_ERROR );
#else
	RETURN_STRING( DBCAPI_NOT_FOUND_ERROR, 1 );
#endif
    }

    if( SAG(context) ) {
	rc = api.sqlany_client_version_ex( SAG(context), buffer, sizeof(buffer) );
    } else {
	rc = api.sqlany_client_version( buffer, sizeof(buffer) );
    }
    if( rc ) {
#if PHP_MAJOR_VERSION >= 7
	RETURN_STRINGL(buffer, strlen(buffer));
#else
	RETURN_STRINGL(buffer, strlen(buffer), 1);
#endif
    } else {
	RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto bool sasql_result_all(int result [,string table_format_str [,string th_format_str [,string tr_format_str [,string tc_format_str]]]] )
   Print result as an HTML table */
PHP_FUNCTION(sasql_result_all)
/**********************************/
{
#if PHP_MAJOR_VERSION >= 7
    zval    args[5];
    zval  result_id, format_str, th_format_str, tr_format_str, td_format_str;
#else
    zval ** args[5] = { NULL, NULL, NULL, NULL, NULL };
    zval ** result_id, **format_str, **th_format_str, **tr_format_str, **td_format_str;
#endif
    sqlany_result_t	* sqlany_result;
    int num_rows;
    int i;
    char *  tr_even = NULL;
    char *  tr_odd = NULL;
    char * del = NULL;
    int	   rc;

#if PHP_MAJOR_VERSION >= 7
    for( i = 0; i < 5; i++ ) {
        ZVAL_UNDEF(&args[i]);
    }
#endif
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_result_all started " ) );

    if( ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 5 ) {
	WRONG_PARAM_COUNT;
    }
    if( zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args ) != SUCCESS) {
	WRONG_PARAM_COUNT;
    }

    result_id = args[0];
    format_str = args[1];
    th_format_str = args[2];
    tr_format_str = args[3];
    td_format_str = args[4];
    switch( ZEND_NUM_ARGS() ) {
	case 5:
#if PHP_MAJOR_VERSION >= 7
	    convert_to_string_ex(&td_format_str);
	    if( strcmp( Z_STRVAL(td_format_str), "none" ) == 0 ) {
		Z_STRVAL(td_format_str)[0] = '\0';
	    }
#else
	    convert_to_string_ex(td_format_str);
	    if( strcmp( Z_STRVAL_PP(td_format_str), "none" ) == 0 ) {
		Z_STRVAL_PP(td_format_str)[0] = '\0';
	    }
#endif
	case 4:
#if PHP_MAJOR_VERSION >= 7
	    convert_to_string_ex(&tr_format_str);
	    if( strcmp( Z_STRVAL(tr_format_str), "none" ) == 0 ) {
		Z_STRVAL(tr_format_str)[0] = '\0';
	    } else {
		del=strstr( Z_STRVAL(tr_format_str) , "><" ) ;
		if ( del != NULL ) {
		    int x = (int)(del - Z_STRVAL(tr_format_str));
		    tr_even = emalloc( x + 1 );
		    memcpy( tr_even, del + 2, x );
		    tr_even[x] = '\0';
		    *del = '\0';
		}
		tr_odd = emalloc( strlen( Z_STRVAL(tr_format_str)) + 1 );
		strcpy( tr_odd, Z_STRVAL(tr_format_str));
		if( del == NULL ) {
		    tr_even = tr_odd;
		}
	    }
#else
	    convert_to_string_ex(tr_format_str);
	    if( strcmp( Z_STRVAL_PP(tr_format_str), "none" ) == 0 ) {
		Z_STRVAL_PP(tr_format_str)[0] = '\0';
	    } else {
		del=strstr( Z_STRVAL_PP(tr_format_str) , "><" ) ;
		if ( del != NULL ) {
		    int x = (int)(del - Z_STRVAL_PP(tr_format_str));
		    tr_even = emalloc( x + 1 );
		    memcpy( tr_even, del + 2, x );
		    tr_even[x] = '\0';
		    *del = '\0';
		}
		tr_odd = emalloc( strlen( Z_STRVAL_PP(tr_format_str)) + 1 );
		strcpy( tr_odd, Z_STRVAL_PP(tr_format_str));
		if( del == NULL ) {
		    tr_even = tr_odd;
		}
	    }
#endif
	case 3:
#if PHP_MAJOR_VERSION >= 7
	    convert_to_string_ex(&th_format_str);
	    if( strcmp( Z_STRVAL(th_format_str), "none" ) == 0 ) {
		Z_STRVAL(th_format_str)[0] = '\0';
	    }
#else
	    convert_to_string_ex(th_format_str);
	    if( strcmp( Z_STRVAL_PP(th_format_str), "none" ) == 0 ) {
		Z_STRVAL_PP(th_format_str)[0] = '\0';
	    }
#endif
	case 2:
#if PHP_MAJOR_VERSION >= 7
	    convert_to_string_ex(&format_str);
	    if( strcmp( Z_STRVAL(format_str), "none" ) == 0 ) {
		Z_STRVAL(format_str)[0] = '\0';
	    }
#else
	    convert_to_string_ex(format_str);
	    if( strcmp( Z_STRVAL_PP(format_str), "none" ) == 0 ) {
		Z_STRVAL_PP(format_str)[0] = '\0';
	    }
#endif
	case 1:
	    ZEND_FETCH_RESOURCE( sqlany_result, sqlany_result_t*, result_id, -1, "SQLAnywhere result", le_result );
	    break;

	default:
	    WRONG_PARAM_COUNT;
	    break;
    }

    if( sqlany_result->num_cols == 0 ) {
	//php_error_docref(NULL TSRMLS_CC, E_WARNING, "No DATA available " );
	_debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_result_all returned error2" ) );
	RETURN_FALSE;
    }

#if PHP_MAJOR_VERSION >= 7
    if( Z_ISUNDEF(format_str) ) {
#else
    if( format_str == NULL ) {
#endif
	php_printf( "<table>\n" );
    } else {
#if PHP_MAJOR_VERSION >= 7
	php_printf( "<table %s>\n", Z_STRVAL(format_str) );
#else
	php_printf( "<table %s>\n", Z_STRVAL_PP(format_str) );
#endif
    }

    if( tr_even == NULL ) {
	php_printf( "<tr>\n" );
    } else {
	php_printf( "<tr %s>\n", tr_even );
    }
    for( i = 0; i < sqlany_result->num_cols; i++ ) {
	a_sqlany_column_info	cinfo;
	result_get_column_info( sqlany_result, i, &cinfo );
#if PHP_MAJOR_VERSION >= 7
	if( Z_ISUNDEF(th_format_str) ) {
#else
	if( th_format_str == NULL ) {
#endif
	    php_printf("\t<th>" );
	} else {
#if PHP_MAJOR_VERSION >= 7
	    php_printf("\t<th %s>", Z_STRVAL(th_format_str) );
#else
	    php_printf("\t<th %s>", Z_STRVAL_PP(th_format_str) );
#endif
	}
	php_printf( "%s</th>\n", cinfo.name );
    }
    php_printf( "</tr>\n" );


    /* now fetch and print data */
    num_rows = 0;

    if( !result_data_seek( sqlany_result, 0, 1 ) ) {
	RETURN_FALSE;
    }

    while( 1 ) {
	rc = result_fetch_next( sqlany_result, 1 );
	if( rc == 0 || rc == -1 ) {
	    break;
	}

	num_rows++;
	if ( tr_odd == NULL ) {
	    php_printf( "<tr>\n" );
	} else {
	    php_printf( "<tr %s>\n", (num_rows % 2 == 0 ? tr_even : tr_odd )  );
	}
	for( i = 0; i < sqlany_result->num_cols; i ++ ) {
	    a_sqlany_data_value dvalue;

#if PHP_MAJOR_VERSION >= 7
	    if( Z_ISUNDEF(td_format_str) ) {
#else
	    if( td_format_str == NULL ) {
#endif
		php_printf( "\t<td>" );
	    } else {
#if PHP_MAJOR_VERSION >= 7
		php_printf( "\t<td %s>", Z_STRVAL(td_format_str) );
#else
		php_printf( "\t<td %s>", Z_STRVAL_PP(td_format_str) );
#endif
	    }

	    result_get_column_data( sqlany_result, i, &dvalue );
	    if( (*(dvalue.is_null)) == 0 ) {
		switch( dvalue.type ) {
		    case A_STRING:
		    case A_BINARY:
			php_write((char *)dvalue.buffer, *(dvalue.length) TSRMLS_CC );
			break;

		    case A_VAL32:
		    case A_UVAL32:
		    case A_VAL16:
		    case A_UVAL16:
		    case A_VAL8:
		    case A_UVAL8:
			{
			    long value;
			    switch( dvalue.type ) {
				case A_VAL32:
				    value = *((int *)dvalue.buffer);
				    break;
				case A_UVAL32:
				    value = *((unsigned int *)dvalue.buffer);
				    break;
				case A_VAL16:
				    value = *((short *)dvalue.buffer);
				    break;
				case A_UVAL16:
				    value = *((unsigned short *)dvalue.buffer);
				    break;
				case A_VAL8:
				    value = *((char *)dvalue.buffer);
				    break;
				case A_UVAL8:
				    value = *((unsigned char *)dvalue.buffer);
				    break;
				default:
				    value = 0;
			    }
			    php_printf( "%ld", value );
			}
			break;

		    case A_VAL64:
			php_printf( "%"SA_FMT64"d", *(sasql_int64*)dvalue.buffer );
			break;

		    case A_UVAL64:
			php_printf( "%"SA_FMT64"u", *(sasql_uint64*)dvalue.buffer );
			break;

		    case A_DOUBLE:
			php_printf( "%f", *(double *)dvalue.buffer );
			break;

		    default: 
			php_printf( "(Unkown Type)" );
			break;
		}
	    }
	    php_printf( "</td>\n" );
	}
	php_printf( "</tr>\n" );
    }
    
    php_printf("</table>\n" );

    if( tr_odd != NULL ) {
	efree( tr_odd );
    }
    if( del && tr_even != NULL ) {
	efree( tr_even );
    }

    if( num_rows == 0 ) {
	php_printf("<h2>No rows found</h2>\n");
    }

    _debug_enabled( SQLAnyDebug( _LOCATION_, "sasql_result_all returned" ) );
    RETURN_LONG( num_rows );
}
/* }}} */

/* {{{ proto int sasql_num_fields( sasql_result $result )
   Get num fields in result */
PHP_FUNCTION(sasql_num_fields)
/*************************************/
{
    DECLARE_ZVAL_ARG1;
    sqlany_result_t	* sqlany_result;

    if( ( ZEND_NUM_ARGS() != 1) || (zend_get_parameters_array_ex(1,&arg1) != SUCCESS )) {
	WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE( sqlany_result, sqlany_result_t*, arg1, -1, "SQLAnywhere result", 
	    le_result );
#if PHP_MAJOR_VERSION >= 7
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_num_fields( %p )", sqlany_result->res ) );
#else
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_num_fields( %d )", sqlany_result->id ) );
#endif
    _debug_enabled( SQLAnyDebug( _LOCATION_, " sasql_num_fields=>%d", sqlany_result->num_cols ) );
    RETURN_LONG( sqlany_result->num_cols );
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
