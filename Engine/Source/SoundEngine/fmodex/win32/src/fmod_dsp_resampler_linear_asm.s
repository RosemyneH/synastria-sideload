%include "c32.mac"

; ==========================================================================================
; GLOBAL UNINITIALIZED DATA
; ==========================================================================================

section .bss 
align   16

%define samples         esp+0
%define samples_2       esp+10h
%define samplesplus1    esp+20h
%define samplesplus1_2  esp+30h
%define fracs           esp+40h
%define fracs_2         esp+50h

; ==========================================================================================
; GLOBAL PREINITIALIZED DATA
; ==========================================================================================

section .text
align   16

ones_over_32768 dd   0.000030517578125,                 0.000030517578125,                 0.000030517578125,                 0.000030517578125
ones_over_2gig  dd   0.0000000004656612873077392578125, 0.0000000004656612873077392578125, 0.0000000004656612873077392578125, 0.0000000004656612873077392578125
ones            dd   1.0, 1.0, 1.0, 1.0

; ==========================================================================================
; CODE
; ==========================================================================================

section .text

; =================================================================================================================================
; void FMOD_Resampler_Linear_PCM16_Mono(float *out, int outlength, void *src, FMOD_UINT64P *position, FMOD_SINT64P *speed);
; =================================================================================================================================
; VERIFY: 87446a50 auto
; OFFSET: 0x00951920
proc FMOD_Resampler_Linear_PCM16_Mono

		%$out           arg 
		%$outlength     arg 
		%$src           arg 
		%$position      arg
		%$speed         arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

        mov     eax, [ebp + %$position]
        mov     eax, [eax]

        mov     esi, [ebp + %$src]
        shr     esi, 1
        mov     ebx, [ebp + %$position]     
        add     esi, [ebx + 4]               ; esi is now addr / 2 + offset.  when accessing we use [esi*2]

        mov     edx, [ebp + %$speed]
        mov     edx, [edx + 4]
        mov     ebx, [ebp + %$speed]
        mov     ebx, [ebx]

        mov     edi, [ebp + %$out]
		mov     ecx, [ebp + %$outlength]

		; eax  = poslo
		; ebx  = speedlo
		; ecx  = count
		; edx  = speedhi
		; esi  = src + poshi
		; edi  = dest

        ; xmm0 = [          ][          ][          ][          ]
        ; xmm1 = [          ][          ][          ][          ]
        ; xmm2 = [          ][          ][          ][          ]
        ; xmm3 = [          ][          ][          ][          ]
        ; xmm4 = [          ][          ][          ][          ]
        ; xmm5 = [          ][          ][          ][          ]
        ; xmm6 = [          ][          ][          ][          ]
        ; xmm7 = [          ][          ][          ][          ]

        push    ebp
        sub     esp, 60h

        xorps    xmm0, xmm0
        movups   [samples], xmm0
        movups   [samples_2], xmm0
        movups   [samplesplus1], xmm0
        movups   [samplesplus1_2], xmm0
        movups   [fracs], xmm0
        movups   [fracs_2], xmm0

%if 1
        shr     ecx, 2
        test    ecx, ecx
        jz      near resample16Mrolledstart

