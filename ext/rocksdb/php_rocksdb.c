#include "db.h"
#include "writebatch.h"
#include "iterator.h"
//#include "snapshot.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_rocksdb.h"

/* If you declare any globals in php_rocksdb.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(rocksdb)
*/

/* True global resources - no need for thread safety here */
/* static int le_rocksdb; */

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("rocksdb.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_rocksdb_globals, rocksdb_globals)
    STD_PHP_INI_ENTRY("rocksdb.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_rocksdb_globals, rocksdb_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_rocksdb_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_rocksdb_init_globals(zend_rocksdb_globals *rocksdb_globals)
{
	rocksdb_globals->global_value = 0;
	rocksdb_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(rocksdb)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/

    rocksdb_init_db(TSRMLS_C);
    rocksdb_init_writebatch(TSRMLS_C);
    rocksdb_init_iterator(TSRMLS_C);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(rocksdb)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(rocksdb)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(rocksdb)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(rocksdb)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "rocksdb support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ rocksdb_functions[]
 *
 * Every user visible function must have an entry in rocksdb_functions[].
 */
const zend_function_entry rocksdb_functions[] = {
	PHP_FE_END	/* Must be the last line in rocksdb_functions[] */
};
/* }}} */

/* {{{ rocksdb_module_entry
 */
zend_module_entry rocksdb_module_entry = {
	STANDARD_MODULE_HEADER,
	"rocksdb",
	rocksdb_functions,
	PHP_MINIT(rocksdb),
	PHP_MSHUTDOWN(rocksdb),
	NULL,
    NULL,
    PHP_MINFO(rocksdb),
	PHP_ROCKSDB_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ROCKSDB
ZEND_GET_MODULE(rocksdb)
#endif

