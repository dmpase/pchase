	.file	"Memcpy.C"
	.text
	.align 2
	.p2align 4,,15
.globl _Z10set_memcpyPFPvS_PKvmE
	.type	_Z10set_memcpyPFPvS_PKvmE, @function
_Z10set_memcpyPFPvS_PKvmE:
.LFB30:
	movq	%rdi, memcpy_used(%rip)
	ret
.LFE30:
	.size	_Z10set_memcpyPFPvS_PKvmE, .-_Z10set_memcpyPFPvS_PKvmE
.globl __gxx_personality_v0
	.align 2
	.p2align 4,,15
.globl _Z9mb_memcpyPvPKvm
	.type	_Z9mb_memcpyPvPKvm, @function
_Z9mb_memcpyPvPKvm:
.LFB31:
	movq	memcpy_used(%rip), %r11
	jmp	*%r11
.LFE31:
	.size	_Z9mb_memcpyPvPKvm, .-_Z9mb_memcpyPvPKvm
	.align 2
	.p2align 4,,15
.globl _Z5naivePvPKvm
	.type	_Z5naivePvPKvm, @function
_Z5naivePvPKvm:
.LFB42:
	testq	%rdx, %rdx
	je	.L6
	movq	%rdi, %rcx
	.p2align 4,,7
.L8:
	movzbl	(%rsi), %eax
	addq	$1, %rsi
	movb	%al, (%rcx)
	addq	$1, %rcx
	subq	$1, %rdx
	jne	.L8
.L6:
	movq	%rdi, %rax
	ret
.LFE42:
	.size	_Z5naivePvPKvm, .-_Z5naivePvPKvm
	.align 2
	.p2align 4,,15
.globl _Z8memwritePvPKvm
	.type	_Z8memwritePvPKvm, @function
_Z8memwritePvPKvm:
.LFB41:
	movq	%rsi, %rcx
	movq	%rdi, %rax
	pushq	%rbx
.LCFI0:
	andl	$15, %ecx
	andl	$15, %eax
	movq	%rdi, %rbx
	cmpq	%rax, %rcx
	movq	%rdi, %r11
	movq	%rsi, %r10
	je	.L38
	call	memcpy
.L27:
	movq	%rbx, %rax
	popq	%rbx
	ret
	.p2align 4,,7
.L38:
	testl	%ecx, %ecx
	movl	%ecx, %r9d
	.p2align 4,,2
	jle	.L15
	movq	%rsi, %rcx
	xorl	%r8d, %r8d
	.p2align 4,,7
.L17:
	movzbl	(%rcx), %eax
	addl	$1, %r8d
	addq	$1, %rcx
	movb	%al, (%rdi)
	addq	$1, %rdi
	cmpl	%r9d, %r8d
	jne	.L17
	leal	-1(%r9), %eax
	leaq	1(%rsi,%rax), %r10
	leaq	1(%rbx,%rax), %r11
.L15:
	movslq	%r9d,%rax
	xorl	%r9d, %r9d
	movq	%r10, %rsi
	subq	%rax, %rdx
	xorl	%eax, %eax
	movq	%r11, %rcx
	movq	%rdx, %rdi
	shrq	$4, %rdi
	movq	%rdi, %r8
	subq	$7, %r8
	je	.L21
	.p2align 4,,7
.L22:
#APP
	movntdq   %xmm0,   0(%rcx);
movntdq   %xmm0,  16(%rcx);
movntdq   %xmm0,  32(%rcx);
movntdq   %xmm0,  48(%rcx);
movntdq   %xmm0,  64(%rcx);
movntdq   %xmm0,  80(%rcx);
movntdq   %xmm0,  96(%rcx);
movntdq   %xmm0, 112(%rcx);

#NO_APP
	addq	$8, %r9
	subq	$-128, %rcx
	cmpq	%r8, %r9
	jb	.L22
	subq	%r11, %r10
	movl	%r9d, %eax
	leaq	(%r10,%rcx), %rsi
.L21:
	cmpq	%r9, %rdi
	jbe	.L24
	addl	$1, %eax
	cltq
	subq	$1, %rax
	.p2align 4,,7
.L26:
#APP
	movntdqa   0(%rsi), %xmm0;
movntdq   %xmm0, 0(%rcx);

#NO_APP
	addq	$1, %rax
	addq	$16, %rsi
	addq	$16, %rcx
	cmpq	%rax, %rdi
	ja	.L26
.L24:
	salq	$4, %rdi
	subq	%rdi, %rdx
	movq	%rdx, %rdi
	je	.L27
	xorl	%edx, %edx
	.p2align 4,,7
