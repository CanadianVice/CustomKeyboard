#include "ch552.h"
#include "usb.h"
#include "Debug.H"
#include <string.h>

#define Fullspeed			//ȫ���豸
#define THIS_ENDP0_SIZE		0x40	//60   DEFAULT_ENDP0_SIZE

UINT8X Ep0Buffer[MAX_PACKET_SIZE] _at_ 0x0000; 		//�˵�0 OUT&IN��������������ż��ַ
UINT8X Ep1Buffer[MAX_PACKET_SIZE + 2] _at_ 0x0082; 		//�˵�1 IN������,������ż��ַ
UINT8X Ep2Buffer[2 * MAX_PACKET_SIZE] _at_ 0x00c4;  //�˵�2 IN������,������ż��ַ
UINT8X Ep3Buffer[MAX_PACKET_SIZE] _at_ 0x0148; 			//�˵�3 IN������,������ż��ַ

UINT8X Ep4Buffer[MAX_PACKET_SIZE] _at_ 0x0040; 		//�˵�4 IN������ �� �˵�0 ����


UINT8   SetupReq,SetupLen,Ready,Count,FLAG,UsbConfig;
PUINT8  pDescr;                                       //USB���ñ�־
USB_SETUP_REQ   SetupReqBuf;                          //�ݴ�Setup��



#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)
#define DEBUG 0



#pragma  NOAREGS
/*�豸������*/
UINT8C DevDesc[18] = {
	/* �豸������ */
	0x12,       //bLength�ֶΡ��豸�������ĳ���Ϊ18(0x12)�ֽ�
    0x01,       //bDescriptorType�ֶΡ��豸�������ı��Ϊ0x01
    0x10, 0x01, //bcdUSB�ֶΡ��������ð汾ΪUSB1.1����0x0110��
				//������С�˽ṹ�����Ե��ֽ����ȣ���0x10��0x01��
    0x00, 		//bDeviceClass�ֶΡ����ǲ����豸�������ж����豸�࣬
				//���ڽӿ��������ж����豸�࣬���Ը��ֶε�ֵΪ0��
    0x00,       //bDeviceSubClass�ֶΡ�bDeviceClass�ֶ�Ϊ0ʱ�����ֶ�ҲΪ0��
    0x00,       //bDeviceProtocol�ֶΡ�bDeviceClass�ֶ�Ϊ0ʱ�����ֶ�ҲΪ0��
    THIS_ENDP0_SIZE,//0x08 //bMaxPacketSize0�ֶΡ� �˵�0��С��8�ֽڡ�
    0x31, 0x51, //VID  idVender�ֶ�,ע��С��ģʽ�����ֽ����ȡ�
    0x19, 0x20, //PID  idProduct�ֶ� ��ƷID�š�ע��С��ģʽ�����ֽ�Ӧ����ǰ��
    0x00, 0x00, //bcdDevice�ֶΡ�ע��С��ģʽ�����ֽ�Ӧ����ǰ��
    0x00,       //iManufacturer�ֶΡ������ַ���������
    0x00,       //iProduct�ֶΡ���Ʒ�ַ���������ֵ,ע���ַ�������ֵ��Ҫʹ����ͬ��ֵ��
    0x00,       //iSerialNumber�ֶΡ��豸�����к��ַ�������ֵ��
    0x01        //bNumConfigurations�ֶΡ����豸�����е���������
};

