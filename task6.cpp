/*���ھ�����ã����ø�ʽ���£�
	
	     
    dm_init();              //DM9000��ʼ��            
	....
    dm_tran_packet(arpsendbuf, 42 );   //����ARP�����   

	
	���� arpsendbuf�ǵھ�����Ҫ�����һ����ѭARPЭ���ʽ�����飺
	unsigned char arpsendbuf[42]={   
    
       0xff,0xff,0xff,0xff,0xff,0xff,                     //��̫��Ŀ���ַ��ȫ1��ʾΪ�㲥��ַ   
       0x00,0x01,0x02,0x03,0x04,0x05,        //��̫��Դ��ַ   
       0x08,0x06,                                        //֡���ͣ�ARP֡   
         
       0x00,0x01,                                        //Ӳ�����ͣ���̫��   
       0x08,0x00,                                        //Э�����ͣ�IPЭ��   
       0x06,                                                //Ӳ����ַ���ȣ�6�ֽ�   
       0x04,                                                //Э���ַ���ȣ�4�ֽ�   
       0x00,0x01,                                        //�����룺ARP����   
         
       0x00,0x01,0x02,0x03,0x04,0x05,        //���Ͷ���̫��Ӳ����ַ   
       192, 168, 1, 50,                                 //���Ͷ�IPЭ���ַ   
       0x00,0x00,0x00,0x00,0x00,0x00,        //���ն���̫��Ӳ����ַ   
       192, 168, 1, 120                                 //���ն�IPЭ���ַ   
};  
������ɲο��ҷ���Ⱥ�����ַ��̫���˾Ͳ�д��ע������ */


/*##############################################
��DM9000�У�ֻ����������ֱ�ӱ����������ʵļĴ�������������ΪCMD�˿ں�DATA�˿ڡ�
�������ƺ�״̬�Ĵ����ķ�����Ҫ�� 
��1�������Ĵ����ĵ�ַд��CMD�˿ڣ�
��2������DATA�˿ڶ�д�Ĵ����е����ݣ�
################################################*/

#define DM_ADD (*((volatile unsigned int *) 0x8000300))//��ַ�� 
#define DM_CMD (*((volatile unsigned int *) 0x8000304))//���ݿ� 
void delay(int count){
	for(int i=0;i<count;i++);
}
//��DM9000�Ĵ���д����
void dm_reg_write(unsigned char reg, unsigned char data){
	delay(20);
	DM_ADD = reg;//���Ĵ�����ַд��INDEX�˿�
	delay(20);
	DM_CMD = data;//������д��DATA�˿ڣ���д���Ĵ���

}

//��DM9000�Ĵ���������
unsigned int dm_reg_read(unsigned char reg){
    delay(20);
    DM_ADD = reg;
    delay(20)
    return DM_CMD;//�����ݴӼĴ����ж���
}

