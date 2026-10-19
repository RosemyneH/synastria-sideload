%include "c32.mac"

; ==========================================================================================
; GLOBAL UNINITIALIZED DATA
; ==========================================================================================

[SEGMENT .data use32 align=32]

; ==========================================================================================
; CODE
; ==========================================================================================

[SEGMENT .text use32 align=32]


; =================================================================================================================================
;  void FMOD_DSP_Connection_MixMonoToStereo_SIMD  (float *inbuffer, float *outbuffer, unsigned int length, float lvolume, float rvolume);
; =================================================================================================================================
; VERIFY: 76e75238 auto
; OFFSET: 0x00944B60
proc FMOD_DSP_Connection_MixMonoToStereo_SIMD

		%$inbuffer		arg 
		%$outbuffer		arg 
		%$length		arg 
		%$lvolume       arg
		%$rvolume       arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

        mov     esi, [ebp+%$inbuffer]
        mov     edi, [ebp+%$outbuffer]

        ; xmm0 = [lvolume   ][rvolume   ][lvolume   ][rvolume   ]
        ; xmm1 = [samp1     ][samp1     ][samp2     ][samp2     ]
        ; xmm2 = [samp3     ][samp3     ][samp4     ][samp4     ]
        ; xmm3 = [          ][          ][          ][          ]
        ; xmm4 = [          ][          ][          ][          ]
        ; xmm5 = [          ][          ][          ][          ]
        ; xmm6 = [          ][          ][          ][          ]
        ; xmm7 = [          ][          ][          ][          ]

        movss    xmm0, [ebp+%$lvolume]
        movss    xmm1, [ebp+%$rvolume]
        shufps   xmm0, xmm0, 0
        shufps   xmm1, xmm1, 0
        unpcklps xmm0, xmm1

		mov     edx, [ebp+%$length]
        mov     ecx, edx
%if 1
        shr     ecx, 2
        test    ecx, ecx
        jz      mixloopMtoSrolledstart

mixloopMtoSunrolled:

        movups   xmm1, [esi]
        movaps   xmm2, xmm1

        movaps   xmm3, [edi]
        movaps   xmm4, [edi + 16]

        unpcklps xmm1, xmm1
        unpckhps xmm2, xmm2

        mulps    xmm1, xmm0
        mulps    xmm2, xmm0

        addps   xmm3, xmm1
        addps   xmm4, xmm2

        movaps  [edi], xmm3
        movaps  [edi+16], xmm4
        		
        add     edi, 32
        add     esi, 16
		dec		ecx
		jnz		near mixloopMtoSunrolled		

mixloopMtoSrolledstart:

        mov     ecx, edx
        and     ecx, 3
%endif
        test    ecx, ecx
        jz      mixMtoSdone

mixloopMtoSrolled:

        movss    xmm1, [esi]
        movss    xmm2, xmm1

        movss    xmm3, [edi]
        movss    xmm4, [edi+4]

        mulss    xmm1, [ebp+%$lvolume]
        mulss    xmm2, [ebp+%$rvolume]

        addss    xmm3, xmm1
        addss    xmm4, xmm2

        movss   [edi], xmm3
        movss   [edi+4], xmm4

        add     edi, 8
        add     esi, 4

		dec		ecx
		jnz		near mixloopMtoSrolled

mixMtoSdone:

		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax
endproc

; =================================================================================================================================
;  void FMOD_DSP_Connection_MixStereoToStereo_SIMD(float *inbuffer, float *outbuffer, unsigned int length, float lvolume, float rvolume);
; =================================================================================================================================
; VERIFY: e7af180e auto
; OFFSET: 0x00944C1C
proc FMOD_DSP_Connection_MixStereoToStereo_SIMD

		%$inbuffer		arg 
		%$outbuffer		arg 
		%$length		arg 
		%$lvolume       arg
		%$rvolume       arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

        mov     esi, [ebp+%$inbuffer]
        mov     edi, [ebp+%$outbuffer]

        ; xmm0 = [lvolume   ][rvolume   ][lvolume   ][rvolume   ]
        ; xmm1 = [samp1     ][samp1     ][samp2     ][samp2     ]
        ; xmm2 = [samp3     ][samp3     ][samp4     ][samp4     ]
        ; xmm3 = [          ][          ][          ][          ]
        ; xmm4 = [          ][          ][          ][          ]
        ; xmm5 = [          ][          ][          ][          ]
        ; xmm6 = [          ][          ][          ][          ]
        ; xmm7 = [          ][          ][          ][          ]

        movss    xmm0, [ebp+%$lvolume]
        movss    xmm1, [ebp+%$rvolume]
        shufps   xmm0, xmm0, 0
        shufps   xmm1, xmm1, 0
        unpcklps xmm0, xmm1

		mov     edx, [ebp+%$length]
        mov     ecx, edx