resample16Munrolled:

        movsx    ebp, word [esi*2]
        mov      [samples      + 0], ebp       ; [        1a][          ][          ][          ]
        movsx    ebp, word [esi*2+2]
        mov      [samplesplus1 + 0], ebp       ; [        1b][          ][          ][          ]
        mov      ebp, eax                     
        shr      ebp, 1
        mov      [fracs + 0], ebp              ; [        1f][          ][          ][          ]
        add      eax, ebx
        adc      esi, edx
                 
        movsx    ebp, word [esi*2]
        mov      [samples_2      + 0], ebp     ; [        2a][          ][          ][          ]
        movsx    ebp, word [esi*2+2]
        mov      [samplesplus1_2 + 0], ebp     ; [        2b][          ][          ][          ]
        mov      ebp, eax                     
        shr      ebp, 1
        mov      [fracs_2 + 0], ebp            ; [        2f][          ][          ][          ]
        add      eax, ebx
        adc      esi, edx
                 
        movsx    ebp, word [esi*2]
        mov      [samples      + 4], ebp      ; [        1a][        3a][          ][          ]
        movsx    ebp, word [esi*2+2]
        mov      [samplesplus1 + 4], ebp      ; [        1b][        3b][          ][          ]
        mov      ebp, eax                     
        shr      ebp, 1
        mov      [fracs + 4], ebp             ; [        1f][        3f][          ][          ]
        add      eax, ebx
        adc      esi, edx

        movsx    ebp, word [esi*2]
        mov      [samples_2      + 4], ebp    ; [        2a][        4a][          ][          ]
        movsx    ebp, word [esi*2+2]
        mov      [samplesplus1_2 + 4], ebp    ; [        2b][        4b][          ][          ]
        mov      ebp, eax                     
        shr      ebp, 1
        mov      [fracs_2 + 4], ebp           ; [        2f][        4f][          ][          ]
        add      eax, ebx
        adc      esi, edx
              
        cvtpi2ps xmm0, [samples]
        cvtpi2ps xmm1, [samples_2]
        cvtpi2ps xmm2, [samplesplus1]
        cvtpi2ps xmm3, [samplesplus1_2]
        cvtpi2ps xmm4, [fracs]
        cvtpi2ps xmm5, [fracs_2]

        unpcklps xmm0, xmm1                   ; [        1a][        2a][        3a][        4a]
        unpcklps xmm2, xmm3                   ; [        1b][        2b][        3b][        4b]
        unpcklps xmm4, xmm5                   ; [        1f][        2f][        3f][        4f]

        mulps    xmm0, [ones_over_32768]
        mulps    xmm2, [ones_over_32768]
        mulps    xmm4, [ones_over_2gig]

        movaps   xmm6, [ones]
        subps    xmm6, xmm4                  ; [     1.0-1f][    1.0-2f][    1.0-3f][    1.0-4f]

        mulps    xmm2, xmm4                  ; 
        mulps    xmm0, xmm6                  ; [1a * 1.0-1f][2a *1.0-1f][3a *1.0-3f][4a * 1.0-4f]
        addps    xmm0, xmm2
        movups  [edi], xmm0
        		
        add     edi, 16
		dec		ecx
		jnz		near resample16Munrolled		

resample16Mrolledstart:

        add     esp, 60h
        pop     ebp
		mov     ecx, [ebp+%$outlength]
        push    ebp
        sub     esp, 60h
        and     ecx, 3
%endif
        xorps   xmm0, xmm0
        xorps   xmm2, xmm2
        xorps   xmm4, xmm4
        test    ecx, ecx
        jz      near resample16Mdone

resample16Mrolled:

        movsx    ebp, word [esi*2]
        mov      [samples      + 0], ebp       ; [        1a][          ][          ][          ]
        movsx    ebp, word [esi*2+2]
        mov      [samplesplus1 + 0], ebp       ; [        1b][          ][          ][          ]
        mov      ebp, eax                     
        shr      ebp, 1
        mov      [fracs + 0], ebp              ; [        1f][          ][          ][          ]
        add      eax, ebx
        adc      esi, edx
                              
        cvtsi2ss xmm0, [samples]
        cvtsi2ss xmm2, [samplesplus1]
        cvtsi2ss xmm4, [fracs]

        mulss    xmm0, [ones_over_32768]
        mulss    xmm2, [ones_over_32768]
        mulss    xmm4, [ones_over_2gig]

        movss    xmm6, [ones]
        subss    xmm6, xmm4                  ; [     1.0-1f][    1.0-2f][    1.0-3f][    1.0-4f]

        mulss    xmm2, xmm4                  ; 
        mulss    xmm0, xmm6                  ; [1a * 1.0-1f][2a *1.0-1f][3a *1.0-3f][4a * 1.0-4f]
        addss    xmm0, xmm2
        movss   [edi], xmm0
        		
        add     edi, 4
		dec		ecx
		jnz		near resample16Mrolled