.L29:
	movzbl	(%rdx,%rsi), %eax
	movb	%al, (%rdx,%rcx)
	addq	$1, %rdx
	cmpq	%rdi, %rdx
	jne	.L29
	movq	%rbx, %rax
	popq	%rbx
	ret
.LFE41:
	.size	_Z8memwritePvPKvm, .-_Z8memwritePvPKvm
	.align 2
	.p2align 4,,15
.globl _Z7memreadPvPKvm
	.type	_Z7memreadPvPKvm, @function
_Z7memreadPvPKvm:
.LFB40:
	movq	%rsi, %rcx
	movq	%rdi, %rax
	pushq	%rbx
.LCFI1:
	andl	$15, %ecx
	andl	$15, %eax
	movq	%rdi, %rbx
	cmpq	%rax, %rcx
	movq	%rsi, %r10
	je	.L61
	call	memcpy
.L52:
	movq	%rbx, %rax
	popq	%rbx
	ret
	.p2align 4,,7
.L61:
	testl	%ecx, %ecx
	.p2align 4,,4
	jle	.L42
	leal	-1(%rcx), %eax
	leaq	1(%rsi,%rax), %r10
.L42:
	movslq	%ecx,%rax
	xorl	%r9d, %r9d
	movq	%r10, %rcx
	subq	%rax, %rdx
	xorl	%eax, %eax
	movq	%rbx, %rsi
	movq	%rdx, %rdi
	shrq	$4, %rdi
	movq	%rdi, %r8
	subq	$7, %r8
	je	.L46
	.p2align 4,,7
.L47:
#APP
	movntdqa   0(%rcx), %xmm0;
	movntdqa  16(%rcx), %xmm0;
	movntdqa  32(%rcx), %xmm0;
	movntdqa  48(%rcx), %xmm0;
	movntdqa  64(%rcx), %xmm0;
	movntdqa  80(%rcx), %xmm0;
	movntdqa  96(%rcx), %xmm0;
	movntdqa 112(%rcx), %xmm0;
	
#NO_APP
	addq	$8, %r9
	subq	$-128, %rcx
	cmpq	%r8, %r9
	jb	.L47
	movq	%rbx, %rax
	subq	%r10, %rax
	leaq	(%rax,%rcx), %rsi
	movl	%r9d, %eax
.L46:
	cmpq	%r9, %rdi
	jbe	.L49
	addl	$1, %eax
	cltq
	subq	$1, %rax
	.p2align 4,,7
.L51:
#APP
	movntdqa   0(%rcx), %xmm0;

#NO_APP
	addq	$1, %rax
	addq	$16, %rcx
	addq	$16, %rsi
	cmpq	%rax, %rdi
	ja	.L51
.L49:
	salq	$4, %rdi
	subq	%rdi, %rdx
	movq	%rdx, %rdi
	je	.L52
	xorl	%edx, %edx
	.p2align 4,,7
.L54:
	movzbl	(%rdx,%rcx), %eax
	movb	%al, (%rdx,%rsi)
	addq	$1, %rdx
	cmpq	%rdi, %rdx
	jne	.L54
	movq	%rbx, %rax
	popq	%rbx
	ret
.LFE40:
	.size	_Z7memreadPvPKvm, .-_Z7memreadPvPKvm
	.align 2
	.p2align 4,,15
.globl _Z3xnnPvPKvm
	.type	_Z3xnnPvPKvm, @function
_Z3xnnPvPKvm:
.LFB39:
	pushq	%rbx
.LCFI2:
	movq	%rdi, %rbx
	movq	%rsi, %rcx
	movq	%rbx, %rax
	andl	$15, %ecx
	movq	%rdi, %r8
	andl	$15, %eax
	movq	%rsi, %rdi
	cmpq	%rax, %rcx
	je	.L88
	movq	%rbx, %rdi
	call	memcpy
.L77:
	movq	%rbx, %rax
	popq	%rbx
	ret
	.p2align 4,,7
.L88:
	testl	%ecx, %ecx
	movl	%ecx, %r9d
	jle	.L65
	movq	%rbx, %rdi
	movq	%rsi, %rcx
	xorl	%r8d, %r8d
	.p2align 4,,7
.L67:
	movzbl	(%rcx), %eax
	addl	$1, %r8d
	addq	$1, %rcx
	movb	%al, (%rdi)
	addq	$1, %rdi
	cmpl	%r9d, %r8d
	jne	.L67
	leal	-1(%r9), %eax
	leaq	1(%rsi,%rax), %rdi
	leaq	1(%rbx,%rax), %r8
.L65:
	movslq	%r9d,%rax
	movq	%rdi, %rsi
	movq	%r8, %rcx
	subq	%rax, %rdx
	xorl	%r8d, %r8d
	xorl	%eax, %eax
	movq	%rdx, %rdi
	shrq	$4, %rdi
	movq	%rdi, %r9
	subq	$7, %r9
	je	.L71
	.p2align 4,,7
