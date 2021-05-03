#include "CH552.H"                                                      
#include "Debug.H"
#include <string.h>
#include <CH552.H>

//#include "pn297_L.h"
//#include "DataFlash.h"
//#include <stdio.h>

#define Fullspeed
#define THIS_ENDP0_SIZE         DEFAULT_ENDP0_SIZE

UINT8X  Ep0Buffer[8 >(THIS_ENDP0_SIZE+2)? 8:(THIS_ENDP0_SIZE+2)] _at_ 0x0000;    //�˵�0 OUT&IN��������������ż��ַ
UINT8X  Ep1Buffer[64>(MAX_PACKET_SIZE+2)?64:(MAX_PACKET_SIZE+2)] _at_ 0x000a;    //�˵�1 IN������,������ż��ַ
UINT8X  Ep2Buffer[64>(MAX_PACKET_SIZE+2)?64:(MAX_PACKET_SIZE+2)] _at_ 0x0050;    //�˵�2 IN������,������ż��ַ
UINT8   SetupReq,SetupLen,Ready,Count,FLAG,UsbConfig;
PUINT8  pDescr;                                                                //USB���ñ�־
USB_SETUP_REQ   SetupReqBuf;                                                   //�ݴ�Setup��

#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)
#define DEBUG 0

#pragma  NOAREGS
/*�豸������*/
UINT8C DevDesc[18] = 
{
	0x12,
	0x01,	/* �豸������ */
	
	0x10,	/*bcdUSB 1.1*/
	0x01,
	
	0x00,
	0x00,
	0x00,
	THIS_ENDP0_SIZE,
	0x3d,	/* VID idVendor (0x0483)*/
	0x40,
	
	0x07,	/* PID idProduct = 0x5710*/
	0x20,
	
	0x00,	/*bcdDevice rel. 2.00*/
	0x00,
	
	0x00,
	0x00,
	0x00,
	0x01
};

