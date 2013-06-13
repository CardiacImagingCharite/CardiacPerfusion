KardioPerfusion
===============

Linux
-----

This code has been tested with
* LinuxMint 14
* gcc 4.7.2
* Qt 4.8.4
* qwt 5.1.2
* boost 1.48.0
* zlib 1.2.5
* VTK 5.8
* ITK 4.2


### Installation

#### Qt

You may have to install libxrender-dev first

    sudo apt-get install libxrender-dev
    sudo apt-get install libxext-dev

Download and extract the Qt 4.7.4 source and change to this directory
    
    ./configure
    make
    sudo make install

In .profile (if your shell is bash, ksh, zsh or sh), add the following
lines:

    PATH=/usr/local/Trolltech/Qt-4.7.4/bin:$PATH
    export PATH

with
     
    pico ~/.profile


#### qwt

Download and extract qwt 5.1.2 source and change to this directory

    qmake
    make
    sudo make install


#### boost

You may have to install `libbz2-dev`, `python-dev` and
`python-bzutils` first

    sudo apt-get install python-dev
    sudo apt-get install python-bzutils
    sudo apt-get install libbz2-dev

Download and extract boost 1.48.0 source and change to this directory.
Apply the following patch:
http://pastebin.com/gYBAr2Td
Do:

    ./bootstrap.sh
    ./b2
    sudo ./b2 install


#### zlib

Download and extract zlib 1.2.5 source. 
Create a build directory

    mkdir binZlib
    cd binZlib

You have do delete `zconf.h`, which will be created by cmake:

    rm ../zlib-1.2.5/zconf.h

Run CMake configuration tool

    ccmake ../zlib-1.2.5

Press "c" to configure and "g" for generate the Makefiles.

    make 
    sudo make install

    
#### VTK

You may have to install `libosmesa6-dev`, `libgl1-mesa-dev` and `libst-dev`
    
    sudo apt-get install libosmesa6-dev
    sudo apt-get install libgl1-mesa-dev
    sudo apt-get install libxt-dev 

Download and extract vtk 5.8. source.
Create a build directory

    mkdir binVTK
    cd binVTK
   
Run CMake configuration tool

    ccmake ../VTK

configure ("c") and change to following options:

    VTK_USE_QT                       ON
    VTK_USE_QVTK_QTOPENGL            OFF

configure ("c") and generate ("g") Makefiles.
  
    make
    sudo make install


#### ITK

Download and extract InsightToolkit 4.2.0 source.
Create a build directory

    mkdir binITK
    cd binITK

Run CMake configuration tool

    ccmake ../InsightToolkit-4.2.0
  
configure ("c") and change the following options:

    BUILD_TESTING                    OFF
    ITKV3_COMPATIBILITY              ON

configure ("c") and generate ("g") Makefiles.

    make
    sudo make install


#### ITK Applications

Download and extract InsightApplications 4.2.0 source. A build is not
necessary.


#### KardioPerfusion

Create a build directory

    cd ..
    mkdir binKardioPerfufion
    cd binKardioPerfusion

Run Cmake configuration tool

    ccmake ../KardioPerfusion

configure ("c") and check if the paths for `Boost-INCLUDE_DIR`,
`ITK_APPS`, `ITK_DIR`, `QT_QMAKE_EXECUTALBE`, `QWT_INCLUDE_DIR`,
`QWT_LIBRARY_DIR` and `VTK_DIR` are set correctly. `Boost_DIR` doesn't
have to be set.  Configure ("c") and generate ("g") Makefiles.

    make


Windows
-------

This code has been tested with
* Windows 7
* Visual Studio 10
* Qt 4.7.4
* qwt 5.1.2
* boost 1.48.0
* zlib 1.2.5
* VTK 5.8
* ITK 4.2


### Installation

#### Qt

Download and extract the Qt 4.7.4 source. Open visual studio command
prompt(on start, vs2010, menu Tools->VisaulStudio command prompt).  Go
to the path where Qt has been extracted, run:

    configure -platform win32-msvc2010       

When it ask you opensource model, answer `o`, next license answer
`yes`. After finished config,run

    nmake

If you've already installed Qt-vs-addin, there will a new nemu added
into the vs2010. You need config "qt options" in this menu, and set
the path of Qt.


#### qwt

Download and extract qwt 5.1.2 source. Open the file
`qwt\src\qwt_valuelist.h` and comment this row:

    template class QWT_EXPORT QList<double>;

Open the file `qwt\qwtconfig.pri` and change the following lines to

    #CONFIG           += debug_and_release     # release/debug/debug_and_release
    CONFIG           += build_all
    CONFIG           += debug_and_release     # release/debug
    DEBUG_SUFFIX      = 
    #CONFIG          +=QwtDll
    #CONFIG     += QwtDesigner
   
Open `qwt.pro` with VS (as Qt project) and build (F5).


#### boost

Download and extract boost 1.48.0 AND zlib 1.2.5 source. Make sure,
zconf.h is still the in zlib source directory.  Go to the boost source
directory and execute

    bootstrap.bat
    bjam.exe
    bjam.exe --toolset=msvc-10.0 debug release threading=multi link=static -sNO_COMPRESSION=0 -sNO_ZLIB=0 -sZLIB_SOURCE="C:\zlib125" stage

Your path for `sZLIB_SOURCE` may vary.


#### zlib

If not done yet, download and extract zlib 1.2.5 source. Now you
have to delete `zconf.h`. Create the project with CMake and build
with VS.

#### VTK

Download and extract VTK 5.8 source. Run CMake, change the following
options:

    VTK_USE_QT                   ON
    VTK_USE_QVTK_QTOPENGL        OFF

and create the project. Build with VS.


#### ITK

Download and extract InsightToolkit 4.2.0 source.
Run CMake, change the following option:

    ITKV3_COMPATIBILITY         ON

and create the project. Build with VS.


#### ITK Applications

Download and extract InsightApplications 4.2.0. A build is not necessary.


#### KardioPerfusion

Run CMake and check if the paths for `Boost-INCLUDE_DIR`, `ITK_APPS`,
`ITK_DIR`, `QT_QMAKE_EXECUTALBE`, `QWT_INCLUDE_DIR`, `QWT_LIBRARY_DIR`
and `VTK_DIR` are set correctly. `Boost_DIR` doesn't have to be set.
Create the project. Build with VS.