.L72:
#APP
	movntdqa   0(%rsi), %xmm0;
movntdq   %xmm0, 0(%rcx);

	movntdqa   16(%rsi), %xmm0;
movntdq   %xmm0, 16(%rcx);

	movntdqa   32(%rsi), %xmm0;
movntdq   %xmm0, 32(%rcx);

	movntdqa   48(%rsi), %xmm0;
movntdq   %xmm0, 48(%rcx);

	movntdqa   64(%rsi), %xmm0;
movntdq   %xmm0, 64(%rcx);

	movntdqa   80(%rsi), %xmm0;
movntdq   %xmm0, 80(%rcx);

	movntdqa   96(%rsi), %xmm0;
movntdq   %xmm0, 96(%rcx);

	movntdqa   112(%rsi), %xmm0;
movntdq   %xmm0, 112(%rcx);

#NO_APP
	addq	$8, %r8
	subq	$-128, %rsi
	subq	$-128, %rcx
	cmpq	%r9, %r8
	jb	.L72
	movl	%r8d, %eax
.L71:
	cmpq	%r8, %rdi
	jbe	.L74
	addl	$1, %eax
	cltq
	subq	$1, %rax
	.p2align 4,,7
.L76:
#APP
	movntdqa   0(%rsi), %xmm0;
movntdq   %xmm0, 0(%rcx);

#NO_APP
	addq	$1, %rax
	addq	$16, %rsi
	addq	$16, %rcx
	cmpq	%rax, %rdi
	ja	.L76
.L74:
	salq	$4, %rdi
	subq	%rdi, %rdx
	movq	%rdx, %rdi
	je	.L77
	xorl	%edx, %edx
	.p2align 4,,7
.L79:
	movzbl	(%rdx,%rsi), %eax
	movb	%al, (%rdx,%rcx)
	addq	$1, %rdx
	cmpq	%rdi, %rdx
	jne	.L79
	movq	%rbx, %rax
	popq	%rbx
	ret
.LFE39:
	.size	_Z3xnnPvPKvm, .-_Z3xnnPvPKvm
	.align 2
	.p2align 4,,15
.globl _Z3xntPvPKvm
	.type	_Z3xntPvPKvm, @function
_Z3xntPvPKvm:
.LFB38:
	pushq	%rbx
.LCFI3:
	movq	%rdi, %rbx
	movq	%rsi, %rcx
	movq	%rbx, %rax
	andl	$15, %ecx
	movq	%rdi, %r8
	andl	$15, %eax
	movq	%rsi, %rdi
	cmpq	%rax, %rcx
	je	.L115
	movq	%rbx, %rdi
	call	memcpy
.L104:
	movq	%rbx, %rax
	popq	%rbx
	ret
	.p2align 4,,7
.L115:
	testl	%ecx, %ecx
	movl	%ecx, %r9d
	jle	.L92
	movq	%rbx, %rdi
	movq	%rsi, %rcx
	xorl	%r8d, %r8d
	.p2align 4,,7
.L94:
	movzbl	(%rcx), %eax
	addl	$1, %r8d
	addq	$1, %rcx
	movb	%al, (%rdi)
	addq	$1, %rdi
	cmpl	%r9d, %r8d
	jne	.L94
	leal	-1(%r9), %eax
	leaq	1(%rsi,%rax), %rdi
	leaq	1(%rbx,%rax), %r8
.L92:
	movslq	%r9d,%rax
	movq	%rdi, %rsi
	movq	%r8, %rcx
	subq	%rax, %rdx
	xorl	%r8d, %r8d
	xorl	%eax, %eax
	movq	%rdx, %rdi
	shrq	$4, %rdi
	movq	%rdi, %r9
	subq	$7, %r9
	je	.L98
	.p2align 4,,7
.L99:
#APP
	movntdqa   0(%rsi), %xmm0;
movdqa    %xmm0, 0(%rcx);

	movntdqa   16(%rsi), %xmm0;
movdqa    %xmm0, 16(%rcx);

	movntdqa   32(%rsi), %xmm0;
movdqa    %xmm0, 32(%rcx);

	movntdqa   48(%rsi), %xmm0;
movdqa    %xmm0, 48(%rcx);

	movntdqa   64(%rsi), %xmm0;
movdqa    %xmm0, 64(%rcx);

	movntdqa   80(%rsi), %xmm0;
movdqa    %xmm0, 80(%rcx);

	movntdqa   96(%rsi), %xmm0;
movdqa    %xmm0, 96(%rcx);

	movntdqa   112(%rsi), %xmm0;