UINT8C CfgDesc[] =		//34
{
//    0x09,0x02,0x3b,0x00,0x02,0x01,0x00,0xA0,0x32,             //����������
//    0x09,0x04,0x00,0x00,0x01,0x03,0x01,0x01,0x00,             //�ӿ�������,����
//    0x09,0x21,0x11,0x01,0x00,0x01,0x22,0x3e,0x00,             //HID��������
//    0x07,0x05,0x81,0x03,0x08,0x00,0x0a,                       //�˵�������
//    0x09,0x04,0x01,0x00,0x01,0x03,0x01,0x02,0x00,             //�ӿ�������,���
//    0x09,0x21,0x10,0x01,0x00,0x01,0x22,0x34,0x00,             //HID��������
//    0x07,0x05,0x82,0x03,0x04,0x00,0x0a                        //�˵�������
//////////////////////////////////////////////////
/***************** ���������� *******************/
	0x09,	/* bLength: ���ȣ��豸�ַ����ĳ���Ϊ9�ֽ� */
	0x02,	/* ���ͣ������������������� */
	0x3b,	//�����������ܳ���
	0x00,
	0x02,	/* �ӿ�����  �ӿںͶ˵��ǲ��õ�*/
	0x01,	/*bConfigurationValue: Configuration value*/
	0x00,	/* �����õ��ַ���������ֵ����ֵΪ0��ʾû���ַ��� */
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
	0x01,	/* �ýӿ����õ����� 1=BOOT, 0=no boot */
	0x01,	/* ������1   �����2  0���Զ����豸 */
	0x00, /* �ýӿ��ַ���������  һ�㶼Ϊ0 */

//////////////////////////////////////////////////
/************** ����HID�������� *****************/
	0x09,	/* bLength: ���ȣ�HID�������ĳ���Ϊ9�ֽ� */
	0x21,	/* HID������������ */
	0x11,	/* HIDЭ��İ汾 */
	0x01,
	0x00,	/* ���Ҵ��� */
	0x01,	/* �¼������������� */
	0x22,	/* �¼������������� */
	0x3e,	/* ��һ���������ĳ��� */
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
	0x01,	/* ��ѯ��� bInterval: Polling Interval (6 ms)*/

//////////////////////////////////////////////////
/************** ���ӿڵ������� ****************/
	0x09,	/* �ӿ��������ĳ���Ϊ9�ֽ� */
	0x04,	/* ����ӿ������� */
	0x01,	/* �ýӿڱ�� */
	0x00,	/* �ýӿڵñ��ñ�� */
	0x01,	/* �ýӿ���ʹ�ö˵����� */
	0x03,	/* �ýӿ���ʹ�õ���   �������HID */
	0x01,	/* �ýӿ����õ����� 1=BOOT, 0=no boot */
	0x02,	/* ������1   �����2  0���Զ����豸 */
	0x00,   /* �ýӿ��ַ���������  һ�㶼Ϊ0 */

/////////////////////////////////////////////
/************** ���HID������ ****************/
	0x09,	/* bLength: ���ȣ�HID�������ĳ���Ϊ9�ֽ� */
	0x21,	/* HID������������ */
	0x10,	/* HIDЭ��İ汾 */
	0x01,
	0x00,	/* ���Ҵ��� */
	0x01,	/* �¼������������� */
	0x22,	/* �¼������������� */
	0x34,	/* ��һ���������ĳ��� */ //0x34  0x36
	0x00,

///////////////////////////////////////////////////////////////
/************** �������˿�������(�˵�������) *****************/
	0x07,	/* �˵��������ĳ���Ϊ7�ֽ� */
	0x05,	/* �˵������������� */
	0x82,	/* �ö˵�(����)�ĵ�ַ,D7:0(OUT),1(IN),D6~D4:����,D3~D0:�˵�� */
	0x03,	/*�˵������ΪΪ�ж϶˵�. D0~D1��ʾ��������:0(���ƴ���),
			 1(��ʱ����),2(��������),3(�жϴ���) �ǵ�ʱ����˵�:D2~D7:����Ϊ0 ��ʱ����˵㣺 
			 D2~D3��ʾͬ��������:0(��ͬ��),1(�첽),2(����),3(ͬ��) D4~D5��ʾ��;:0(���ݶ˵�),
			 1(�����˵�),2(�������������ݶ˵�),3(����)��D6~D7:����*/
	0x04,	/* �ö˵�֧�ֵ��������� */
	0x00,
	0x01    /* ��ѯ��� bInterval: Polling Interval (32 ms)*/	
	
};
/*�ַ���������*/
/*HID�౨��������*/
UINT8C KeyRepDesc[62] =
{
	0x05,0x01,	// USAGE_PAGE (Generic Desktop)	//62
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


/*�������*/
UINT8 HIDMouse[4] = {0x0,0x0,0x0,0x0}; 

/*��������*/
UINT8 HIDKey[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

/*******************************************************************************
* Function Name  : CH552SoftReset()
* Description    : CH552��λ
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
//void CH552SoftReset( )
//{
//    SAFE_MOD = 0x55;
//    SAFE_MOD = 0xAA;
//    GLOBAL_CFG	|=bSW_RESET;
//}

/*******************************************************************************
* Function Name  : CH552USBDevWakeup()
* Description    : CH552�豸ģʽ��������������K�ź�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
//void CH552USBDevWakeup( )
//{
//#ifdef Fullspeed
//	UDEV_CTRL |= bUD_LOW_SPEED;
//	mDelaymS(2);
//	UDEV_CTRL &= ~bUD_LOW_SPEED;		
//#else
//	UDEV_CTRL &= ~bUD_LOW_SPEED;
//	mDelaymS(2);
//	UDEV_CTRL |= bUD_LOW_SPEED;	
//#endif
//}

/*******************************************************************************
* Function Name  : USBDeviceInit()
* Description    : USB�豸ģʽ����,�豸ģʽ�������շ��˵����ã��жϿ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceInit()
{
	IE_USB = 0;	//�ر�USB�ж�
	USB_CTRL = 0x00;          // ���趨USB�豸ģʽ
	UDEV_CTRL = bUD_PD_DIS;   // ��ֹDP/DM��������
#ifndef Fullspeed
    UDEV_CTRL |= bUD_LOW_SPEED;                                                //ѡ�����1.5Mģʽ
    USB_CTRL |= bUC_LOW_SPEED;
#else
    UDEV_CTRL &= ~bUD_LOW_SPEED;                                               //ѡ��ȫ��12Mģʽ��Ĭ�Ϸ�ʽ
    USB_CTRL &= ~bUC_LOW_SPEED;
#endif

	UEP2_DMA = Ep2Buffer;                                                      //�˵�2���ݴ����ַ
	UEP2_3_MOD = UEP2_3_MOD & ~bUEP2_BUF_MOD | bUEP2_TX_EN;                    //�˵�2����ʹ�� 64�ֽڻ�����
	UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;                                 //�˵�2�Զ���תͬ����־λ��IN���񷵻�NAK
	UEP0_DMA = Ep0Buffer;                                                      //�˵�0���ݴ����ַ
	UEP4_1_MOD &= ~(bUEP4_RX_EN | bUEP4_TX_EN);                                //�˵�0��64�ֽ��շ�������
	UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;                                 //OUT���񷵻�ACK��IN���񷵻�NAK
	UEP1_DMA = Ep1Buffer;                                                      //�˵�1���ݴ����ַ
	UEP4_1_MOD = UEP4_1_MOD & ~bUEP1_BUF_MOD | bUEP1_TX_EN;                    //�˵�1����ʹ�� 64�ֽڻ�����
	UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;                                 //�˵�1�Զ���תͬ����־λ��IN���񷵻�NAK	
	
	USB_DEV_AD = 0x00;
	USB_CTRL |= bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;                      // ����USB�豸��DMA�����ж��ڼ��жϱ�־δ���ǰ�Զ�����NAK
	UDEV_CTRL |= bUD_PORT_EN;                                                  // ����USB�˿�
	USB_INT_FG = 0xFF;                                                         // ���жϱ�־
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
    memcpy( Ep1Buffer, HIDKey, sizeof(HIDKey));                              //�����ϴ�����
    UEP1_T_LEN = sizeof(HIDKey);                                             //�ϴ����ݳ���
    UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;                //������ʱ�ϴ����ݲ�Ӧ��ACK
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
    memcpy( Ep2Buffer, HIDMouse, sizeof(HIDMouse));                              //�����ϴ�����
    UEP2_T_LEN = sizeof(HIDMouse);                                              //�ϴ����ݳ���
    UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;                  //������ʱ�ϴ����ݲ�Ӧ��ACK
}




/*******************************************************************************
* Function Name  : DeviceInterrupt()
* Description    : CH552USB�жϴ�����
********************************************************************************/
void DeviceInterrupt( void ) interrupt INT_NO_USB                     					//USB�жϷ������,ʹ�üĴ�����1
{
    UINT8 len = 0;
    if(UIF_TRANSFER)                                                            //USB������ɱ�־
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
        case UIS_TOKEN_IN | 2:                                                  //endpoint 2# �ж϶˵��ϴ�
            UEP2_T_LEN = 0;                                                     //Ԥʹ�÷��ͳ���һ��Ҫ���
//            UEP1_CTRL ^= bUEP_T_TOG;                                          //����������Զ���ת����Ҫ�ֶ���ת
            UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //Ĭ��Ӧ��NAK
			FLAG = 1;
            break;
        case UIS_TOKEN_IN | 1:                                                  //endpoint 1# �ж϶˵��ϴ�
            UEP1_T_LEN = 0;                                                     //Ԥʹ�÷��ͳ���һ��Ҫ���
//            UEP2_CTRL ^= bUEP_T_TOG;                                          //����������Զ���ת����Ҫ�ֶ���ת
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //Ĭ��Ӧ��NAK
            FLAG = 1;                                                           /*������ɱ�־*/
            break;
        case UIS_TOKEN_SETUP | 0:                                                //SETUP����
            len = USB_RX_LEN;
            if(len == (sizeof(USB_SETUP_REQ)))
            {
                SetupLen = UsbSetupBuf->wLengthL;
                if(UsbSetupBuf->wLengthH || SetupLen > 0x7F )
                {
                    SetupLen = 0x7F;    // �����ܳ���
                }
                len = 0;                                                        // Ĭ��Ϊ�ɹ������ϴ�0����
                SetupReq = UsbSetupBuf->bRequest;								
                if ( ( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )/* HID������ */
                {
					switch( SetupReq ) 
					{
						case 0x01://GetReport
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
							len = 0xFF;  								   /*���֧��*/					
							break;
					}	
                }
                else
                {                                                           //��׼����
                    switch(SetupReq)                                        //������
                    {
                    case USB_GET_DESCRIPTOR:
                        switch(UsbSetupBuf->wValueH)
                        {
                        case 1:                                             //�豸������
                            pDescr = DevDesc;                               //���豸�������͵�Ҫ���͵Ļ�����
                            len = sizeof(DevDesc);
                            break;
                        case 2:                                             //����������
                            pDescr = CfgDesc;                               //���豸�������͵�Ҫ���͵Ļ�����
                            len = sizeof(CfgDesc);
                            break;
                        case 0x22:                                          //����������
                            if(UsbSetupBuf->wIndexL == 0)                   //�ӿ�0����������
                            {
                                pDescr = KeyRepDesc;                        //����׼���ϴ�
                                len = sizeof(KeyRepDesc);
                            }
														//��ö�����
                            else if(UsbSetupBuf->wIndexL == 1)              //�ӿ�1����������
                            {
                                pDescr = MouseRepDesc;                      //����׼���ϴ�
                                len = sizeof(MouseRepDesc);                                
                            }
                            else
                            {
                                len = 0xff;                                 //������ֻ��2���ӿڣ���仰����������ִ��
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
												#ifdef DE_PRINTF 							
													//printf("SET CONFIG.\n");
												#endif
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
                            case 0x82:
                                UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
                            case 0x81:
                                UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
                            case 0x01:
                                UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
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
            else if(len)                                                //�ϴ����ݻ���״̬�׶η���0���Ȱ�
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
        USB_DEV_AD = 0x00;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
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








UINT8 MARCO_KEYCODE [100]= { 0x15,0x06,0x10,0x07,0x58,0x15,0x10,0x16,0x17,0x16,  //Win+r,c,m,d,Enter,Win+r,m,s,t,s,
														 0x06,0x58,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	 //c,Enter,,,,,,,,,
														 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
														 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
														 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
														 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
														 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
														 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
														 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
														 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };  //marco keycode

UINT8 MARCO_SPLIT_INDX [11] = {0,5,12,29,39,49,59,69,79,89,99};  //split marco key position

UINT8 MARCO_SPE_KEYINDX [20] = {0x00,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
UINT8 MARCO_SPE_KEYCODE [20] = {0x08,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

UINT8 MARCO_DELAY_INDX  [20] = {0x00,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
UINT8 MARCO_DELAY       [20] = {0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

UINT8 MOUSE_CODE [10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  //temporily used for mouse 
UINT8 KEY_CODE   [10] = {0x00,0x00,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27};  //temporily used for key 
UINT8 SP_KEY_CODE[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  //temporily used for sepcial key

//unpressed : 0x00
//  pressed :key_pressed[n] == 0xff
//if key(n) is a marco key 

UINT8 KEY_PRESS  [10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  
UINT8 KEY_MARCO  [10] = {0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


//delay time*100ms
void MarcoDelay(UINT8 time){
	while(time--)
		mDelaymS(100);
}

//report key code to computer
void HIDsend(){
	FLAG = 0;
	Enp2IntIn();      //send mouse event
	while(FLAG == 0); /*�ȴ���һ���������*/
	Enp1IntIn();      //send keyboard event
	while(FLAG == 0); 
}

//send marco key
void HIDmarco(UINT8 key_num){
	UINT8 i,j,delay_n00ms;
	//init HIDKey[]
	HIDKey[0] = 0x00;  //special key
	HIDKey[1] = 0x00;  //reserved
	HIDKey[2] = 0x00;  //key
	HIDKey[3] = 0x00;  
	HIDKey[4] = 0x00;
	HIDKey[5] = 0x00;
	HIDKey[6] = 0x00;
	HIDKey[7] = 0x00;
	HIDKey[8] = 0x00;
	//HIDKeysend();
	for(i=0;i<(MARCO_SPLIT_INDX[key_num]-MARCO_SPLIT_INDX[key_num-1]);i++){
		//prepare special key
		delay_n00ms = 0;
		for(j=0;j<20;j++){
			if(!(MARCO_SPE_KEYINDX[j]|MARCO_SPE_KEYCODE[j]))
				break;
			if(MARCO_SPE_KEYINDX[j]<MARCO_SPLIT_INDX[key_num-1])
				continue;
			if(MARCO_SPE_KEYINDX[j]>MARCO_SPLIT_INDX[key_num])
				break;
			if(MARCO_SPE_KEYINDX[j] == (MARCO_SPLIT_INDX[key_num-1]+i)){
				HIDKey[0] = MARCO_SPE_KEYCODE[j];
				break;
			}
		}
		//prepare delay
		for(j=0;j<20;j++){
			if(!(MARCO_DELAY_INDX[j]|MARCO_DELAY[j]))
				break;
			if(MARCO_DELAY_INDX[j]<MARCO_SPLIT_INDX[key_num-1])
				continue;
			if(MARCO_DELAY_INDX[j]>MARCO_SPLIT_INDX[key_num])
				break;
			if(MARCO_DELAY_INDX[j] == (MARCO_SPLIT_INDX[key_num-1]+i)){
				delay_n00ms = MARCO_DELAY[j];
				break;
			}
		}
		//prepare normal key
		HIDKey [2] = MARCO_KEYCODE[MARCO_SPLIT_INDX[key_num-1]+i];
		HIDsend();
		//delay
		MarcoDelay(delay_n00ms);
		//bounce up
		HIDKey[0] = 0x00;
		HIDKey[2] = 0x00;
		//HIDsend();
	}
	//clear buffer
	HIDKey[0] = 0x00;
	HIDKey[2] = 0x00;
	HIDsend();
	//wait marco-key bounce up
	mDelaymS(200);
}


//10-key maping 
sbit Key1  = P3^2;
sbit Key2  = P1^4;
sbit Key3  = P1^5;
sbit Key4  = P1^6;
sbit Key5  = P1^7;
sbit Key6  = P3^1;
sbit Key7  = P3^0;
sbit Key8  = P1^1;
sbit Key9  = P3^3;
sbit Key10 = P3^4;


void scanKey(){
	UINT8 i = 0;
	UINT8 mouse_code = 0x00;
	UINT8 sp_key_code = 0x00;
	UINT8 key_count = 0;
	if(!Key1){
		KEY_PRESS[0]=0xff;
	}
	else{
		KEY_PRESS[0]=0x00;
	}
	if(!Key2){
		KEY_PRESS[1]=0xff;
	}
	else{
		KEY_PRESS[1]=0x00;
	}
	if(!Key3){
		KEY_PRESS[2]=0xff;
	}
	else{
		KEY_PRESS[2]=0x00;
	}
	if(!Key4){
		KEY_PRESS[3]=0xff;
	}
	else{
		KEY_PRESS[3]=0x00;
	}
	if(!Key5){
		KEY_PRESS[4]=0xff;
	}
	else{
		KEY_PRESS[4]=0x00;
	}
	if(!Key6){
		KEY_PRESS[5]=0xff;
	}
	else{
		KEY_PRESS[5]=0x00;
	}
	if(!Key7){
		KEY_PRESS[6]=0xff;
	}
	else{
		KEY_PRESS[6]=0x00;
	}
	if(!Key8){
		KEY_PRESS[7]=0xff;
	}
	else{
		KEY_PRESS[7]=0x00;
	}
	if(!Key9){
		KEY_PRESS[8]=0xff;
	}
	else{
		KEY_PRESS[8]=0x00;
	}
	if(!Key10){
		KEY_PRESS[9]=0xff;
	}
	else{
		KEY_PRESS[9]=0x00;
	}
	mDelaymS(6); //avoid jitter
	if(!Key1){
		if(KEY_PRESS[0]!=0xff){
			KEY_PRESS[0] =0x00;
		}
		else{
			if(KEY_MARCO[0]==0xff)
				HIDmarco(1);
		}
	}
	if(!Key2){
		if(KEY_PRESS[1]!=0xff){
			KEY_PRESS[1] =0x00;
		}
		else{
			if(KEY_MARCO[1]==0xff)
				HIDmarco(2);
		}
	}
	if(!Key3){
		if(KEY_PRESS[2]!=0xff){
			KEY_PRESS[2] =0x00;
		}
	}
	if(!Key4){
		if(KEY_PRESS[3]!=0xff){
			KEY_PRESS[3] =0x00;
		}
	}
	if(!Key5){
		if(KEY_PRESS[4]!=0xff){
			KEY_PRESS[4] =0x00;
		}
	}
	if(!Key6){
		if(KEY_PRESS[5]!=0xff){
			KEY_PRESS[5] =0x00;
		}
	}
	if(!Key7){
		if(KEY_PRESS[6]!=0xff){
			KEY_PRESS[6] =0x00;
		}
	}
	if(!Key8){
		if(KEY_PRESS[7]!=0xff){
			KEY_PRESS[7] =0x00;
		}
	}
	if(!Key9){
		if(KEY_PRESS[8]!=0xff){
			KEY_PRESS[8] =0x00;
		}
	}
	if(!Key10){
		if(KEY_PRESS[9]!=0xff){
			KEY_PRESS[9] =0x00;
		}
	}
	
	
	//get final result
	for(;i<10;i++){
		mouse_code  |= MOUSE_CODE[i] &KEY_PRESS[i]; //generate mouse_code
		sp_key_code |= SP_KEY_CODE[i]&KEY_PRESS[i]; //generate special_key_code
		//generate normal key_code
		if(key_count<6 && (KEY_PRESS[i]==0xff)){
			HIDKey[2+key_count]=KEY_CODE[i];
			key_count ++;
		}
	}
	
	//FINAL ASSIGNING
	//mouse codes
	HIDMouse[0] = mouse_code;
	//key codes
	HIDKey  [0] = sp_key_code; //special key Byte
	HIDKey  [1] = 0x00;        //reserved
	if(key_count<6){					 //fill blank
		for(i=7;i>=key_count+2;i--){
			HIDKey[i]=0x00;
		}
	}
	
}



void main()
{
    CfgFsys( );                    //CH552ʱ��ѡ������
    mDelaymS(50);                 //�޸���Ƶ�ȴ��ڲ������ȶ�,�ؼ�

	/* ����P1��Ϊ׼˫��IO�� */
	P1_MOD_OC = 0xff;
	P1_DIR_PU = 0xff;
	//set P3 dual-dir IO port
	P3_MOD_OC = 0xFF;
	P3_DIR_PU = 0xFF;

	USBDeviceInit();              //USB�豸ģʽ��ʼ��
    EA = 1;                     //����Ƭ���ж�
    
	UEP1_T_LEN = 0;               //Ԥʹ�÷��ͳ���һ��Ҫ���
    FLAG = 0;
    Ready = 0;
	
	//�ȴ�USBö�ٳɹ�
	while(Ready == 0);
	while(1)
	{
		if(Ready)   //waiting for usb enum finish
		{
			scanKey();
			HIDsend();
			FLAG = 0;
		}
	}
}

