#include <stdint.h>;
#include "2440Reg_addr.h"
#include "def.h"
#include "option.h" 

typedef uint32_t u32;///32λ
typedef uint16_t u16;///16λ
typedef uint8_t u8;///8λ

struct AlarmTime
{
    int year;
    int month;
    int date;
    int hour;
    int minute;
    int second;
    AlarmTime::AlarmTime(char* RTCtime);
};

u8 SD_Init(void) // ��ʼ��SD��
{
    u32 i;
    rGPECON |= 0xaaaaa800;    // ����GPE5-10ΪSD������
    rSDIPRE = PCLK / (SD_BAUD_RATE)-1;   //����SD�ӿڲ�����Ϊ500kh
    rSDICON = (1 << 4) | 1;
    rSDIFSTA = rSDIFSTA | (1 << 16);
    for (i = 0; i < 0x1000; i++);   //�ȴ�74��SDCLK
    CMD0();  //��ִ��CMD0����ÿ�����idel״̬
    Check_SD_OCR();   //���SD���Ƿ����
    Read_Card_CID(SDCard_Info->CardCID);  //��ȡSD����CID
    Set_Card_RCA(0, &SDCard_Info->CardRCA);  //����RCA���߼���ַ
    Read_Card_CSD(SDCard_info->CardRCA, SDCard_info->CardCSD);     // CSD�����ԼĴ���
    Select_or_Deselect_Card(1, SDCard_Info->CardRCA); //ѡ�п��봫��״̬
    Set_DataBus_Width(SDCard_Info->CardType, 1, SDCard_Info->CardRCA); //�������ߴ���
    return 1;
}

u8  SD_SendCommand(u8 cmd, u32 arg, u8 crc)  //��SD����������
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

u8  SPI_ReadWriteByte(u8 TxData) // SPIģʽ��ȡ���ֽڵ�����
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, TxData);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

u8 SD_ReceiveData(u8* data, u16 len, u8 release) //SD����ȡָ�����ȵ�����
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

u8 SD_WriteSingleBlock(u32 sector, const u8* data)
{
    u8 r1;
    u16 i;
    16 retry;
    //����ģʽ
    SPI_SetSpeed(SPI_SPEED_HIGH);
    //�������SDHC������sector��ַתΪbyte��ַ
    if (SD_Type != SD_TYPE_V2HC)
    {
        sector = sector << 9;
    }
    //д����ָ��
    r1 = SD_SendCommand(CMD24, sector, 0x00);
    if (r1 != 0x00)
    {
        //Ӧ�����ֱ�ӷ���
        return r1;
    }
    //��ʼ׼�����ݴ���
    SD_CS_ENABLE();
    //�ȷ�3�������ݣ��ȴ�SD��׼����
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    //����ʼ����0xFE
    SPI_ReadWriteByte(0xFE);
    //��һ��sector����
    for (i = 0; i < 512; i++)
    {
        SPI_ReadWriteByte(*data++);
    }
    //����2��αCRCУ��
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    //�ȴ�SD��Ӧ��
    r1 = SPI_ReadWriteByte(0xff);
    //���Ϊ0x05˵������д��ɹ�
    if ((r1 & 0x1F) != 0x05)
    {
        SD_CS_DISABLE();
        return r1;
    }
    //�ȴ��������
    retry = 0;
    //���Ա��ʱ�������߱�����
    while (!SPI_ReadWriteByte(0xff))
    {
        retry++;
        if (retry > 65534)        //�����ʱ��û��д����ɣ��˳�����
        {
            SD_CS_DISABLE();
            return 1;           //д�볬ʱ������1
        }
    }
    //д����ɣ�Ƭѡ��1
    SD_CS_DISABLE();
    SPI_ReadWriteByte(0xff);
    return 0;
}

int main()
{
    SD_Init();
    AlarmTime alarm_time(RTC.get_time());   //��RTCģ���ȡ������ʱ��
    SD_WriteSingleBlock(alarm_time.time, alarm_time.release);  //�ѱ���ʱ��д��SD��
    SD_ReceiveData(alarm_time.address, alarm_time.len, alarm_time.release);  //������ʱ��ͨ��SD����ȡ
    return 0;
}
