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

;**************************************************************************************
instr 1 ; Example Instrument
;**************************************************************************************
kSineControlVal chnget "sineControlVal"
kRandomParam chnget "randomVal"

aSig oscil .7, kRandomParam 

gaOut = aSig

;kRms	rms	gaOut
;	chnset	kRms,	"rmsOut"

iFftSize = 1024
iOverlap = iFftSize / 4 
iWinSize = iFftSize 
iWinType = 0

fSig	pvsanal	aSig, iFftSize, iOverlap, iWinSize, iWinType
kCent	pvscent	fSig

	chnset	kCent, "centOut"

endin

;**************************************************************************************
instr 2 ; Hrtf Instrument
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

</CsInstruments>
<CsScore>

f0	86400 ;keep csound running for a day

i1 1 -1 

i2 1 -1

e
</CsScore>
</CsoundSynthesizer>