UINT8C CfgDesc[116] =		//34
{
/***************** ���������� *******************/
	0x09,	/* bLength: ���ȣ��豸�ַ����ĳ���Ϊ9�ֽ� */
	0x02,	/* ���ͣ������������������� */
	116,	//�����������ܳ��� 0x42:66    0x5b:91
	0x00,
	0x04,	/* �ӿ�����  �ӿںͶ˵��ǲ��õ�*/
	0x01,	/*bConfigurationValue: Configuration value*/
	0x04,	//0x00 /* �����õ��ַ���������ֵ����ֵΪ0��ʾû���ַ��� */
	0xA0,	/*bmAttributes: Self powered */
	0x32,	/* �������ϻ�õ�Դ��С���������100ma */
	
//////////////////////////////////////////////////
/************** ���̽ӿڵ������� ****************/
	0x09,	/* �ӿ��������ĳ���Ϊ9�ֽ� */
	0x04,	/* ����ӿ������� */
	0x00,	/* �ýӿڱ�� */
	0x00,	/* �ýӿڵñ��ñ�� */
	0x01,	/* �ýӿ���ʹ�ö˵����� */
	0x03,	/* �ýӿ���ʹ�õ���   ��������HID */
	0x00,	//0x00	/* �ýӿ����õ����� 1=BOOT, 0=no boot */
	0x01,	/* ������1   �����2  0���Զ����豸 */
	0x00,   /* �ýӿ��ַ���������  һ�㶼Ϊ0 */

//////////////////////////////////////////////////
/************** ����HID�������� *****************/
	0x09,	/* bLength: ���ȣ�HID�������ĳ���Ϊ9�ֽ� */
	0x21,	/* HID������������ */
	0x10,	/* HIDЭ��İ汾 */
	0x01,
	0x00,	/* ���Ҵ��� */
	0x01,	/* �¼������������� */
	0x22,	/* �¼������������� */
	0x3e,	/* ��һ���������ĳ���  62 */
	0x00,

/////////////////////////////////////////////////////////////////
/************** ��������˿�������(�˵�������) *****************/
	0x07,	/* �˵��������ĳ���Ϊ7�ֽ� */
	0x05,	/* �˵������������� */
	0x81,	/* �ö˵�(����)�ĵ�ַ,D7:0(OUT),1(IN),D6~D4:����,D3~D0:�˵�� */
	0x03,	/* �˵������ΪΪ�ж϶˵�. D0~D1��ʾ��������:0(���ƴ���),
			 1(��ʱ����),2(��������),3(�жϴ���) �ǵ�ʱ����˵�:D2~D7:����Ϊ0 ��ʱ����˵㣺 
			 D2~D3��ʾͬ��������:0(��ͬ��),1(�첽),2(����),3(ͬ��) D4~D5��ʾ��;:0(���ݶ˵�),
			 1(�����˵�),2(�������������ݶ˵�),3(����)��D6~D7:���� */
	0x08,	/* �ö˵�֧�ֵ��������� */
	0x00,
	0x02,	/* ��ѯ��� bInterval: Polling Interval (6 ms)*/

///////////////////////////////////////////////////////////
/************** �Զ���HID�豸  �ӿ������� ****************/
	0x09,	/* �ӿ��������ĳ���Ϊ9�ֽ� */
	0x04,	/* ����ӿ������� */
	0x01,	/* �ýӿڱ�� */
	0x00,	/* �ýӿڵñ��ñ�� */
	0x02,	/* �ýӿ���ʹ�ö˵����� */
	0x03,	/* �ýӿ���ʹ�õ���   �������HID */
	0x00,	/* �ýӿ����õ����� 1=BOOT, 0=no boot */
	0x00,	/* ������1   �����2  0���Զ����豸 */
	0x00,//   /* �ýӿ��ַ���������  һ�㶼Ϊ0 */

/////////////////////////////////////////////
/************** �Զ���HID�豸 HID������ ****************/
	0x09,	/* bLength: ���ȣ�HID�������ĳ���Ϊ9�ֽ� */
	0x21,	/* HID������������ */
	0x00,	/* HIDЭ��İ汾 1.00 */
	0x01,
	0x00,	/* ���Ҵ��� */
	0x01,	/* �¼������������� */
	0x22,	/* �¼������������� */
	0x22,//	/* ��һ���������ĳ��� */ //0x34  0x36
	0x00,

///////////////////////////////////////////////////////////////
/************** �Զ���HID�豸 ������(����˵�������) *****************/
	0x07,	/* �˵��������ĳ���Ϊ7�ֽ� */
	0x05,	/* �˵������������� */
	0x82,	/* �ö˵�(����)�ĵ�ַ,D7:0(OUT),1(IN),D6~D4:����,D3~D0:�˵�� */
	0x03,	/*�˵������ΪΪ�ж϶˵�. D0~D1��ʾ��������:0(���ƴ���),
			 1(��ʱ����),2(��������),3(�жϴ���) �ǵ�ʱ����˵�:D2~D7:����Ϊ0 ��ʱ����˵㣺 
			 D2~D3��ʾͬ��������:0(��ͬ��),1(�첽),2(����),3(ͬ��) D4~D5��ʾ��;:0(���ݶ˵�),
			 1(�����˵�),2(�������������ݶ˵�),3(����)��D6~D7:����*/
	0x40,	/* �ö˵�֧�ֵ���������  64 */
	0x00,
	0x0a,    /* ��ѯ��� bInterval: Polling Interval (2 ms)*/	

///////////////////////////////////////////////////////////////
/************** �Զ���HID�豸 ������(����˵�������) *****************/
	0x07,	/* �˵��������ĳ���Ϊ7�ֽ� */
	0x05,	/* �˵������������� */
	0x02,	/* �ö˵�(����)�ĵ�ַ,D7:0(OUT),1(IN),D6~D4:����,D3~D0:�˵�� */
	0x03,	/*�˵������ΪΪ�ж϶˵�. D0~D1��ʾ��������:0(���ƴ���),
			 1(��ʱ����),2(��������),3(�жϴ���) �ǵ�ʱ����˵�:D2~D7:����Ϊ0 ��ʱ����˵㣺 
			 D2~D3��ʾͬ��������:0(��ͬ��),1(�첽),2(����),3(ͬ��) D4~D5��ʾ��;:0(���ݶ˵�),
			 1(�����˵�),2(�������������ݶ˵�),3(����)��D6~D7:����*/
	0x40,	/* �ö˵�֧�ֵ���������  64 */
	0x00,
	0x0a,    /* ��ѯ��� bInterval: Polling Interval (2 ms)*/	
//66

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/************** HID ����豸  �ӿ������� ****************/
	0x09,	/* �ӿ��������ĳ���Ϊ9�ֽ� */
	0x04,	/* ����ӿ������� */
	0x02,	/* �ýӿڱ�� */
	0x00,	/* �ýӿڵñ��ñ�� */
	0x01,	/* �ýӿ���ʹ�ö˵����� */
	0x03,	/* �ýӿ���ʹ�õ���   �������HID */
	0x01,	/* �ýӿ����õ����� 1=BOOT, 0=no boot */
	0x02,	/* ������1   �����2  0���Զ����豸 */
	0x00,//   /* �ýӿ��ַ���������  һ�㶼Ϊ0 */

/////////////////////////////////////////////
/************** HID ����豸 HID������ ****************/
	0x09,	/* bLength: ���ȣ�HID�������ĳ���Ϊ9�ֽ� */
	0x21,	/* HID������������ */
	0x10,	/* HIDЭ��İ汾 1.00 */
	0x01,
	0x00,	/* ���Ҵ��� */
	0x01,	/* �¼������������� */
	0x22,	/* �¼������������� */
	0x34,//	/* ��һ���������ĳ��� */ //0x34  0x36
	0x00,

///////////////////////////////////////////////////////////////
/************** HID ����豸 ������(����˵�������) *****************/
	0x07,	/* �˵��������ĳ���Ϊ7�ֽ� */
	0x05,	/* �˵������������� */
	0x83,	/* �ö˵�(����)�ĵ�ַ,D7:0(OUT),1(IN),D6~D4:����,D3~D0:�˵�� */
	0x03,	/*�˵������ΪΪ�ж϶˵�. D0~D1��ʾ��������:0(���ƴ���),
			 1(��ʱ����),2(��������),3(�жϴ���) �ǵ�ʱ����˵�:D2~D7:����Ϊ0 ��ʱ����˵㣺 
			 D2~D3��ʾͬ��������:0(��ͬ��),1(�첽),2(����),3(ͬ��) D4~D5��ʾ��;:0(���ݶ˵�),
			 1(�����˵�),2(�������������ݶ˵�),3(����)��D6~D7:����*/
	0x04,	/* �ö˵�֧�ֵ���������  64 */
	0x00,
	0x01,    /* ��ѯ��� bInterval: Polling Interval (2 ms)*/

//��ý��
	0x09,0x04,0x03,0x00,0x01,0x03,0x00,0x00,0x00,	//�ӿ�������
	0x09,0x21,0x10,0x01,0x00,0x01,0x22,41,0x00,	//HID������
	0x07,0x05,0x84,0x03,0x02,0x00,0x01 //�˵�������

};


