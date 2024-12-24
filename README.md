## Getting Started
WebCamera filter implementation for Zebra Aurora Vision Studio

This filter adds mising WebCamera filter for linux.
The implementation is based on V4L Library (linux/videodev2)
also v4l is mandatory, but more than likely is already on on your linux machine.


### Prerequisites
Compiling the filter requires Linux based operating system with GNU gcc compiler installed and CMake. 
Having Aurora™ Vision Studio [Runtime](https://www.adaptive-vision.com/en/user_area/download/) downloaded is also mandatory for header files.

### Usage
1. Clone this repository
   ```sh
   git clone https://github.com/MK6554/TestUserFilter.git
   ```
2. Go to CMakeLists.txt file
   ```sh
   gedit CMakeLists.txt
   ```
4. Edit Zebra Aurora™ Vision Studio Runtime path
   ```sh
   #Specify the source directory
   set(AVL_LITE_DIR  "/etc/avexecutor/")
   ```
5. Open the console in directory, and run
   ```sh
   sudo chmod +x build.sh
   ./build.sh
   ```
6. After building, the filter should be ready to use, you can connect the web camera and test it.
   ```sh
   /path/to/avexecutor/bin/Console this/project/path/avs/uf_linux.avproj
   ```

Project can use full Aurora™ Vision Library insted of Lite Version from the Aurora™ Vision Studio Runtime includes, to enable full library please check this [CMakeFile](https://github.com/ErykDevZebra/EmptyAVLProject/blob/main/build/CMakeLists.txt)

Creating custom [User filters for Aurora™ Vision Runtime](https://docs.adaptive-vision.com/current/studio/extensibility/CreatingUserFilters.html#struct)

More about Linux user filter [setup](https://docs.adaptive-vision.com/5.5/avl/getting_started/UserFilterOnLinux.html)
