
<!-- GETTING STARTED -->
## Getting Started
This repository contains ready to use project template for compiling a User Filer for Aurora Vision Studio.
To get a local copy up and running follow these simple steps.

### Prerequisites
Compiling the filter requires Ubuntu operating system with GNU gcc compiler installed. 
Having Aurora™ Vision Studio [Runtime](https://www.adaptive-vision.com/en/user_area/download/) downloaded is also mandatory for header files.

### Usage
1. Clone this repository
   ```sh
   git clone https://github.com/ErykDevZebra/ExampleUserFilterLinux.git
   ```
2. Go to CMakeLists.txt file
   ```sh
   gedit build/CMakeLists.txt
   ```
4. Edit Aurora™ Vision Studio Runtime path
   ```sh
   #Specify the source directory
   set(AVEXECUTOR_DIR "/home/mk6554/Downloads/Installers/5.5/avexecutor/")
   ```
5. Open console at build directory, and run
   ```sh
   cmake ./
   make
   ```

Project can use full Aurora™ Vision Library insted of Lite Version from the Aurora™ Vision Studio Runtime includes, to enable full library please check this [CMakeFile](https://github.com/ErykDevZebra/EmptyAVLProject/blob/main/build/CMakeLists.txt)

Creating custom [User filters for Aurora™ Vision Runtime](https://docs.adaptive-vision.com/current/studio/extensibility/CreatingUserFilters.html#struct)

More about Linux user filter [setup](https://docs.adaptive-vision.com/5.5/avl/getting_started/UserFilterOnLinux.html)
