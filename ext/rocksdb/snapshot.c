/*#include "snapshot.h"

#include "db.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/spl/spl_exceptions.h"
#include "zend_exceptions.h"
#include "php_rocksdb.h"

//#include "rocksdb/c.h"


zend_class_entry *rocksdb_ce_snapshot;

void free_wrapped_rocksdb_snapshot(void *object TSRMLS_DC) {
  wrapped_rocksdb_snapshot *p = (wrapped_rocksdb_snapshot*)object;

  if (p->snapshot) {
    rocksdb_t *db = ((wrapped_rocksdb_db*)zend_object_store_get_object(p->db TSRMLS_CC))->db;
    if (db != NULL) {
      rocksdb_release_snapshot(db, p->snapshot);
    }
    zend_ptr_dtor(&p->db TSRMLS_CC);
  }

  zend_objects_free_object_storage((zend_object*)object TSRMLS_CC);
}

static zend_object_value create_wrapped_rocksdb_snapshot(zend_class_entry *class_type TSRMLS_DC) {
  wrapped_rocksdb_snapshot *intern;
  zend_object_value retval;
 
  intern = (wrapped_rocksdb_snapshot*)emalloc(sizeof(wrapped_rocksdb_snapshot));
  memset(intern, 0, sizeof(wrapped_rocksdb_snapshot));
                                                  
  zend_object_std_init(&intern->std, class_type TSRMLS_CC);
  object_properties_init(&intern->std, class_type);
  //init_properties(intern);
                                    
  retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, free_wrapped_rocksdb_snapshot, NULL TSRMLS_CC);
  retval.handlers = zend_get_std_object_handlers();
  return retval;
}

PHP_METHOD(RocksDBSnapshot, __construct) {
}

PHP_METHOD(RocksDBSnapshot, release) {
}

static zend_function_entry snapshot_methods[] = {
  PHP_ME(RocksDBSnapshot, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
  PHP_ME(RocksDBSnapshot, release, NULL, ZEND_ACC_PUBLIC)   
  PHP_FE_END
};

void rocksdb_init_db(TSRMLS_D) {
  zend_class_entry ce;
  INIT_CLASS_ENTRY(ce, "RocksDB\\RocksDBSnapshot", snapshot_methods);
  ce.create_object = create_wrapped_rocksdb_snapshot;
  rocksdb_ce_snapshot = zend_register_internal_class(&ce TSRMLS_CC);
}
*/