/*HID�౨��������*/
UINT8C KeyRepDesc[62] =
{
	0x05,0x01,	// USAGE_PAGE (Generic Desktop)
	0x09,0x06,	// USAGE (Keyboard)
	0xA1,0x01,	// COLLECTION (Application)

	0x05,0x07,	//   USAGE_PAGE (Keyboard)
	0x19,0xe0,	//   USAGE_MINIMUM (Keyboard LeftControl)
	0x29,0xe7,	//   USAGE_MAXIMUM (Keyboard Right GUI)
	0x15,0x00,	//   LOGICAL_MINIMUM (0)
	0x25,0x01,	//   LOGICAL_MAXIMUM (1)
	0x75,0x01,	//   REPORT_SIZE (1)
	0x95,0x08,	//   REPORT_COUNT (8)
	0x81,0x02,	//   INPUT (Data,Var,Abs)

	0x95,0x01,	//   REPORT_COUNT (1)
	0x75,0x08,	//   REPORT_SIZE (8)
	//
	0x81,0x01,	//   INPUT (Cnst,Var,Abs) 
	0x95,0x03,	//   REPORT_COUNT (3)	   

	0x75,0x01,	//   REPORT_SIZE (1)	  
	0x05,0x08,	//   USAGE_PAGE (LEDs)
	0x19,0x01,	//   USAGE_MINIMUM (Num Lock)
	0x29,0x03,	//   USAGE_MAXIMUM (Kana)
	0x91,0x02,	//   Output (Data,Var,Abs,NWrp,Lin,Pref,NNul,NVol,Bit)
	0x95,0x05,	//   REPORT_COUNT (1)
	0x75,0x01,	//   REPORT_SIZE (3)
	0x91,0x01,	//   Output (Cnst,Ary,Abs,NWrp,Lin,Pref,NNul,NVol,Bit)
	0x95,0x06,	//   REPORT_COUNT (6)
	0x75,0x08,	//   REPORT_SIZE (8)
	0x26,0xff,0x00,		//Logical Maximum (255)
	0x05,0x07,	//   USAGE_PAGE (Keyboard/Keypad)
	0x19,0x00,	//   USAGE_MINIMUM (Undefined)
	0x29,0x91,	//   USAGE_MAXIMUM (Keyboard LANG2)
	0x81,0x00,	//   INPUT (Data,Ary,Abs)
	0xC0		//	 End Collection
};
UINT8C HID_RepDesc[34] =
{
	0x06, 0x00,0xff,
    0x09, 0x01,
    0xa1, 0x01,                     //���Ͽ�ʼ
    0x09, 0x02,                     //Usage Page  �÷�
    0x15, 0x00,                     //Logical  Minimun
    0x26, 0x00,0xff,				//Logical  Maximun
    0x75, 0x08,                     //Report Size
    0x95, THIS_ENDP0_SIZE,          //Report Counet
    0x81, 0x06,                     //Input
    0x09, 0x02,                     //Usage Page  �÷�
    0x15, 0x00,                     //Logical  Minimun
    0x26, 0x00,0xff,                //Logical  Maximun
    0x75, 0x08,                     //Report Size
    0x95, THIS_ENDP0_SIZE,          //Report Counet
    0x91, 0x06,                     //Output
    0xC0
};
UINT8C MouseRepDesc[52] =
{
    0x05,0x01,0x09,0x02,0xA1,0x01,0x09,0x01,
    0xA1,0x00,0x05,0x09,0x19,0x01,0x29,0x03,
    0x15,0x00,0x25,0x01,0x75,0x01,0x95,0x03,
    0x81,0x02,0x75,0x05,0x95,0x01,0x81,0x01,
    0x05,0x01,0x09,0x30,0x09,0x31,0x09,0x38,
    0x15,0x81,0x25,0x7f,0x75,0x08,0x95,0x03,
    0x81,0x06,0xC0,0xC0
};

