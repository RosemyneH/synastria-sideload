%include "c32.mac"

extern _FMOD_Mpeg_DecWin

section .data

scale32767 dd 32767.0

section .text

; VERIFY: f3ea0b91 auto
; OFFSET: 0x0094B920
proc FMOD_Mpeg_Synth_FPU

		%$b0            arg
		%$bo1           arg
		%$numchannels   arg
		%$samps         arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi
		push	eax

		mov		eax, [ebp+%$bo1]
		shl		eax, 2
		mov		edi, [ebp+%$numchannels]
		shl		edi, 1
		mov		ebx, [_FMOD_Mpeg_DecWin]
		add		ebx, strict dword 40h
		mov		esi, [ebp+%$b0]
		sub		ebx, eax
		mov		edx, [ebp+%$samps]
		mov		ecx, 10h

synthfpuloop1:
		fld		dword [ebx]
		fmul	dword [esi]
		fld		dword [ebx+8]
		fmul	dword [esi+8]
		fld		dword [ebx+10h]
		fmul	dword [esi+10h]
		fld		dword [ebx+18h]
		fmul	dword [esi+18h]
		fld		dword [ebx+20h]
		fmul	dword [esi+20h]
		fld		dword [ebx+28h]
		fmul	dword [esi+28h]
		fld		dword [ebx+30h]
		fmul	dword [esi+30h]
		fld		dword [ebx+38h]
		fmul	dword [esi+38h]
		fxch	st3
		faddp	st7, st0
		faddp	st5, st0
		faddp	st3, st0
		faddp	st1, st0
		fld		dword [ebx+4]
		fmul	dword [esi+4]
		fld		dword [ebx+0Ch]
		fmul	dword [esi+0Ch]
		fld		dword [ebx+14h]
		fmul	dword [esi+14h]
		fld		dword [ebx+1Ch]
		fmul	dword [esi+1Ch]
		fxch	st3
		fsubp	st7, st0
		fsubp	st5, st0
		fsubp	st3, st0
		fsubp	st1, st0
		fld		dword [ebx+24h]
		fmul	dword [esi+24h]
		fld		dword [ebx+2Ch]
		fmul	dword [esi+2Ch]
		fld		dword [ebx+34h]
		fmul	dword [esi+34h]
		fld		dword [ebx+3Ch]
		fmul	dword [esi+3Ch]
		fxch	st3
		fsubp	st7, st0
		fsubp	st5, st0
		fsubp	st3, st0
		fsubp	st1, st0
		fxch	st2
		faddp	st3, st0
		faddp	st1, st0
		add		ebx, 80h
		faddp	st1, st0
		add		esi, strict dword 40h
		fmul	dword [scale32767]
		fistp	dword [esp-4]
		mov		eax, [esp-4]
		cmp		eax, 7FFFh
		jle		short synthfpuclamp1
		mov		eax, 7FFFh
synthfpuclamp1:
		cmp		eax, 0FFFF8000h
		jge		short synthfpustore1
		mov		eax, 0FFFF8000h
synthfpustore1:
		mov		[edx], ax
		add		edx, edi
		dec		ecx
		jnz		near synthfpuloop1

		fld		dword [ebx]
		fmul	dword [esi]
		fld		dword [ebx+8]
		fmul	dword [esi+8]
		fld		dword [ebx+10h]
		fmul	dword [esi+10h]
		fld		dword [ebx+18h]
		fmul	dword [esi+18h]
		fld		dword [ebx+20h]
		fmul	dword [esi+20h]
		fld		dword [ebx+28h]
		fmul	dword [esi+28h]
		fld		dword [ebx+30h]
		fmul	dword [esi+30h]
		fld		dword [ebx+38h]
		fmul	dword [esi+38h]
		fxch	st3
		faddp	st7, st0
		faddp	st5, st0
		faddp	st3, st0
		faddp	st1, st0
		fxch	st2
		faddp	st3, st0
		faddp	st1, st0
		faddp	st1, st0
		fmul	dword [scale32767]
		fistp	dword [esp-4]
		mov		eax, [esp-4]
		cmp		eax, 7FFFh
		jle		short synthfpuclamp2
		mov		eax, 7FFFh