movdqa    %xmm0, 112(%rcx);

#NO_APP
	addq	$8, %r8
	subq	$-128, %rsi
	subq	$-128, %rcx
	cmpq	%r9, %r8
	jb	.L99
	movl	%r8d, %eax
.L98:
	cmpq	%r8, %rdi
	jbe	.L101
	addl	$1, %eax
	cltq
	subq	$1, %rax
	.p2align 4,,7
.L103:
#APP
	movntdqa   0(%rsi), %xmm0;
movdqa    %xmm0, 0(%rcx);

#NO_APP
	addq	$1, %rax
	addq	$16, %rsi
	addq	$16, %rcx
	cmpq	%rax, %rdi
	ja	.L103
.L101:
	salq	$4, %rdi
	subq	%rdi, %rdx
	movq	%rdx, %rdi
	je	.L104
	xorl	%edx, %edx
	.p2align 4,,7
.L106:
	movzbl	(%rdx,%rsi), %eax
	movb	%al, (%rdx,%rcx)
	addq	$1, %rdx
	cmpq	%rdi, %rdx
	jne	.L106
	movq	%rbx, %rax
	popq	%rbx
	ret
.LFE38:
	.size	_Z3xntPvPKvm, .-_Z3xntPvPKvm
	.align 2
	.p2align 4,,15
.globl _Z3xtnPvPKvm
	.type	_Z3xtnPvPKvm, @function
_Z3xtnPvPKvm:
.LFB37:
	pushq	%rbx
.LCFI4:
	movq	%rdi, %rbx
	movq	%rsi, %rcx
	movq	%rbx, %rax
	andl	$15, %ecx
	movq	%rdi, %r8
	andl	$15, %eax
	movq	%rsi, %rdi
	cmpq	%rax, %rcx
	je	.L142
	movq	%rbx, %rdi
	call	memcpy
.L131:
	movq	%rbx, %rax
	popq	%rbx
	ret
	.p2align 4,,7
.L142:
	testl	%ecx, %ecx
	movl	%ecx, %r9d
	jle	.L119
	movq	%rbx, %rdi
	movq	%rsi, %rcx
	xorl	%r8d, %r8d
	.p2align 4,,7
.L121:
	movzbl	(%rcx), %eax
	addl	$1, %r8d
	addq	$1, %rcx
	movb	%al, (%rdi)
	addq	$1, %rdi
	cmpl	%r9d, %r8d
	jne	.L121
	leal	-1(%r9), %eax
	leaq	1(%rsi,%rax), %rdi
	leaq	1(%rbx,%rax), %r8
.L119:
	movslq	%r9d,%rax
	movq	%rdi, %rsi
	movq	%r8, %rcx
	subq	%rax, %rdx
	xorl	%r8d, %r8d
	xorl	%eax, %eax
	movq	%rdx, %rdi
	shrq	$4, %rdi
	movq	%rdi, %r9
	subq	$7, %r9
	je	.L125
	.p2align 4,,7
.L126:
#APP
	movdqa     0(%rsi), %xmm0;
movntdq   %xmm0, 0(%rcx);

	movdqa     16(%rsi), %xmm0;
movntdq   %xmm0, 16(%rcx);

	movdqa     32(%rsi), %xmm0;
movntdq   %xmm0, 32(%rcx);

	movdqa     48(%rsi), %xmm0;
movntdq   %xmm0, 48(%rcx);

	movdqa     64(%rsi), %xmm0;
movntdq   %xmm0, 64(%rcx);

	movdqa     80(%rsi), %xmm0;
movntdq   %xmm0, 80(%rcx);

	movdqa     96(%rsi), %xmm0;
movntdq   %xmm0, 96(%rcx);

	movdqa     112(%rsi), %xmm0;
movntdq   %xmm0, 112(%rcx);

#NO_APP
	addq	$8, %r8
	subq	$-128, %rsi
	subq	$-128, %rcx
	cmpq	%r9, %r8
	jb	.L126
	movl	%r8d, %eax
.L125:
	cmpq	%r8, %rdi
	jbe	.L128
	addl	$1, %eax
	cltq
	subq	$1, %rax
	.p2align 4,,7
.L130:
#APP
	movdqa     0(%rsi), %xmm0;
movntdq   %xmm0, 0(%rcx);

#NO_APP
	addq	$1, %rax
	addq	$16, %rsi
	addq	$16, %rcx
	cmpq	%rax, %rdi
	ja	.L130
.L128:
	salq	$4, %rdi
	subq	%rdi, %rdx
	movq	%rdx, %rdi
	je	.L131
	xorl	%edx, %edx
	.p2align 4,,7
