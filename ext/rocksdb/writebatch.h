#ifndef ROCKSDB_WRITEBATCH_H_
#define ROCKSDB_WRITEBATCH_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_rocksdb.h"

#include "rocksdb/c.h"

extern zend_class_entry *rocksdb_ce_writebatch;

typedef struct wrapped_rocksdb_writebatch {
  zend_object std;
  rocksdb_writebatch_t *writebatch;
} wrapped_rocksdb_writebatch;

void rocksdb_init_writebatch(TSRMLS_D);

#endif // ROCKSDB_WRITEBATCH_H_