synthfpuclamp2:
		cmp		eax, 0FFFF8000h
		jge		short synthfpustore2
		mov		eax, 0FFFF8000h
synthfpustore2:
		mov		[edx], ax

		mov		eax, [ebp+%$bo1]
		sub		ebx, 80h
		sub		esi, strict dword 40h
		mov		ecx, 10h
		lea		ebx, [ebx+eax*8]
		dec		ecx
		add		edx, edi

synthfpuloop2:
		fld		dword [ebx-4]
		fmul	dword [esi]
		fld		dword [ebx-8]
		fmul	dword [esi+4]
		fld		dword [ebx-0Ch]
		fmul	dword [esi+8]
		fld		dword [ebx-10h]
		fmul	dword [esi+0Ch]
		fld		dword [ebx-14h]
		fmul	dword [esi+10h]
		fld		dword [ebx-18h]
		fmul	dword [esi+14h]
		fld		dword [ebx-1Ch]
		fmul	dword [esi+18h]
		fld		dword [ebx-20h]
		fmul	dword [esi+1Ch]
		fxch	st3
		faddp	st7, st0
		faddp	st5, st0
		faddp	st3, st0
		faddp	st1, st0
		fld		dword [ebx-24h]
		fmul	dword [esi+20h]
		fld		dword [ebx-28h]
		fmul	dword [esi+24h]
		fld		dword [ebx-2Ch]
		fmul	dword [esi+28h]
		fld		dword [ebx-30h]
		fmul	dword [esi+2Ch]
		fxch	st3
		faddp	st7, st0
		faddp	st5, st0
		faddp	st3, st0
		faddp	st1, st0
		fld		dword [ebx-34h]
		fmul	dword [esi+30h]
		fld		dword [ebx-38h]
		fmul	dword [esi+34h]
		fld		dword [ebx-3Ch]
		fmul	dword [esi+38h]
		fld		dword [ebx]
		fmul	dword [esi+3Ch]
		fxch	st3
		faddp	st7, st0
		faddp	st5, st0
		faddp	st3, st0
		faddp	st1, st0
		fxch	st2
		faddp	st3, st0
		faddp	st1, st0
		sub		ebx, 80h
		faddp	st1, st0
		sub		esi, strict dword 40h
		fchs
		fmul	dword [scale32767]
		fistp	dword [esp-4]
		mov		eax, [esp-4]
		cmp		eax, 7FFFh
		jle		short synthfpuclamp3
		mov		eax, 7FFFh
synthfpuclamp3:
		cmp		eax, 0FFFF8000h
		jge		short synthfpustore3
		mov		eax, 0FFFF8000h
synthfpustore3:
		mov		[edx], ax
		add		edx, edi
		dec		ecx
		jnz		near synthfpuloop2

		pop		eax
		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax

endproc

; VERIFY: 75aff083 auto
; OFFSET: 0x0094BB60
proc FMOD_Mpeg_Synth_FPU_Float

		%$b0            arg
		%$bo1           arg
		%$numchannels   arg
		%$out           arg

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

		mov		eax, [ebp+%$bo1]
		shl		eax, 2
		mov		edi, [ebp+%$numchannels]
		shl		edi, 2
		mov		ebx, [_FMOD_Mpeg_DecWin]
		add		ebx, strict dword 40h
		mov		esi, [ebp+%$b0]
		sub		ebx, eax
		mov		edx, [ebp+%$out]
		mov		ecx, 10h