%if 1
        shr     ecx, 2
        test    ecx, ecx
        jz      mixloopStoSrolledstart

mixloopStoSunrolled:

        movups   xmm1, [esi]
        movups   xmm2, [esi+16]

        movaps   xmm3, [edi]
        movaps   xmm4, [edi+16]

        mulps    xmm1, xmm0
        mulps    xmm2, xmm0

        addps   xmm3, xmm1
        addps   xmm4, xmm2

        movaps  [edi], xmm3
        movaps  [edi+16], xmm4
        		
        add     edi, 32
        add     esi, 32
		dec		ecx
		jnz		near mixloopStoSunrolled		

mixloopStoSrolledstart:

        mov     ecx, edx
        and     ecx, 3
%endif
        test    ecx, ecx
        jz      mixStoSdone

mixloopStoSrolled:

        movss    xmm1, [esi]
        movss    xmm2, [esi+4]

        movss    xmm3, [edi]
        movss    xmm4, [edi+4]

        mulss    xmm1, [ebp+%$lvolume]
        mulss    xmm2, [ebp+%$rvolume]

        addss    xmm3, xmm1
        addss    xmm4, xmm2

        movss   [edi], xmm3
        movss   [edi+4], xmm4

        add     edi, 8
        add     esi, 8

		dec		ecx
		jnz		near mixloopStoSrolled

mixStoSdone:

		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax

endproc


; =================================================================================================================================
;  void FMOD_DSP_Connection_MixMonoTo5_1_SIMD(float *inbuffer, float *outbuffer, unsigned int length, float *volume0to3, float *volume4to1, float *volume2to5);
; =================================================================================================================================
; VERIFY: 186c834c auto
; OFFSET: 0x00944CD4
proc FMOD_DSP_Connection_MixMonoTo5_1_SIMD

		%$inbuffer		arg 
		%$outbuffer		arg 
		%$length		arg 
        %$volume0to3    arg
        %$volume4to1    arg
        %$volume2to5    arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

        mov     esi, [ebp+%$inbuffer]
        mov     edi, [ebp+%$outbuffer]

        ; xmm0 = [l00       ][l10       ][l20       ][l30       ]
        ; xmm1 = [l40       ][l50       ][l00       ][l10       ]
        ; xmm2 = [l20       ][l30       ][l40       ][l50       ]
        ; xmm3 = [          ][          ][          ][          ]
        ; xmm4 = [          ][          ][          ][          ]
        ; xmm5 = [          ][          ][          ][          ]
        ; xmm6 = [          ][          ][          ][          ]
        ; xmm7 = [          ][          ][          ][          ]

		mov     edx, [ebp+%$length]
        mov     ecx, edx

        mov     eax, [ebp+%$volume0to3]
        movups  xmm0, [eax]
        mov     eax, [ebp+%$volume4to1]
        movups  xmm1, [eax]
        mov     eax, [ebp+%$volume2to5]
        movups  xmm2, [eax]

%if 1
        shr     ecx, 2
        test    ecx, ecx
        jz      near mixloopMto51rolledstart