/*##############################################
��ʼ��DM9000������һϵ�мĴ�����ֵ 
���мĴ����ĵ�ַ�궨����DM9000.h���Ѷ���á�
################################################*/
void dm_init(void)   
{   
    dm_reg_write(DM9000_NCR,1); //�����λDM9000   
    delay(30);              				  
    dm_reg_write(DM9000_NCR,0); //�����λλ   
    
    dm_reg_write(DM9000_NCR,1); //Ϊ��ȷ����λ��ȷ���ٴθ�λ   
    delay(30);   
    dm_reg_write(DM9000_NCR,0);   
         
    dm_reg_write(DM9000_GPCR,1); //����GPIO0Ϊ���   
    dm_reg_write(DM9000_GPR,	//�����ڲ�PHY   
         
    dm_reg_write(DM9000_NSR,	0x2c); 	//��TX״̬   
    dm_reg_write(DM9000_ISR,0xf);    //���ж�״̬   
         
    dm_reg_write(DM9000_RCR,0x39); //����RX����   
    dm_reg_write(DM9000_TCR,0);   //����TX����   
    dm_reg_write(DM9000_BPTR,0x3f);            
	dm_reg_write(DM9000_FCTR,0x3a);   
	dm_reg_write(DM9000_FCR,0xff);   
	dm_reg_write(DM9000_SMCR,0x00);   
      
    dm_reg_write(DM9000_PAR1,0x00); //����MAC��ַ��00-01-02-03-04-05   
    dm_reg_write(DM9000_PAR2,0x01);           
    dm_reg_write(DM9000_PAR3,0x02);   
	dm_reg_write(DM9000_PAR4,0x03);   
    dm_reg_write(DM9000_PAR5,0x04);   
	dm_reg_write(DM9000_PAR6,0x05);   
      
    dm_reg_write(DM9000_NSR,0x2c); //�ٴ���TX״̬   
    dm_reg_write(DM9000_ISR,0xf);  //�ٴ����ж�״̬   
    
    dm_reg_write(DM9000_IMR,0x81); //�򿪽��������ж�   
}  

/*##############################################
��������
DM9000�ڲ���SRAM���ڽ��ܺͷ������ݻ���
����Ҫ�������ͻ��������ʱ����Ҫ�ֱ��DM9000�Ĵ���MWCMD��MRCMD�������ݶ˿ڣ�������ָ����SRAM�е�ĳ����ַ��
�����ڴ�����һ�����ݺ�ָ���ָSRAM�е���һ����ַ���Ӷ�����������������ݵ�Ŀ�ġ�
������ָ��SRAM������ָ�벻��Ҫ�仯ʱ����Ҫ��MWCMDX�� MRCMDX�������ݶ˿ڡ�
################################################*/
void dm_tran_packet(unsigned char *datas, int length)   
{   
    int i;   
         
    dm_reg_write(DM9000_IMR, 0x80);          //�ڷ������ݹ����н�ֹ�����ж�   
    
    dm_reg_write(DM9000_TXPLH, (length>>8) & 0x0ff);  //���÷������ݳ���   
	dm_reg_write(DM9000_TXPLL, length & 0x0ff);     
    DM_ADDR_PORT = DM9000_MWCMD;            //�������ݻ��渳�����ݶ˿�   
	
	//��������   
    for(i=0;i<length;i+=2)   {   
        delay(50);   
        DM_DATA_PORT = datas[i]|(datas[i+1]<<8);  //8λ����ת��Ϊ16λ�������   
    }       
    dm_reg_write(DM9000_TCR, 0x01);          //�����ݷ��͵���̫����   
    
	while((dm_reg_read(DM9000_NSR) & 0x0c) == 0)   
       ;                           //�ȴ����ݷ������   
    delay(50); 
	  
	dm_reg_write(DM9000_NSR, 0x2c);          //���TX״̬   
	dm_reg_write(DM9000_IMR, 0x81);          //��DM9000���������ж�   
}   

/*##############################################
��������
�������ݱȽϼ򵥣��������ݾ����Ը��ӣ���Ϊ������һ����ʽҪ���
���ֽ����Ϊ0x01�����ʾ����һ�����Խ��յ����� �������Ϊ0x0�����ʾû�пɽ��յ����ݰ���
�ڶ����ֽ�Ϊ���ݰ���һЩ��Ϣ�����ĸ� �ֽڵĸ�ʽ��DM9000�ļĴ���RSR��ȫһ�¡�
�������͵��ĸ��ֽ�Ϊ���ݰ��ĳ��ȡ�
��������ݾ�������Ҫ���յ�������
################################################*/
int dm_rec_packet(unsigned char *datas)  //��������������������׵�ַ������������ݵĳ��ȡ�
{   
    unsigned char int_status;   
    unsigned char rx_ready;   
    unsigned short rx_status;   
    unsigned short rx_length;   
    unsigned short temp;   
    int i;   
    
    int_status = dm_reg_read(DM9000_ISR);           //��ȡISR   
    if(int_status & 0x1)                     //�ж��Ƿ�������Ҫ����   
    {   
        rx_ready = dm_reg_read(DM9000_MRCMDX);       //�ȶ�ȡһ����Ч������   
        rx_ready = (unsigned char)DM_DATA_PORT;      //������ȡ�������ݰ����ֽ�   
                
        if(rx_ready == 1)                 
        {   
            DM_ADDR_PORT = DM9000_MRCMD;      //������ȡ���ݰ�����   
            rx_status = DM_DATA_PORT;        //״̬�ֽ�   
                       
            rx_length = DM_DATA_PORT;       //���ݳ���   
                       
            if(!(rx_status & 0xbf00) && (rx_length < 10000))     //�ж������Ƿ����Ҫ��   
            {   
                for(i=0; i<rx_length; i+=2)          //16λ����ת��Ϊ8λ���ݴ洢   
                {   
                    delay(50);   
                    temp = DM_DATA_PORT;   
                    datas[i] = temp & 0x0ff;   
                    datas[i + 1] = (temp >> 8) & 0x0ff;   
                }   
            }   
        }   
        else if(rx_ready !=0)      //ֹͣ�豸   
        {   
            //dm_reg_write(DM9000_IMR,0x80);  //ֹͣ�ж�   
            //dm_reg_write(DM9000_ISR,0x0F);   //���ж�״̬   
        	//dm_reg_write(DM9000_RCR,0x0);    //ֹͣ����   
            //����Ҫ��λϵͳ��������ʱû�д���   
        }   
    }   
    dm_reg_write(DM9000_ISR, 0x1);             //���ж�   
    return rx_length;   
}  
