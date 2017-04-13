#include "db.h"

//#include "snapshot.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/spl/spl_exceptions.h"
#include "zend_exceptions.h"
#include "php_rocksdb.h"

zend_class_entry *rocksdb_ce_db;

void free_wrapped_rocksdb_db(void *object TSRMLS_DC) {
  wrapped_rocksdb_db *p = (wrapped_rocksdb_db *)object;
  if (p->db) {
    rocksdb_close(p->db);
  }
  if (p->comparator) {
    rocksdb_comparator_destroy(p->comparator);
    efree(p->callable);
  }
  zend_object_std_dtor(&p->std TSRMLS_CC);
  efree(p);
}

zend_object_value create_wrapped_rocksdb_db(zend_class_entry *class_type
                                       TSRMLS_DC) {
  wrapped_rocksdb_db *intern;
  zend_object_value retval;
  intern = (wrapped_rocksdb_db *)emalloc(sizeof(wrapped_rocksdb_db));
  memset(intern, 0, sizeof(wrapped_rocksdb_db));
  zend_object_std_init(&intern->std, class_type TSRMLS_CC);
  object_properties_init(&intern->std, class_type);
  retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, free_wrapped_rocksdb_db, NULL TSRMLS_CC);
  retval.handlers = zend_get_std_object_handlers();
  return retval;
}

static void rocksdb_custom_comparator_destructor(void* stat) {
  zval* callable = (zval*)stat;
  zval_ptr_dtor(&callable);
}

static int rocksdb_custom_comparator_compare(void* stat, const char* a, size_t alen, const char* b, size_t blen) {
  zval* callable = (zval*)stat;
  zval* params[2];
  zval* result = NULL;
  int ret;
  TSRMLS_FETCH();

  MAKE_STD_ZVAL(params[0]);
  MAKE_STD_ZVAL(params[1]);
  MAKE_STD_ZVAL(result);

  ZVAL_STRINGL(params[0], (char*)a, alen, 1);
  ZVAL_STRINGL(params[1], (char*)b, blen, 1);

  if (call_user_function(EG(function_table), NULL, callable, result, 2, params TSRMLS_CC) == SUCCESS) {
    convert_to_long(result);
  }
  
  zval_ptr_dtor(&params[0]);
  zval_ptr_dtor(&params[1]);

  ret = Z_LVAL_P(result);
  zval_ptr_dtor(&result);

  return ret;
}

#define PHP_ROCKSDB_CUSTORM_COMPARATOR_NAME "php_rocksdb.custom_comparator"

static const char* rocksdb_custom_comparator_name(void* stat) {
  return PHP_ROCKSDB_CUSTORM_COMPARATOR_NAME;
}

static inline rocksdb_options_t* php_rocksdb_get_openoptions(zval* options, rocksdb_comparator_t** comparator, char** callable TSRMLS_DC) {
  zval** value;
  HashTable* ht;
  rocksdb_options_t* op = rocksdb_options_create();

  rocksdb_options_set_create_if_missing(op, 1);

  if (options == NULL) {
    return op;
  }

  ht = Z_ARRVAL_P(options);
  if (zend_hash_find(ht, "create_if_missing", sizeof("create_if_missing"), (void**)&value) == SUCCESS) {
    rocksdb_options_set_create_if_missing(op, zend_is_true(*value));
  }
  if (zend_hash_find(ht, "error_if_exists", sizeof("error_if_exists"), (void**)&value) == SUCCESS) {
    rocksdb_options_set_error_if_exists(op, zend_is_true(*value));
  }
  if (zend_hash_find(ht, "paranoid_checks", sizeof("paranoid_checks"), (void**)&value) == SUCCESS) {
    rocksdb_options_set_paranoid_checks(op, zend_is_true(*value));
  }
  if (zend_hash_find(ht, "write_buffer_size", sizeof("write_buffer_size"), (void**)&value) == SUCCESS) {
    convert_to_long(*value);
    rocksdb_options_set_write_buffer_size(op, Z_LVAL_PP(value));
  }
  if (zend_hash_find(ht, "max_open_files", sizeof("max_open_files"), (void**)&value) == SUCCESS) {
    convert_to_long(*value);
    rocksdb_options_set_max_open_files(op, Z_LVAL_PP(value));
  }
  if (zend_hash_find(ht, "compression", sizeof("compression"), (void**)&value) == SUCCESS) {
    convert_to_long(*value);
    if (Z_LVAL_PP(value) != rocksdb_no_compression && Z_LVAL_PP(value) != rocksdb_snappy_compression) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid compression type");
    } else {
      rocksdb_options_set_compression(op, Z_LVAL_PP(value));
    }
  }
  if (zend_hash_find(ht, "comparator", sizeof("comparator"), (void**)&value) == SUCCESS && !ZVAL_IS_NULL(*value)) {
    rocksdb_comparator_t *comp;
    if (!zend_is_callable(*value, 0, callable TSRMLS_CC)) {
      zend_throw_exception(spl_ce_InvalidArgumentException, "Invalid open options: comparator", 1 TSRMLS_CC);
      efree(*callable);
      *callable = NULL;
      rocksdb_options_destroy(op);
      return NULL;
    }
    Z_ADDREF_PP(value);
    comp = rocksdb_comparator_create((void*)(*value),
                                     rocksdb_custom_comparator_destructor,
                                     rocksdb_custom_comparator_compare,
                                     rocksdb_custom_comparator_name);
    if (comp) {
      *comparator = comp;      
    }
    rocksdb_options_set_comparator(op, comp);
  }
  return op;
}

