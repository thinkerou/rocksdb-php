#include "writebatch.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/spl/spl_exceptions.h"
#include "zend_exceptions.h"
#include "php_rocksdb.h"

zend_class_entry *rocksdb_ce_writebatch;

void free_wrapped_rocksdb_writebatch(void *object TSRMLS_DC) {
  wrapped_rocksdb_writebatch *p = (wrapped_rocksdb_writebatch *)object;
  if (p->writebatch) {
    rocksdb_writebatch_destroy(p->writebatch);
  }
  zend_object_std_dtor(&p->std TSRMLS_CC);
  efree(p);

  // zend_objects_free_object_storage((zend_object *)object TSRMLS_CC);
}

zend_object_value create_wrapped_rocksdb_writebatch(zend_class_entry *class_type
                                       TSRMLS_DC) {
  wrapped_rocksdb_writebatch *intern;
  zend_object_value retval;
  intern = (wrapped_rocksdb_writebatch *)emalloc(sizeof(wrapped_rocksdb_writebatch));
  memset(intern, 0, sizeof(wrapped_rocksdb_writebatch));
  zend_object_std_init(&intern->std, class_type TSRMLS_CC);
  object_properties_init(&intern->std, class_type);
  retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, free_wrapped_rocksdb_writebatch, NULL TSRMLS_CC);
  retval.handlers = zend_get_std_object_handlers();
  return retval;
}

PHP_METHOD(RocksDBWriteBatch, __construct) {
  rocksdb_writebatch_t *wbatch;
  wrapped_rocksdb_writebatch *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_writebatch *)zend_object_store_get_object(getThis() TSRMLS_CC);
  wbatch = rocksdb_writebatch_create();

  intern->writebatch = wbatch;
}

PHP_METHOD(RocksDBWriteBatch, put) {
  char *key;
  char *value;
  int klen;
  int vlen;

  wrapped_rocksdb_writebatch *intern;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &key, &klen, &value, &vlen) == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_writebatch *)zend_object_store_get_object(getThis() TSRMLS_CC);
  rocksdb_writebatch_put(intern->writebatch, key, klen, value, vlen);

  RETURN_TRUE;
}

PHP_METHOD(RocksDBWriteBatch, delete) {
  char *key;
  int klen;

  wrapped_rocksdb_writebatch *intern;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &klen) == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_writebatch *)zend_object_store_get_object(getThis() TSRMLS_CC);
  rocksdb_writebatch_delete(intern->writebatch, key, klen);

  RETURN_TRUE;
}

PHP_METHOD(RocksDBWriteBatch, clear) {
  wrapped_rocksdb_writebatch *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_writebatch *)zend_object_store_get_object(getThis() TSRMLS_CC);
  rocksdb_writebatch_clear(intern->writebatch);

  RETURN_TRUE;
}

static zend_function_entry writebatch_methods[] = {
  PHP_ME(RocksDBWriteBatch, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
  PHP_ME(RocksDBWriteBatch, put, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDBWriteBatch, delete, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDBWriteBatch, clear, NULL, ZEND_ACC_PUBLIC)
  PHP_FE_END 
};

void rocksdb_init_writebatch(TSRMLS_D) {
  zend_class_entry ce;
  INIT_CLASS_ENTRY(ce, "RocksDB\\RocksDBWriteBatch", writebatch_methods);
  ce.create_object = create_wrapped_rocksdb_writebatch;
  rocksdb_ce_writebatch = zend_register_internal_class(&ce TSRMLS_CC);
}

