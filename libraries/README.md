
# 依赖的库

## libpq

下载postgresql的源码, 编译安装:

```
./configure --prefix ${GREENLEAF_PROJECT_DIR}/libraries
cd src/interfaces/libpq; make; make install; cd -
cd src/bin/pg_config; make install; cd -
cd src/backend; make generated-headers; cd -
cd src/include; make install; cd -

cd src/port; make install; cd -
cd src/common; make install; cd -

```

## quickjs

下载quickjs的源码, 编译安装:

注意: 因为`libpq`也有`unicode_to_utf8`这个函数, 导致冲突, 需要修改quickjs的源码, 将这个函数重命名为`qjs_unicode_to_utf8`.

```
./configure --prefix ${GREENLEAF_PROJECT_DIR}/libraries/quickjs
make
make install
make clean
```
