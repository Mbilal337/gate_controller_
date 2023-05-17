# _2xGateController-s_




## initial Folder Contains

The project **2xGateController-aws** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── components
│   ├── pairing
│   |   ├── CMakeLists.txt
│   |   ├── common.h
│   |   ├── pairing.c
│   |   ├── pairing.h
│   |   ├── pairing_wifi.c
│   |   ├── pairing_wifi.h
│   |   ├── pairing_ble.c
│   |   └── pairing_ble.h
│   ├── WiFi
│   |   ├── CMakeLists.txt
│   |   ├── common.h
│   |   ├── pairing.c
│   |   ├── pairing.h
│   |   ├── pairing_wifi.c
│   |   ├── pairing_wifi.h
│   |   ├── pairing_ble.c
│   |   └── pairing_ble.h
│
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.
