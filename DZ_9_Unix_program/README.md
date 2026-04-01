### Патч для устранения утечек памяти в проекте clib/test/package.  

1. Для проверки работоспособности патча запустить команду:
```sh
patch --dry-run -p1 -d clib/ < my_project.patch
```

Если всё успешно, то получится следующий выволд:
```sh
checking file deps/hash/hash.h
checking file src/common/clib-package.c
checking file test/package/Makefile
checking file test/package/package-dependency-new.c
checking file test/package/package-install-dependencies.c
checking file test/package/package-install-dev-dependencies.c
checking file test/package/package-install.c
checking file test/package/package-load-from-manifest.c
checking file test/package/package-load-manifest-file.c
checking file test/package/package-new-from-slug.c
checking file test/package/package-new.c
checking file test/package/package-parse-author.c
checking file test/package/package-parse-name.c
checking file test/package/package-parse-version.c
checking file test/package/package-url.c
checking file test/package/test_helper.h
checking file test/test_helper.h
```

2. Для установки патча выполнить команду:
```sh
patch -p1 -d clib/ < my_project.patch
```

Результаты изменений находятся в файле __Исходный_и_измененный_clib.txt__