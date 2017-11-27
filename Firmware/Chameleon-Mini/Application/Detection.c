#include "Detection.h"

#include "ISO14443-3A.h"
#include "../Codec/ISO14443-2A.h"
#include "../Memory.h"
#include "../LED.h"
#include "../Random.h"
#include "../Settings.h"
#include <string.h>

//#define MFCLASSIC_1K_ATQA_VALUE     0x0004
//#define MFCLASSIC_1K_SAK_CL1_VALUE  0x08
//
//#define MEM_UID_CL1_ADDRESS         0x00
//#define MEM_UID_CL1_SIZE            4
//
//#define CMD_HALT                    0x50
//#define CMD_HALT_FRAME_SIZE         2        /* Bytes without CRCA */
//#define CMD_AUTH_A                  0x60
//#define CMD_AUTH_B                  0x61
//#define CMD_AUTH_FRAME_SIZE         2         /* Bytes without CRCA */
//#define CMD_AUTH_RB_FRAME_SIZE      4        /* Bytes */
//#define CMD_AUTH_BA_FRAME_SIZE      4        /* Bytes */
//#define CMD_READ                    0x30
//#define CMD_WRITE                   0xA0
//#define CMD_DECREMENT               0xC0
//#define CMD_INCREMENT               0xC1
//#define CMD_RESTORE                 0xC2
//#define CMD_TRANSFER                0xB0
//
//#define NAK_INVALID_ARG             0x00
//#define NAK_CRC_ERROR               0x01
//#define NAK_NOT_AUTHED              0x04
//
//#define ACK_NAK_FRAME_SIZE          4         /* Bits */
//
//#define MEM_SECTOR_ADDR_MASK        0x3C
//#define MEM_BYTES_PER_BLOCK         16        /* Bytes */
//
//
//
//#define MEM_KEY_A_OFFSET            48        /* Bytes */
//#define MEM_KEY_B_OFFSET            58        /* Bytes */
//#define MEM_KEY_SIZE                6        /* Bytes */

 #define MFCLASSIC_1K_ATQA_VALUE     0x0004
#define MFCLASSIC_4K_ATQA_VALUE     0x0002
 #define MFCLASSIC_1K_SAK_CL1_VALUE  0x08
#define MFCLASSIC_4K_SAK_CL1_VALUE  0x18
 
 #define MEM_UID_CL1_ADDRESS         0x00
 #define MEM_UID_CL1_SIZE            4
#define MEM_UID_BCC1_ADDRESS        0x04
#define MEM_KEY_A_OFFSET            48        /* Bytes */
#define MEM_KEY_B_OFFSET            58        /* Bytes */
#define MEM_KEY_SIZE                6        /* Bytes */
#define MEM_SECTOR_ADDR_MASK        0x3C
#define MEM_BYTES_PER_BLOCK         16        /* Bytes */
#define MEM_VALUE_SIZE              4       /* Bytes */

#define ACK_NAK_FRAME_SIZE          4         /* Bits */
#define ACK_VALUE                   0x0A
#define NAK_INVALID_ARG             0x00
#define NAK_CRC_ERROR               0x01
#define NAK_NOT_AUTHED              0x04
#define NAK_EEPROM_ERROR            0x05
#define NAK_OTHER_ERROR             0x06

 #define CMD_AUTH_A                  0x60
 #define CMD_AUTH_B                  0x61
 #define CMD_AUTH_FRAME_SIZE         2         /* Bytes without CRCA */
 #define CMD_AUTH_RB_FRAME_SIZE      4        /* Bytes */
#define CMD_AUTH_AB_FRAME_SIZE      8        /* Bytes */
 #define CMD_AUTH_BA_FRAME_SIZE      4        /* Bytes */


 static enum {
	 STATE_HALT,
	 STATE_IDLE,
	 STATE_READY,
	 STATE_ACTIVE,
	 STATE_AUTHING,
	 STATE_AUTHED_IDLE,
	 STATE_WRITE,
	 STATE_INCREMENT,
	 STATE_DECREMENT,
	 STATE_RESTORE
 } State;

 static uint16_t CardATQAValue;
 static uint8_t CardSAKValue;
 uint8_t data_svae[16];
	static uint8_t turn_falga=0;
	static uint8_t turn_falgb=0;
	static uint8_t keyb_falg=0;
