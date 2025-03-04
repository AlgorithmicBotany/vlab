# vlab
## The virtual laboratory

## Developer Getting Started Guide

Note: our current version has been developed using QT 5.15.16.

### OSX

You will need to have Xcode Command Line Tools and Qt5 installed.
If you do not have Xcode, get it via Terminal with the command:
```
xcode-select -install 
```
This should install the command line tools only and not the IDE.
Then install Qt: https://doc.qt.io/qt-5/gettingstarted.html
Because Qt 5.15 support is ending on 26 May, 2025, it is recommended to install Qt via
Homebrew (https://formulae.brew.sh/formula/qt@5) or Macports (https://ports.macports.org/port/qt5/).

To compile vlab, go to the source directory and run: 
```
./configure.sh -A {arm64 | x86 | x86_64} -M {macOS version}
./compile_all.sh
```

After compiling, to make an application bundle, change to the ./Distribution directory and run:
```
./vlab-macdeployqt.sh
```
The script will create a vlab-5******.dmg file.

### Ubuntu

The supported version of Ubuntu is listed in `install_ubuntu_reqs.sh`, though
other versions may also work. This may require first running: `sudo add-apt-repository universe `

```
sudo ./install_ubuntu_reqs.sh
./configure.sh
./compile_all.sh
```

### Fedora

A legacy version of Fedora is listed in `install_fedora_reqs.sh`.
Newer versions were not tested.

Configure script auto-detection seems spotty on Fedora, so you may need to be
explicit about being 64-bit, using the -A flag.

```
sudo ./install_fedora_reqs.sh
./configure.sh -A 64
./compile_all.sh
```

### Windows, using Windows Subsystem for Linux (WSL)

On Windows 10 (build 19041 and higher) or Windows 11, you will need to install WSL 2 and a Linux distribution.
You can install both at the same time using the Windows Command Prompt (in administator mode):
```
wsl --install
```
giving you Ubuntu as the default distribution. Then follow the instructions for compiling vlab for Ubuntu given above. 

## Version Numbering

The build number and date are stored in two files:

 1. `libs/misc/version.cpp`: there are functions `version_major`,
   `version_minor`, `version_minor_minor`, `build_number`, and
   `build_date_string` which return the current values.
 2. `Lstudio/Lstudio.rc`: the `VS_VERSION_INFO` block has the version/build
   number four times: under keys `FILEVERSION` and `PRODUCTVERSION`, then
   again a few lines later as the argument to `VALUE "FileVersion"` and
   `VALUE "ProductVersion"`.

## Contact

The home page for vlab is:
http://www.algorithmicbotany.org/virtual_laboratory/

Email: vlab.algorithmicbotany@gmail.com