mixloopMto51unrolled:

        movss    xmm3, [esi + 0]
        movlps   xmm4, [esi + 0]
        shufps   xmm3, xmm3, 0
        shufps   xmm4, xmm4, 50h
        mulps    xmm3, xmm0
        mulps    xmm4, xmm1
        addps    xmm3, [edi]
        addps    xmm4, [edi + 16]
        movaps   [edi], xmm3
        movaps   [edi+16], xmm4

        movss    xmm3, [esi + 4]
        movss    xmm4, [esi + 8]
        shufps   xmm3, xmm3, 0
        shufps   xmm4, xmm4, 0
        mulps    xmm3, xmm2
        mulps    xmm4, xmm0
        addps    xmm3, [edi + 32]
        addps    xmm4, [edi + 48]
        movaps   [edi+32], xmm3
        movaps   [edi+48], xmm4

        movlps   xmm3, [esi + 8]
        movss    xmm4, [esi + 12]
        shufps   xmm3, xmm3, 50h
        shufps   xmm4, xmm4, 0
        mulps    xmm3, xmm1
        mulps    xmm4, xmm2
        addps    xmm3, [edi + 64]
        addps    xmm4, [edi + 80]
        movaps   [edi+64], xmm3
        movaps   [edi+80], xmm4
        		
        add     edi, 96
        add     esi, 16
		dec		ecx
		jnz		near mixloopMto51unrolled		

mixloopMto51rolledstart:

        mov     ecx, ebp
        and     ecx, 3
%endif
        test    ecx, ecx
        jz      mixMto51done

mixloopMto51rolled:

        movss    xmm3, [esi + 0]
        shufps   xmm3, xmm3, 0
        movups   xmm4, [edi]
        mulps    xmm3, xmm0
        addps    xmm4, xmm3
        movups   [edi], xmm4

        movlps   xmm3, [esi + 0]
        shufps   xmm3, xmm3, 50h
        movlps   xmm4, [edi + 16]
        mulps    xmm3, xmm1
        addps    xmm4, xmm3
        movlps   [edi + 16], xmm4

        add     edi, 24
        add     esi, 4

		dec		ecx
		jnz		near mixloopMto51rolled

mixMto51done:

		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax

endproc


; =================================================================================================================================
;  void FMOD_DSP_Connection_Mix5_1To5_1_SIMD(float *inbuffer, float *outbuffer, unsigned int length, float *volume0to3, float *volume4to1, float *volume2to5);
; =================================================================================================================================
; VERIFY: 33f6b5a1 auto
; OFFSET: 0x00944DDA
proc FMOD_DSP_Connection_Mix5_1To5_1_SIMD

		%$inbuffer		arg 
		%$outbuffer		arg 
		%$length		arg 
        %$volume0to3    arg
        %$volume4to1    arg
        %$volume2to5    arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

        mov     esi, [ebp+%$inbuffer]
        mov     edi, [ebp+%$outbuffer]

        ; xmm0 = [l00       ][l10       ][l20       ][l30       ]
        ; xmm1 = [l40       ][l50       ][l00       ][l10       ]
        ; xmm2 = [l20       ][l30       ][l40       ][l50       ]
        ; xmm3 = [          ][          ][          ][          ]
        ; xmm4 = [          ][          ][          ][          ]
        ; xmm5 = [          ][          ][          ][          ]
        ; xmm6 = [          ][          ][          ][          ]
        ; xmm7 = [          ][          ][          ][          ]

		mov     edx, [ebp+%$length]
        mov     ecx, edx

        mov     eax, [ebp+%$volume0to3]
        movups  xmm0, [eax]
        mov     eax, [ebp+%$volume4to1]
        movups  xmm1, [eax]
        mov     eax, [ebp+%$volume2to5]
        movups  xmm2, [eax]

%if 1
        shr     ecx, 2
        test    ecx, ecx
        jz      near mixloop51to51rolledstart

mixloop51to51unrolled:

        movaps   xmm3, [esi + 0]
        movaps   xmm4, [esi + 16]
        mulps    xmm3, xmm0
        mulps    xmm4, xmm1
        addps    xmm3, [edi + 0]
        addps    xmm4, [edi + 16]
        movaps   [edi], xmm3
        movaps   [edi+16], xmm4

        movaps   xmm3, [esi + 32]
        movaps   xmm4, [esi + 48]
        mulps    xmm3, xmm2
        mulps    xmm4, xmm0
        addps    xmm3, [edi + 32]
        addps    xmm4, [edi + 48]
        movaps   [edi+32], xmm3
        movaps   [edi+48], xmm4

        movaps   xmm3, [esi + 64]
        movaps   xmm4, [esi + 80]
        mulps    xmm3, xmm1
        mulps    xmm4, xmm2
        addps    xmm3, [edi + 64]
        addps    xmm4, [edi + 80]
        movaps   [edi+64], xmm3
        movaps   [edi+80], xmm4
        		
        add     edi, 96
        add     esi, 96
		dec		ecx
		jnz		near mixloop51to51unrolled		

