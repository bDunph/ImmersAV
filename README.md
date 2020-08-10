ImmersAV by Bryan Dunphy

ImmersAV is an open source toolkit for immersive audiovisual composition using interactive machine learning techniques.

Aims:
- Provide well defined, independent areas for generating audio and visual material.
- Provide a class that can be used to generate, send and receive data from: 
    - machine learning algorithms
    - VR hardware sensors
    - audio and visual processing engines
- Allow for direct rendering on a VR headset.

Dependencies:
- OpenVR
- Csound6
- OpenGL4
- glm
- glfw3
- Glew
- CMake3
- RapidLib
- libsndfile

Windows:

Setup:

1. Create ImmersAV/bin/ directory.
2. Create ImmersAV/lib/ directory.
3. Create ImmersAV/include directory.
3. Download (64 bit):
    - CMake:        https://cmake.org/download/
    - OpenVR:       https://github.com/ValveSoftware/openvr
    - Csound 6:     https://csound.com/download.html
    - glm:          https://github.com/g-truc/glm/tags
    - glfw3:        https://www.glfw.org/download.html
    - glew:         http://glew.sourceforge.net/
    - libsndfile:   http://www.mega-nerd.com/libsndfile/#Download
4. Install CMake and Csound according to their instructions.
5. Move the following files to bin/:
    - csound64.dll
    - glew32.dll
    - openvr_api.dll
    - libsndfile-1.dll
6. Move the following files to lib/:
    - csound64.lib
    - openvr_api.lib
    - glew32.lib
    - glfw3.lib
    - libsndfile-1.lib
7. Move header files from OpenVR to include/.

Build and run using the Visual Studio command line:

1. Run the newCmakeBuild.bat script.
2. Use the following commands to build the project:
    - cd build/
    - nmake
3. Move the following files to ImmersAV/build/src/:
    From ImmersAV/bin/:
        - csound64.dll
        - openvr_api.dll
        - glew32.dll
        - libsndfile-1.dll
    From ImmersAV/src/Studio/:
        - *_example.csd
        - *_example.vert
        - *_example.frag
        - hrtf-48000-left.dat
        - hrtf-48000-right.dat
    From ImmersAV/src/VR/:
        - avr_iml_actions.json
        - avr_iml_default_bindings.json
4. Navigate (cd) to ImmersAV/build/src/.
5. Type the following command to run the application with one of the examples:
    - avr audio
  

