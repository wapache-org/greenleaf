
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

## libssh

下载libssh的源码, 编译安装:

安装动态库:

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${GREENLEAF_PROJECT_DIR}/libraries/libssh -DCMAKE_BUILD_TYPE=Debug ..
make install

```

安装静态库:
```
rm -rf * # build dir

vi ../DefineOptions.cmake
# set BUILD_SHARED_LIBS to off
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

cmake -DCMAKE_INSTALL_PREFIX=${GREENLEAF_PROJECT_DIR}/libraries/libssh -DCMAKE_BUILD_TYPE=Debug ..
make install


```

另外还需要复制build目录下的config.h, 一些例子需要用到

```
cp config.h ${GREENLEAF_PROJECT_DIR}/libraries/libssh/include/libssh
```


## libyaml


安装静态库:
```
git clone https://github.com/yaml/libyaml.git
cd libyaml ; mkdir build ; cd build
cmake -DCMAKE_INSTALL_PREFIX=${GREENLEAF_PROJECT_DIR}/libraries/libyaml -DCMAKE_BUILD_TYPE=Debug ..
make -j8
make install

```