mixloop51to51rolledstart:

        mov     ecx, ebp
        and     ecx, 3
%endif
        test    ecx, ecx
        jz      mix51to51done

mixloop51to51rolled:

        movups   xmm3, [esi + 0]
        movups   xmm4, [edi + 0]
        mulps    xmm3, xmm0
        addps    xmm4, xmm3
        movups   [edi], xmm4

        movlps   xmm3, [esi + 16]
        movlps   xmm4, [edi + 16]
        mulps    xmm3, xmm1
        addps    xmm4, xmm3
        movlps   [edi+16], xmm4

        add     edi, 24
        add     esi, 24

		dec		ecx
		jnz		near mixloop51to51rolled

mix51to51done:

		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax

endproc


; =================================================================================================================================
;  void FMOD_DSP_Connection_MixMonoTo7_1_SIMD(float *inbuffer, float *outbuffer, unsigned int length, float *volume0to3, float *volume4to7);
; =================================================================================================================================
; VERIFY: 3df2e75a auto
; OFFSET: 0x00944EBD
proc FMOD_DSP_Connection_MixMonoTo7_1_SIMD

		%$inbuffer		arg 
		%$outbuffer		arg 
		%$length		arg 
        %$volume0to3    arg
        %$volume4to7    arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

        mov     esi, [ebp+%$inbuffer]
        mov     edi, [ebp+%$outbuffer]

        ; xmm0 = [l00       ][l10       ][l20       ][l30       ]
        ; xmm1 = [l40       ][l50       ][l00       ][l10       ]
        ; xmm2 = [l20       ][l30       ][l40       ][l50       ]
        ; xmm3 = [          ][          ][          ][          ]
        ; xmm4 = [          ][          ][          ][          ]
        ; xmm5 = [          ][          ][          ][          ]
        ; xmm6 = [          ][          ][          ][          ]
        ; xmm7 = [          ][          ][          ][          ]

		mov     edx, [ebp+%$length]
        mov     ecx, edx

        mov     eax, [ebp+%$volume0to3]
        movups  xmm0, [eax]
        mov     eax, [ebp+%$volume4to7]
        movups  xmm1, [eax]

%if 1
        shr     ecx, 2
        test    ecx, ecx
        jz      near mixloopMto71rolledstart

mixloopMto71unrolled:

        movss    xmm3, [esi + 0]
        shufps   xmm3, xmm3, 0
        movaps   xmm4, xmm3
        mulps    xmm3, xmm0
        mulps    xmm4, xmm1
        addps    xmm3, [edi + 0]
        addps    xmm4, [edi + 16]
        movaps   [edi+0], xmm3
        movaps   [edi+16], xmm4

        movss    xmm3, [esi + 4]
        shufps   xmm3, xmm3, 0
        movaps   xmm4, xmm3
        mulps    xmm3, xmm0
        mulps    xmm4, xmm1
        addps    xmm3, [edi + 32]
        addps    xmm4, [edi + 48]
        movaps   [edi+32], xmm3
        movaps   [edi+48], xmm4

        movss    xmm3, [esi + 8]
        shufps   xmm3, xmm3, 0
        movaps   xmm4, xmm3
        mulps    xmm3, xmm0
        mulps    xmm4, xmm1
        addps    xmm3, [edi + 64]
        addps    xmm4, [edi + 80]
        movaps   [edi+64], xmm3
        movaps   [edi+80], xmm4

        movss    xmm3, [esi + 12]
        shufps   xmm3, xmm3, 0
        movaps   xmm4, xmm3
        mulps    xmm3, xmm0
        mulps    xmm4, xmm1
        addps    xmm3, [edi + 96]
        addps    xmm4, [edi + 112]
        movaps   [edi+96], xmm3
        movaps   [edi+112], xmm4
        		
        add     edi, 128
        add     esi, 16
		dec		ecx
		jnz		near mixloopMto71unrolled		

mixloopMto71rolledstart:

        mov     ecx, ebp
        and     ecx, 3
%endif
        test    ecx, ecx
        jz      mixMto71done

