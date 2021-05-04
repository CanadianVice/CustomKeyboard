#ifndef __usb__
#define __usb__


/*�û��Զ�������*/
extern UINT8X UserEp2Buf[64];
extern UINT8X Ep2Buffer[];
/*��������*/
extern UINT8 HIDKey[8];
/*�������*/
extern UINT8 HIDMouse[4];


extern UINT8 SetupReq,SetupLen,Ready,Count,FLAG,UsbConfig;
extern UINT8 Endp2Rev;
extern UINT8 Endp2Busy;


void USBDeviceInit(void);
void Enp1IntIn( );
void Enp2IntIn( );
void Enp3IntIn( );

#endif
