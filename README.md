# smart-fridge

## Smart Fridge

### Dependencies

The following packages need to be installed if you want to use the Camera dependency.

On debian based:

- sudo apt-get -y install build-essential cmake pkg-config
+ sudo apt-get -y install libjpeg-dev libtiff5-dev libjasper-dev libpng12-dev
- sudo apt-get -y install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
- sudo apt-get -y install libxvidcore-dev libx264-dev
~ sudo apt-get -y install libgtk2.0-dev
- sudo apt-get -y install libatlas-base-dev gfortran
~ sudo apt-get -y install python2.7-dev python-numpy
- sudo apt-get -y install libmagickwand-dev
~ sudo apt-get -y install xorg-dev libqt4-dev python-gtk2-dev

### Compile

Makefile options:

- SQLITE -> RUN SQLITE INTERFACE
- DEBUG=YES -> COMPILE WITH DEBUG FLAGS
- TEST=YES 	-> COMPILE WITH TESTS BINARIES
- PARALLEL_DEPS=<THREADS> -> COMPILES DEPENDENCIES IN PARALLEL
- ENABLE_LDAP=YES -> ENABLE OPEN LDAP INTEGRATION (TBD)
- DB -> LAUNCHES THE DATABASE INTERFACEs
- ENABLE_CAM=YES -> COMPILE WITH CAMERA SUPPORT

Example of use:

    make TEST=YES DEBUG=YES PARALLEL_DEPS=4 ENABLE_CAM=YES PARALLEL_DEPS=8

### Execute

    LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libv4l/v4l1compat.so ./bin/sf_reader -ddf
    ./bin/sf_db -ddf
