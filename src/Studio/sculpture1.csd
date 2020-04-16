<CsoundSynthesizer>
<CsOptions>
; Select audio/midi flags here according to platform
; Audio out   Audio in    No messages
-odac          ;-iadc          ;;;RT audio I/O
; For Non-realtime ouput leave only the line below:
; -o moogvcf.wav -W ;;; for file output any platform
</CsOptions>
<CsInstruments>

; Initialize the global variables.
sr = 48000
kr = 4800
ksmps = 10
nchnls = 2

; Set 0dbfs to 1
0dbfs = 1

;*************************************************************************************
instr 1; Modal Synthesis Percussive Instrument 
;*************************************************************************************
idur 	init p3
iamp    init ampdbfs(p4)

kFreqScale chnget "randFreq" ; random frequency scale value sent from application

; to simulate the shock between the excitator and the resonator
krand	random	1,	10	
ashock  mpulse ampdbfs(-1), krand,	2

; felt excitator from mode.csd
;aexc1	mode	ashock,	80 * (kFreqScale + 1.0),	8
aexc1	mode	ashock,	80,	8
aexc1 = aexc1 * iamp

;aexc2	mode	ashock,	188 * (kFreqScale * 1.0),	3
aexc2	mode	ashock,	188,	3
aexc2 = aexc2 * iamp

aexc = (aexc1 + aexc2)/2

;"Contact" condition : when aexc reaches 0, the excitator looses 
;contact with the resonator, and stops "pushing it"
aexc limit	aexc,	0,	3*iamp 

; Wine Glass with ratios from http://www.csounds.com/manual/html/MiscModalFreq.html
;ares1	mode	aexc,	220 * (kFreqScale + 1),	420 ; A3 fundamental frequency
ares1	mode	aexc,	220,	420 ; A3 fundamental frequency

ares2	mode	aexc,	510.4,	480

ares3	mode	aexc,	935,	500

ares4	mode	aexc,	1458.6,	520

ares5	mode	aexc,	2063.6,	540

ares = (ares1+ares2+ares3+ares4+ares5)/5

gaOut1 = aexc + ares 
	;outs	gaOut1,	gaOut1

kRms	rms	gaOut1
	chnset	kRms,	"rmsOut"
endin

;*************************************************************************************
instr 2	; Physical Bowed String Instrument
;*************************************************************************************
kpres = 4
krat = 0.046
kvibf = 6.12723

kvib	linseg	0,	0.5,	0,	1,	1,	2.5,	1
kvamp = kvib * 0.01

asig	wgbow	0.7,	55,	kpres,	krat,	kvibf,	kvamp

	outs	asig,	asig

endin
			
;*************************************************************************************
instr 3 ; Physical Bowed Bar Instrument
;*************************************************************************************

kSineControlVal	chnget	"sineControlVal"

kEnv	adsr	0.45,	0.8,	0.05,	0.6

kp = 1.9

asig	wgbowedbar	ampdbfs(-1),	133,	0.7*kSineControlVal,	kp*kSineControlVal,	0.995
gaOut1 = (asig + asig + asig + asig + asig + asig) * kEnv

     ;outs asig, asig

endin

;*************************************************************************************
instr 4 ; Waveguide + Mode Instrument - Noise Filter
;*************************************************************************************

kSineControlVal	chnget	"sineControlVal"

;envelope
kEnv	adsr	0.3,	0.1,	0.8,	0.6

;noise
kbeta	line	-0.9999,	p3,	0.9999
asig	noise	ampdbfs(-3),	0.3999	
asig = asig * kEnv

;waveguide model1
kfreq		init	200
kcutoff		init	300
kfeedback	init	0.2

awg1	wguide1	asig,	kfreq,	kcutoff * kSineControlVal,	kfeedback

;waveguide model2
kfreq2		linseg	800,	p3/2,	100,	p3/2,	300
kcutoff2	linseg	500,	p3/2,	700,	p3/2,	200
kfeedback2	linseg	0.5,	p3/2,	0.8,	p3/2,	0.1

awg2	wguide1	awg1,	kfreq2,	kcutoff2,	kfeedback2	

;tone filter
khp	linseg	700,	p3/4,	350,	p3/4,	400,	p3/4,	300,	p3/4,	200

aToneOut	tone	awg2,	khp

;mode bank
; felt excitator from mode.csd
iamp	init	ampdbfs(-3)

kModeFreq	linseg	80,	p3/2,	99,	p3/2,	89
kQFactor	linseg	8,	p3/2,	21,	p3/2,	18

aexc1	mode	aToneOut,	kModeFreq,	kQFactor	
aexc1 = aexc1 * iamp 

kModeFreq2	linseg	188,	p3/2,	159,	p3/2,	201
kQFactor2	linseg	3,	p3/2,	15,	p3/2,	5

aexc2	mode	aToneOut,	kModeFreq2,	kQFactor2
aexc2 = aexc2 * iamp 

gaOut1 = (aexc1 + aexc2)/2

	;outs	aexc,	aexc

endin

