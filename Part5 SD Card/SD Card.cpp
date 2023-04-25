#include <stdint.h>;
#include "RTC_time.h"
typedef uint32_t u32;///32λ
typedef uint16_t u16;///16λ
typedef uint8_t u8;///8λ

u8 SD_Init(void)
{
    u16 i;
    u8 r1;
    u16 retry;
    u8 buff[6];
    SPI_ControlLine();
    //SD����ʼ��ʱʱ�Ӳ��ܳ���400KHz
    SPI_SetSpeed(SPI_SPEED_LOW);
    //CSΪ�͵�ƽ��Ƭѡ�õͣ�ѡ��SD��		
    SD_CS_ENABLE();
    //����ʱ���ȴ�SD���ϵ��ȶ�
    for (i = 0; i < 0xf00; i++);
    //�Ȳ�������74�����壬��SD����ʼ�����
    for (i = 0; i < 10; i++)
    {
        //���������д������10��ѭ��������80������
        SPI_ReadWriteByte(0xff);
    }
    //-----------------SD����λ��idle״̬----------------
    //ѭ������CMD0��ֱ��SD������0x01,����idle״̬
    //��ʱ��ֱ���˳�
    retry = 0;
    do
    {
        //����CMD0��CRCΪ0x95
        r1 = SD_SendCommand(CMD0, 0, 0x95);
        retry++;
    } while ((r1 != 0x01) && (retry < 200));
    //����ѭ���󣬼������ԭ��
    if (retry == 200)	//˵���ѳ�ʱ	
    {
        return 1;
    }
    //���δ��ʱ��˵��SD����λ��idle����
    //����CMD8�����ȡSD���İ汾��Ϣ
    r1 = SD_SendCommand(CMD8, 0x1aa, 0x87);
    //������SD2.0���ĳ�ʼ��		
    if (r1 == 0x01)
    {
        // V2.0�Ŀ���CMD8�����ᴫ��4�ֽڵ����ݣ�Ҫ�����ٽ���������
        buff[0] = SPI_ReadWriteByte(0xFF);
        buff[1] = SPI_ReadWriteByte(0xFF);
        buff[2] = SPI_ReadWriteByte(0xFF);
        buff[3] = SPI_ReadWriteByte(0xFF);
        SD_CS_DISABLE();
        //�෢8��ʱ��	  
        SPI_ReadWriteByte(0xFF);
        retry = 0;
        //������ʼ��ָ��CMD55+ACMD41
        do
        {
            r1 = SD_SendCommand(CMD55, 0, 0);
            //Ӧ����0x01
            if (r1 != 0x01)
                return r1;
            r1 = SD_SendCommand(ACMD41, 0x40000000, 1);
            retry++;
            if (retry > 200)
                return r1;
        } while (r1 != 0);
        //��ʼ��ָ�����ɣ���������ȡOCR��Ϣ	   
        //----------����SD2.0���汾��ʼ-----------
        //��OCRָ��
        r1 = SD_SendCommand_NoDeassert(CMD58, 0, 0);
        //�������û�з�����ȷӦ��ֱ���˳�������Ӧ��
        if (r1 != 0x00)
            return r1;
        //Ӧ����ȷ�󣬻�ش�4�ֽ�OCR��Ϣ
        buff[0] = SPI_ReadWriteByte(0xFF);
        buff[1] = SPI_ReadWriteByte(0xFF);
        buff[2] = SPI_ReadWriteByte(0xFF);
        buff[3] = SPI_ReadWriteByte(0xFF);
        //OCR������ɣ�Ƭѡ�ø�
        SD_CS_DISABLE();
        SPI_ReadWriteByte(0xFF);
        //�����յ���OCR�е�bit30λ��CSS����ȷ����ΪSD2.0����SDHC
        //CCS=1��SDHC   CCS=0��SD2.0
        if (buff[0] & 0x40)
        {
            SD_Type = SD_TYPE_V2HC;
        }
        else
        {
            SD_Type = SD_TYPE_V2;
        }
        //-----------����SD2.0���汾����----------- 
        SPI_SetSpeed(1); 		//����SPIΪ����ģʽ
    }
}

u8  SPI_ReadWriteByte(u8 TxData)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, TxData);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

u8 SD_ReceiveData(u8* data, u16 len, u8 release)
{
    u16 retry;
    u8 r1;
    //����һ�δ���
    SD_CS_ENABLE();
    retry = 0;
    do
    {
        r1 = SPI_ReadWriteByte(0xFF);
        retry++;
        if (retry > 4000)  //4000�εȴ���û��Ӧ���˳������ɶ��Լ��Σ�
        {
            SD_CS_DISABLE();
            return 1;
        }
    }
    //�ȴ�SD������������ʼ����0xFE
    while (r1 != 0xFE);
    //����ѭ���󣬿�ʼ��������
    while (len--)
    {
        *data = SPI_ReadWriteByte(0xFF);
        data++;
    }
    //����2��αCRC
    SPI_ReadWriteByte(0xFF);
    SPI_ReadWriteByte(0xFF);
    //�����ͷ�����
    if (release == RELEASE)
    {
        SD_CS_DISABLE();
        SPI_ReadWriteByte(0xFF);
    }
    return 0;
}

