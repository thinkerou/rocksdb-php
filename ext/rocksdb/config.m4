dnl $Id$
dnl config.m4 for extension rocksdb

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(rocksdb, for rocksdb support,
[  --with-rocksdb             Include rocksdb support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(rocksdb, whether to enable rocksdb support,
dnl [  --enable-rocksdb           Enable rocksdb support])

if test "$PHP_ROCKSDB" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-rocksdb -> check with-path
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="include/rocksdb/c.h"  # you most likely want to change this
  SEARCH_LIB="librocksdb"

  dnl search default path list
  AC_MSG_CHECKING([for rocksdb files in default path])
  for i in $PHP_ROCKSDB $SEARCH_PATH ; do
    if test -r $i/$SEARCH_FOR; then
      ROCKSDB_INCLUDE_DIR=$i
      AC_MSG_RESULT(found in $i)
    fi

    if test -r $i/$PHP_LIBDIR/$SEARCH_LIB.a || test -r $i/$PHP_LIBDIR/$SEARCH_LIB.$SHLIB_SUFFIX_NAME; then
      ROCKSDB_LIB_DIR=$i/$PHP_LIBDIR
      AC_MSG_RESULT(rocksdb lib found in $i/lib)
    fi

    dnl from rocksdb build dir
    if test -r $i/$SEARCH_LIB.a || test -r $i/$SEARCH_LIB.$SHLIB_SUFFIX_NAME; then
      ROCKSDB_LIB_DIR=$i
      AC_MSG_RESULT(rocksdb lib found in $i)
    fi

    if test -z "$ROCKSDB_LIB_DIR"; then
      for j in "lib/x86_64-linux-gnu" "lib/x86_64-linux-gnu"; do
        echo find "--$i/$j"
        if test -r $i/$j/$SEARCH_LIB.a || test -r $i/$j/$SEARCH_LIB.$SHLIB_SUFFIX_NAME; then
          ROCKSDB_LIB_DIR=$i/$j
          AC_MSG_RESULT(rocksdb lib found in $i/$j)
        fi
      done
    fi
  done

  if test -z "$ROCKSDB_INCLUDE_DIR" || test -z "$ROCKSDB_LIB_DIR"; then
     AC_MSG_RESULT([rocksdb not found])
     AC_MSG_ERROR([Please reinstall the rocksdb distribution])
  fi

  dnl # --with-rocksdb -> add include path
  PHP_ADD_INCLUDE($ROCKSDB_INCLUDE_DIR/include)

  dnl # --with-rocksdb -> check for lib and symbol presence
  LIBNAME=rocksdb

  PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $ROCKSDB_LIB_DIR, ROCKSDB_SHARED_LIBADD)

  PHP_SUBST(ROCKSDB_SHARED_LIBADD)

  PHP_NEW_EXTENSION(rocksdb, db.c writebatch.c iterator.c php_rocksdb.c, $ext_shared)
fi
