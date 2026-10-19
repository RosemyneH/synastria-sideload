%include "c32.mac"

; ==========================================================================================
; GLOBAL UNINITIALIZED DATA
; ==========================================================================================

[SEGMENT .bss use32 align=32]

; ==========================================================================================
; GLOBAL PREINITIALIZED DATA
; ==========================================================================================

section .text
align   16

negones dd  -32768.0, -32768.0, -32768.0, -32768.0
ones    dd   32767.0,  32767.0,  32767.0,  32767.0

; ==========================================================================================
; CODE
; ==========================================================================================

section .text

; =================================================================================================================================
; void FMOD_DSP_Convert_FloatToPCM16(short *outbuffer, float *inbuffer, unsigned int length, int destchannelstep, int srcchannelstep, float volume);
; =================================================================================================================================
; VERIFY: 87e240f3 auto
; OFFSET: 0x00951810
proc FMOD_DSP_Convert_FloatToPCM16

		%$outbuffer       arg 
		%$inbuffer        arg 
		%$length          arg 
		%$destchannelstep arg
		%$srcchannelstep  arg
        %$volume          arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

        ; xmm0 = [          ][          ][          ][          ]
        ; xmm1 = [          ][          ][          ][          ]
        ; xmm2 = [          ][          ][          ][          ]
        ; xmm3 = [          ][          ][          ][          ]
        ; xmm4 = [          ][          ][          ][          ]
        ; xmm5 = [          ][          ][          ][          ]
        ; xmm6 = [          ][          ][          ][          ]
        ; xmm7 = [          ][          ][          ][          ]

        mov     esi, [ebp+%$inbuffer]
        mov     edi, [ebp+%$outbuffer]

        mov     eax, [ebp+%$srcchannelstep]
        shl     eax, 2
        mov     ebx, [ebp+%$destchannelstep]
        shl     ebx, 1

        movss   xmm4, [ebp+%$volume]
        shufps  xmm4, xmm4, 0
        movaps  xmm5, [negones]
        movaps  xmm6, [ones]

%if 1
        mov     ecx, [ebp+%$length]
        shr     ecx, 2
        test    ecx, ecx
        jz      near convertf2int16rolledstart

convertf2int16unrolled:

        movss     xmm0, [esi]
        movss     xmm1, [esi + eax]
        lea       esi, [esi + eax * 2]
        movss     xmm2, [esi]
        movss     xmm3, [esi + eax]
        lea       esi, [esi + eax * 2]
        
        unpcklps  xmm0, xmm1
        unpcklps  xmm2, xmm3
        shufps    xmm0, xmm2, 044h
       
        mulps     xmm0, xmm4      ; now from -1/+1 to -32768/+32767 (and may be outside of this range)
        
        movhlps   xmm1, xmm0

        cvttps2pi mm0, xmm0
        cvttps2pi mm1, xmm1

        packssdw  mm0, mm0    ; clamps to -32768 to 32767
        packssdw  mm1, mm1    ; clamps to -32768 to 32767

        movd      edx, mm0
        mov       [edi + 0], dl
        mov       [edi + 1], dh
        shr       edx, 16
        mov       [edi + ebx + 0], dl
        mov       [edi + ebx + 1], dh
        lea       edi, [edi + ebx * 2]
        
        movd      edx, mm1
        mov       [edi + 0], dl
        mov       [edi + 1], dh
        shr       edx, 16
        mov       [edi + ebx + 0], dl
        mov       [edi + ebx + 1], dh
        lea       edi, [edi + ebx * 2]         		

		dec		  ecx
		jnz near  convertf2int16unrolled		

convertf2int16rolledstart:

		mov     ecx, [ebp+%$length]
        and     ecx, 3
%endif
        test    ecx, ecx
        jz      near convertf2int16done

convertf2int16rolled:

        movss     xmm0, [esi]
        mulps     xmm0, xmm4      ; now from -1/+1 to -32768/+32767 (and may be outside of this range)
        cvttps2pi mm0, xmm0
        packssdw  mm0, mm0    ; clamps to -32768 to 32767
        movd      edx, mm0
        mov       [edi + 0], dl
        mov       [edi + 1], dh
        		
        add     edi, ebx
        add     esi, eax
		dec		ecx
		jnz		near convertf2int16rolled

convertf2int16done:

        emms

		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax
endproc

