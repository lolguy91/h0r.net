global _start

_start:
mov rax,1
mov rbx,'H'
int 0x80

mov rax,1
mov rbx,'e'
int 0x80

mov rax,1
mov rbx,'l'
int 0x80

mov rax,1
mov rbx,'l'
int 0x80

mov rax,1
mov rbx,'o'
int 0x80

mov rax,1
mov rbx,' '
int 0x80

mov rax,1
mov rbx,'W'
int 0x80

mov rax,1
mov rbx,'o'
int 0x80

mov rax,1
mov rbx,'r'
int 0x80

mov rax,1
mov rbx,'l'
int 0x80

mov rax,1
mov rbx,'d'
int 0x80

mov rax,1
mov rbx,'!'
int 0x80

mov rax,1
mov rbx,12
int 0x80

mov rax,0
int 0x80

jmp $