static inline rocksdb_readoptions_t *php_rocksdb_get_readoptions(wrapped_rocksdb_db *intern, zval *options TSRMLS_DC) {
  zval **value;
  HashTable *ht;
  rocksdb_readoptions_t *read_options = rocksdb_readoptions_create();
  if (options == NULL) {
    return read_options;
  }
 
  ht = Z_ARRVAL_P(options);
  if (zend_hash_find(ht, "verify_check_sum", sizeof("verify_check_sum"), (void**)&value) == SUCCESS) {
    rocksdb_readoptions_set_verify_checksums(read_options, zend_is_true(*value));
  } else {
    rocksdb_readoptions_set_verify_checksums(read_options, intern->verify_check_sum);
  }
  if (zend_hash_find(ht, "fill_cache", sizeof("fill_cache"), (void**)&value) == SUCCESS) {
    rocksdb_readoptions_set_fill_cache(read_options, zend_is_true(*value));
  } else {
    rocksdb_readoptions_set_fill_cache(read_options, intern->fill_cache);
  }
  /*if (zend_hash_find(ht, "snapshot", sizeof("snapshot"), (void**)&value) == SUCCESS &&
      !ZVAL_IS_NULL(*value)) {
    if (Z_TYPE_PP(value) == IS_OBJECT && Z_OBJECT_PP(value) == rocksdb_ce_snapshot) {
      wrapped_rocksdb_snapshot *obj = (wrapped_rocksdb_snapshot*)zend_object_store_get_object(*value TSRMLS_CC);
      if (obj->snapshot == NULL) {
        //zend_throw_exception_ex(php_rocksdb_ce_RocksDBException, 0 TSRMLS_CC, "Invalid snapshot parameter, it has been released");
        rocksdb_readoptions_destroy(read_options);
        return NULL;
      }

      rocksdb_readoptions_set_snapshot(read_options, obj->snapshot);
    } else {
      //zend_throw_exception_ex(php_rocksdb_ce_RocksDBException, 0 TSRMLS_CC, "Invalid snapshot parameter, it must be an instance of RocksDBSnapshot");
      rocksdb_readoptions_destroy(read_options);
      return NULL;
    }
  }*/
  return read_options;
}

static inline void php_rocksdb_set_readoptions(wrapped_rocksdb_db *intern, zval *options) {
  zval **value;
  HashTable *ht;

  if (options == NULL) {
    return;
  }

  ht = Z_ARRVAL_P(options);
  if (zend_hash_find(ht, "verify_check_sum", sizeof("verify_check_sum"), (void**)&value) == SUCCESS) {
    intern->verify_check_sum = zend_is_true(*value);
  }
  if (zend_hash_find(ht, "fill_cache", sizeof("fill_cache"), (void**)&value) == SUCCESS) {
    intern->fill_cache = zend_is_true(*value);
  }
}

static inline rocksdb_writeoptions_t *php_rocksdb_get_writeoptions(wrapped_rocksdb_db *intern, zval *options) {
  zval **value;
  HashTable *ht;
  rocksdb_writeoptions_t *write_options = rocksdb_writeoptions_create();
  if (options == NULL) {
    return write_options;
  }

  ht = Z_ARRVAL_P(options);
  if (zend_hash_find(ht, "sync", sizeof("sync"), (void**)&value) == SUCCESS) {
    rocksdb_writeoptions_set_sync(write_options, zend_is_true(*value));
  } else {
    rocksdb_writeoptions_set_sync(write_options, intern->sync);
  }

  return write_options;
}

