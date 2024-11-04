# timetemplate.S
# Written 2015 by F Lundevall
# Ported 2024/06 by W Szczerek (from MIPS to RISC-V)
# Copyright abandonded - this file is in the public domain.

#############################################################
# Choose the macro syntax for RARS or DTEK-V board.         #
# The syntax for RARS is probably due to its MIPS heritage. #
#############################################################
###################
# Macros for RARS #
###################
#.macro	PUSH (%reg)
#	addi	sp,sp,-4
#	sw	%reg,0(sp) 
#.end_macro

#.macro	POP (%reg)
#	lw	%reg,0(sp)
#	addi	sp,sp,4
#.end_macro
###################
# Macros for DTEK #
###################
.macro	PUSH reg
	addi sp,sp,-4
	sw \reg,0(sp) 
.endm

.macro	POP reg
	lw	\reg,0(sp)
	addi	sp,sp,4
.endm
#############################################################

	.data
	.align 2
mytime:	.word 	0x5957
timstr:	.asciz 	"text more text lots of text\0"
	.text
	.globl timetemplate, tick, time2string, delay, display_string

# Function for displaying a string with a newline at the end	
display_string:	
	li a7,4
	ecall
	li a0, 10
	li a7,11
	ecall
	jr ra
	
timetemplate:
	la	a0, timstr
	jal     display_string
	
	# wait a little
	li	a0, -1		# ms
	jal	delay
	
	# call tick
	la 	a0, mytime
	jal	tick
	
	# call your function time2string
	la	a0, timstr
	la	t0, mytime
	lw	a1, 0(t0)
	jal	time2string
	
	# go back and do it all again
	j	timetemplate

	
# tick: update time pointed to by $a0
tick:	lw	t0, 0(a0)	# get time
	addi	t0, t0, 1	# increase
	andi	t1, t0, 0xf	# check lowest digit
	sltiu	t2, t1, 0xa	# if digit < a, okay
	bnez	t2, tiend
	addi	t0, t0, 0x6	# adjust lowest digit
	
	andi	t1, t0, 0xf0	# check next digit
	sltiu	t2, t1, 0x60	# if digit < 6, okay
	bnez	t2, tiend
	addi	t0, t0, 0xa0	# adjust digit
	
	li	t3, 0xF
	slli	t3, t3, 0x8
	and	t1, t0, t3	# check minute digit
	addi	t3, x0, 0xA
	slli	t3, t3, 0x8
	slt	t2, t1, t3	# if digit < a, okay
	bnez	t2, tiend
	addi	t0, t0, 0x600	# adjust digit - this one's okay, it's lower than 0x7FF 
	
	li	t3, 0xF
	slli	t3, t3, 0xC
	and	t1, t0, t3	# check last digit
	addi	t3, x0, 0x6
	slli	t3, t3, 0xC
	slt	t2, t1, t3	# if digit < 6, okay
	bnez	t2, tiend
	
	li	t3, 0xA
	slli	t3, t3, 0xC
	add	t0, t0, t3	# adjust last digit
tiend:	sw	t0,0(a0)	# save updated result
	jr	ra		# return
	
hexasc:
  	addi 	a0,a0,48
  	jr 	ra
delay:		
	li	t1,1
	blt	a0,t1,end	
	loop:
	li	t0,950
	addi	a0,a0,-1
	
	msloop:
	addi	t0,t0,-1
	bgtz  	t0,msloop
	
	bgtz    a0,loop
end:
	jr	ra
	
time2hour:
	
	
time2string:
	PUSH	ra	
	
	mv	t0,a0		#save a0 into t0
	mv	t1,a1		#save a1 into t1
	
	PUSH	t1
	PUSH	t0
	srli	a0,t1,12	#shift right so that fourth LSN is 1st LSN (LSN = least significant nibble) and store in a0
	andi	a0,a0,0x0f	#mask so that only the LSN is represented in the word
	jal 	hexasc		#convert to ASCII
	sb		a0,0(t0)
	POP		t0
	POP 	t1
	
	PUSH	t1
	PUSH	t0
	srli	a0,t1,8		
	andi	a0,a0,0x0f	
	jal 	hexasc		
	sb	a0,1(t0)	
	POP		t0
	POP 	t1
	
	li	t2,0x3A		#load colon
	sb	t2,2(t0)	#store colon
	
	PUSH	t1
	PUSH	t0
	srli	a0,t1,4		
	andi	a0,a0,0x0f	
	jal 	hexasc		
	sb	a0,3(t0)
	POP		t0
	POP 	t1
	
	PUSH	t1
	PUSH	t0
	andi	a0,t1,0x0f	
	jal 	hexasc		
	sb	a0,4(t0)
	POP		t0
	POP 	t1
	
	li	t2,0x00
	sb	t2,5(t0)
	
	POP	ra	
	jr	ra		
