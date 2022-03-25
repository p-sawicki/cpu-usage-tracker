## CPU usage tracker

CLI app to track CPU usage every second.

### Requirements
- Linux
- CMake 3.10+
- gcc or clang supporting C99

### Usage
- Download
```
git clone https://github.com/p-sawicki/cpu-usage-tracker
```
- Configure
```
cd cpu-usage-tracker
cmake -B build
```

- Build
```
cmake --build build
```

- Run
```
./build/bin/cut
```

### Details
Usage is computed based on data from `/proc/stat` and then printed to `stdout`. Errors and debug information is saved to `/tmp/cut_log.txt`