synthfpufloatloop1:
		fld		dword [ebx]
		fmul	dword [esi]
		fld		dword [ebx+8]
		fmul	dword [esi+8]
		fld		dword [ebx+10h]
		fmul	dword [esi+10h]
		fld		dword [ebx+18h]
		fmul	dword [esi+18h]
		fld		dword [ebx+20h]
		fmul	dword [esi+20h]
		fld		dword [ebx+28h]
		fmul	dword [esi+28h]
		fld		dword [ebx+30h]
		fmul	dword [esi+30h]
		fld		dword [ebx+38h]
		fmul	dword [esi+38h]
		fxch	st3
		faddp	st7, st0
		faddp	st5, st0
		faddp	st3, st0
		faddp	st1, st0
		fld		dword [ebx+4]
		fmul	dword [esi+4]
		fld		dword [ebx+0Ch]
		fmul	dword [esi+0Ch]
		fld		dword [ebx+14h]
		fmul	dword [esi+14h]
		fld		dword [ebx+1Ch]
		fmul	dword [esi+1Ch]
		fxch	st3
		fsubp	st7, st0
		fsubp	st5, st0
		fsubp	st3, st0
		fsubp	st1, st0
		fld		dword [ebx+24h]
		fmul	dword [esi+24h]
		fld		dword [ebx+2Ch]
		fmul	dword [esi+2Ch]
		fld		dword [ebx+34h]
		fmul	dword [esi+34h]
		fld		dword [ebx+3Ch]
		fmul	dword [esi+3Ch]
		fxch	st3
		fsubp	st7, st0
		fsubp	st5, st0
		fsubp	st3, st0
		fsubp	st1, st0
		fxch	st2
		faddp	st3, st0
		faddp	st1, st0
		add		ebx, 80h
		faddp	st1, st0
		add		esi, strict dword 40h
		fstp	dword [edx]
		add		edx, edi
		dec		ecx
		jnz		near synthfpufloatloop1

		fld		dword [ebx]
		fmul	dword [esi]
		fld		dword [ebx+8]
		fmul	dword [esi+8]
		fld		dword [ebx+10h]
		fmul	dword [esi+10h]
		fld		dword [ebx+18h]
		fmul	dword [esi+18h]
		fld		dword [ebx+20h]
		fmul	dword [esi+20h]
		fld		dword [ebx+28h]
		fmul	dword [esi+28h]
		fld		dword [ebx+30h]
		fmul	dword [esi+30h]
		fld		dword [ebx+38h]
		fmul	dword [esi+38h]
		fxch	st3
		faddp	st7, st0
		faddp	st5, st0
		faddp	st3, st0
		faddp	st1, st0
		fxch	st2
		faddp	st3, st0
		faddp	st1, st0
		faddp	st1, st0
		fstp	dword [edx]

		mov		eax, [ebp+%$bo1]
		sub		ebx, 80h
		sub		esi, strict dword 40h
		mov		ecx, 10h
		lea		ebx, [ebx+eax*8]
		dec		ecx
		add		edx, edi

synthfpufloatloop2:
		fld		dword [ebx-4]
		fmul	dword [esi]
		fld		dword [ebx-8]
		fmul	dword [esi+4]
		fld		dword [ebx-0Ch]
		fmul	dword [esi+8]
		fld		dword [ebx-10h]
		fmul	dword [esi+0Ch]
		fld		dword [ebx-14h]
		fmul	dword [esi+10h]
		fld		dword [ebx-18h]
		fmul	dword [esi+14h]
		fld		dword [ebx-1Ch]
		fmul	dword [esi+18h]
		fld		dword [ebx-20h]
		fmul	dword [esi+1Ch]
		fxch	st3
		faddp	st7, st0
		faddp	st5, st0
		faddp	st3, st0
		faddp	st1, st0
		fld		dword [ebx-24h]
		fmul	dword [esi+20h]
		fld		dword [ebx-28h]
		fmul	dword [esi+24h]
		fld		dword [ebx-2Ch]
		fmul	dword [esi+28h]
		fld		dword [ebx-30h]
		fmul	dword [esi+2Ch]
		fxch	st3
		faddp	st7, st0
		faddp	st5, st0
		faddp	st3, st0
		faddp	st1, st0
		fld		dword [ebx-34h]
		fmul	dword [esi+30h]
		fld		dword [ebx-38h]
		fmul	dword [esi+34h]
		fld		dword [ebx-3Ch]
		fmul	dword [esi+38h]
		fld		dword [ebx]
		fmul	dword [esi+3Ch]
		fxch	st3
		faddp	st7, st0
		faddp	st5, st0
		faddp	st3, st0
		faddp	st1, st0
		fxch	st2
		faddp	st3, st0
		faddp	st1, st0
		sub		ebx, 80h
		faddp	st1, st0
		sub		esi, strict dword 40h
		fchs
		fstp	dword [edx]
		add		edx, edi
		dec		ecx
		jnz		near synthfpufloatloop2

		pop		edi
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax

endproc
