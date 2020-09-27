# ImmersAV 
by Bryan Dunphy

ImmersAV is an open source toolkit for immersive audiovisual composition. It was built around a focused approach to composition based on generative audio, raymarching and interactive machine learning techniques. Here are two examples of work created with the toolkit:

[obj_#3](https://youtu.be/RdvezMCTt-I)

[Ag Fás Ar Ais Arís](https://youtu.be/LewHeC5e1fM)

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
        - immersAV_iml_actions.json
        - immersAV_iml_default_bindings.json
4. Navigate (cd) to ImmersAV/build/src/.
5. Type the following command to run the application with one of the examples:
    - With VR rendering:
        - immersAV audioReactive_example
    - Without VR rendering (for development):
        - immersAV audioReactive_example -dev

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
    - With VR rendering (NB. virtual reality rendering has not been tested on macOS and will almost certainly not work at the moment.):
        - ./immersAV audioReactive_example
    - Without VR (for development):
        - ./immersAV audioReactive_example -dev
    
## Workflow Overview
The ImmersAV toolkit is intended to provide three main contexts for creating and mapping material.

	- Csound `csd` file.
	- GLSL fragment shader.
	- `Studio()` class.

All the audio is generated in the `csd` file. All the visuals (with the exception of default virtual controllers) are generated in the fragment shader. The `Studio()` conceptually sits between the audio and visual areas. Any data passes through here and is mapped to the audio and visuals. Data can also be generated here.

## Controller Sensor Data 
When in VR, the controller position and orientation can be accessed through `Studio::Update()`. The vectors `glm::vec3 controllerWorldPos_0` and `glm::vec3 controllerWorldPos_1` contain the `x`, `y` and `z` cartesian coordinates of the controller. The quaternions `glm::quat controllerQuat_0` and `glm::quat controllerQuat_1` contain the orientations of the controllers.

## Audio Engine

### Audio Thread and CSD Setup

The Csound API is used to communicate between the main application and the Csound instance. For an introduction to the Csound API see http://floss.booktype.pro/csound/a-the-csound-api/. Here, the Csound API is wrapped in the `StudioTools()` class, which contains functions that create the Csound instance and set up the sound sources used in the scene.   

```
m_pStTools = new StudioTools();

//audio setup
CsoundSession* csSession = m_pStTools->PCsoundSetup(csd);
	
if(!m_pStTools->BSoundSourceSetup(csSession, NUM_SOUND_SOURCES))
{
	std::cout << "Studio::setup sound sources not set up" << std::endl;
	return false;
}
```

The pointer `m_pStTools` is used to access functions from the `StudioTools()` class. The function `PCsoundSetup()` initialises the Csound instance and creates a new thread for it to run on. This function returns a pointer to Csound, `csSession`. 

### Virtual Sound Source Placement 

The function `BSoundSourcesSetup()` passes the `csSession` pointer and the number of required sound sources to be placed in the virtual scene. These sound sources can then be placed and moved around in the `Studio::Update()` function.

```
// example sound source at origin
StudioTools::SoundSourceData soundSource1;
glm::vec4 sourcePosWorldSpace = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
soundSource1.position = sourcePosWorldSpace;
std::vector<StudioTools::SoundSourceData> soundSources;
soundSources.push_back(soundSource1);

m_pStTools->SoundSourceUpdate(soundSources, viewMat);
```

The struct `StudioTools::SoundSourceData` contains all the data needed to place the sound source in the space. The world space coordinates of the sound source are assigned to `soundSource1.position`. Here it is simply placed at the origin. The sound source is then added to a vector, `soundSources`, that contains all the sound sources. Here there is only one. The sound source can be moved and rotated through the space by updating `soundSource1.position`. The function `SoundSourceUpdate()` sends the position, elevation and azimuth values to the `csd` file. 

```
;**************************************************************************************
instr 2 ; HRTF Instrument
;**************************************************************************************
kPortTime linseg 0.0, 0.001, 0.05 

iNumAudioSources init 1

kAzimuths[] 	init 	iNumAudioSources
kElevations[] 	init	iNumAudioSources
kDistances[]	init	iNumAudioSources

kCount = 0

channelLoop:

	S_azimuth sprintfk "azimuth%d", kCount
	kAzimuths[kCount] 	chnget S_azimuth

	S_elevation sprintfk "elevation%d", kCount 
	kElevations[kCount] 	chnget S_elevation 

	S_distance sprintfk "distance%d", kCount
	kDistances[kCount]	chnget S_distance 

	loop_lt	kCount, 1, iNumAudioSources, channelLoop
	
aInstSigs[]	init	iNumAudioSources
aInstSigs[0] = gaOut

aLeftSigs[]	init	iNumAudioSources
aRightSigs[]	init	iNumAudioSources
kDistVals[]	init	iNumAudioSources

kDistVals[0] portk kDistances[0], kPortTime	;to filter out audio artifacts due to the distance changing too quickly
	
aLeftSigs[0], aRightSigs[0]  hrtfmove2	aInstSigs[0], kAzimuths[0], kElevations[0], "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
aLeftSigs[0] = aLeftSigs[0] / (kDistVals[0] + 0.00001)
aRightSigs[0] = aRightSigs[0] / (kDistVals[0] + 0.00001)

aL init 0
aR init 0

aL = aLeftSigs[0]
aR = aRightSigs[0]

outs	aL,	aR
endin

``` 

The `HRTF Instrument` receives position, elevation and azimuth data from `Studio::Update()` every frame. The number of audio sources is initialised at the start of the instrument and assigned to the variable `iNumAudioSources`. This number is used to determine the size of the arrays `kAzimuths[]`, `kElevations[]` and `kDistances[]`. These arrays are then populated in `channelLoop` using `chnget` to retrieve the data from named channels. The audio signals are then stored in the array `aInstSigs[]`. Here there is just one signal `gaOut`. Arrays for the left and right audio channels and distance values are then initialised. The distance values need to be smoothed out using `portk` as rapidly changing values will introduce discontinuities into the audio signal. The opcode `hrtfmove2` is used to apply HRTF filters to the audio source. It receives the audio signal, azimuth and elevation values as input and uses the data files `"hrtf-48000-left.dat"` and `"hrtf-48000-right.dat"` to generate dynamic 3D binaural audio signals. The sample rate is specified as 48kHz so the overall Csound sample rate, `sr`, has to match this. The left and right signals are then divided by the distance values contained in `kDistVals[]`. The stereo signal is then output to the main output channel. This instrument needs to run continuously. This is achieved in the score section like so:

```
<CsScore>

f0	86400 ;keep csound running for a day

i1 1 -1 

i2 1 -1

e
</CsScore>

``` 
Here an `f0` statement is used to keep Csound running. Then the `i` statements specify a note length of -1 which means the instrument is always on.

### Sends and Returns
Data can be sent to and returned from Csound using the functions `StudioTools::BCsoundSend()` and `StudioTools::BCsoundReturn()`. 

```
//setup sends to csound
std::vector<const char*> sendNames;

sendNames.push_back("sineControlVal");
m_vSendVals.push_back(m_cspSineControlVal);	

sendNames.push_back("randomVal");
m_vSendVals.push_back(m_cspRandVal);

m_pStTools->BCsoundSend(csSession, sendNames, m_vSendVals);

//setup returns from csound 
std::vector<const char*> returnNames;

returnNames.push_back("pitchOut");
m_vReturnVals.push_back(m_pPitchOut);

returnNames.push_back("freqOut");
m_vReturnVals.push_back(m_pFreqOut);

m_pStTools->BCsoundReturn(csSession, returnNames, m_vReturnVals);	
```

A vector of type `const char*` is used to store the channel names. For send channels this vector is `sendNames` and for return channels the vector is called `returnNames`. These names must match the string arguments given to `chnget` and `chnset` in the `csd` file. The data values are stored in seperate member vectors named `m_vSendVals` and `m_vReturnVals` respectively. These are declared in *Studio.hpp*. Csound pointers of type `MYFLT*` are sent to Csound. The values can be assigned to specific elements in the send vector within `Studio::Update()`.

```
//example control signal - sine function
//sent to shader and csound
m_fSineControlVal = sin(glfwGetTime() * 0.33f);
*m_vSendVals[0] = (MYFLT)m_fSineControlVal;
```  

For example, the `float m_fSineControlVal` is cast to `MYFLT` and assigned to element 0 of `m_vSendVals`. This aligns with the setup code above in that the Csound pointer `m_cspSineControlVal` is the first value to be added to the vector. It follows then that the value intended for `m_cspRandVal` is referenced using `*m_vSendVals[1]`. The return values are accessed by referencing the specific element of the `m_vReturnVals` vector. 

```
if(*m_vReturnVals[0] > 0) m_fTargetVal = *m_vReturnVals[0];	
``` 

For example, element 0 is dereferenced here and assigned to the `float m_fTargetVal`. Once the channel names and data vectors are set up above, they are passed along with the `csSession` pointer to `StudioTools::BCsoundSend()` and `StudioTools::BCsoundReturn()`.

## Graphics Renderer
The graphics are rendered using OpenGL and GLSL shaders. The shaders are set up specifically to allow for raymarching combined with raster graphics. This is to account for the rendering of virtual controllers within VR and to ensure they interact as expected with the raymarched graphics.

### Shader Setup
Just as all the audio is generated in the `csd` file, the shaders are set up to encourage the generation of all the visual material in the fragment shader. In `Studio::Setup()` the OpenGL code necessary to set up a raymarching quad is called using:

```
//setup quad to use for raymarching
m_pStTools->RaymarchQuadSetup(shaderProg);
```

This sets up the quad vertices to be accessed in the vertex shader. The standard set up of the vertex shader is as follows:

```
#version 410 

layout(location = 0) in vec3 position;
 
uniform mat4 InvMVEP;

out vec4 nearPos;
out vec4 farPos;

void main() 
{
	//********* code from https://encreative.blogspot.com/2019/05/computing-ray-origin-and-direction-from.html *******//

	gl_Position = vec4(position, 1.0);

	//get 2D projection of this vertex in normalised device coordinates
	vec2 ndcPos = gl_Position.xy / gl_Position.w;
	
	//compute rays start and end points in the unit cube defined by ndc's
	nearPos = InvMVEP * vec4(ndcPos, -1.0, 1.0);
	farPos = InvMVEP * vec4(ndcPos, 1.0, 1.0);
}
```

Here the vertex position is converted to normalised device coordinates in order to use the unit cube to calculate the beginning and end positions of the ray. These are then projected back into world space using the inverse model view eye projection matrix. The use of the term 'eye' here is due to the extra matrix used in OpenVR to calculate the offset angle between the eyes in the head mounted display (HMD). The near and far positions of the ray are then output to the fragment shader where the direction of the ray can be calculated. The vertex shader as it appears here does not need to be modified and can be re-used from project to project because the majority of the visual material is generated in the fragment shader. The default components of the fragment shader are as follows:

```
#version 410  
uniform mat4 MVEPMat;

in vec4 nearPos;
in vec4 farPos;

layout(location = 0) out vec4 fragColour; 
layout(location = 1) out vec4 dataOut;

void main()
{
	//************* ray setup code from 
	//https://encreative.blogspot.com/2019/05/computing-ray-origin-and-direction-from.html*/
	
	//set up the ray
	vec3 rayOrigin = nearPos.xyz / nearPos.w;
	vec3 rayEnd = farPos.xyz / farPos.w;
	vec3 rayDir = rayEnd - rayOrigin;
	rayDir = normalize(rayDir);	
	
	// raymarch the point
	float dist = march(rayOrigin, rayDir);
	vec3 pos = rayOrigin + dist * rayDir;

	// calculate colour and lighting here
	vec3 colour;

	// gamma correction
	colour = pow(colour, vec3(1.0/2.2));
	
	// Output to screen
	fragColour = vec4(colour,1.0);

	// Output to PBO
	dataOut = fragColour;

//-----------------------------------------------------------------------------
// To calculate depth for use with rasterized material e.g. VR controllers
//-----------------------------------------------------------------------------
// code adapted from Michael Hvidtfeldt Christensen and James Susinno's blogs
// http://jimbo00000.github.io/opengl/portable/programming/vr/perspective/matrixmath/2016/02/15/raymarching-and-rasterization.html
// http://blog.hvidtfeldts.net/index.php/2014/01/combining-ray-tracing-and-polygons/

	vec4 pClipSpace =  MVEPMat * vec4(pos, 1.0);
	vec3 pNdc = vec3(pClipSpace.x / pClipSpace.w, pClipSpace.y / pClipSpace.w, pClipSpace.z / pClipSpace.w);
	float ndcDepth = pNdc.z;
	
	float d = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0; 
	gl_FragDepth = d;
}
``` 

The model view eye projection matrix is passed in as a uniform through the `StudioTools::RaymarchQuadSetup()` function. The vectors `nearPos` and `farPos` are passed in from the vertex shader. There are two output specified. The output at `location = 0`, `fragColour`, sends the colour of the fragment to the default framebuffer which is then drawn to the screen. The output at `location = 1`, `dataOut`, writes the data contained in it to a pixel buffer object which is then read back asynchronously to the CPU. This allows data to be sent from the fragment shader to be `Studio::Update()` where it can be manipulated further. In `main()` the direction and the origin of the ray are calculated before being used in a `march()` function. This function is not shown here as there are many ways to do this. The 'hit' position is then calculated and used to determine the colour of the material. Gamma correction is applied before the colour vector is sent to the output. Although `fragColour` is also sent to `dataOut`, this does not need to be the case as any value can be output here. Finally, at the bottom of the shader, `pos` is transformed to clip space and then to normalised device coordinates. The `z` component is then used to calculate the depth `d` which is finally assigned to `gl_FragDepth`. For a discussion on how these calculations are derived see Michael Hvidtfeldt Christensen and James Sussino's blogs linked above the code section. This bloack of code allows rasterised graphics to interact with raymarched graphics and is useful when rendering the default controllers in VR. Once the shaders are initialised, graphics are drawn to the screen in `Studio::Draw()`.

```
m_pStTools->DrawStart(projMat, eyeMat, viewMat, shaderProg, translateVec);
	
glUniform1f(m_gliSineControlValLoc, m_fSineControlVal);
glUniform1f(m_gliPitchOutLoc, m_fPitch);
glUniform1f(m_gliFreqOutLoc, *m_vReturnVals[1]);

m_pStTools->DrawEnd();
``` 

The function `DrawStart()` is accessed through the `m_pStTools` pointer. The projection matrix, eye matrix, view matrix, shader program handle and camera translation vector are passed to the function.Data can be passed to the shader here using `glUniform1f` or any of its variations. The uniform location handles are declared in `Studio::Setup()`. Finally, `DrawEnd()` is called which concludes the draw loop. 

## Mapping Overview
As mentioned above, data can be mapped throughout the toolkit. Here is a brief overview of the functions used to map data.

From `Studio()` to Csound:

	- `StudioTools::BCsoundSend()` to `chnget`.

From Csound to `Studio()`:

	- `chnset` to `StudioTools::BCsoundReturn()`.

From `Studio()` to fragment shader:

	- `glUniform1f` (and related OpenGL calls https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glUniform.xhtml) to `uniform ...`.

From the fragment shader to `Studio()`:

	- `out vec4 dataOut' to `PBOInfo& pboInfo.pboPtr`. 

## Machine Learning Walkthrough
The RapidLib machine learning library is an integral part of the toolkit. Access to a neural network regression algorithm is provided through the functions `Studio::MLRegressionSetup()` and `Studio::MLRegressionUpdate()`. These functions will eventually be moved to `StudioTools()` in order to keep `Studio()` as minimal as possible. `Studio::MLRegressionSetup()` is called in `Studio::Setup()` and simply initialises various `bool`s and variables that are used in `Studio::MLRegressionUpdate()`. `Studio::MLRegressionUpdate()` is called in `Studio::Update()` and is passed the `machineLearning` struct which provides access to the machine learning controls. It is also passed the `pboInfo` struct which provides access to data returned from the fragment shader. Finally, it is passed `std::vector<MLAudioParameter> paramVec` which provdes access to discrete parameters that are to be processed by the neural network. `Studio::MLRegressionUpdate()` is divided into several blocks of functionality.

	- Randomisation of parameters.
	- Recording of training examples.
	- Neural network training.
	- Running and halting of trained regression model.
	- Save trained model.
	- Load saved model.

The interactive machine learning (IML) workflow is as follows:

	- Choose the input and output parameters for the neural network. 
	- Create input/output examples to train the neural network. The approach here is to randomise the parameters. There are many other approaches possible.
	- Record the training examples to create a training set.
	- Train the neural network.
	- Run the model.
	- Save the model.
	- Load a previously saved model next time.

Keyboard controls for `dev` mode:

	- Randomise parameters -> space bar.
	- Record training examples -> r.
	- Train neural network -> t.
	- Run trained model -> g.
	- Halt model -> h.
	- Save model -> k.
	- Load model -> l.

When using a VR system the controls can be mapped to the controller buttons using the SteamVR runtime interface. The machine learning functionality is discussed further below as it relates specifically to the 'Cyclical Mapping Example'.

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
This example demonstrates a method of mapping data in a cyclical way between the audio and visuals using a neural network based regression algorithm. Pixel data from the fragment shader is used as input into the neural network. Ocsillator frequency values are then output and sent to the audio engine. An FFT analysis is used to determine the pitch of the resulting audio signal. This pitch value is sent to the shader to inversely affect the size of the sphere. The pixel data changes accordingly and is sent back to the neural network beginning the cycle again.

***link to video of example on YouTube***

The frequency of the audio signal is set up to be randomised. The random parameter is defined in `Studio::Update()` using the `MLAudioParameter` struct.

```
//run machine learning
MLAudioParameter paramData;
paramData.distributionLow = 400.0f;
paramData.distributionHigh = 1000.0f;
paramData.sendVecPosition = 1;
std::vector<MLAudioParameter> paramVec;
paramVec.push_back(paramData);
MLRegressionUpdate(machineLearning, pboInfo, paramVec);
```

The value will always be between `400.0f` and `1000.0f`. It is assigned to element number 1 in the vector `m_vSendVals`. This is declared in `Studio::Setup()` where the channel is named `"randomVal"`. This allows Csound to receive the random value on this channel. The object `paramData` is then pushed back onto the vector `paramVec` and passed to `Studio::MLRegressionUpdate()`. When the randomise button is pressed (space bar in dev mode), this random value is calculated and sent to Csound.

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
;**************************************************************************************
instr 1 ; Example Instrument
;**************************************************************************************
kRandomParam chnget "randomVal"

aSig oscil .7, kRandomParam 

gaOut = aSig

iFftSize = 1024
iOverlap = iFftSize / 4 
iWinSize = iFftSize 
iWinType = 1

fSig	pvsanal	aSig, iFftSize, iOverlap, iWinSize, iWinType

kThresh = 0.1
kPitch, kAmp	pvspitch fSig, kThresh

	chnset	kPitch, "pitchOut"

endin
```

Here the value recieved on the `"randomVal"` channel is stored in `kRandomParam`. This is then used as the frequency value for `oscil`. The signal `aSig` is sent to the output. It is also passed to `pvsanal` where an FFT analysis converts the time domain signal to the frequency domain. This signal, `fSig` is then passed to `pvspitch` which calculates its pitch and amplitude. The pitch value, `kPitch` is then sent back to `Studio()` on the channel `"pitchOut"`, which was defined in `Studio::Setup()`. This data stream is then processed in `Studio::Update()`.

```
// spectral pitch data processing
m_fCurrentFrame = glfwGetTime();
m_fDeltaTime = m_fCurrentFrame - m_fLastFrame;	
m_fDeltaTime *= 1000.0f;
if(*m_vReturnVals[0] > 0) m_fTargetVal = *m_vReturnVals[0];	
if(m_fTargetVal > m_fCurrentVal)
{
	m_fCurrentVal += m_fDeltaTime;
} else if(m_fTargetVal <= m_fCurrentVal)
{
	m_fCurrentVal -= m_fDeltaTime;
} else if(m_fTargetVal == m_fCurrentVal)
{
	m_fCurrentVal = m_fTargetVal;
}
if(m_fCurrentVal < 0.0f) m_fCurrentVal = 0.0f;
m_fPitch = m_fCurrentVal;
```

Here element 0 of `m_vReturnVals[]` is dereferenced and assigned to the `float m_fTargetVal`. The values in the data stream can vary quickly. To create smooth visual movement, it is necessary to move gradually from one value to the next rather than instantly jump between values. The `m_fTargetVal` value is compared to the value represented by `m_fCurrentVal`. If it is larger, `m_fCurrentVal` is increased gradually by `m_fDeltaTime` increments each frame. These increments are calculated by taking the timestamp of the current frame and subtracting it from the timestamp of the next frame. This gives a constant value from frame to frame. If `m_fTargetVal` is less than `m_fCurrentVal`, it is decreased by `m_fDeltaTime` increments each frame. If the two values are equal, then the value of `m_fTargetVal` is assigned to `m_fCurrentVal`. Finally, `m_fCurrentVal` is checked to make sure it is greater than zero before being assigned to `m_fPitch`. This `float` is passed to the shader through the uniform `"pitchOut"`.

```
m_pStTools->DrawStart(projMat, eyeMat, viewMat, shaderProg, translateVec);
glUniform1f(m_gliPitchOutLoc, m_fPitch);
m_pStTools->DrawEnd();
```

In a similar way to the audio reactive example above, the uniform value is used to affect the size of the sphere.

```
float DE(vec3 p)
{
	p.y -= 1.0;
	float rad = 1.0 * (1.0 / (pitchOut * 0.01));
	float sphereDist = sphereSDF(p, rad);
	return sphereDist;
}
```

The value `pitchOut` is inversely related to the radius of the sphere which means the higher the pitch value the smaller the sphere. Conversely, the lower the pitch value, the larger the sphere. The fragment shader sends pixel colour data back to `Studio()` asynchronously using a pixel buffer object (PBO). 

```
layout(location = 0) out vec4 fragColour; 
layout(location = 1) out vec4 dataOut;

// Output to screen
fragColour = vec4(colour,1.0);

// Output to PBO
dataOut = fragColour;
```   

There are two outputs of type `vec4` specified in the fragment shader. The vector `fragColour` at `location = 0` is the usual output of the fragment colour to the screen. The other output, `dataOut`, writes the data to a PBO. In this example, the colour vector of the fragment is passed to it. The data in the PBO is then read back into memory on the CPU and is accessible from `Studio::Update()` through the struct `PBOInfo`. This struct is then passed to `MLRegressionUpdate()` where the data can be used as input to the neural network.

```
if(machineLearning.bRecord)
{
	//shader values provide input to neural network
	for(int i = 0; i < pboInfo.pboSize; i+=pboInfo.pboSize * 0.01)
	{
		inputData.push_back((double)pboInfo.pboPtr[i]);
	}

	//neural network outputs to audio engine 
	for(int i = 0; i < params.size(); i++)
	{
		outputData.push_back((double)*m_vSendVals[params[i].sendVecPosition]);
	}

	trainingData.input = inputData;
	trainingData.output = outputData;
	trainingSet.push_back(trainingData);

	std::cout << "Recording Data" << std::endl;
	inputData.clear();
	outputData.clear();
}
machineLearning.bRecord = false;
```

When a desired sound and visual pairing is found, they can then be recorded (`R` in `dev` mode) to create a training example. In this example you would be pairing the position of the sphere in the visual field and the perceived pitch of the tone. This process can be repeated as many times as needed to create a *training set*. When the record button is pressed, a `for loop` steps through the buffer in `pboInfo.pboSize * 0.01` increments. This is to ensure the number of input values is reduced to cut down on training time. The values are pushed back onto the vector `inputData`. The format of the shader data is `unsigned char`. Therefore each value will be between 0 and 255. In a real world situation these values should be normalised before they are used as training data. The next `for loop` iterates through the `params` vector and retrieves the `sendVecPosition` of each one. Here there is only one parameter. This is then used as an index to the `m_vSendVals` vector. The value at this position in the vector is pushed back into the `outputData` vector. The input and output vectors are then added to the `trainingData` struct and pushed back into the `trainingSet` vector. Once the training set is complete, the neural network is trained (`T` in `dev` mode). Once training is complete, the model can be activated by pressing the run button (`G` in `dev` mode) or deactivated by pressing the halt button (`H` in `dev` mode). The model can then be saved (`K` in `dev` mode) and loaded (`L` in `dev` mode) for easy future access. When not in `dev` mode the controls can be assigned to the controller buttons through the desktop interface.

To summarize, when the neural network is trained and running, it is processing pixel colour values and mapping them to a specific frequency value. The audio is then analysed using an FFT and a pitch value is calculated. This pitch value is mapped to the size of the sphere. The changes in pixel values are then fed back into the neural network to produce another frequency value. This results in a continuous mapping of data between the audio and the visuals. In this example it results in a glitchy type audio effect and rapidly moving visuals that are tightly synchronised.


