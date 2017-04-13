#ifndef ROCKSDB_ITERATOR_H_
#define ROCKSDB_ITERATOR_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_rocksdb.h"

#include "rocksdb/c.h"

extern zend_class_entry *rocksdb_ce_iterator;

typedef struct wrapped_rocksdb_iterator {
  zend_object std;
  rocksdb_iterator_t *iterator;
  zval *db;
} wrapped_rocksdb_iterator;

void rocksdb_init_iterator(TSRMLS_D);

#endif // ROCKSDB_ITERATOR_H_

