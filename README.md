# ImmersAV 
by Bryan Dunphy

ImmersAV is an open source toolkit for immersive audiovisual composition using interactive machine learning techniques.

## Aims:
- Provide well defined, independent areas for generating audio and visual material.
- Provide a class that can be used to generate, send and receive data from: 
    - machine learning algorithms
    - VR hardware sensors
    - audio and visual processing engines
- Allow for direct rendering on a VR headset.

## Dependencies:
- OpenVR
- Csound6
- OpenGL4
- glm
- glfw3
- Glew
- CMake3
- RapidLib
- libsndfile

## Windows:

### Setup:
1. Download (64 bit):
    - CMake:        https://cmake.org/download/
    - OpenVR:       https://github.com/ValveSoftware/openvr
    - Csound 6:     https://csound.com/download.html
    - glm:          https://github.com/g-truc/glm/tags
    - glfw3:        https://www.glfw.org/download.html
    - glew:         http://glew.sourceforge.net/
    - libsndfile:   http://www.mega-nerd.com/libsndfile/#Download
2. Install CMake and Csound according to their instructions.
3. Create directories:
    - ImmersAV/bin/
    - ImmersAV/lib/ 
    - ImmersAV/include/ 
4. Move the following files to bin/:
    - csound64.dll
    - glew32.dll
    - openvr_api.dll
    - libsndfile-1.dll
5. Move the following files to lib/:
    - csound64.lib
    - openvr_api.lib
    - glew32.lib
    - glfw3.lib
    - libsndfile-1.lib
6. Move header files from OpenVR to include/.

### Build and run using the Visual Studio command line:

1. Run the newCmakeBuild.bat script.
2. Use the following commands to build the project:
    - cd build/
    - nmake
3. Move the following files to ImmersAV/build/src/:
    - From ImmersAV/bin/:
        - csound64.dll
        - openvr_api.dll
        - glew32.dll
        - libsndfile-1.dll
    - From ImmersAV/examples/:
        - *_example.csd
        - *_example.vert
        - *_example.frag
    - From ImmersAV/data/: 
        - hrtf-48000-left.dat
        - hrtf-48000-right.dat
        - avr_iml_actions.json
        - avr_iml_default_bindings.json
4. Navigate (cd) to ImmersAV/build/src/.
5. Type the following command to run the application with one of the examples:
    - With VR rendering:
        - avr audioReactive_example
    - Without VR rendering (for development):
        - avr audioReactive_example -dev

## MacOS:

### Setup:
1. Download (64 bit):
    - CMake         https://cmake.org/download/
    - OpenVR:       https://github.com/ValveSoftware/openvr
    - Csound 6:     https://csound.com/download.html
    - glm:          https://github.com/g-truc/glm/tags
    - glfw3:        https://www.glfw.org/download.html
    - RAPID-MIX:    https://www.doc.gold.ac.uk/eavi/rapidmixapi.com/index.php/getting-started/download-and-setup/
2. Install CMake and Csound according to their instructions.
3. Move the following to /Library/Frameworks/:
    - CsoundLib64.framework 
    - OpenVR.framework
4. Move the following to /usr/local/lib/:
    - libcsnd.6.0.dylib
    - libcsnd6.6.0.dylib
    - libopenvr_api.dylib
    - libRAPID-MIX_API.dylib
5. Create ImmersAV/include/ directory.
6. Move the following to ImmersAV/include/:
    - From openvr/:
        - headers/*
    - From RAPID-MIX_API/src:
        - rapidmix.h
        - machineLearning/*
        - signalProcessing/*
        
### Build and Run using the terminal:

1. Type the following commands into the terminal:
    - mkdir build/
    - cd build
    - cmake ..
2. When CMake has prepared the build files type 'make'.
3. Move the following files: 
    - From ImmersAV/examples/:
        - *_example.csd
        - *_example.vert
        - *_example.frag
    - From ImmersAV/data/:
        - hrtf-48000-left.dat
        - hrtf-48000-right.dat
4. Navigate (cd) to ImmersAV/build/src/.
5. To run an example project type the following into the terminal:
    - With VR rendering:
        - ./avr audioReactive_example
    - Without VR (for development):
        - ./avr audioReactive_example -dev
    
## Example Walkthroughs
This section describes the construction and execution of the examples. All of the examples are found in *ImmersiveAV/examples/*.

### Audio Reactive
This example demonstrates audio reactive mapping. The frequency is mapped from the audio engine to the visual structure.

***Link to video of example on YouTube***

The file *audioReactive_example.csd* generates the audio tone. Below is the code for the *Example Instrument*.

```
**************************************************
instr 1 ; Example Instrument
**************************************************

kFreq   linseg  400, 2.0, 500, 2.0, 450, 4.0, 950, 5.0, 200

        chnset  kFreq, "freqOut"

aSig    oscil   0.7, kFreq
gaOut = aSig

endin
```
Here `chnset` is used to send the value `kFreq` to the `Studio()` class using the channel `"freqOut"`. The channel `"freqOut"` is declared in `Studio::Setup()`.

```
//setup returns from Csound
std::vector<const char*> returnNames;
returnNames.push_back("freqOut");

m_returnVals.push_back(m_pFreqOut);

m_pStTools->BCsoundReturn(csSession, returnNames, m_vReturnVals);
```
Here the string `"freqOut"` is pushed back into the vector `returnNames`. The Csound pointer `m_pFreqOut` points to the value returned from Csound. This is pushed back onto a member vector `returnVals`. These vectors are then used as arguments to the function `BCsoundReturn()` which sets up the channel. 

The frequency value is then sent to the frag shader using `glUniform1f()` which is called in `Studio::Draw()`.

```
m_pStTools->DrawStart(projMat, eyeMat, viewMat, shaderProg, translateVec);
glUniform1f(m_gliFreqOutLoc, *m_vReturnVals[0]);
m_pStTools->DrawEnd();
```
The value is sent to the shader by dereferencing the specific element in the vector `m_vReturnVals`. For the purposes of this walkthrough it is element 0 but in real use it depends in which order you push back elements in `Studio::Setup()`. The location of the uniform is given by the handle `m_gliFreqOutLoc`. This is declared in `Studio::Setup()`.

```
//shader uniforms
m_gliFreqOutLoc = glGetUniformLocation(shaderProg, "freqOut");
```

In the frag shader, the uniform `freqOut` is declared at the top. The value is then used to manupulate the size of the sphere in the distance estimator function.

```
float DE(vec3 p)
{
    float rad = 1.0 * (1.0 / (freqOut * 0.01));
    float sphereDist = sphereSDF(p, rad);
    return sphereDist;
}
```

Here there is an inverse relationship between the value of `freqOut` and the radius of the sphere `rad`. Higher frequency values make the sphere smaller, lower frequency values make the sphere larger.


  

