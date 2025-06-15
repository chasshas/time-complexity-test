.section .data
    test_data: .quad 0x123456789ABCDEF0, 0xFEDCBA9876543210
    iterations: .quad 10000000

.section .text
    .global test_add
    .global main

test_add:
    push %rbp
    mov %rsp, %rbp
    
    mov iterations(%rip), %rcx
    mov test_data(%rip), %rax
    mov test_data+8(%rip), %rdx
    
loop_add:
    add %rdx, %rax
    add $1, %rax
    dec %rcx
    jnz loop_add
    
    pop %rbp
    ret

main:
    call test_add
    
    mov $60, %rax       # sys_exit
    mov $0, %rdi        # status
    syscall