;**************************************************************************************		
instr 5 ; Beaten Plate 
;**************************************************************************************		

kFreq	linseg	50,	p3,	20
kPhase	phasor	5
kAmp = 0.4 * kPhase

aSig		lfo	kAmp,	kFreq,	1
aFreq1		linseg	900,	p3/2,	870,	p3/2,	1100
aFreq2		linseg	500,	p3/3,	340,	p3/3,	624,	p3/3,	300
kCutOff1 = 3000
kCutOff2 = 1500
kFeedback1 = 0.35
kFeedback2 = 0.15

aWGOut	wguide2	aSig,	aFreq1,	aFreq2,	kCutOff1,	kCutOff2,	kFeedback1,	kFeedback2
aWGOut	dcblock2	aWGOut

	outs	aWGOut,	aWGOut

endin

;**************************************************************************************		
instr 6 ; Crunch Instrument
;**************************************************************************************		

idmp = p4

a1	crunch		0.8,	0.04,	56,	idmp	
a2	crunch		0.8,	0.1,	7,	idmp

aOutR,	aOutL	reverbsc	a1,	a2,	0.9,	12000	

	outs		aOutL,	aOutR

endin

;**************************************************************************************		
instr 7 ; Bowed String Resonator
;**************************************************************************************		

kSineControlVal	chnget	"sineControlVal"

kRandPressure	random	2.0,	3.3
kRandPos	random	0.025,	0.035

kAmpBow = ampdbfs(-3)
kFreqBow = 50
kPresBow = kRandPressure
kRatBow = kRandPos
kVibfBow = 12 * kSineControlVal 
kVAmpBow = ampdbfs(-3)

aBow	wgbow	kAmpBow,	kFreqBow,	kPresBow,	kRatBow,	kVibfBow,	kVAmpBow	

;Tibetan Bowl Resonances
kfr1 = 221
kfr2 = 614
kfr3 = 1145
kfr4 = 1804
kfr5 = 2577
kfr6 = 3456
kfr7 = 4419

ifdbgain = 0.90 

;astr1	streson	aBow,	kfr1,	ifdbgain
;aSum1	sum	aBow,	astr1
;astr2	streson	aSum1,	kfr2,	ifdbgain
;aSum2	sum	aBow,	astr2
;astr3	streson	aSum2,	kfr3,	ifdbgain
;aSum3	sum	aBow,	astr3
;astr4	streson	aSum3,	kfr4,	ifdbgain
;aSum4	sum	aBow,	astr4
;astr5	streson	aSum4,	kfr5,	ifdbgain
;aSum5	sum	aBow,	astr5
;astr6	streson	aSum5,	kfr6,	ifdbgain
;aSum6	sum	aBow,	astr6
;astr7	streson	aSum6,	kfr7,	ifdbgain

;astr1	streson	aBow,	kfr1,	ifdbgain
;astr2	streson	aBow,	kfr2,	ifdbgain + 0.01
;astr3	streson	aBow,	kfr3,	ifdbgain + 0.03
;astr4	streson	aBow,	kfr4,	ifdbgain + 0.06
;astr5	streson	aBow,	kfr5,	ifdbgain - 0.02
;astr6	streson	aBow,	kfr6,	ifdbgain - 0.04
;astr7	streson	aBow,	kfr7,	ifdbgain + 0.02

astr1	streson	aBow,	kfr1,	ifdbgain
astr2	streson	astr1,	kfr2,	ifdbgain
astr3	streson	astr2,	kfr3,	ifdbgain
astr4	streson	astr3,	kfr4,	ifdbgain
astr5	streson	astr4,	kfr5,	ifdbgain
astr6	streson	astr5,	kfr6,	ifdbgain
astr7	streson	astr6,	kfr7,	ifdbgain

asig1	clip	astr1,	0,	1
asig2	clip	astr2,	0,	1
asig3	clip	astr3,	0,	1
asig4	clip	astr4,	0,	1
asig5	clip	astr5,	0,	1
asig6	clip	astr6,	0,	1
asig7	clip	astr7,	0,	1

gaOut1	sum	asig1,	asig2,	asig3,	asig4,	asig5,	asig6,	asig7
	;outs	aOut,	aOut

kRms	rms	gaOut1
	chnset	kRms,	"rmsOut"

endin

;****************************************************************************************
instr 8 ; FFT Instrument
;****************************************************************************************

idur = p3
ilock = 1
ipitch = 10 
itimescale = 0.15
iamp = ampdbfs(-3)

atime	line	0,	idur,	idur * itimescale
asig	mincer	atime,	iamp,	ipitch,	1,	ilock

	outs	asig,	asig

endin

;****************************************************************************************
instr 9 ; Formant Instrument
;****************************************************************************************

; Combine five formants together to create 
; a transformation from an alto-"a" sound
; to an alto-"i" sound.
; Values common to all of the formants.
kfund init 261.659
koct init 0
kris init 0.003
kdur init 0.02
kdec init 0.007
iolaps = 100
ifna = 2
ifnb = 3
itotdur = p3

