## Запускалось в VSCode с параметрами:

```
"command": "/usr/bin/gcc-13",
"args": [
    "-fdiagnostics-color=always",
    "-g",
    "${file}",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-std=c11",
    "-lcurl",
    "-lcjson",
    "-o",
    "${fileDirname}/${fileBasenameNoExtension}"
]
```