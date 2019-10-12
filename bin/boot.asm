 OUTPUT boot.com

 ORG 100h

;    ld c,9
;    ld de,textstring
;    call 05
;    ld c,0

    rst $30
    db  %10000000
    dw  $0000

;textstring :
; db 'Press a key to reset',13,10,'$'


