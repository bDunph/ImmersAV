<CsoundSynthesizer>
<CsOptions>
; Select audio/midi flags here according to platform
; Audio out   Audio in    No messages
-odac          ;-iadc          ;;;RT audio I/O
; For Non-realtime ouput leave only the line below:
; -o moogvcf.wav -W ;;; for file output any platform

--nodisplays

</CsOptions>
<CsInstruments>

; Initialize the global variables.
sr = 48000 
kr = 480 
ksmps = 100 
nchnls = 2

; Set 0dbfs to 1
0dbfs = 1

giBuzz  ftgen 1,0,4096,11,40,1,0.9
giWave  ftgen  2,0,2^10,10,1,1/2,1/4,1/8,1/16,1/32,1/64
;window function - used as an amplitude envelope for each grain
;(bartlett window)
giWFn   ftgen 3,0,16384,20,3,1

;**************************************************************************************
instr 1 ; Real-time Spectral Instrument - Environmental Noise 
;**************************************************************************************

; get control value from application
;kSineControlVal	chnget	"sineControlVal"

ares	fractalnoise	ampdbfs(-24),	2 ; pink noise generator

	;outs	ares * 0.05, ares * 0.05
ifftsize = 2048
ioverlap = ifftsize / 4
iwinsize = ifftsize * 2
iwinshape = 0

fsig	pvsanal	ares,	ifftsize,	ioverlap,	iwinsize,	iwinshape

ifn = 1
;kdepth = 0.99 + (0.01 * kSineControlVal)
kdepth = 0.99

fmask	pvsmaska	fsig,	ifn,	kdepth		

aOut0	pvsynth	fmask
	outs	aOut0 * 0.05,	aOut0 * 0.05

endin


;**************************************************************************************
instr 2 ; Granular Instrument 
;**************************************************************************************

kCps	chnget	"grainFreq"
kPhs	chnget	"grainPhase"
kFmd	chnget	"randFreq"
kDens	chnget	"grainDensity"

kPmd = 0.5
kGDur = 0.1
kFrPow = 0.2
kPrPow = 0.2

kGDur = 0.01 + kGDur ; initialisation to avoid perf error 0.0
kDens = 1 + kDens

iMaxOvr = 2000 
kFn = 2

aOut3    grain3  kCps, kPhs, kFmd, kPmd, kGDur, kDens, iMaxOvr, kFn, giWFn, kFrPow, kPrPow

kAmp	linseg 0.0,	p3 * 0.1,	0.95,	p3 * 0.1,	0.8,	p3 * 0.6,	0.8,	p3 * 0.1,	0.0

kfe  expseg p4, p3*0.3, p5, p3*0.1, p6, p3*0.2, p7, p3*0.3, p8, p3*0.1, p9
kres linseg 0.1, p3 * 0.2, 0.3, p3 * 0.4, 0.25, p3 * 0.2, 0.5, p3 * 0.2, 0.35	;vary resonance
afil moogladder aOut3, kfe, kres

gaGranularOut = (afil * kAmp) * 0.5
;aGranularOut = afil * kAmp

	;outs	aGranularOut, aGranularOut 

endin

;**************************************************************************************
instr 3 ; note scheduler
;**************************************************************************************

kGaussVal gauss 6.0

seed 0
kRand random 0.1, 10.0

seed 0
kRand2 random 1, 5 

kTrigger metro kRand2 
kMinTim	= 0 
kMaxNum = 1 
kInsNum = 2
kWhen = 0
kDur = kRand 

schedkwhen kTrigger, kMinTim, kMaxNum, kInsNum, kWhen, kDur, 1000+kGaussVal, 1400+kGaussVal, 1200+kGaussVal, 800+kGaussVal, 700+kGaussVal, 1000+kGaussVal

endin

;**************************************************************************************
instr 4 ; HRTF Instrument
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
aInstSigs[0] = gaGranularOut

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

</CsInstruments>
<CsScore>

f0	86400 ;keep csound running for a day

i1 1 -1 

i3 1 -1

i4 1 -1

e
</CsScore>
</CsoundSynthesizer>