uint16_t MifareDetectionAppProcess(uint8_t* Buffer, uint16_t BitCount)
{
	//����0x26 ׼���ڴ˴����������
	if(BitCount==7)
	{
		if (ISO14443AWakeUp(Buffer, &BitCount, CardATQAValue))
		{
			State=STATE_READY;
			return BitCount;
		}
	}
	//����0x93 0x20 �� 0x93 0x70
	if(BitCount==16 || BitCount==72)
	{
		if (Buffer[0] == ISO14443A_CMD_SELECT_CL1)
		{
			uint8_t UidCL1[4];
			MemoryReadBlock(UidCL1, MEM_UID_CL1_ADDRESS, MEM_UID_CL1_SIZE);
			if (ISO14443ASelect(Buffer, &BitCount, UidCL1, CardSAKValue))
			return BitCount;
		}
	}

	if(State != STATE_AUTHED_IDLE)
	{
		if(BitCount==32)
		{
			if((Buffer[0] == CMD_AUTH_A) || (Buffer[0] == CMD_AUTH_B))
			{
				if (ISO14443ACheckCRCA(Buffer, CMD_AUTH_FRAME_SIZE)) {

					uint8_t CardNonce[4]={0x01,0x20,0x01,0x45};

					/* Generate a random nonce and read UID and key from memory */
					RandomGetBuffer(CardNonce, sizeof(CardNonce));
					memcpy(data_svae,Buffer,4);
					memcpy(data_svae+4,CardNonce,4);

					if(Buffer[0] == CMD_AUTH_B) keyb_falg=1;
					else keyb_falg=0;

					State = STATE_AUTHING;

					for (uint8_t i=0; i<sizeof(CardNonce); i++)
					Buffer[i] = CardNonce[i];

					return CMD_AUTH_RB_FRAME_SIZE * BITS_PER_BYTE;
				}
			}
		}
		////����8λ����
		if(BitCount==64 && State==STATE_AUTHING)
		{
			//LEDPulse(LED_RED);
				switch(GlobalSettings.ActiveSetting)
				{
					case 0:
					LEDPulse(LED_ONE);
					break;
					case 1:
					LEDPulse(LED_TWO);
					break;
					case 2:
					LEDPulse(LED_THREE);
					break;
					case 3:
					LEDPulse(LED_FOUR);
					break;
					case 4:
					LEDPulse(LED_FIVE);
					break;
					case 5:
					LEDPulse(LED_SIX);
					break;
					case 6:
					LEDPulse(LED_SEVEN);
					break;
					case 7:
					LEDPulse(LED_EIGHT);
					break;
					default:
					break;
				}
		//������Ϣ
		memcpy(data_svae+8,Buffer,8);

		if(!keyb_falg)
		{
		MemoryWriteBlock(data_svae, (turn_falga+1) * MEM_BYTES_PER_BLOCK+4096, MEM_BYTES_PER_BLOCK);
		turn_falga= (++turn_falga)%6;
		}	
		else
		{
		MemoryWriteBlock(data_svae, (turn_falgb) * MEM_BYTES_PER_BLOCK + 112+4096, MEM_BYTES_PER_BLOCK);
		turn_falgb= (++turn_falgb)%6;
		}
				
		}
	}


	/* No response has been sent, when we reach here */
    return ISO14443A_APP_NO_RESPONSE;

 }

 void MifareDetectionInit(void)
 {
     //������Ը��£���Ϊ�Ǻ���ָ�롣
	 State = STATE_IDLE;
	 CardATQAValue = MFCLASSIC_1K_ATQA_VALUE;
	 CardSAKValue = MFCLASSIC_1K_SAK_CL1_VALUE;
 }

 void MifareDetectionReset(void)
 {
	//State = STATE_IDLE;
 }