mixloopMto71rolled:

        movss    xmm3, [esi + 0]
        shufps   xmm3, xmm3, 0
        movaps   xmm4, xmm3
        mulps    xmm3, xmm0
        mulps    xmm4, xmm1
        addps    xmm3, [edi + 0]
        addps    xmm4, [edi + 16]
        movaps   [edi+0], xmm3
        movaps   [edi+16], xmm4

        add     edi, 32
        add     esi, 4

		dec		ecx
		jnz		near mixloopMto71rolled

mixMto71done:

		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax

endproc


; =================================================================================================================================
;  void FMOD_DSP_Connection_Mix7_1To7_1_SIMD(float *inbuffer, float *outbuffer, unsigned int length, float *volume0to3, float *volume4to7);
; =================================================================================================================================
; VERIFY: 3ef53c49 auto
; OFFSET: 0x00944FC6
proc FMOD_DSP_Connection_Mix7_1To7_1_SIMD

		%$inbuffer		arg 
		%$outbuffer		arg 
		%$length		arg 
        %$volume0to3    arg
        %$volume4to7    arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

        mov     esi, [ebp+%$inbuffer]
        mov     edi, [ebp+%$outbuffer]

        ; xmm0 = [l00       ][l10       ][l20       ][l30       ]
        ; xmm1 = [l40       ][l50       ][l00       ][l10       ]
        ; xmm2 = [l20       ][l30       ][l40       ][l50       ]
        ; xmm3 = [          ][          ][          ][          ]
        ; xmm4 = [          ][          ][          ][          ]
        ; xmm5 = [          ][          ][          ][          ]
        ; xmm6 = [          ][          ][          ][          ]
        ; xmm7 = [          ][          ][          ][          ]

		mov     edx, [ebp+%$length]
        mov     ecx, edx

        mov     eax, [ebp+%$volume0to3]
        movups  xmm0, [eax]
        mov     eax, [ebp+%$volume4to7]
        movups  xmm1, [eax]

%if 1
        shr     ecx, 2
        test    ecx, ecx
        jz      near mixloop71to71rolledstart

mixloop71to71unrolled:

        movaps   xmm3, [esi + 0]
        movaps   xmm4, [esi + 16]
        mulps    xmm3, xmm0
        mulps    xmm4, xmm1
        addps    xmm3, [edi + 0]
        addps    xmm4, [edi + 16]
        movaps   [edi], xmm3
        movaps   [edi+16], xmm4

        movaps   xmm3, [esi + 32]
        movaps   xmm4, [esi + 48]
        mulps    xmm3, xmm0
        mulps    xmm4, xmm1
        addps    xmm3, [edi + 32]
        addps    xmm4, [edi + 48]
        movaps   [edi+32], xmm3
        movaps   [edi+48], xmm4

        movaps   xmm3, [esi + 64]
        movaps   xmm4, [esi + 80]
        mulps    xmm3, xmm0
        mulps    xmm4, xmm1
        addps    xmm3, [edi + 64]
        addps    xmm4, [edi + 80]
        movaps   [edi+64], xmm3
        movaps   [edi+80], xmm4

        movaps   xmm3, [esi + 96]
        movaps   xmm4, [esi + 112]
        mulps    xmm3, xmm0
        mulps    xmm4, xmm1
        addps    xmm3, [edi + 96]
        addps    xmm4, [edi + 112]
        movaps   [edi+96], xmm3
        movaps   [edi+112], xmm4
        		
        add     edi, 128
        add     esi, 128
		dec		ecx
		jnz		near mixloop71to71unrolled		

mixloop71to71rolledstart:

        mov     ecx, ebp
        and     ecx, 3
%endif
        test    ecx, ecx
        jz      mix71to71done

mixloop71to71rolled:

        movups   xmm3, [esi + 0]
        movups   xmm4, [edi + 0]
        mulps    xmm3, xmm0
        addps    xmm4, xmm3
        movups   [edi], xmm4

        movups   xmm3, [esi + 16]
        movups   xmm4, [edi + 16]
        mulps    xmm3, xmm1
        addps    xmm4, xmm3
        movups   [edi+16], xmm4

        add     edi, 32
        add     esi, 32

		dec		ecx
		jnz		near mixloop71to71rolled

mix71to71done:

		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax

endproc
