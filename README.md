# ImmersAV 
by Bryan Dunphy

ImmersAV is an open source toolkit for immersive audiovisual composition. It was built around a focused approach to composition based on generative audio, raymarching and interactive machine learning techniques.

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
glUniform1f(m_gliFreqOutLoc, *m_vReturnVals[1]);
m_pStTools->DrawEnd();
```
The value is sent to the shader by dereferencing the specific element in the vector `m_vReturnVals`. For the purposes of this walkthrough it is element 1 but in real use it depends in which order you push back elements in `Studio::Setup()`. The location of the uniform is given by the handle `m_gliFreqOutLoc`. This is declared in `Studio::Setup()`.

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

## Simultaneous Data Mapping
This example utilises the `Studio()` class to generate a stream of data. This data is then sent to the audio engine and fragment shader to simultaneously affect the audio and visuals.

***Link to video of example on YouTube***

The data signal is initially generated in `Studio::Update()`.

```
//example control signal - sine function
//sent to shader and csound
m_fSineControlVal = sin(glfwGetTime() * 0.33f);
*m_vSendVals[0] = (MYFLT)m_fSineControlVal;
```

The sine control value is generated using `sin()`. Then it is cast to type `MYFLT` and assigned to element 0 of the vector `m_vSendVals`. This element was assigned to the sine control signal in `Studio::Setup()`.

```
//setup sends to csound
std::vector<const char*> sendNames;
sendNames.push_back("sineControlVal");
m_vSendVals.push_back(m_cspSineControlVal);
m_pStTools->BCsoundSend(csSession, sendNames, m_vSendVals);
```

Here element 0 of `sendNames` is assigned to `"sineControlVal"`. The next line then assignes the Csound pointer `m_cspSineControlVal` to element 0 of `m_vSendVals`. The elements in these two vectors need to be aligned or else the values will get paired with incorrectly names channels when they are received by Csound. Both vectors are then used as arguments for the function `BCsoundSend()`. This sets up the send channel with the instance of Csound. The file *simultaneousDataMapping.csd* receives the data stream through the channel named `"sineControlVal"`.

```
********************************************************
instr 1 ; Example Instrument
********************************************************
kSineControlVal     chnget      "sineControlVal"

aSig        oscil   0.7, 400.0 * (1 / kSineControlVal)
gaOut       aSig

endin
```

Here the opcode `chnget` is used to assigne the control data to `kSineControlVal`. This is then used to alter the frequency value. The sine data is also sent from `Studio::Draw()` to the frag shader.

```
m_pStTools->DrawStart(projMat, eyeMat, viewMat, shaderProg, translateVec);
glUniform1f(m_gliSineControlValLoc, m_fSineControlVal);
m_pStTools->DrawEnd();
```

Here the float value is sent as a uniform using the `m_gliSineControlValLoc` handle that was set up in `Studio::Setup()`.

```
//shader uniforms
m_gliSineControlValLoc = glGetUniformLocation(shaderProg, "sineControlVal");
```
  
The name of the shader is specified as `"sineControlVal"` within the shader.

```
uniform float sineControlVal;

float DE(vec3 p)
{
    p.y *= 1.0 * sineControlVal;
    float rad = 2.0;
    float sphereDist = sphereSDF(p, rad);
    return sphereDist;
}
```

Here `sineControlVal` is used to increase and decrease `p.y` which is the height of the sphere. The point `p` is then send to `sphereSDF()` which estimates the distance to the spere.

## Cyclical Mapping Example
This example demonstrates a method of mapping data in a cyclical way between the audio and visuals using a neural network based regression algorithm.

***link to video of example on YouTube***

The frequency of the audio signal is set up to be randomised. This allows for a quick and easy way to match the audio to a particular visual state. The random parameter is defined in `Studio::Update()` using the `MLAudioParameter` struct.

```
//run machine learning
MLAudioParameter paramData;
paramData.distributionLow = 50.0f;
paramData.distributionHigh = 200.0f;
paramData.sendVecPosition = 1;
std::vector<MLAudioParameter> paramVec;
paramVec.push_back(paramData);
MLRegressionUpdate(machineLearning, pboInfo, paramVec);
```

The value will always be between `50.0f` and `200.0f`. It is assigned to element number 1 in the vector `m_vSendVals`. This is declared in `Studio::Setup()` where the channel is named `"randomVal"`. This allows Csound to receive the random value on this channel. The object `paramData` is then pushed back onto the vector `paramVec` and passed to `Studio::MLRegressionUpdate()`. When the space bar is pressed, this random value is calculated and sent to Csound.

```
for(int i = 0; i < params.size(); i++)
{
    std::uniform_real_distribution<float> distribution(params[i].distributionLow, params[i].distributionHigh);
    std::default_random_engine generator(rd());
    float val = distribution(generator);
    *m_vSendVals[params[i].sendVecPosition] = (MYFLT)val;
}
```

This `for` loop iterates through the vector `std::vector<MLAudioParameter> params`. For each parameter it calculates the `float val` within the given range. It then casts the value to `MYFLT` and assigns it to the relevant position in the vector `m_vSendVals`. This assigns the value to the previously declared channel which is received in the file `cyclicalMapping_example.csd`.

```
******************************************************
instr 1 ; Example Instrument
******************************************************
kSineControlVal chnget  "sineControlVal"
kRandomParam    chnget  "randomVal"

aSig    oscil   0.7 * kSineControlVal, kRandomParam
gaOut = aSig

kRms    rms     gaOut
        chnset  kRms, "rmsOut"

endin
```

Here the value recieved on the `"randomVal"` channel is stored in `kRandomParam`. This is then used as the frequency value for `oscil`. Here you can see the value received on the `"sineControlVal"` channel is used to make the volume of the tone increase and decrease. 
