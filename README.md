## Getting Started
WebCamera filter implementation for Zebra Aurora Vision Studio

This filter adds missing WebCamera filter for linux.
The implementation is based on v4l2 Library (linux/videodev2) also uses modified [libv4l2cpp](https://github.com/mpromonet/libv4l2cpp) wrapper.

### Prerequisites
Compiling the filter requires Linux based operating system with GNU gcc compiler installed and CMake. 
Having Aurora™ Vision Studio [Runtime](https://www.adaptive-vision.com/en/user_area/download/) downloaded is also mandatory for header files.

CMake installation on Ubuntu
```bash
sudo apt update && sudo apt install -y build-essential cmake
```

### Usage
1. Clone this repository
   ```sh
   git clone https://github.com/MK6554/av_userfilter_v4l2_webcamera.git
   ```
2. Edit [CMakeLists.txt](CMakeLists.txt) file and change Zebra Aurora™ Vision Studio Runtime path
   ```sh
   #Specify the source directory
   set(AVEXECUTOR_DIR "/etc/avexecutor/")
   ```
3. Open the console in directory, and run
   ```sh
   sudo chmod +x build.sh
   ./build.sh
   ```
4. Change camera resolution in AVS project file [/avs/uf_linux.avcode](/avs/uf_linux.avcode)
   ```sh
   inWidth: 2320
   inHeight: 1744
   ```
5. After building, the filter should be ready to use, you can connect the web camera and test it.
   ```sh
   /path/to/avexecutor/bin/Console ./avs/uf_linux.avproj
   ```
6. Open a web browser and visit hosted WebHMI You should see live feed from the connected camera.
   ```sh
   localhost:9090/
   ```


Project can use full Aurora™ Vision Library insted of Lite Version from the Aurora™ Vision Studio Runtime includes, to enable full library please check this [CMakeFile](https://github.com/ErykDevZebra/EmptyAVLProject/blob/main/build/CMakeLists.txt)

Creating custom [User filters for Aurora™ Vision Runtime](https://docs.adaptive-vision.com/current/studio/extensibility/CreatingUserFilters.html#struct)

More about Linux user filter [setup](https://docs.adaptive-vision.com/5.5/avl/getting_started/UserFilterOnLinux.html)
