#include "db.h"
#include "iterator.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/spl/spl_exceptions.h"
#include "zend_exceptions.h"
#include "php_rocksdb.h"

zend_class_entry *rocksdb_ce_iterator;

void free_wrapped_rocksdb_iterator(void *object TSRMLS_DC) {
  wrapped_rocksdb_iterator *p = (wrapped_rocksdb_iterator *)object;
  if (p->iterator) {
    if (((wrapped_rocksdb_db *)zend_object_store_get_object(p->db TSRMLS_CC))->db != NULL) {
      rocksdb_iter_destroy(p->iterator);
    }
  }

  if (p->db) {
    zval_ptr_dtor(&p->db);
  }

  zend_object_std_dtor(&p->std TSRMLS_CC);
  efree(p);

  // zend_objects_free_object_storage((zend_object *)object TSRMLS_CC);
}

zend_object_value create_wrapped_rocksdb_iterator(zend_class_entry *class_type
                                       TSRMLS_DC) {
  wrapped_rocksdb_iterator *intern;
  zend_object_value retval;
  intern = (wrapped_rocksdb_iterator *)emalloc(sizeof(wrapped_rocksdb_iterator));
  memset(intern, 0, sizeof(wrapped_rocksdb_iterator));
  zend_object_std_init(&intern->std, class_type TSRMLS_CC);
  object_properties_init(&intern->std, class_type);
  retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, free_wrapped_rocksdb_iterator, NULL TSRMLS_CC);
  retval.handlers = zend_get_std_object_handlers();
  return retval;
}

PHP_METHOD(RocksDBIterator, __construct) {
  zval *zdb = NULL;
  zval *zreadoptions = NULL;
  wrapped_rocksdb_db *wrapped_db;

  rocksdb_readoptions_t *readop;
  wrapped_rocksdb_iterator *intern;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|z", &zdb, rocksdb_ce_db, &zreadoptions) == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_iterator *)zend_object_store_get_object(getThis() TSRMLS_CC);
  wrapped_db = (wrapped_rocksdb_db *)zend_object_store_get_object(zdb TSRMLS_CC);

  // check

  readop = php_rocksdb_get_readoptions(wrapped_db, zreadoptions TSRMLS_CC);
  if (!readop) {
    return;
  }

  intern->iterator = rocksdb_create_iterator(wrapped_db->db, readop);
  rocksdb_readoptions_destroy(readop);

  Z_ADDREF_P(zdb);
  intern->db = zdb;

  rocksdb_iter_seek_to_first(intern->iterator);
}

PHP_METHOD(RocksDBIterator, destroy) {
  wrapped_rocksdb_iterator *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_iterator *)zend_object_store_get_object(getThis() TSRMLS_CC);
  if (intern->iterator) {
    rocksdb_iter_destroy(intern->iterator);
    intern->iterator = NULL;
  }

  RETURN_TRUE;
}

PHP_METHOD(RocksDBIterator, getError) {
  char *err = NULL;

  wrapped_rocksdb_iterator *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_iterator *)zend_object_store_get_object(getThis() TSRMLS_CC);

  // check

  rocksdb_iter_get_error(intern->iterator, &err);

  if (err == NULL) {
    RETURN_FALSE;
  }

  RETVAL_STRING(err, 1);
  free(err);
}

PHP_METHOD(RocksDBIterator, current) {
  char *value = NULL;
  size_t vlen;

  wrapped_rocksdb_iterator *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_iterator *)zend_object_store_get_object(getThis() TSRMLS_CC);

  // check

  if (!rocksdb_iter_valid(intern->iterator) ||
      !(value = (char *)rocksdb_iter_value(intern->iterator, &vlen))) {
    RETURN_FALSE;
  }

  RETURN_STRINGL(value, vlen, 1);
}

PHP_METHOD(RocksDBIterator, key)
{
  char *key = NULL;
  size_t klen;

  wrapped_rocksdb_iterator *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_iterator *)zend_object_store_get_object(getThis() TSRMLS_CC);

  // check

  if (!rocksdb_iter_valid(intern->iterator) ||
      !(key = (char *)rocksdb_iter_key(intern->iterator, &klen))) {
    RETURN_FALSE;
  }

  RETURN_STRINGL(key, klen, 1);
}

PHP_METHOD(RocksDBIterator, next) {
  wrapped_rocksdb_iterator *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_iterator *)zend_object_store_get_object(getThis() TSRMLS_CC);

  // check

  if (rocksdb_iter_valid(intern->iterator)) {
    rocksdb_iter_next(intern->iterator);
  }
}

PHP_METHOD(RocksDBIterator, prev) {
  wrapped_rocksdb_iterator *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_iterator *)zend_object_store_get_object(getThis() TSRMLS_CC);

  // check

  if (rocksdb_iter_valid(intern->iterator)) {
    rocksdb_iter_prev(intern->iterator);
  }
}

PHP_METHOD(RocksDBIterator, rewind) {
  wrapped_rocksdb_iterator *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_iterator *)zend_object_store_get_object(getThis() TSRMLS_CC);

  // check

  rocksdb_iter_seek_to_first(intern->iterator);
}

PHP_METHOD(RocksDBIterator, last) {
  wrapped_rocksdb_iterator *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_iterator *)zend_object_store_get_object(getThis() TSRMLS_CC);

  // check
 
  rocksdb_iter_seek_to_last(intern->iterator);
}

PHP_METHOD(RocksDBIterator, seek) {
  char *key;
  int klen;

  wrapped_rocksdb_iterator *intern;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &klen ) == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_iterator *)zend_object_store_get_object(getThis() TSRMLS_CC);

  // check
 
  rocksdb_iter_seek(intern->iterator, key, (size_t)klen);
}

PHP_METHOD(RocksDBIterator, valid) {
  wrapped_rocksdb_iterator *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_iterator *)zend_object_store_get_object(getThis() TSRMLS_CC);

  // check

  RETURN_BOOL(rocksdb_iter_valid(intern->iterator));
}

static zend_function_entry iterator_methods[] = {
  PHP_ME(RocksDBIterator, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
  PHP_ME(RocksDBIterator, destroy, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDBIterator, getError, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDBIterator, current, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDBIterator, key, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDBIterator, prev, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDBIterator, next, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDBIterator, seek, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDBIterator, last, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDBIterator, rewind, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDBIterator, valid, NULL, ZEND_ACC_PUBLIC)
  PHP_FE_END 
};

void rocksdb_init_iterator(TSRMLS_D) {
  zend_class_entry ce;
  INIT_CLASS_ENTRY(ce, "RocksDB\\RocksDBIterator", iterator_methods);
  ce.create_object = create_wrapped_rocksdb_iterator;
  rocksdb_ce_iterator = zend_register_internal_class(&ce TSRMLS_CC);
}

