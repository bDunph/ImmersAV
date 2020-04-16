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

instr 1; Vert0 Modal Synthesis Instrument 

idur 	init p3
iamp    init ampdbfs(p4)
ifreq11 init p5
ifreq12 init p6
ifreq13 init p7
ifreq14 init p8
ifreq15 init p9
iQ11    init p10
iQ12    init p11
iQ13 	init p12
iQ14	init p13
iQ15	init p14
ifreq21 init p15
ifreq22 init p16
ifreq23 init p17
ifreq24 init p18
ifreq25 init p19
iQ21    init p20
iQ22    init p21
iQ23	init p22
iQ24	init p23
iQ25	init p24

; to simulate the shock between the excitator and the resonator
krand	random	1,	10
ashock  mpulse ampdbfs(-1), krand,	1

;aexc1	mode 	ashock,	ifreq11,	iQ11
;aexc1 = aexc1*iamp
;
;aexc2  	mode 	ashock,	ifreq12,	iQ12
;aexc2 = aexc2*iamp
;
;aexc3   mode 	ashock,	ifreq13,	iQ13
;aexc3 = aexc3*iamp
;
;aexc4   mode 	ashock, ifreq14,	iQ14
;aexc4 = aexc4*iamp
;
;aexc5   mode 	ashock,	ifreq15,	iQ15
;aexc5 = aexc5*iamp

;aexc = (aexc1+aexc2+aexc3+aexc4+aexc5)/5

; felt excitator from mode.csd
aexc1	mode	ashock,	80,	8
aexc1 = aexc1 * iamp

aexc2	mode	ashock,	188,	3
aexc2 = aexc2 * iamp

aexc = (aexc1 + aexc2)/2

;"Contact" condition : when aexc reaches 0, the excitator looses 
;contact with the resonator, and stops "pushing it"
aexc limit aexc,0,3*iamp 

; 5modes resonator

;ares1	mode	aexc1,	ifreq21,	iQ21	
;
;ares2	mode	aexc2,	ifreq22,	iQ22
;
;ares3	mode	aexc3,	ifreq23,	iQ23
;
;ares4	mode	aexc4,	ifreq24,	iQ24
;
;ares5	mode	aexc5,	ifreq25,	iQ25

; Wine Glass with ratios from http://www.csounds.com/manual/html/MiscModalFreq.html
ares1	mode	aexc,	220,	420 ; A3 fundamental frequency

ares2	mode	aexc,	510.4,	480

ares3	mode	aexc,	935,	500

ares4	mode	aexc,	1458.6,	520

ares5	mode	aexc,	2063.6,	540

ares = (ares1+ares2+ares3+ares4+ares5)/5

gaOut1 = aexc + ares

kRms	rms	gaOut1
	chnset	kRms,	"vert0"
endin

instr 2 ; Vert1 Mode Instrument

iamp    init ampdbfs(p4)

; to simulate the shock between the excitator and the resonator
krand	random	1,	10
ashock  mpulse ampdbfs(-1), krand,	1

; felt excitator from mode.csd
aexc1	mode	ashock,	80,	8
aexc1 = aexc1 * iamp

aexc2	mode	ashock,	188,	3
aexc2 = aexc2 * iamp

aexc = (aexc1 + aexc2)/2

;"Contact" condition : when aexc reaches 0, the excitator looses 
;contact with the resonator, and stops "pushing it"
aexc limit aexc,0,3*iamp 

; Wine Glass with ratios from http://www.csounds.com/manual/html/MiscModalFreq.html
ares1	mode	aexc,	65.41,	420 ; C2 fundamental frequency

ares2	mode	aexc,	151.7512,	480

ares3	mode	aexc,	277.9925,	500

ares4	mode	aexc,	433.6683,	520

ares5	mode	aexc,	613.5458,	540

ares = (ares1+ares2+ares3+ares4+ares5)/5

gaOut2 = aexc + ares

kRms	rms	gaOut2
	chnset	kRms,	"vert1"

endin

instr 3 ; Vert2 Mode Instrument

iamp    init ampdbfs(p4)

; to simulate the shock between the excitator and the resonator
krand	random	1,	10
ashock  mpulse ampdbfs(-1), krand,	1

; felt excitator from mode.csd
aexc1	mode	ashock,	80,	8
aexc1 = aexc1 * iamp

aexc2	mode	ashock,	188,	3
aexc2 = aexc2 * iamp