resample16Mdone:
        add     esp, 60h
        pop     ebp

        ; restore position 
        mov     ecx, [ebp + %$position]
        mov    [ecx + 0], eax
        
        mov     eax, [ebp + %$src]
        shr     eax, 1
        sub     esi, eax
        mov     ecx, [ebp + %$position]
        mov     [ecx + 4], esi

		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax
endproc


; =================================================================================================================================
; void FMOD_Resampler_Linear_PCM16_Stereo(float *out, int outlength, void *src, FMOD_UINT64P *position, FMOD_SINT64P *speed);
; =================================================================================================================================
; VERIFY: beb36440 auto
; OFFSET: 0x00951B04
proc FMOD_Resampler_Linear_PCM16_Stereo

		%$out           arg 
		%$outlength     arg 
		%$src           arg 
		%$position      arg
		%$speed         arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

        mov     eax, [ebp + %$position]
        mov     eax, [eax]

        mov     esi, [ebp + %$src]
        shr     esi, 2
        mov     ebx, [ebp + %$position]     
        add     esi, [ebx + 4]               ; esi is now addr / 2 + offset.  when accessing we use [esi*2]

        mov     edx, [ebp + %$speed]
        mov     edx, [edx + 4]
        mov     ebx, [ebp + %$speed]
        mov     ebx, [ebx]

        mov     edi, [ebp + %$out]
		mov     ecx, [ebp + %$outlength]

        push    ebp
        sub     esp, 60h

		; eax  = poslo
		; ebx  = speedlo
		; ecx  = count
		; edx  = speedhi
		; esi  = src + poshi
		; edi  = dest
        ; ebp  = temp

        ; xmm0 = [          ][          ][          ][          ]
        ; xmm1 = [          ][          ][          ][          ]
        ; xmm2 = [          ][          ][          ][          ]
        ; xmm3 = [          ][          ][          ][          ]
        ; xmm4 = [          ][          ][          ][          ]
        ; xmm5 = [          ][          ][          ][          ]
        ; xmm6 = [          ][          ][          ][          ]
        ; xmm7 = [          ][          ][          ][          ]

        xorps    xmm0, xmm0
        movups   [samples], xmm0
        movups   [samples_2], xmm0
        movups   [samplesplus1], xmm0
        movups   [samplesplus1_2], xmm0
        movups   [fracs], xmm0
        movups   [fracs_2], xmm0

%if 1
        shr     ecx, 1
        test    ecx, ecx
        jz      near resample16Srolledstart

