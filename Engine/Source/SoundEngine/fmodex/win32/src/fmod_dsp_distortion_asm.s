%include "c32.mac"

; ==========================================================================================
; GLOBAL UNINITIALIZED DATA
; ==========================================================================================

[SEGMENT .data use32 align=32]

distortion_ones     dd 1.0,1.0,1.0,1.0
distortion_fabsmask dd 7FFFFFFFh, 7FFFFFFFh, 7FFFFFFFh, 7FFFFFFFh

; ==========================================================================================
; CODE
; ==========================================================================================

[SEGMENT .text use32 align=32]

; =================================================================================================================================
; void FMOD_DSP_Distortion_SIMD(float * inbuffer, float * outbuffer, unsigned int length, int inchannels, int outchannels, float k);
; =================================================================================================================================
; VERIFY: cfd29e0d auto
; OFFSET: 0x00929700
proc FMOD_DSP_Distortion_SIMD

		%$inbuffer		arg 
		%$outbuffer		arg 
		%$length		arg 
		%$inchannels	arg
		%$outchannels	arg
		%$k         	arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

        mov     esi, [ebp+%$inbuffer]
        mov     edi, [ebp+%$outbuffer]

        ; xmm0 = [0x7fffffff][0x7fffffff][0x7fffffff][0x7fffffff]
        ; xmm1 = [k         ][k         ][k         ][k         ]
        ; xmm2 = [k+1       ][k+1       ][k+1       ][k+1       ]
        ; xmm3 = 
        ; xmm4 = 
        ; xmm5 = [1.0f      ][1.0f      ][1.0f      ][1.0f      ]
        ; xmm6 = 
        ; xmm7 = 

        movaps  xmm0, [distortion_fabsmask]
		movss	xmm1, [ebp+%$k]
        shufps  xmm1, xmm1, 0x00
        movaps  xmm2, xmm1
        movaps  xmm5, [distortion_ones]
        addps   xmm2, xmm5

		mov     edx, [ebp+%$length]
 		imul    edx, [ebp+%$inchannels]
        mov     ecx, edx
        shr     ecx, 3
        test    ecx, ecx
        jz      distortionlooprolledstart

distortionloopunrolled:

        movups  xmm3, [esi]
        movups  xmm4, xmm3
        andps   xmm3, xmm0
        mulps   xmm3, xmm1
        addps   xmm3, xmm5
        rcpps   xmm3, xmm3      ; <--- this is the reciprical calc.
        mulps   xmm4, xmm2      ;     |
;        divps   xmm4, xmm3     ;     |
        mulps   xmm4, xmm3      ; reciprical multiply instead of a div.  Only 12bit accuracy but good enough for here.  much faster.
        movups  [edi], xmm4

        movups  xmm6, [esi+16]
        movups  xmm7, xmm6
        andps   xmm6, xmm0
        mulps   xmm6, xmm1
        addps   xmm6, xmm5
        rcpps   xmm6, xmm6      ; <--- this is the reciprical calc.
        mulps   xmm7, xmm2      ;     |
;        divps   xmm7, xmm6     ;     |
        mulps   xmm7, xmm6      ; reciprical multiply instead of a div.  Only 12bit accuracy but good enough for here.  much faster.
        movups  [edi+16], xmm7
        		
        add     edi, 32
        add     esi, 32
		dec		ecx
		jnz		near distortionloopunrolled		

distortionlooprolledstart:

        xorps   xmm3, xmm3

        mov     ecx, edx
        and     ecx, 7
        test    ecx, ecx
        jz      distortiondone

distortionlooprolled:

        movss   xmm3, [esi]
        movss   xmm4, xmm3
        andps   xmm3, xmm0
        mulss   xmm3, xmm1
        addss   xmm3, xmm5
        rcpss   xmm3, xmm3      ; <--- this is the reciprical calc.
        mulss   xmm4, xmm2      ;     |
;        divss   xmm4, xmm3     ;     |
        mulss   xmm4, xmm3      ; reciprical multiply instead of a div.  Only 12bit accuracy but good enough for here.  much faster.
        movss  [edi], xmm4

        add     edi, 4
        add     esi, 4

		dec		ecx
		jnz		near distortionlooprolled

distortiondone:

		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax
endproc
