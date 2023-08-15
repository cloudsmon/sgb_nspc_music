;.include        "global.s"
; N-SPC VCMD (command codes)
; full listing: https://sneslab.net/wiki/N-SPC_Engine
tempo = 0xE7
gvol = 0xE5 ; main volume
instr = 0xE0 ; sample nr
vol = 0xED ; channel volume
pan = 0xE1
tup = 0xEA ;transpose semitones up
pat = 0xEF ; $EF [lo hi cnt]: jmp to address and repeat [cnt] times
ecv = 0xF5 ; echo volume
edl = 0xF7 ; echo delay
eoff = 0xF6 ; echo off
tun = 0xF4 ; fine tune 1/256 of a semitune
; Set note duration: nXX [release time/note volume]
; second param is optional and sets how long (yyy) and loud (zzzz)
; the note is played ($01-$7F); $7f is max long and loud
; %0xxxxxxx (%0yyyzzzz)
n1 = 0x60
n2 = 0x30
n4 = 0x18
n8 = 0x0C
n16 = 0x06
n32 = 0x03
n1_ = 0x40 ; triplets
n2_ = 0x20
n4_ = 0x10
n8_ = 0x08
n16_ = 0x04
n32_ = 0x02

c20 = 0x98 ; c in third octave (octave counting starts at 0)
c21 = 0x99 ; c#
d20 = 0x9A
d21 = 0x9B
e20 = 0x9C
f20 = 0x9D
f21 = 0x9E
g20 = 0x9F
g21 = 0xA0
a20 = 0xA1
a21 = 0xA2
b20 = 0xA3
c30 = 0xA4 ; c in fourth octave
c31 = 0xA5 ; c#
d30 = 0xA6
d31 = 0xA7
e30 = 0xA8 ; e
d30 = 0xA6
f30 = 0xA9
g30 = 0xAB
g31 = 0xAC
a30 = 0xAD
a31 = 0xAE
b30 = 0xAF


rest = 0xC9 ; rest for the amount of current note duration
rept = 0xC8 ; repeat prev note (or rest)
; command codes end

.macro GOSUB myaddr mycnt
  .db pat
  .dw myaddr
  .db mycnt
.endm

.macro NOTEREPT mynote mycnt
.rept mycnt
  .db mynote, rest
.endm
.endm

;; ****************************************
.title  "nspc_music_data"
.module nspc_music_data
.area   _SPCDATA (ABS)

; (hack) define _main so that lcc stops complaining
.org 0x500
_main::
  .db 0x42

.org 0x2afe   ; 0x2b00 minus 2 (hack) Python script uses this address to read the len of the song data section
.dw my_data_end

; this is the data that will be transfered to the SPC
song_table:: ;must start at 0x2b00
  .dw bgm1
  .dw bgm2
  .dw bgm1
  .dw bgm1
  .dw bgm1
  .dw bgm1
  .dw bgm1
  .dw bgm1
  .dw bgm1
  .dw bgm1
  .dw bgm1
  .dw bgm1
  .dw bgm1
  .dw bgm1
  .dw bgm1        ; ... up to bgm15 max

bgm1::
  .dw bgm1_pattern
  .dw 0xFF ; repeat
  .dw bgm1 ; to
  .dw 0x00
bgm1_pattern::
  .dw bgm1_channel0 ; melody
  .dw bgm1_channel1 ; bass
  .dw 0x00
  .dw 0x00
  .dw 0x00
  .dw 0x00
  .dw 0x00
  .dw 0x00 ; channel 2-7 unused
bgm1_channel0::
  .db tempo,25,gvol,200 ; global song settings
  .db instr,0x17, tun,040, vol,180
  .db ecv , 31, 40, 40, edl, 2, 95, 2
  ;.db eoff
  GOSUB blues_melody, 4
  .db 0
