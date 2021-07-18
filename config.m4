PHP_ARG_ENABLE(apm, whether to enable apm support,
dnl Make sure that the comment is aligned:
[  --enable-apm           Enable apm support])

if test "$PHP_APM" != "no"; then
  AC_DEFINE(HAVE_APM, 1, [Whether you have apm])
  PHP_NEW_EXTENSION(apm, apm.c, $ext_shared)
fi
