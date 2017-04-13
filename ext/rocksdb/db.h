#ifndef ROCKSDB_DB_H_
#define ROCKSDB_DB_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_rocksdb.h"

#include "rocksdb/c.h"

extern zend_class_entry *rocksdb_ce_db;

typedef struct wrapped_rocksdb_db {
  zend_object std;
  rocksdb_t *db;
  unsigned char verify_check_sum;
  unsigned char fill_cache;
  unsigned char sync;
  rocksdb_comparator_t *comparator;
  char *callable;
} wrapped_rocksdb_db;

void rocksdb_init_db(TSRMLS_D);

#endif // ROCKSDB_DB_H_

