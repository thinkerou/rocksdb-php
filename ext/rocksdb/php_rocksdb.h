#ifndef PHP_ROCKSDB_H
#define PHP_ROCKSDB_H

extern zend_module_entry rocksdb_module_entry;
#define phpext_rocksdb_ptr &rocksdb_module_entry

#define PHP_ROCKSDB_VERSION "0.1.0"

#ifdef PHP_WIN32
#	define PHP_ROCKSDB_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_ROCKSDB_API __attribute__ ((visibility("default")))
#else
#	define PHP_ROCKSDB_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

//#include "php.h"

//#include "rocksdb/c.h"

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(rocksdb)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(rocksdb)
*/

/* In every utility function you add that needs to use variables 
   in php_rocksdb_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as ROCKSDB_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define ROCKSDB_G(v) TSRMG(rocksdb_globals_id, zend_rocksdb_globals *, v)
#else
#define ROCKSDB_G(v) (rocksdb_globals.v)
#endif

#endif	/* PHP_ROCKSDB_H */

