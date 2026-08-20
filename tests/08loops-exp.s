        .data

a:
        .long   0

b:
        .long   0

str0:
        .string "two args needed\n"

str1:
        .string "a bigger\n"

str2:
        .string "b bigger\n"

str3:
        .string "%d\n"

str4:
        .string "%d\n"


        .text
        .globl  main

main:
        pushl   %ebp
        movl    %esp, %ebp

        cmpl    $3, 8(%ebp)
        jge     L0

        pushl   $str0
        call    printf
        addl    $4, %esp

        movl    $1, %eax
        leave
        ret


L0:
        movl    12(%ebp), %eax
        pushl   4(%eax)
        call    atoi
        addl    $4, %esp

        movl    %eax, a


        movl    12(%ebp), %eax
        pushl   8(%eax)
        call    atoi
        addl    $4, %esp

        movl    %eax, b


        movl    a, %eax
        cmpl    b, %eax
        jle     L1

        pushl   $str1
        call    printf
        addl    $4, %esp
        jmp     L2


L1:
        pushl   $str2
        call    printf
        addl    $4, %esp


L2:
        subl    $4, %esp
        movl    $1, -4(%ebp)


L3:
        cmpl    $10, -4(%ebp)
        jg      L4

        pushl   -4(%ebp)
        pushl   $str3
        call    printf
        addl    $8, %esp

        incl    -4(%ebp)
        jmp     L3


L4:
        movl    $0, -4(%ebp)


L5:
        cmpl    $10, -4(%ebp)
        jg      L6

        pushl   -4(%ebp)
        pushl   $str4
        call    printf
        addl    $8, %esp

        incl    -4(%ebp)
        jmp     L5


L6:
        movl    %ebp, %esp
        popl    %ebp
        ret
