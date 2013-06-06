/****************************************************************************
*
* �ļ���: bsp_TEA5767.c
* ���ݼ���: TEA5767 FM������оƬ����
*
* �ļ���ʷ:
* �汾�� ����       ����    ˵��
* 1.0    2010-09-11 armfly  �������ļ�
*
*/
#include "stm32f4xx.h"

#include "bsp_WM8978.h"
#include "i2c_gpio.h"
#include "bsp_tea5767.h"

/****************************************************************************
* ������: void tea5767_Write
* ��  ��: ����TEA5767, ����д��_count���ֽ�;
* ��  ��: _ucaBuf �� ��д�������;
*         _count  : ���ݸ���
* ��  ��: ��.
* ��  ��: ��.
*/
static void tea5767_Write(uint8_t *_ucaBuf, uint8_t _count)
{
	uint8_t i;
	uint8_t ucAck;

	i2c_Start();

	/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
	i2c_SendByte(TEA5767_SLAVE_ADDRESS | I2C_WR);

	/* ���ACK */
	ucAck = i2c_Ack();
	if (ucAck == 1)
	{
		goto err_quit;
	}

	for (i = 0; i < _count; i++)
	{
		/* �������� */
		i2c_SendByte(_ucaBuf[i]);

		/* ���ACK */
		ucAck = i2c_Ack();
		if (ucAck == 1)
		{
			goto err_quit;
		}
	}

err_quit:
	i2c_Stop();
}

/****************************************************************************
* ������: void tea5767_Write
* ��  ��: ��ȡTEA5767�ļĴ���, ��������5���ֽڣ������_ucaBuf
* ��  ��: _ucaBuf �� ���������ָ��
* ��  ��: ��������_ucaBuf.
* ��  ��: ��.
*/
static void tea5767_Read(uint8_t *_ucaBuf)
{
	uint8_t i;
	uint8_t ucAck;

	i2c_Start();

	/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
	i2c_SendByte(TEA5767_SLAVE_ADDRESS | I2C_RD);

	/* ���ACK */
	ucAck = i2c_Ack();
	if (ucAck == 1)
	{
		goto err_quit;
	}

	for (i = 0; i < 5; i++)
	{
		_ucaBuf[i] = i2c_ReadByte();
		if (i == 4)
		{
			i2c_Ack();
		}
		else
		{
			i2c_NAck();
		}
	}

err_quit:
	i2c_Stop();
}

/****************************************************************************
* ������: CaclPLL
* ��  ��: ����ʵ�ʵ�Ƶ��ֵ�����PLL�Ĵ�����ֵ
*	���㹫ʽ ����TEA5767HN  datasheet��20ҳ)
*	N = 4 X (fRF - fIF) / fref;
*	N = decimal value of PLL word;
*	fRF = the wanted tuning frequency [Hz]; ��� 108000000Hz
*	fIF = the intermediate frequency [Hz] = 225 kHz;
*	fref = the reference frequency [Hz] = 32.768 kHz
* ��  ��: _freq �� Ƶ��ֵ;
* ��  ��: ��.
* ��  ��: ��.
*/
static uint16_t CaclPLL(uint32_t _freq)
{
	//pll = (unsigned int)(((temp*10-225)*4000)/32768); 

	return (uint16_t)(((_freq - 225000) * 4) / 32768);
}

/* fRF = pll * fref / 4 + fIF */
static uint32_t CaclFreq(uint16_t _pll)
{
	return ((uint32_t)_pll * 32768) / 4 + 225000;
}

/****************************************************************************
* ������: tea5767_Init
* ��  ��: ��ʼ��TEA767
* ��  ��: 	_Freq 	: Ƶ��
*		  	_ucMuteEn	: ����ѡ��
*			_ucSerchEn ������ѡ��
*			_ucSearchUp ����������ʹ��
*			_ucAdcLevel �� �Զ���ֹ̨ͣ��ƽ��1��������3�����
* ��  ��: ��.
* ��  ��: ��.
*/
void tea5767_Set(uint32_t _Freq, uint8_t _ucMuteEn, uint8_t _ucSerchEn, uint8_t _ucSearchUp, uint8_t _ucAdcLevel)
{
	uint8_t buf[5];
	uint16_t pll;

	/* ����/ŷ��(87.5-108M), �ձ�(76-91MHz) */
	pll = CaclPLL(_Freq);

	/*
		��1���Ĵ�����
		MUTE (bit7) = 0, L R�����Ч
		SM(bit6) = 0 , ������
	*/
	buf[0] = ((pll >> 8) & 0x3F);
	if (_ucMuteEn == MUTE_ON)
	{
		buf[0] |= (1 << 7);
	}
	if (_ucSerchEn == 1)
	{
		buf[0] |= (1 << 6);
	}
		
	/*
		��2���Ĵ�����PLL�ĵ�λ
	*/
	buf[1] = pll;

	/*
		��3���Ĵ���
		SUD(BIT7) = 1, ��������
		SSL(bit6 bit5) = 10  ����ֹͣ����
			00 ��������
			01 �ͼ���ADC ��� = 5
			10 �ͼ���ADC ��� = 7
			11 �ͼ���ADC ��� = 10
			
		HLSI(BIT4) = 0, �ͱ�LOע��
		MS (BIT3) = 0, ѡ��������
		MR (BIT2) = 0, Rͨ��������
		ML (BIT1) = 0, Lͨ��������
		SWP1��bit0) = 0 : �����ɱ�̶˿�
	*/
	if ((_ucAdcLevel < 1) || (_ucAdcLevel > 2))
	{
		_ucAdcLevel = 2;
	} 
	buf[2] = (2 << 5);
	if (_ucSearchUp == SEARCH_UP)
	{
		buf[2] |= (1 << 7);
	}

	/*
		��4���Ĵ���
		SWP2��bit7) = 0 : �����ɱ�̶˿�
		STBY��bit6) = 0 : ѡ��Ǵ���ģʽ
		BL (bit5) = 0, ѡ������/ŷ��Ƶ��
		XTAL(bit4) - 1�� ѡ��32768Hz����
		SMUTE(bit3) = 0, ���������ر�
		HCC(bit2) = 0, ���п��ƹ�(������)
		SNC(bit1) = 1, ���������빦�ܴ�
		SI(bit0) = 1, swport������Ϊ������־��δ��)
	*/
	buf[3] = (1 << 4) | (1 << 1) | (1 << 0);

	/*
		��5���Ĵ���
		PLLREF(BIT7) = 0, PLL��6.5MHz�ο�Ƶ�ʹر�
		DTC(bit6) = 1, ȥ����ʱ�䳣��75uS
	*/
	buf[4] = (1 << 6);

	tea5767_Write(buf, 5);
}

/****************************************************************************
* ������: tea5767_ReadStatus
* ��  ��: ��ȡTEAоƬ��״̬
* ��  ��: ��.
* ��  ��: ��.
* ��  ��: ��.
*/
void tea5767_ReadStatus(TEA5767_T *_tStatus)
{
	uint8_t buf[5];

	tea5767_Read(buf);

	_tStatus->ucReady = (buf[0] >> 7) & 0x01;
	_tStatus->ucBandLimit = (buf[0] >> 6) & 0x01;
	_tStatus->usPll = ((buf[0] & 0x3f) << 8) + buf[1];
	_tStatus->ucStereo =  (buf[2] >> 7) & 0x01;
	_tStatus->ucIFCount = buf[2] & 0x7F;
	_tStatus->ucAdcLevel = (buf[3] >> 4) & 0x0F;

	_tStatus->ulFreq = CaclFreq(_tStatus->usPll);
}