aexc = (aexc1 + aexc2)/2

;"Contact" condition : when aexc reaches 0, the excitator looses 
;contact with the resonator, and stops "pushing it"
aexc limit aexc,0,3*iamp 

; Wine Glass with ratios from http://www.csounds.com/manual/html/MiscModalFreq.html
ares1	mode	aexc,	155.56,	420 ; Eb3 fundamental frequency

ares2	mode	aexc,	360.8992,	480

ares3	mode	aexc,	661.13,		500

ares4	mode	aexc,	1031.3628,	520

ares5	mode	aexc,	1459.1528,	540

ares = (ares1+ares2+ares3+ares4+ares5)/5

gaOut3 = aexc + ares

kRms	rms	gaOut3
	chnset	kRms,	"vert2"

endin

instr 4 ; Vert3 Mode Instrument

iamp    init ampdbfs(p4)

; to simulate the shock between the excitator and the resonator
krand	random	1,	10
ashock  mpulse ampdbfs(-1), krand,	1

; felt excitator from mode.csd
aexc1	mode	ashock,	80,	8
aexc1 = aexc1 * iamp

aexc2	mode	ashock,	188,	3
aexc2 = aexc2 * iamp

aexc = (aexc1 + aexc2)/2

;"Contact" condition : when aexc reaches 0, the excitator looses 
;contact with the resonator, and stops "pushing it"
aexc limit aexc,0,3*iamp 

; Wine Glass with ratios from http://www.csounds.com/manual/html/MiscModalFreq.html
ares1	mode	aexc,	493.88,		420 ; Bb3 fundamental frequency

ares2	mode	aexc,	1145.8016,	480

ares3	mode	aexc,	2098.99,	500

ares4	mode	aexc,	3274.4244,	520

ares5	mode	aexc,	4632.5944,	540

ares = (ares1+ares2+ares3+ares4+ares5)/5

gaOut4 = aexc + ares

kRms	rms	gaOut4
	chnset	kRms,	"vert3"

endin

instr 5 ; Vert4 Mode Instrument

iamp    init ampdbfs(p4)

; to simulate the shock between the excitator and the resonator
krand	random	1,	10
ashock  mpulse ampdbfs(-1), krand,	1

; felt excitator from mode.csd
aexc1	mode	ashock,	80,	8
aexc1 = aexc1 * iamp

aexc2	mode	ashock,	188,	3
aexc2 = aexc2 * iamp

aexc = (aexc1 + aexc2)/2

;"Contact" condition : when aexc reaches 0, the excitator looses 
;contact with the resonator, and stops "pushing it"
aexc limit aexc,0,3*iamp 

; Wine Glass with ratios from http://www.csounds.com/manual/html/MiscModalFreq.html
ares1	mode	aexc,	196,		420 ; G3 fundamental frequency

ares2	mode	aexc,	454.72,		480

ares3	mode	aexc,	833,		500

ares4	mode	aexc,	1299.48,	520

ares5	mode	aexc,	1838.48,	540

ares = (ares1+ares2+ares3+ares4+ares5)/5

gaOut5 = aexc + ares

kRms	rms	gaOut5
	chnset	kRms,	"vert4"

endin

instr 6 ; Hrtf Instrument

S_AzimuthVals[] init 5
S_ElevationVals[] init 5
S_DistanceVals[] init 5

iCount = 0
loop:
	S_VertNumber sprintf "%i", iCount

	S_AzimuthChannel strcpy "azimuth"	
	S_ChannelNameAz strcat S_AzimuthChannel, S_VertNumber
	S_AzimuthVals[iCount] sprintf "%s", S_ChannelNameAz

	S_ElevationChannel strcpy "elevation"
	S_ChannelNameEl strcat S_ElevationChannel, S_VertNumber
	S_ElevationVals[iCount] sprintf "%s", S_ChannelNameEl

	S_DistanceChannel strcpy "distance"
	S_ChannelNameDist strcat S_DistanceChannel, S_VertNumber
	S_DistanceVals[iCount] sprintf "%s", S_ChannelNameDist

	loop_lt iCount, 1, 5, loop

kAzimuthVals[] init 5
kElevationVals[] init 5
kDistanceVals[] init 5

kPortTime linseg 0.0, 0.001, 0.05 

