[ORG 0x00]	;코드의 시작 주소는 0x00
[BITS 16]	

SECTION .text

jmp 0x1000:START	;CS세그먼트 레지스터에 0x1000을 복사

SECTORCOUNT: dw 0x0000	;현재 실행중인 섹터 번호를 저장
TOTALSECTORCOUNT equ 1024

START:
	mov ax, cs	;cs 세그먼트 레지스터의 값을 ax레지스터에 설정
	mov ds, ax	;ax 레지스터 값을 ds 세그먼트 레지스터에 설정
	mov ax, 0xB800	;비디오 메모리 주소인 0x0B800을 세그먼트 레지스터 값으로 변환
	mov es, ax	;es레지스터에 0xB800 설정

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 각 섹터 별로 코드를 생성
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	%assign i 0
	%rep TOTALSECTORCOUNT	;TOTALSECTORCOUNT에 지정된 값만큼 아래 코드 반복
		%assign i i+1

	
		mov ax, 2	;한 문자를 나타내는 바이트 수(2)를 ax레지스터에 설정
		mul word [ SECTORCOUNT ]	;ax레지스터와 섹터 수를 곱함
		mov si, ax
	
		;mov byte[es: si+(160*3)], '0' +(i % 10)	; 계산된 결과를 오프셋삼아 화면에 0 출력

		add word [ SECTORCOUNT ], 1	; 섹터 수를 1 증가

		%if i == TOTALSECTORCOUNT	
			jmp $
		%else
			jmp (0x1000 + i*0x20): 0x0000
		%endif

		times (512 - ( $ - $$ ) % 512) db 0x00
	%endrep