resample16Sunrolled:

        movsx    ebp, word [esi*4+0]
        mov      [samples      + 0], ebp       ; [       1al][          ][          ][          ]
        movsx    ebp, word [esi*4+2]
        mov      [samples      + 4], ebp       ; [       1al][       1ar][          ][          ]
        movsx    ebp, word [esi*4+4]
        mov      [samplesplus1 + 0], ebp       ; [       1bl][          ][          ][          ]
        movsx    ebp, word [esi*4+6]
        mov      [samplesplus1 + 4], ebp       ; [       1bl][       1br][          ][          ]
        mov      ebp, eax                     
        shr      ebp, 1
        mov      [fracs + 0], ebp              ; [        1f][          ][          ][          ]
        mov      [fracs + 4], ebp              ; [        1f][        1f][          ][          ]
        add      eax, ebx
        adc      esi, edx
                 
        movsx    ebp, word [esi*4+0]
        mov      [samples_2     + 0], ebp      ; [       2al][          ][          ][          ]
        movsx    ebp, word [esi*4+2]
        mov      [samples_2     + 4], ebp      ; [       2al][       2ar][          ][          ]
        movsx    ebp, word [esi*4+4]
        mov      [samplesplus1_2 + 0], ebp     ; [       2bl][          ][          ][          ]
        movsx    ebp, word [esi*4+6]
        mov      [samplesplus1_2 + 4], ebp     ; [       2bl][       2br][          ][          ]
        mov      ebp, eax                     
        shr      ebp, 1
        mov      [fracs_2 + 0], ebp            ; [        2f][          ][          ][          ]
        mov      [fracs_2 + 4], ebp            ; [        2f][        2f][          ][          ]
        add      eax, ebx
        adc      esi, edx
                              
        cvtpi2ps xmm0, [samples]
        cvtpi2ps xmm1, [samples_2]
        cvtpi2ps xmm2, [samplesplus1]
        cvtpi2ps xmm3, [samplesplus1_2]
        cvtpi2ps xmm4, [fracs]
        cvtpi2ps xmm5, [fracs_2]

        shufps   xmm0, xmm1, 44h       
        shufps   xmm2, xmm3, 44h
        shufps   xmm4, xmm5, 44h

        mulps    xmm0, [ones_over_32768]
        mulps    xmm2, [ones_over_32768]
        mulps    xmm4, [ones_over_2gig]

        movaps   xmm6, [ones]
        subps    xmm6, xmm4                  ; [     1.0-1f][    1.0-1f][    1.0-2f][    1.0-2f]

        mulps    xmm2, xmm4                  ; 
        mulps    xmm0, xmm6                  ; [1al* 1.0-1f][1ar*1.0-1f][2al*1.0-2f][2ar* 1.0-2f]
        addps    xmm0, xmm2
        movups  [edi], xmm0
        		
        add     edi, 16
		dec		ecx
		jnz		near resample16Sunrolled		

resample16Srolledstart:

        add     esp, 60h
        pop     ebp
		mov     ecx, [ebp+%$outlength]
        push    ebp
        sub     esp, 60h
        and     ecx, 1
%endif
        xorps   xmm0, xmm0
        xorps   xmm2, xmm2
        xorps   xmm4, xmm4
        test    ecx, ecx
        jz      near resample16Sdone

resample16Srolled:

        movsx    ebp, word [esi*4]
        mov      [samples      + 0], ebp       ; [       1al][          ][          ][          ]
        movsx    ebp, word [esi*4+2]
        mov      [samples      + 4], ebp       ; [       1al][       1ar][          ][          ]

        movsx    ebp, word [esi*4+4]
        mov      [samplesplus1 + 0], ebp       ; [       1bl][          ][          ][          ]
        movsx    ebp, word [esi*4+6]
        mov      [samplesplus1 + 4], ebp       ; [       1bl][       1br][          ][          ]

        mov      ebp, eax                     
        shr      ebp, 1
        mov      [fracs + 0], ebp              ; [        1f][          ][          ][          ]
        mov      [fracs + 4], ebp              ; [        1f][        1f][          ][          ]
        add      eax, ebx
        adc      esi, edx
                              
        cvtpi2ps xmm0, [samples]
        cvtpi2ps xmm2, [samplesplus1]
        cvtpi2ps xmm4, [fracs]

        mulps    xmm0, [ones_over_32768]
        mulps    xmm2, [ones_over_32768]
        mulps    xmm4, [ones_over_2gig]
        movups   xmm6, [ones]
        subps    xmm6, xmm4                  ; [     1.0-1f][    1.0-1f][          ][          ]

        mulps    xmm2, xmm4                  ; 
        mulps    xmm0, xmm6                  ; [1a * 1.0-1f][2a *1.0-1f][3a *1.0-3f][4a * 1.0-4f]
        addps    xmm0, xmm2
        movlps  [edi],  xmm0
        		
        add     edi, 8
		dec		ecx
		jnz		near resample16Srolled

resample16Sdone:

        add     esp, 60h
        pop     ebp

        ; restore position 
        mov     ecx, [ebp + %$position]
        mov    [ecx + 0], eax
        
        mov     eax, [ebp + %$src]
        shr     eax, 2
        sub     esi, eax
        mov     ecx, [ebp + %$position]
        mov     [ecx + 4], esi

		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax
endproc