.L133:
	movzbl	(%rdx,%rsi), %eax
	movb	%al, (%rdx,%rcx)
	addq	$1, %rdx
	cmpq	%rdi, %rdx
	jne	.L133
	movq	%rbx, %rax
	popq	%rbx
	ret
.LFE37:
	.size	_Z3xtnPvPKvm, .-_Z3xtnPvPKvm
	.align 2
	.p2align 4,,15
.globl _Z4xttuPvPKvm
	.type	_Z4xttuPvPKvm, @function
_Z4xttuPvPKvm:
.LFB36:
	pushq	%rbx
.LCFI5:
	movq	%rdi, %rbx
	movq	%rsi, %rcx
	movq	%rbx, %rax
	andl	$15, %ecx
	movq	%rdi, %r8
	andl	$15, %eax
	movq	%rsi, %rdi
	cmpq	%rax, %rcx
	je	.L169
	movq	%rbx, %rdi
	call	memcpy
.L158:
	movq	%rbx, %rax
	popq	%rbx
	ret
	.p2align 4,,7
.L169:
	testl	%ecx, %ecx
	movl	%ecx, %r9d
	jle	.L146
	movq	%rbx, %rdi
	movq	%rsi, %rcx
	xorl	%r8d, %r8d
	.p2align 4,,7
.L148:
	movzbl	(%rcx), %eax
	addl	$1, %r8d
	addq	$1, %rcx
	movb	%al, (%rdi)
	addq	$1, %rdi
	cmpl	%r9d, %r8d
	jne	.L148
	leal	-1(%r9), %eax
	leaq	1(%rsi,%rax), %rdi
	leaq	1(%rbx,%rax), %r8
.L146:
	movslq	%r9d,%rax
	movq	%rdi, %rsi
	movq	%r8, %rcx
	subq	%rax, %rdx
	xorl	%r8d, %r8d
	xorl	%eax, %eax
	movq	%rdx, %rdi
	shrq	$4, %rdi
	movq	%rdi, %r9
	subq	$7, %r9
	je	.L152
	.p2align 4,,7
.L153:
#APP
	movdqu     0(%rsi), %xmm0;
movdqu    %xmm0, 0(%rcx);

	movdqu     16(%rsi), %xmm0;
movdqu    %xmm0, 16(%rcx);

	movdqu     32(%rsi), %xmm0;
movdqu    %xmm0, 32(%rcx);

	movdqu     48(%rsi), %xmm0;
movdqu    %xmm0, 48(%rcx);

	movdqu     64(%rsi), %xmm0;
movdqu    %xmm0, 64(%rcx);

	movdqu     80(%rsi), %xmm0;
movdqu    %xmm0, 80(%rcx);

	movdqu     96(%rsi), %xmm0;
movdqu    %xmm0, 96(%rcx);

	movdqu     112(%rsi), %xmm0;
movdqu    %xmm0, 112(%rcx);

#NO_APP
	addq	$8, %r8
	subq	$-128, %rsi
	subq	$-128, %rcx
	cmpq	%r9, %r8
	jb	.L153
	movl	%r8d, %eax
.L152:
	cmpq	%r8, %rdi
	jbe	.L155
	addl	$1, %eax
	cltq
	subq	$1, %rax
	.p2align 4,,7
.L157:
#APP
	movdqu     0(%rsi), %xmm0;
movdqu    %xmm0, 0(%rcx);

#NO_APP
	addq	$1, %rax
	addq	$16, %rsi
	addq	$16, %rcx
	cmpq	%rax, %rdi
	ja	.L157
.L155:
	salq	$4, %rdi
	subq	%rdi, %rdx
	movq	%rdx, %rdi
	je	.L158
	xorl	%edx, %edx
	.p2align 4,,7
.L160:
	movzbl	(%rdx,%rsi), %eax
	movb	%al, (%rdx,%rcx)
	addq	$1, %rdx
	cmpq	%rdi, %rdx
	jne	.L160
	movq	%rbx, %rax
	popq	%rbx
	ret
.LFE36:
	.size	_Z4xttuPvPKvm, .-_Z4xttuPvPKvm
	.align 2
	.p2align 4,,15
.globl _Z4xttaPvPKvm
	.type	_Z4xttaPvPKvm, @function
_Z4xttaPvPKvm:
.LFB35:
	pushq	%rbx
.LCFI6:
	movq	%rdi, %rbx
	movq	%rsi, %rcx
	movq	%rbx, %rax
	andl	$15, %ecx
	movq	%rdi, %r8
	andl	$15, %eax
	movq	%rsi, %rdi
	cmpq	%rax, %rcx
	je	.L196
	movq	%rbx, %rdi
	call	memcpy
.L185:
	movq	%rbx, %rax
	popq	%rbx
	ret
	.p2align 4,,7