blues_melody::
  ; n4 == 24, n8 == 12... 16 + 8 n4_+n8_ for swinging eights
  .db n4_, 0x4F, g30, n8_, g30, n8, a31, n8, g30, rest, n4, f30, n8, rest
  .db n4_, 0x4F, g30, n8_, g30, n8, a31, n8, g30, rest, n4, f30, n8, rest
  .db n4_, 0x6F, g30, n8_, g30, n4, rest, rest, rest
  .db n1, rest

  .db tup, 5
  .db n4_, 0x4F, g30, n8_, g30, n8, a31, n8, g30, rest, n4, f30, n8, rest
  .db n4_, 0x4F, g30, n8_, g30, n8, a31, n8, g30, rest, n4, f30, n8, rest
  .db tup, 0
  .db n4_, 0x6F, g30, n8_, g30, n4, rest, rest, rest
  .db n1, rest

  .db tup, 7
  .db n8_, 0x6F, g30, n4_, rest, n4+n8, g30, n4, rest ;missing n8
  .db tup, 5
  .db n8_, 0x6F, g30, n4_, rest, n4+n8, g30, n4, rest, rest ; compensating n8
  .db tup, 0
  .db n4_, 0x6F, g30, n8_, g30, n4, rest, rest, rest
  .db n1, rest

  .db 0

bgm1_channel1::
  .db instr,0x0c, tun,040, vol,180
  .db eoff

  GOSUB blues_chorus_bass, 4
  .db 0

blues_chorus_bass::
  ; I I I I
  GOSUB walking_bass_2bars, 2
  .db tup, 5 ;c c# d d# e f

  ; IV IV I I
  GOSUB walking_bass_2bars, 1
  .db tup, 0
  GOSUB walking_bass_2bars, 1

  ; V IV I I
  .db tup, 7
  .db c20, e20, g20, a21
  .db tup, 5
  .db c20, e20, g20, a21
  .db tup, 0
  GOSUB walking_bass_2bars, 1
  .db 0

walking_bass_2bars::
  .db n4, 0x6F, c20, e20, f20, f21
  .db           g20, f21, f20, e20
  .db 00


bgm2::
  .dw bgm2_pattern0
  .dw bgm2_pattern0
  .dw bgm2_pattern0
  .dw 0xFF ; repeat
  .dw bgm2 ; where to repeat to
  .dw 0x00 ; song end code
bgm2_pattern0::
  .dw bgm2_channel0 ; played by channel 0
  .dw bgm2_channel1
  .dw 0x00
  .dw 0x00
  .dw 0x00
  .dw 0x00
  .dw 0x00
  .dw 0x00 ; channel 2-7 unused
bgm2_channel0::
  .db tempo,25,gvol,200 ; global song settings
  .db instr,0x2d, tun,040, vol,180
  .db eoff

  ;.db n1, 0x7F, c31, rest, c31, rest, c31, rest, c31, rest

; in theory I believe this should play 4 c30 whole notes, with some seperation because of the release time
; in practice this plays one continous tone and ignores the release time? :/
;  .db n1, 0x1F, c30
;  GOSUB repeat3times 1
;  .db rest

  ; (hack) play whole notes by playing half notes and adding a rest after every half note
  ; result is unsatisfactory
  ;.db n2, 0x7F
  ;NOTEREPT c30 4

  ; (hack improvement) whole note = 16 16-th notes == note, rept x 14, rest
    .db n16, 0x7F, c21
    GOSUB repeat3times 2
    GOSUB repeat4times 2
    .db rest

; play 4 whole notes c20 subdivided into 16-ths
.rept 4
    .db n16, 0x7F, c20
    GOSUB repeat3times 2
    GOSUB repeat4times 2
    .db rest
.endm

  ;play 4 whole notes by always resetting note duration (best approach?)
  .db n1, d30
  .db n1, d30
  .db n1, d30
  .db n1, d30

  .db n4, 0x17, f30
  .db n4, f30
  .db n4, f30
  .db n4, f30
  .db rest

  .db n4, e30
  GOSUB repeat3times 1
  GOSUB repeat4times 3
  .db rest

  .db n8, f30
  GOSUB repeat3times 1
  GOSUB repeat4times 7
  .db rest

  .db 00 ; phrase end code

bgm2_channel1::
  .db instr,0x2d, tun,040, vol,180

  ; 1 quarth = 4/16 == note, rept x 2, rest
  .db n16, 0x7F, c31, rept, rept, rest
.rept 3
  .db c31, rept, rept, rest
.endm

.rept 16
  .db c30, rept, rept, rest
.endm
  ; rest 16 bars
  .db n1
.rept 16
  .db rest
.endm
  .db 00


repeat3times::
  .db rept, rept, rept
  .db 00
repeat4times::
  .db rept, rept, rept, rept
  .db 00
my_data_end::