kAzimuthVals[0] chnget S_AzimuthVals[0] 
kElevationVals[0] chnget S_ElevationVals[0] 
kDistanceVals[0] chnget S_DistanceVals[0] 
kDist0 portk kDistanceVals[0], kPortTime ;to filter out audio artifacts due to the distance changing too quickly


kAzimuthVals[1] chnget S_AzimuthVals[1] 
kElevationVals[1] chnget S_ElevationVals[1] 
kDistanceVals[1] chnget S_DistanceVals[1] 
kDist1 portk kDistanceVals[1], kPortTime ;to filter out audio artifacts due to the distance changing too quickly

kAzimuthVals[2] chnget S_AzimuthVals[2] 
kElevationVals[2] chnget S_ElevationVals[2] 
kDistanceVals[2] chnget S_DistanceVals[2] 
kDist2 portk kDistanceVals[2], kPortTime ;to filter out audio artifacts due to the distance changing too quickly

kAzimuthVals[3] chnget S_AzimuthVals[3] 
kElevationVals[3] chnget S_ElevationVals[3] 
kDistanceVals[3] chnget S_DistanceVals[3] 
kDist3 portk kDistanceVals[3], kPortTime ;to filter out audio artifacts due to the distance changing too quickly

kAzimuthVals[4] chnget S_AzimuthVals[4] 
kElevationVals[4] chnget S_ElevationVals[4] 
kDistanceVals[4] chnget S_DistanceVals[4] 
kDist4 portk kDistanceVals[4], kPortTime ;to filter out audio artifacts due to the distance changing too quickly

aRightSigs[] init 5
aLeftSigs[] init 5 

aLeftSigs[0], aRightSigs[0]  hrtfmove2	gaOut1, kAzimuthVals[0], kElevationVals[0], "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
aLeftSigs[0] = aLeftSigs[0] / (kDist0 + 0.00001)
aRightSigs[0] = aRightSigs[0] / (kDist0 + 0.00001)
	
aLeftSigs[1], aRightSigs[1]  hrtfmove2	gaOut2, kAzimuthVals[1], kElevationVals[1], "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
aLeftSigs[1] = aLeftSigs[1] / (kDist1 + 0.00001)
aRightSigs[1] = aRightSigs[1] / (kDist1 + 0.00001)

aLeftSigs[2], aRightSigs[2]  hrtfmove2	gaOut3, kAzimuthVals[2], kElevationVals[2], "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
aLeftSigs[2] = aLeftSigs[2] / (kDist2 + 0.00001)
aRightSigs[2] = aRightSigs[2] / (kDist2 + 0.00001)

aLeftSigs[3], aRightSigs[3]  hrtfmove2	gaOut4, kAzimuthVals[3], kElevationVals[3], "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
aLeftSigs[3] = aLeftSigs[3] / (kDist3 + 0.00001)
aRightSigs[3] = aRightSigs[3] / (kDist3 + 0.00001)

aLeftSigs[4], aRightSigs[4]  hrtfmove2	gaOut5, kAzimuthVals[4], kElevationVals[4], "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
aLeftSigs[4] = aLeftSigs[4] / (kDist4 + 0.00001)
aRightSigs[4] = aRightSigs[4] / (kDist4 + 0.00001)

aL = (aLeftSigs[0] + aLeftSigs[1] + aLeftSigs[2] + aLeftSigs[3] + aLeftSigs[4]) / 5
aR = (aRightSigs[0] + aRightSigs[1] + aRightSigs[2] + aRightSigs[3] + aRightSigs[4]) / 5

;aLimL	limit	aL,	ampdbfs(-96),	ampdbfs(0)
;aLimR	limit	aR,	ampdbfs(-96),	ampdbfs(0)

outs	aL,	aR
endin

instr 7 ;test tone

kamp = ampdbfs(-3) 
kcps = 440

asig 	oscil 	kamp, 	kcps 	
	outs 	asig, 	asig

endin

</CsInstruments>
<CsScore>
;p1	p2	p3	p4	p5	p6	p7	p8	p9	p10	p11	p12	p13	p14	p15	p16	p17	p18	p19	p20	p21	p22	p23	p24

;i1 	0 	90 	-6	50	70	82	80	90	1000  	720  	850	700	820	440	882  	660	220	442	500	400	350	130	200

;i1	0	180	-3		
;i2	0	180	-3
;i3	0	180	-3
;i4	0	180	-3
;i5	0	180	-3
;
;i6	0	180	

;i7	0	240

</CsScore>
</CsoundSynthesizer>