.L196:
	testl	%ecx, %ecx
	movl	%ecx, %r9d
	jle	.L173
	movq	%rbx, %rdi
	movq	%rsi, %rcx
	xorl	%r8d, %r8d
	.p2align 4,,7
.L175:
	movzbl	(%rcx), %eax
	addl	$1, %r8d
	addq	$1, %rcx
	movb	%al, (%rdi)
	addq	$1, %rdi
	cmpl	%r9d, %r8d
	jne	.L175
	leal	-1(%r9), %eax
	leaq	1(%rsi,%rax), %rdi
	leaq	1(%rbx,%rax), %r8
.L173:
	movslq	%r9d,%rax
	movq	%rdi, %rsi
	movq	%r8, %rcx
	subq	%rax, %rdx
	xorl	%r8d, %r8d
	xorl	%eax, %eax
	movq	%rdx, %rdi
	shrq	$4, %rdi
	movq	%rdi, %r9
	subq	$7, %r9
	je	.L179
	.p2align 4,,7
.L180:
#APP
	movdqa     0(%rsi), %xmm0;
movdqa    %xmm0, 0(%rcx);

	movdqa     16(%rsi), %xmm0;
movdqa    %xmm0, 16(%rcx);

	movdqa     32(%rsi), %xmm0;
movdqa    %xmm0, 32(%rcx);

	movdqa     48(%rsi), %xmm0;
movdqa    %xmm0, 48(%rcx);

	movdqa     64(%rsi), %xmm0;
movdqa    %xmm0, 64(%rcx);

	movdqa     80(%rsi), %xmm0;
movdqa    %xmm0, 80(%rcx);

	movdqa     96(%rsi), %xmm0;
movdqa    %xmm0, 96(%rcx);

	movdqa     112(%rsi), %xmm0;
movdqa    %xmm0, 112(%rcx);

#NO_APP
	addq	$8, %r8
	subq	$-128, %rsi
	subq	$-128, %rcx
	cmpq	%r9, %r8
	jb	.L180
	movl	%r8d, %eax
.L179:
	cmpq	%r8, %rdi
	jbe	.L182
	addl	$1, %eax
	cltq
	subq	$1, %rax
	.p2align 4,,7
.L184:
#APP
	movdqa     0(%rsi), %xmm0;
movdqa    %xmm0, 0(%rcx);

#NO_APP
	addq	$1, %rax
	addq	$16, %rsi
	addq	$16, %rcx
	cmpq	%rax, %rdi
	ja	.L184
.L182:
	salq	$4, %rdi
	subq	%rdi, %rdx
	movq	%rdx, %rdi
	je	.L185
	xorl	%edx, %edx
	.p2align 4,,7
.L187:
	movzbl	(%rdx,%rsi), %eax
	movb	%al, (%rdx,%rcx)
	addq	$1, %rdx
	cmpq	%rdi, %rdx
	jne	.L187
	movq	%rbx, %rax
	popq	%rbx
	ret
.LFE35:
	.size	_Z4xttaPvPKvm, .-_Z4xttaPvPKvm
	.align 2
	.p2align 4,,15
.globl _Z7copy_ttPvPKvm
	.type	_Z7copy_ttPvPKvm, @function
_Z7copy_ttPvPKvm:
.LFB32:
	movq	%rsi, %rcx
	movq	%rdi, %rax
	pushq	%rbx
.LCFI7:
	andl	$7, %ecx
	andl	$7, %eax
	movq	%rdi, %rbx
	cmpq	%rax, %rcx
	movq	%rsi, %r8
	je	.L217
	call	memcpy
.L207:
	movq	%rbx, %rax
	popq	%rbx
	ret
	.p2align 4,,7
.L217:
	testl	%ecx, %ecx
	movl	%ecx, %r10d
	.p2align 4,,2
	jle	.L200
	movq	%rdi, %r8
	movq	%rsi, %rcx
	xorl	%r9d, %r9d
	.p2align 4,,7
.L202:
	movzbl	(%rcx), %eax
	addl	$1, %r9d
	addq	$1, %rcx
	movb	%al, (%r8)
	addq	$1, %r8
	cmpl	%r10d, %r9d
	jne	.L202
	leal	-1(%r10), %eax
	leaq	1(%rsi,%rax), %r8
	leaq	1(%rdi,%rax), %rbx
.L200:
	movslq	%r10d,%rax
	movq	%r8, %rsi
	movq	%rbx, %rcx
	subq	%rax, %rdx
	movq	%rdx, %r8
	shrq	$3, %r8
	testl	%r8d, %r8d
	jle	.L204
	xorl	%edi, %edi
	.p2align 4,,7