UINT8C MultimediaRepDesc[41] =
{
	0x05, 0x0C,  
	0x09, 0x01,
	0xA1, 0x01,
	0x15, 0x00,
	0x25, 0x01,
	0x0A, 0xEA, 0x00,
	0x0A, 0xE9, 0x00,
	0x0A, 0xCD, 0x00,
	0x0A, 0xB6, 0x00,
	0x0A, 0xB5, 0x00,
	0x0A, 0xB7, 0x00,
	0x0A, 0x00, 0x00,
	0x0A, 0x00, 0x00,
	0x75, 0x01,
	0x95, 0x08,
	0x81, 0x02,
	0xC0
};

/*�Զ���HID����*/
UINT8 UserEp2Buf[64] = {0x0};
UINT8 Endp2Busy = 0;			//�Զ���HID���ͱ�־
UINT8 Endp2Rev = 0;				//�Զ���HID���ձ�־	

/*��������*/
UINT8 HIDKey[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
/* ������� */
UINT8 HIDMouse[4] = {0x0,0x0,0x0,0x0};
/* ��ý������ */
UINT8 HIDMultimedia[1] = {0x0};
/*******************************************************************************
* Function Name  : USBDeviceInit()
* Description    : USB�豸ģʽ����,�豸ģʽ�������շ��˵����ã��жϿ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceInit(void)
{
	IE_USB = 0;					// �ر�USB�ж�
	USB_CTRL = 0x00;          	// ���趨USB�豸ģʽ
	UDEV_CTRL = bUD_PD_DIS;   	// ��ֹDP/DM��������

#ifndef Fullspeed
    UDEV_CTRL |= bUD_LOW_SPEED;          //ѡ�����1.5Mģʽ
    USB_CTRL |= bUC_LOW_SPEED;
#else
    UDEV_CTRL &= ~bUD_LOW_SPEED;         //ѡ��ȫ��12Mģʽ��Ĭ�Ϸ�ʽ
    USB_CTRL &= ~bUC_LOW_SPEED;
#endif
	
	UEP3_DMA = Ep3Buffer;                              //�˵�3���ݴ����ַ
	UEP2_3_MOD |= bUEP3_TX_EN;           				//�˵�3���ͽ���ʹ��
	UEP2_3_MOD &= ~bUEP3_RX_EN;
    UEP2_3_MOD &= ~bUEP3_BUF_MOD;                      //�˵�3�շ���64�ֽڻ�����
    UEP3_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;     	   //�˵�3�Զ���תͬ����־λ��IN���񷵻�NAK��OUT����ACK
	
    UEP2_DMA = Ep2Buffer;                              //�˵�2���ݴ����ַ
	UEP2_3_MOD |= bUEP2_TX_EN | bUEP2_RX_EN;           //�˵�2���ͽ���ʹ��
    UEP2_3_MOD &= ~bUEP2_BUF_MOD;                      //�˵�2�շ���64�ֽڻ�����
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;     //�˵�2�Զ���תͬ����־λ��IN���񷵻�NAK��OUT����ACK

	UEP1_DMA = Ep1Buffer;                              //�˵�1���ݴ����ַ
    UEP4_1_MOD = bUEP1_TX_EN;							//�˵�1����ʹ�� 64�ֽڻ�����
	UEP4_1_MOD &= ~bUEP1_RX_EN;
	UEP4_1_MOD &= ~bUEP1_BUF_MOD;
    UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;         //�˵�1�Զ���תͬ����־λ��IN���񷵻�NAK	

	UEP0_DMA = Ep0Buffer;                              //�˵�0���ݴ����ַ
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;         //OUT���񷵻�ACK��IN���񷵻�NAK
	
	UEP4_1_MOD |= bUEP4_TX_EN;                         //�˵�4����ʹ��
    UEP4_1_MOD &= ~bUEP4_RX_EN;    
	UEP4_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;
	
/**/
	USB_DEV_AD = 0x00;
	USB_CTRL |= bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;		// ����USB�豸��DMA�����ж��ڼ��жϱ�־δ���ǰ�Զ�����NAK
	UDEV_CTRL |= bUD_PORT_EN;                                 	// ����USB�˿�
	USB_INT_FG = 0xFF;                                        	// ���жϱ�־
	USB_INT_EN = bUIE_SUSPEND | bUIE_TRANSFER | bUIE_BUS_RST;
	IE_USB = 1;
}
/*******************************************************************************
* Function Name  : Enp1IntIn()
* Description    : USB�豸ģʽ�˵�1���ж��ϴ�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Enp1IntIn( )
{
	while((UEP1_CTRL & MASK_UEP_T_RES) == UEP_T_RES_ACK);    //�ȴ��ϴ���ɣ�����ͬʱ��д������
    memcpy( Ep1Buffer, HIDKey, sizeof(HIDKey));                                //�����ϴ�����
    UEP1_T_LEN = sizeof(HIDKey);                                               //�ϴ����ݳ���
    UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;                  //������ʱ�ϴ����ݲ�Ӧ��ACK
}
/*******************************************************************************
* Function Name  : Enp2IntIn()
* Description    : USB�豸ģʽ�˵�2���ж��ϴ�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Enp2IntIn( )
{
	while((UEP2_CTRL & MASK_UEP_T_RES) == UEP_T_RES_ACK);    //�ȴ��ϴ���ɣ�����ͬʱ��д������
    memcpy( Ep2Buffer + MAX_PACKET_SIZE, UserEp2Buf, sizeof(UserEp2Buf));      //�����ϴ�����
    UEP2_T_LEN = sizeof(UserEp2Buf);                                           //�ϴ����ݳ���
    UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;                  //������ʱ�ϴ����ݲ�Ӧ��ACK
}
/*******************************************************************************
* Function Name  : Enp3IntIn()
* Description    : USB�豸ģʽ�˵�3���ж��ϴ�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Enp3IntIn( )
{
	while((UEP3_CTRL & MASK_UEP_T_RES) == UEP_T_RES_ACK);    //�ȴ��ϴ���ɣ�����ͬʱ��д������
    memcpy( Ep3Buffer, HIDMouse, sizeof(HIDMouse));          //�����ϴ�����
    UEP3_T_LEN = sizeof(HIDMouse);                                             //�ϴ����ݳ���
    UEP3_CTRL = UEP3_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;                  //������ʱ�ϴ����ݲ�Ӧ��ACK
}

void Enp4IntIn( )
{
	while((UEP4_CTRL & MASK_UEP_T_RES) == UEP_T_RES_ACK);    //�ȴ��ϴ���ɣ�����ͬʱ��д������
    memcpy( Ep4Buffer, HIDMultimedia, sizeof(HIDMultimedia));          //�����ϴ�����
    UEP4_T_LEN = sizeof(HIDMultimedia);                                             //�ϴ����ݳ���
    UEP4_CTRL = UEP4_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;                  //������ʱ�ϴ����ݲ�Ӧ��ACK
}
/*******************************************************************************
* Function Name  : DeviceInterrupt()
* Description    : CH552USB�жϴ�����
*******************************************************************************/
void DeviceInterrupt( void ) interrupt INT_NO_USB using 1                      //USB�жϷ������,ʹ�üĴ�����1
{
    UINT8 len,i;
    if(UIF_TRANSFER)                                                           //USB������ɱ�־
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
		case UIS_TOKEN_IN | 4:                                                 //endpoint 4# �ж϶˵��ϴ�
            UEP4_T_LEN = 0;                                                    //Ԥʹ�÷��ͳ���һ��Ҫ���
			UEP4_CTRL ^= bUEP_T_TOG;                                           //�˵�4ֻ���ֶ���ת
			UEP4_CTRL = UEP4_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;          //Ĭ��Ӧ��NAK
            break;
		case UIS_TOKEN_IN | 3:        //endpoint 3# �˵������ϴ�
            UEP3_T_LEN = 0;           //Ԥʹ�÷��ͳ���һ��Ҫ���
            UEP3_CTRL = UEP3_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;          //Ĭ��Ӧ��NAK
			FLAG = 1; 
            break;
			
        case UIS_TOKEN_IN | 2:        //endpoint 2# �˵������ϴ�
            UEP2_T_LEN = 0;           //Ԥʹ�÷��ͳ���һ��Ҫ���
            UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;          //Ĭ��Ӧ��NAK
			Endp2Busy = 0;
            break;
		
		case UIS_TOKEN_OUT | 2:        //endpoint 2# �˵������´�
            if(U_TOG_OK)
			{
				len = USB_RX_LEN;	//�������ݳ���,���ݴ�Ep2Buffer�׵�ַ��ʼ���
				for(i=0;i<len;i++)
				{
					Ep2Buffer[MAX_PACKET_SIZE+i] = Ep2Buffer[i] ^ 0xFF; // OUT����ȡ����IN�ɼ������֤
				}
				UEP2_T_LEN = len;
				UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;       // �����ϴ�
				Endp2Rev = 1;
			}
            break;
			
        case UIS_TOKEN_IN | 1:                                                  //endpoint 1# �ж϶˵��ϴ�
            UEP1_T_LEN = 0;                                                     //Ԥʹ�÷��ͳ���һ��Ҫ���
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //Ĭ��Ӧ��NAK
            FLAG = 1;                                                           /*������ɱ�־*/
            break;
		
        case UIS_TOKEN_SETUP | 0:  //SETUP����  USB�ϱ�
            len = USB_RX_LEN;
            if(len == (sizeof(USB_SETUP_REQ)))
            {
                SetupLen = UsbSetupBuf->wLengthL;
                if(UsbSetupBuf->wLengthH || SetupLen > 0x7F )
                {
                    SetupLen = 0x7F;   // �����ܳ���
                }
                len = 0;               // Ĭ��Ϊ�ɹ������ϴ�0����
                SetupReq = UsbSetupBuf->bRequest;
                if ( ( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )/* HID������ */
                {
					switch( SetupReq ) 
					{
						case 0x01://GetReport
							pDescr = UserEp2Buf;              //���ƶ˵��ϴ����
							if(SetupLen >= THIS_ENDP0_SIZE)   //���ڶ˵�0��С����Ҫ���⴦��
							{
								len = THIS_ENDP0_SIZE;
							}													 
							else
							{								 
								len = SetupLen;
							}		
							break;
						case 0x02://GetIdle
							break;	
						case 0x03://GetProtocol
							break;				
						case 0x09://SetReport										
							break;
						case 0x0A://SetIdle
							break;	
						case 0x0B://SetProtocol
							break;
						default:
							len = 0xFF; /*���֧��*/					
							break;
					}	
                }
                else	  //��׼����
                {           
                    switch(SetupReq)          //������
                    {
                    case USB_GET_DESCRIPTOR:
                        switch(UsbSetupBuf->wValueH)
                        {
                        case USB_DESCR_TYP_DEVICE:    //�豸������
                            pDescr = DevDesc;         //���豸�������͵�Ҫ���͵Ļ�����
                            len = sizeof(DevDesc);
                            break;
                        case USB_DESCR_TYP_CONFIG:    //����������
                            pDescr = CfgDesc;         //���豸�������͵�Ҫ���͵Ļ�����
                            len = sizeof(CfgDesc);
                            break;
                        case USB_DESCR_TYP_REPORT:    //����������
                            if(UsbSetupBuf->wIndexL == 0)                   //�ӿ�0����������
                            {
                                pDescr = KeyRepDesc;                        //����׼���ϴ�
                                len = sizeof(KeyRepDesc);
                            }
                            else if(UsbSetupBuf->wIndexL == 1)              //�ӿ�1����������
                            {
                                pDescr = HID_RepDesc;                      //����׼���ϴ�
                                len = sizeof(HID_RepDesc);                                
                            }
							else if(UsbSetupBuf->wIndexL == 2)              //�ӿ�2����������
                            {
                                pDescr = MouseRepDesc;                      //����׼���ϴ�
                                len = sizeof(MouseRepDesc);                                
                            }
							else if(UsbSetupBuf->wIndexL == 3)              //�ӿ�3����������
                            {
                                pDescr = MultimediaRepDesc;                 //����׼���ϴ�
                                len = sizeof(MultimediaRepDesc);                                
                            }
                            else
                            {
                                len = 0xff;                                 //������ֻ��3���ӿڣ���仰����������ִ��
                            }
                            break;
							
                        default:
                            len = 0xff;                                     //��֧�ֵ�������߳���
                            break;
                        }
                        if ( SetupLen > len )
                        {
                            SetupLen = len;    //�����ܳ���
                        }
                        len = SetupLen >= THIS_ENDP0_SIZE ? THIS_ENDP0_SIZE : SetupLen; //���δ��䳤��
                        memcpy(Ep0Buffer,pDescr,len);                        //�����ϴ�����
                        SetupLen -= len;
                        pDescr += len;
                        break;
                    case USB_SET_ADDRESS:
                        SetupLen = UsbSetupBuf->wValueL;                     //�ݴ�USB�豸��ַ
                        break;
                    case USB_GET_CONFIGURATION:
                        Ep0Buffer[0] = UsbConfig;
                        if ( SetupLen >= 1 )
                        {
                            len = 1;
                        }
                        break;
                    case USB_SET_CONFIGURATION:
                        UsbConfig = UsbSetupBuf->wValueL;
						if(UsbConfig)
						{
							Ready = 1;                                      //set config����һ�����usbö����ɵı�־
						}
						break;
                    case 0x0A:
                        break;
                    case USB_CLEAR_FEATURE:                                            //Clear Feature
                        if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )// �˵�
                        {
                            switch( UsbSetupBuf->wIndexL )
                            {
								case 0x84:
									UEP4_CTRL = UEP4_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
									break;
								case 0x83:
									UEP3_CTRL = UEP3_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
									break;
								case 0x82:
									UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
									break;
								case 0x81:
									UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
									break;
								case 0x01:
									UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
									break;
								case 0x02:
									UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
									break;
								case 0x03:
									UEP3_CTRL = UEP3_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
								case 0x04:
									UEP4_CTRL = UEP4_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
									break;
								default:
									len = 0xFF;                                            // ��֧�ֵĶ˵�
									break;
                            }
                        }
                        if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_DEVICE )// �豸
                        {
							break;
                        }													
                        else
                        {
                            len = 0xFF;                                                // ���Ƕ˵㲻֧��
                        }
                        break;
                    case USB_SET_FEATURE:                                              /* Set Feature */
                        if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x00 )             /* �����豸 */
                        {
                            if( ( ( ( UINT16 )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
                            {
                                if( CfgDesc[ 7 ] & 0x20 )
                                {
                                    /* ���û���ʹ�ܱ�־ */
                                }
                                else
                                {
                                    len = 0xFF;                                        /* ����ʧ�� */
                                }
                            }
                            else
                            {
                                len = 0xFF;                                            /* ����ʧ�� */
                            }
                        }
                        else if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x02 )        /* ���ö˵� */
                        {
                            if( ( ( ( UINT16 )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x00 )
                            {
                                switch( ( ( UINT16 )UsbSetupBuf->wIndexH << 8 ) | UsbSetupBuf->wIndexL )
                                {
									case 0x84:
										UEP4_CTRL = UEP4_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
										break;
									case 0x83:
										UEP3_CTRL = UEP3_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* ���ö˵�3 IN STALL */
										break;
									case 0x82:
										UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* ���ö˵�2 IN STALL */
										break;
									case 0x02:
										UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* ���ö˵�2 OUT Stall */
										break;
									case 0x81:
										UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* ���ö˵�1 IN STALL */
										break;
									default:
										len = 0xFF;                               //����ʧ��
										break;
                                }
                            }
                            else
                            {
                                len = 0xFF;                                   //����ʧ��
                            }
                        }
                        else
                        {
                            len = 0xFF;                                      //����ʧ��
                        }
                        break;
                    case USB_GET_STATUS:
                        Ep0Buffer[0] = 0x00;
                        Ep0Buffer[1] = 0x00;
                        if ( SetupLen >= 2 )
                        {
                            len = 2;
                        }
                        else
                        {
                            len = SetupLen;
                        }
                        break;
                    default:
                        len = 0xff;                                           //����ʧ��
                        break;
                    }
                }
            }
            else
            {
                len = 0xff;                                                   //�����ȴ���
            }
            if(len == 0xff)
            {
                SetupReq = 0xFF;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;//STALL
            }
            else if(len <= THIS_ENDP0_SIZE)       //�ϴ����ݻ���״̬�׶η���0���Ȱ�
            {
                UEP0_T_LEN = len;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//Ĭ�����ݰ���DATA1������Ӧ��ACK
            }
            else
            {
                UEP0_T_LEN = 0;  //��Ȼ��δ��״̬�׶Σ�������ǰԤ���ϴ�0�������ݰ��Է�������ǰ����״̬�׶�
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//Ĭ�����ݰ���DATA1,����Ӧ��ACK
            }
            break;
        case UIS_TOKEN_IN | 0:                                               //endpoint0 IN
            switch(SetupReq)
            {
            case USB_GET_DESCRIPTOR:
                len = SetupLen >= THIS_ENDP0_SIZE ? THIS_ENDP0_SIZE : SetupLen;    //���δ��䳤��
                memcpy( Ep0Buffer, pDescr, len );                            //�����ϴ�����
                SetupLen -= len;
                pDescr += len;
                UEP0_T_LEN = len;
                UEP0_CTRL ^= bUEP_T_TOG;                                     //ͬ����־λ��ת
                break;
            case USB_SET_ADDRESS:
                USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            default:
                UEP0_T_LEN = 0;                                              //״̬�׶�����жϻ�����ǿ���ϴ�0�������ݰ��������ƴ���
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            }
            break;
        case UIS_TOKEN_OUT | 0:  // endpoint0 OUT
            len = USB_RX_LEN;
            if(SetupReq == 0x09)
            {
                if(Ep0Buffer[0])
                {
                    //printf("Light on Num Lock LED!\n");
                }
                else if(Ep0Buffer[0] == 0)
                {
                    //printf("Light off Num Lock LED!\n");
                }				
            }
            UEP0_CTRL ^= bUEP_R_TOG;                                     //ͬ����־λ��ת						
            break;
        default:
            break;
        }
        UIF_TRANSFER = 0;                                                 //д0����ж�
    }
    if(UIF_BUS_RST)                                                       //�豸ģʽUSB���߸�λ�ж�
    {
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        UEP1_CTRL = bUEP_AUTO_TOG | UEP_R_RES_ACK;
        UEP2_CTRL = bUEP_AUTO_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
		UEP3_CTRL = bUEP_AUTO_TOG | UEP_R_RES_ACK;
        USB_DEV_AD = 0x00;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
        Endp2Busy = 0;
		UIF_BUS_RST = 0;                                                 //���жϱ�־
    }
    if (UIF_SUSPEND)                                                     //USB���߹���/�������
    {
        UIF_SUSPEND = 0;
        if ( USB_MIS_ST & bUMS_SUSPEND )                                 //����
        {
#if DEBUG
           // printf( "zz" );                                              //˯��״̬
#endif
//             while ( XBUS_AUX & bUART0_TX );                              //�ȴ��������
//             SAFE_MOD = 0x55;
//             SAFE_MOD = 0xAA;
//             WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO;                      //USB����RXD0���ź�ʱ�ɱ�����
//             PCON |= PD;                                                  //˯��
//             SAFE_MOD = 0x55;
//             SAFE_MOD = 0xAA;
//             WAKE_CTRL = 0x00;
        }
    }
    else {                                                               //������ж�,�����ܷ��������
        USB_INT_FG = 0xFF;                                               //���жϱ�־
//      printf("UnknownInt  N");
    }
}