; First formant.
k1amp = ampdb(0)
k1form line 600, p3, 250
k1band line 60, p3, 60

; Second formant.
k2amp line ampdb(-7), p3, ampdb(-30)
k2form line 1040, p3, 1750
k2band line 70, p3, 90

; Third formant.
k3amp line ampdb(-9), p3, ampdb(-16)
k3form line 2250, p3, 2600
k3band line 110, p3, 100

; Fourth formant.
k4amp  line ampdb(-9), p3, ampdb(-22)
k4form line 2450, p3, 3050
k4band init 120

; Fifth formant.
k5amp line ampdb(-20), p3, ampdb(-28)
k5form line 2750, p3, 3340 
k5band line 130, p3, 120

a1 fof k1amp, kfund, k1form, koct, k1band, kris, \
       kdur, kdec, iolaps, ifna, ifnb, itotdur
a2 fof k2amp, kfund, k2form, koct, k2band, kris, \
       kdur, kdec, iolaps, ifna, ifnb, itotdur
a3 fof k3amp, kfund, k3form, koct, k3band, kris, \
       kdur, kdec, iolaps, ifna, ifnb, itotdur
a4 fof k4amp, kfund, k4form, koct, k4band, kris, \
       kdur, kdec, iolaps, ifna, ifnb, itotdur
a5 fof k5amp, kfund, k5form, koct, k5band, kris, \
       kdur, kdec, iolaps, ifna, ifnb, itotdur

; Combine all of the formants together
asig sum a1,	a2,	a3,	a4,	a5
     outs asig, asig

endin

;**************************************************************************************		
instr 10 ; Spectral Instrument
;**************************************************************************************		

ain  diskin2 "24cellRow_mono.wav", 1
fs1,fsi2 pvsifd ain,2048,512,1		; ifd analysis
fst  partials fs1,fsi2,.003,1,3,500	; partial tracking
gaOut1 resyn fst, 1, 0.5, 250, 2		; resynthesis 
     ;outs aout, aout

endin

;**************************************************************************************		
instr 11 ; Spectral Warping Instrument
;**************************************************************************************		

;kscal = p4
kscal	linseg	1,	p3/3,	2.3,	p3/3,	2.1,	p3/3,	3
kshift	linseg	0,	p3/3,	0.9,	p3/3,	-0.3,	p3/3,	0
	
asig  soundin "24cellRow_mono.wav"			; get the signal in
fsig  pvsanal asig, 1024, 256, 1024, 1	; analyse it
ftps  pvswarp fsig, kscal, kshift		; warp it
gaOut1 pvsynth ftps			; synthesise it                      
      ;outs atps, atps

endin

;**************************************************************************************
instr 12 ; Hrtf Instrument
;**************************************************************************************
kPortTime linseg 0.0, 0.001, 0.05 

kAzimuthVal chnget "azimuth" 
kElevationVal chnget "elevation" 
kDistanceVal chnget "distance" 
kDist portk kDistanceVal, kPortTime ;to filter out audio artifacts due to the distance changing too quickly

aLeftSig, aRightSig  hrtfmove2	gaOut1, kAzimuthVal, kElevationVal, "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
aLeftSig = aLeftSig / (kDist + 0.00001)
aRightSig = aRightSig / (kDist + 0.00001)
	
aL = aLeftSig
aR = aRightSig

outs	aL,	aR
endin

;********************************************************************
instr 13 ;test tone
;********************************************************************

kamp = ampdbfs(-3) 
kcps = 440

asig 	oscil 	kamp, 	kcps 	
	outs 	asig, 	asig

endin

</CsInstruments>
<CsScore>

;********************************************************************
; f tables
;********************************************************************
; read file
f1	0	0	1	"24cellRow_mono.wav"	0	0	0

; sine wave
f2	0 	4096 	10 	1

; sigmoid wave
f3 	0 	1024 	19 	0.5 	0.5 	270 	0.5

;********************************************************************
; score events
;********************************************************************

;p1	p2	p3	p4	p5	p6	p7	p8	p9	p10	p11	p12	p13	p14	p15	p16	p17	p18	p19	p20	p21	p22	p23	p24

;i1	2	100000	-2		

;i2	14	2
;i.	+	2
;i.	+	2	
;
i3	2	10000	
;i.	+	5
;i.	+	5
;i.	+	5
;
;i4	2	10000
;i.	+	5
;i.	+	2
;
;i5	60	20
;i.	+	10
;i.	+	5
;
;i6	96	5	0.7
;i.	+	5	0.85
;i.	+	5	0.94

;i7	2	5	
;i.	+	5
;i.	+	5
;i.	+	10000

;i8	133	4
;i.	+	4
;i.	+	4	
;
;i9	146	6
;i.	+	6
;i.	+	6

;i10	2	3
;i.	+	5
;i.	+	10000

;i11 	2 	5 	1
;i. 	+ 	5 	1.5
;i. 	+ 	5 	3
;i. 	+ 	5 	.25

i12	2	100000	

;i13	0	240

</CsScore>
</CsoundSynthesizer>
