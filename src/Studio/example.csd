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
asig oscils .7, 220, 0
     outs asig, asig

endin

</CsInstruments>
<CsScore>

i 1 0 2 

e
</CsScore>
</CsoundSynthesizer>

