[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07C0:START

START:
	mov ax, 0x07C0
	mov ds, ax	;ds세그먼트 레지스터에 부트로더 시작어드레스 설정
	mov ax,0xB800
	mov es, ax	;es세그먼트 레지스터에 비디오메모리 시작 어드레스 설정
	
	mov si,0	;i=0
	
.SCREENCLEARLOOP:		;화면 설정
	mov byte [ es: si ], 0		;검정바탕
	mov byte [ es: si+1 ], 0x0A	;초록글씨

	add si, 2	;i+=2
	cmp si, 80*25*2		;if문
	jl .SCREENCLEARLOOP	

	mov si, 0	;i=0
	mov di, 0	;j=0

.MESSAGELOOP:
	mov cl, byte[ si + MESSAGE1]	;출력할 차례의 위치를 cl레지스터에 복사
	
	cmp cl, 0	;if문
	je .MESSAGEEND	

	mov byte[ es:di ], cl	;비디오메모리 어드레스에 문자출력

	add si, 1	;i+=1 si는 다음 문자열로 이동
	add di, 2	;i+=2 di는 비디오 메모리 다음위치로 이동(문자,속성 쌍이므로 2씩 이동)

	jmp .MESSAGELOOP ;while문

MESSAGE1: db 'MINT64 OS Boot Loader Start~', 0 ;마지막을 0으로 해서 MESSAGEEND로 감

times 510 - ($ - $$)    db    0x00

db 0x55
db 0xAA