.L206:
	movq	(%rsi), %rax
	addl	$8, %edi
	movq	%rax, (%rcx)
	movq	8(%rsi), %rax
	movq	%rax, 8(%rcx)
	movq	16(%rsi), %rax
	movq	%rax, 16(%rcx)
	movq	24(%rsi), %rax
	movq	%rax, 24(%rcx)
	movq	32(%rsi), %rax
	movq	%rax, 32(%rcx)
	movq	40(%rsi), %rax
	movq	%rax, 40(%rcx)
	movq	48(%rsi), %rax
	movq	%rax, 48(%rcx)
	movq	56(%rsi), %rax
	addq	$64, %rsi
	movq	%rax, 56(%rcx)
	addq	$64, %rcx
	cmpl	%edi, %r8d
	jg	.L206
.L204:
	movslq	%r8d,%rax
	sall	$6, %r8d
	leaq	0(,%rax,8), %rdi
	movl	%edx, %eax
	subl	%r8d, %eax
	testl	%eax, %eax
	leaq	(%rcx,%rdi), %rbx
	jle	.L207
	subl	$1, %eax
	leaq	(%rdi,%rsi), %rsi
	movq	%rbx, %rcx
	leaq	1(%rbx,%rax), %rdx
	.p2align 4,,7
.L209:
	movzbl	(%rsi), %eax
	addq	$1, %rsi
	movb	%al, (%rcx)
	addq	$1, %rcx
	cmpq	%rdx, %rcx
	jne	.L209
	movq	%rcx, %rbx
	movq	%rbx, %rax
	popq	%rbx
	ret
.LFE32:
	.size	_Z7copy_ttPvPKvm, .-_Z7copy_ttPvPKvm
	.align 2
	.p2align 4,,15
.globl _Z7copy_tnPvPKvm
	.type	_Z7copy_tnPvPKvm, @function
_Z7copy_tnPvPKvm:
.LFB34:
	movq	%rsi, %rcx
	movq	%rdi, %rax
	pushq	%rbx
.LCFI8:
	andl	$7, %ecx
	andl	$7, %eax
	movq	%rdi, %rbx
	cmpq	%rax, %rcx
	movq	%rsi, %r8
	je	.L237
	call	memcpy
.L228:
	movq	%rbx, %rax
	popq	%rbx
	ret
	.p2align 4,,7
.L237:
	testl	%ecx, %ecx
	movl	%ecx, %r10d
	.p2align 4,,2
	jle	.L221
	movq	%rdi, %r8
	movq	%rsi, %rcx
	xorl	%r9d, %r9d
	.p2align 4,,7
.L223:
	movzbl	(%rcx), %eax
	addl	$1, %r9d
	addq	$1, %rcx
	movb	%al, (%r8)
	addq	$1, %r8
	cmpl	%r10d, %r9d
	jne	.L223
	leal	-1(%r10), %eax
	leaq	1(%rsi,%rax), %r8
	leaq	1(%rdi,%rax), %rbx
.L221:
	movslq	%r10d,%rax
	movq	%r8, %rsi
	movq	%rbx, %rcx
	subq	%rax, %rdx
	movq	%rdx, %r8
	shrq	$3, %r8
	testl	%r8d, %r8d
	jle	.L225
	xorl	%edi, %edi
	.p2align 4,,7
.L227:
	addl	$8, %edi
	movq	(%rsi), %rax
#APP
	movntiq    %rax, (%rcx);
#NO_APP
	movq	8(%rsi), %rax
#APP
	movntiq    %rax, 8(%rcx);
#NO_APP
	movq	16(%rsi), %rax
#APP
	movntiq    %rax, 16(%rcx);
#NO_APP
	movq	24(%rsi), %rax
#APP
	movntiq    %rax, 24(%rcx);
#NO_APP
	movq	32(%rsi), %rax
#APP
	movntiq    %rax, 32(%rcx);
#NO_APP
	movq	40(%rsi), %rax
#APP
	movntiq    %rax, 40(%rcx);
#NO_APP
	movq	48(%rsi), %rax
#APP
	movntiq    %rax, 48(%rcx);
#NO_APP
	movq	56(%rsi), %rax
	addq	$64, %rsi
#APP
	movntiq    %rax, 56(%rcx);
#NO_APP
	addq	$64, %rcx
	cmpl	%edi, %r8d
	jg	.L227
.L225:
	movslq	%r8d,%rax
	sall	$6, %r8d
	leaq	0(,%rax,8), %rdi
	movl	%edx, %eax
	subl	%r8d, %eax
	testl	%eax, %eax
	leaq	(%rcx,%rdi), %rbx
	jle	.L228
	leaq	(%rdi,%rsi), %rcx
	subl	$1, %eax
	leaq	1(%rcx,%rax), %rdx
	.p2align 4,,7
