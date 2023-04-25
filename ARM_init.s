Mode_USR	EQU     0x10	;����IRQ��FIQ�ж�
Mode_SVC 	EQU     0xD3	;�ر�IRQ��FIQ�ж�
Mode_SYS	EQU     0xDF	;�ر�IRQ��FIQ�ж�
Mode_IRQ	EQU     0xD2	;�ر�IRQ��FIQ�ж�

	GBLS C_Entry		;����һ��ȫ�ֵ��ַ������ű���
	GBLS	C_EINT0
	GBLS	C_EINT1
	GBLS	INIT_Clock
	GBLS	INIT_MemSetup
	GBLS	INIT_Enternet
	GBLS	INIT_LCD
	GBLS	INIT_NORFlash
	GBLS	INIT_keyboard_INT
	GBLS	INIT_SDCARD	
C_EINT0	SETS	"Handle_EINT0"
C_EINT1	SETS	"Handle_EINT1"
INIT_Clock	SETS	"Clock_init"
INIT_MemSetup	SETS	"MemSetup"
INIT_Enternet	SETS	"Enternet"
INIT_LCD		SETS	"LCD"
INIT_NORFlash	SETS	"Init_NORFlash"
INIT_keyboard_INT	SETS	"Init_keyboard_INT"
INIT_SDCARD	SETS	"Init_SDCARD"
C_Entry SETS "C_Interrupt"
	IMPORT $C_Entry;$��ʾ����ֵ�������ű���������õ�����C_Interrupt
	IMPORT $C_EINT0
	IMPORT $C_EINT1
	IMPORT $INIT_Clock
	IMPORT $INIT_MemSetup
	IMPORT $INIT_Enternet
	IMPORT $INIT_LCD	
	IMPORT $INIT_NORFlash
	IMPORT $INIT_keyboard_INT
	IMPORT $INIT_SDCARD
	GET		2440Reg_addr.s
	AREA MyCode,CODE,READONLY
	ENTRY			;�����ж�������
	B	ResetHandler		;Reset�жϷ������
	B	.			;handlerUndef
	B	.			;SWI interrupt handler
	B	.			;handlerPAbort
	B	.			;handlerDAbort
	B	.			;handlerReserved
	B	HandlerIRQ		;HandlerIRQ, INTMOD��λΪ0
	B	.			;HandlerFIQ

ResetHandler
	B	$INIT_Clock	;��ʼ�����Ź���ʱ��
	B	$INIT_MemSetup	;��ʼ��SDRAM
	BL	$INIT_Enternet	;��ʼ����̫��
	BL	$INIT_LCD			;��ʼ��LCD
	BL	$INIT_NORFlash	;��ʼ��NOR Flash
	BL	$INIT_keyboard_INT	;��ʼ������
	BL	$INIT_SDCARD		;��ʼ��SD Card
	LDR	SP,=SvcStackSpace	;���ù���ģʽ��ջ
	MSR	CPSR_c,#Mode_IRQ
	LDR	SP,=IrqStackSpace	;����IRQ�ж�ģʽ��ջ
	MSR 	CPSR_c,#Mode_USR
	LDR	SP,=UsrStackSpace	;�����û���ϵͳģʽ��ջ
		;�����жϵĳ�ʼ��
	LDR	R0,=GPFCON
	LDR	R1,=0b1010			;������GPF0��GPF1���ⲿ�ж��������ţ���ӦEINT0��EINT1�ж�
	STR	R1,[R0]
	
	LDR	R0,EXTINT0
	LDR R1,=0x0		;��ʾEINT0/1�ǵ͵�ƽ����
	STR	R1,[R0]
	
	LDR	R0,=INTMSK
	LDR	R1,=0xFFFFFFFC
	STR	R1,[R0]
	
	LDR R0,=pEINT0	;EINT0���ж����ɢת���е�ַ
	LDR	R1,=EINT0	;�жϷ���������
	STR	R1,[R0]		
	
	
	LDR R0,=pEINT1	;EINT1���ж����ɢת���е�ַ
	LDR	R1,=EINT1	;�жϷ���������
	STR	R1,[R0]
	
	BL	Loop

Loop
	NOP
	B Loop	;��ѭ�������жϴ��
		


HandlerIRQ
		SUB	LR, LR, #4	;���㷵�ص�ַ
		STMFD	SP!, { LR }	;����ϵ㵽IRQģʽ�Ķ�ջ
		LDR	LR,=Int_Return	;�޸�LR��ִ�����жϺ󷵻ص�Int_Return�� 
		LDR	R0,=INTOFFSET	;ȡ���ж�Դ�ı��
		LDR	R1,[R0]
		LDR	R2,= Int_EntryTable 
		LDR	PC,[R2,R1,LSL#2]	;����ɢת����ת
Int_Return	
		LDMFD  	SP!, { PC }^	
	

EINT0	;Ϊ�����������жϷ������
	STMFD	SP!,{LR}
	LDR	LR,=Int_Return	;�޸�LR��ִ�����жϺ󷵻ص�Int_Return��
	B	$C_EINT0

EINT1	;Ϊ������̵��жϴ������
	STMFD	SP!,{LR}
	LDR	LR,=Int_Return	;�޸�LR��ִ�����жϺ󷵻ص�Int_Return��
	B	$C_EINT1

	
		AREA    	MyRWData, DATA, READWRITE	;����RW Base=0x33ffe700
Int_EntryTable
		;�ж�ɢת����32�����
pEINT0	DCD	0
pEINT1	DCD	0
pEINT2	DCD	0
pEINT3	DCD	0
pEINT4_7	DCD	0
pEINT8_23	DCD	0
pINT_CAM	DCD	0
pnBATT_FLT	DCD	0
pINT_TICK	DCD	0
pINT_WDT	DCD	0
pINT_TIMER0	DCD	0
pINT_TIMER1	DCD	0
pINT_TIMER2	DCD	0
pINT_TIMER3	DCD	0
pINT_TIMER4	DCD	0
pINT_UART2	DCD	0
pINT_LCD	DCD	0
pINT_DMA0	DCD	0
pINT_DMA1	DCD	0
pINT_DMA2	DCD	0
pINT_DMA3	DCD	0
pINT_SDI	DCD	0
pINT_SPI0	DCD	0
pINT_UART1	DCD	0
pINT_NFCON	DCD	0
pINT_USBD	DCD	0
pINT_USBH	DCD	0
pINT_IIC	DCD	0
pINT_UART0	DCD	0
pINT_SPI1	DCD	0
pINT_RTC	DCD	0
pINT_ADC	DCD	0
		AREA    	MyZIData, DATA, READWRITE, NOINIT,ALIGN =8
;�˴��Ķ���Ͳ�������Ϊ�˷���ջ���ռ���㣬ͬʱҲΪC���Զ����ȫ�ֱ��������洢�ռ�
		;���ø�ģʽջ���ռ�
			SPACE	0x100*4	;����ģʽ��ջ�ռ�
		SvcStackSpace	SPACE	0x100*4	;�ж�ģʽ��ջ�ռ�
		IrqStackSpace	SPACE	0x100*4	;�û���ϵͳ��ģʽ��ջ�ռ�
		UndtStackSpace	SPACE	0x100*4	;�û���ϵͳ��ģʽ��ջջ��ָ��
		UsrStackSpace
		END 