static inline void php_rocksdb_set_writeoptions(wrapped_rocksdb_db *intern, zval *options) {
  zval **value;

  if (options == NULL) {
    return;
  }

  if (zend_hash_find(Z_ARRVAL_P(options), "sync", sizeof("sync"), (void**)&value) == SUCCESS) {
    intern->sync = zend_is_true(*value);
  }
}

PHP_METHOD(RocksDB, __construct) {
  char *name;
  int len;
  zval *options = NULL;
  zval *read_options = NULL;
  zval *write_options = NULL;
  char *err = NULL;
  rocksdb_t *db = NULL;
  rocksdb_options_t *open_options;
  wrapped_rocksdb_db *intern;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|a!a!a!",
                            &name, &len, &options, &read_options,
                            &write_options) == FAILURE) {
    return;
  }

  // dir check

  intern = (wrapped_rocksdb_db *)zend_object_store_get_object(getThis() TSRMLS_CC);
  if (intern->db) {
    rocksdb_close(db);
  }

  open_options = php_rocksdb_get_openoptions(options, &intern->comparator, &intern->callable TSRMLS_CC);
  if (!open_options) {
    return;
  }

  php_rocksdb_set_readoptions(intern, read_options);
  php_rocksdb_set_writeoptions(intern, write_options);

  db = rocksdb_open(open_options, (const char *)name, &err);

  rocksdb_options_destroy(open_options);

  // rocksdb_check_error(err);

  intern->db = db;
}

PHP_METHOD(RocksDB, close) {
  wrapped_rocksdb_db *intern;

  if (zend_parse_parameters_none() == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_db*)zend_object_store_get_object(getThis() TSRMLS_CC);
  if (intern->db) {
    rocksdb_close(intern->db);
    intern->db = NULL;
  }

  RETURN_TRUE;
}

PHP_METHOD(RocksDB, get) {
  char *key;
  char *value;
  int klen;
  size_t vlen;
  zval *read_options = NULL;
  char *err = NULL;
  rocksdb_readoptions_t *rp;
  wrapped_rocksdb_db *intern;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &key, &klen, &read_options) == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_db*)zend_object_store_get_object(getThis() TSRMLS_CC);
  // check

  rp = php_rocksdb_get_readoptions(intern, read_options TSRMLS_CC);
  value = rocksdb_get(intern->db, rp, key, klen, &vlen, &err);
  rocksdb_readoptions_destroy(rp);

  // check
  if (value == NULL) {
    RETURN_FALSE;
  }

  RETVAL_STRINGL(value, vlen, 1);
  free(value); // ?
}

PHP_METHOD(RocksDB, put) {
  char *key;
  char *value;
  int klen;
  int vlen;
  zval *write_options = NULL;
  char *err = NULL;
  rocksdb_writeoptions_t *wp;
  wrapped_rocksdb_db *intern;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|z", &key, &klen, &value, &vlen, &write_options) == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_db*)zend_object_store_get_object(getThis() TSRMLS_CC);
  //check

  wp = php_rocksdb_get_writeoptions(intern, write_options);
  rocksdb_put(intern->db, wp, key, klen, value, vlen, &err);
  rocksdb_writeoptions_destroy(wp);

  //check

  RETURN_TRUE;
}

PHP_METHOD(RocksDB, write) {
}

PHP_METHOD(RocksDB, delete) {
  char *key;
  int klen;
  zval *write_options = NULL;
  char *err;
  rocksdb_writeoptions_t *wp;
  wrapped_rocksdb_db *intern;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &key, &klen, &write_options) == FAILURE) {
    return;
  }

  intern = (wrapped_rocksdb_db*)zend_object_store_get_object(getThis() TSRMLS_CC);
  //check
  
  wp = php_rocksdb_get_writeoptions(intern, write_options);
  rocksdb_delete(intern->db, wp, key, klen, &err);
  rocksdb_writeoptions_destroy(wp);

  //check

  if (err != NULL) {
    RETURN_FALSE;
  }
  RETURN_TRUE;
}

static zend_function_entry db_methods[] = {
  PHP_ME(RocksDB, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
  PHP_ME(RocksDB, close, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDB, get, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDB, put, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDB, write, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(RocksDB, delete, NULL, ZEND_ACC_PUBLIC)
  PHP_FE_END 
};

void rocksdb_init_db(TSRMLS_D) {
  zend_class_entry ce;
  INIT_CLASS_ENTRY(ce, "RocksDB\\RocksDB", db_methods);
  ce.create_object = create_wrapped_rocksdb_db;
  rocksdb_ce_db = zend_register_internal_class(&ce TSRMLS_CC);
}