u8  SD_SendCommand(u8 cmd, u32 arg, u8 crc)
{
    unsigned char r1;
    unsigned int Retry = 0;
    SD_CS_DISABLE();
    //����8��ʱ�ӣ���߼�����
    SPI_ReadWriteByte(0xff);
    //ѡ��SD��
    SD_CS_ENABLE();
    /*����SD�����������п�ʼ�������� */
    //cmd�����ĵڶ�λΪ����λ����ֵΪ1�����Ի�0x40  
    SPI_ReadWriteByte(cmd | 0x40);
    //�����ε�24-31λ����[31..24]
    SPI_ReadWriteByte((u8)(arg >> 24));
    //�����ε�16-23λ����[23..16]	
    SPI_ReadWriteByte((u8)(arg >> 16));
    //�����ε�8-15λ����[15..8]	
    SPI_ReadWriteByte((u8)(arg >> 8));
    //�����ε�0-7λ����[7..0]
    SPI_ReadWriteByte((u8)arg);
    SPI_ReadWriteByte(crc);
    //�ȴ���Ӧ��ʱ�˳�
    while ((r1 = SPI_ReadWriteByte(0xFF)) == 0xFF)
    {
        Retry++;
        if (Retry > 800)	break; 	//��ʱ����
    }
    //�ر�Ƭѡ
    SD_CS_DISABLE();
    //�������϶��ⷢ��8��ʱ�ӣ���SD�����ʣ�µĹ���
    SPI_ReadWriteByte(0xFF);
    //����״ֵ̬	
    return r1;
}

u8 SD_GetCID(u8* cid_data)
{
    u8 r1;
    //��CMD10�����ȡCID��Ϣ
    r1 = SD_SendCommand(CMD10, 0, 0xFF);
    if (r1 != 0x00)
        return r1;  	//��Ӧ�����˳� 
    //����16���ֽڵ�����
    SD_ReceiveData(cid_data, 16, RELEASE);
    return 0;
}

u32 SD_GetCapacity(void)
{
    u8 csd[16];
    u32 Capacity;
    u8 r1;
    u16 i;
    u16 temp;
    //ȡCSD��Ϣ�������������0
    if (SD_GetCSD(csd) != 0)
        return 0;
    //�����CSD�Ĵ�����2.0�汾�������淽ʽ����
    if ((csd[0] & 0xC0) == 0x40)
    {
        Capacity = ((u32)csd[8]) << 8;
        Capacity += (u32)csd[9] + 1;
        Capacity = (Capacity) * 1024;	//�õ�������
        Capacity *= 512;	//�õ��ֽ���   
    }
    else		//CSD�Ĵ�����1.0�汾
    {
        i = csd[6] & 0x03;
        i <<= 8;
        i += csd[7];
        i <<= 2;
        i += ((csd[8] & 0xc0) >> 6);
        r1 = csd[9] & 0x03;
        r1 <<= 1;
        r1 += ((csd[10] & 0x80) >> 7);
        r1 += 2;
        temp = 1;
        while (r1)
        {
            temp *= 2;
            r1--;
        }
        Capacity = ((u32)(i + 1)) * ((u32)temp);
        i = csd[5] & 0x0f;
        temp = 1;
        while (i)
        {
            temp *= 2;
            i--;
        }
        //���ս��
        Capacity *= (u32)temp;
        //�ֽ�Ϊ��λ  
    }
    return (u32)Capacity;
}

u8 SD_ReadSingleBlock(u32 sector, u8* buffer)
{
    u8 r1;
    //����ģʽ
    SPI_SetSpeed(SPI_SPEED_HIGH);
    if (SD_Type != SD_TYPE_V2HC)	//�������SDHC��
    {
        sector = sector << 9;	//512*sector�����������ı߽�����ַ
    }
    r1 = SD_SendCommand(CMD17, sector, 1);	//����CMD17 ������
    if (r1 != 0x00)	return r1;
    r1 = SD_ReceiveData(buffer, 512, RELEASE);	//һ������Ϊ512�ֽ�
    if (r1 != 0)
        return r1;   //�����ݳ���
    else
        return 0; 		//��ȡ��ȷ������0
}

int main(void)
{
    u16 i;
    USART1_Config();
    send_data[1536] = RTC.get_time()
    switch (SD_Init())
    {
    case 0:
        USART1_Puts("\r\nSD Card Init Success!\r\n");
        break;
    case 1:
        USART1_Puts("Time Out!\n");
        break;
    case 99:
        USART1_Puts("No Card!\n");
        break;
    default: USART1_Puts("unknown err\n");
        break;
    }
    SD_WriteSingleBlock(30, send_data);
    SD_ReadSingleBlock(30, receive_data);
    if (Buffercmp(send_data, receive_data, 512))
    {
        USART1_Puts("\r\n single read and write success \r\n");
        //USART1_Puts(receive_data);
    }
    while (1);
}