.L230:
	movzbl	(%rcx), %eax
	addq	$1, %rcx
	movb	%al, (%rbx)
	addq	$1, %rbx
	cmpq	%rdx, %rcx
	jne	.L230
	movq	%rbx, %rax
	popq	%rbx
	ret
.LFE34:
	.size	_Z7copy_tnPvPKvm, .-_Z7copy_tnPvPKvm
.globl Memcpy_C
	.section	.rodata
	.align 8
.LC0:
	.string	""
	.string	"@ID 2012-04-05 08:59:51 EDT 4976587e6dd4e6403f1843228db65af4c9845f02ee25b307883965b15cd74eb2 gcc version 4.1.2 20080704 (Red Hat 4.1.2-46) g++ -O3 -m64 -I ../../lib -DNT_INOUT  Memcpy.C"
	.data
	.align 8
	.type	Memcpy_C, @object
	.size	Memcpy_C, 8
Memcpy_C:
	.quad	.LC0
	.local	memcpy_used
	.comm	memcpy_used,8,8
	.section	.eh_frame,"a",@progbits
.Lframe1:
	.long	.LECIE1-.LSCIE1
.LSCIE1:
	.long	0x0
	.byte	0x1
	.string	"zPR"
	.uleb128 0x1
	.sleb128 -8
	.byte	0x10
	.uleb128 0x6
	.byte	0x3
	.long	__gxx_personality_v0
	.byte	0x3
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.byte	0x90
	.uleb128 0x1
	.align 8
.LECIE1:
.LSFDE1:
	.long	.LEFDE1-.LASFDE1
.LASFDE1:
	.long	.LASFDE1-.Lframe1
	.long	.LFB30
	.long	.LFE30-.LFB30
	.uleb128 0x0
	.align 8
.LEFDE1:
.LSFDE3:
	.long	.LEFDE3-.LASFDE3
.LASFDE3:
	.long	.LASFDE3-.Lframe1
	.long	.LFB31
	.long	.LFE31-.LFB31
	.uleb128 0x0
	.align 8
.LEFDE3:
.LSFDE5:
	.long	.LEFDE5-.LASFDE5
.LASFDE5:
	.long	.LASFDE5-.Lframe1
	.long	.LFB42
	.long	.LFE42-.LFB42
	.uleb128 0x0
	.align 8
.LEFDE5:
.LSFDE7:
	.long	.LEFDE7-.LASFDE7
.LASFDE7:
	.long	.LASFDE7-.Lframe1
	.long	.LFB41
	.long	.LFE41-.LFB41
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI0-.LFB41
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE7:
.LSFDE9:
	.long	.LEFDE9-.LASFDE9
.LASFDE9:
	.long	.LASFDE9-.Lframe1
	.long	.LFB40
	.long	.LFE40-.LFB40
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI1-.LFB40
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE9:
.LSFDE11:
	.long	.LEFDE11-.LASFDE11
.LASFDE11:
	.long	.LASFDE11-.Lframe1
	.long	.LFB39
	.long	.LFE39-.LFB39
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI2-.LFB39
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE11:
.LSFDE13:
	.long	.LEFDE13-.LASFDE13
.LASFDE13:
	.long	.LASFDE13-.Lframe1
	.long	.LFB38
	.long	.LFE38-.LFB38
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI3-.LFB38
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE13:
.LSFDE15:
	.long	.LEFDE15-.LASFDE15
.LASFDE15:
	.long	.LASFDE15-.Lframe1
	.long	.LFB37
	.long	.LFE37-.LFB37
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI4-.LFB37
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE15:
.LSFDE17:
	.long	.LEFDE17-.LASFDE17
.LASFDE17:
	.long	.LASFDE17-.Lframe1
	.long	.LFB36
	.long	.LFE36-.LFB36
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI5-.LFB36
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE17:
.LSFDE19:
	.long	.LEFDE19-.LASFDE19
.LASFDE19:
	.long	.LASFDE19-.Lframe1
	.long	.LFB35
	.long	.LFE35-.LFB35
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI6-.LFB35
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE19:
.LSFDE21:
	.long	.LEFDE21-.LASFDE21
.LASFDE21:
	.long	.LASFDE21-.Lframe1
	.long	.LFB32
	.long	.LFE32-.LFB32
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI7-.LFB32
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE21:
.LSFDE23:
	.long	.LEFDE23-.LASFDE23
.LASFDE23:
	.long	.LASFDE23-.Lframe1
	.long	.LFB34
	.long	.LFE34-.LFB34
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI8-.LFB34
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE23:
	.ident	"GCC: (GNU) 4.1.2 20080704 (Red Hat 4.1.2-46)"
	.section	.note.GNU-stack,"",@progbits
