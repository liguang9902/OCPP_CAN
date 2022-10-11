#ifndef ENMETER_H
#define ENMETER_H


		
#include <stdint.h>		
#include "SoftwareSerial.h"
typedef enum {
	phaseA,
	phaseB,
	phaseC
}PHASE;

//С��λ��һλ
typedef  struct OBJ_Angle{
	uint8_t		dataType;
	uint8_t		value[2];
}OBJ_VoltageAngle , OBJ_CurrentAngle;

//С��λ����λ ��λ V/A/HZ 
typedef  struct OBJ_VCF{
	uint8_t		dataType;
	uint8_t		value[2];
}OBJ_VOLTAGE , OBJ_CURRENT , OBJ_FREQUENCE;

//С��λ����λ
typedef  struct {
	uint8_t		dataType;
	uint8_t		value[2];
}OBJ_FACTOR ;



//С��λ��һλ ��λw
typedef  struct {
	uint8_t		dataType;
	uint8_t		value[4];
}OBJ_POWER;

//С��λ����λ ��λkwh
typedef  struct {
	uint8_t		dataType;
	uint8_t		value[4];
}OBJ_ENERGY;

typedef  struct {
	uint8_t 	head[11]   ;								//7E A0 88 23 03 52 A5 5A E6 E7 00  
	uint16_t	operationID;								//C4 01  ��DLMS��֤��һ�׶Σ���Լ�����еĻ�Ӧ����������ID��ΪC401
	uint8_t		ivkproi    ;								//81 invoke-id(000 0001) and priority(1)  ���������ݲ�����������涨����ʱ���̶�Ϊ81��������Ϊ8C
	uint8_t 	rsvd1      ;								//00 by data
	uint8_t 	objType    ;								//02 ��������  02��ʾ�ṹ��
	uint8_t 	objCount   ;								//1E 30�� ��������
	
	OBJ_VOLTAGE	aVoltage ;						//A���ѹ 12 52 53 ��0x12 �������� �޷���int�������ֽڣ�0x52 0x53ת��ʮ������ 21075 A���ѹ ��С��λ����λ 210.75V�� 
	OBJ_VOLTAGE	bVoltage ;						//B���ѹ 12 52 76 ��0x52 0x76ת��ʮ������ 21110 B���ѹ ��С��λ����λ 211.10V��
	OBJ_VOLTAGE	cVoltage ;						//C���ѹ 12 52 64 ��0x52 0x64ת��ʮ������ 21092 C���ѹ ��С��λ����λ 210.92V��

	OBJ_CURRENT	aCurrent ;						//A����� 12 03 7A ��0x03 0x7Aת��ʮ������ 890 A����� ��С��λ����λ 8.90A��
	OBJ_CURRENT	bCurrent ;						//B����� 12 03 82 ��0x03 0x82ת��ʮ������ 898 B����� ��С��λ����λ 8.98A��
	OBJ_CURRENT	cCurrent ;						//C����� 12 03 86 ��0x03 0x86ת��ʮ������ 902 C����� ��С��λ����λ 9.02A��

	OBJ_POWER		totalActivePower 	;				//���й����� 	06 00 00 6E 8F ��0x06 �������� �޷���long���ĸ��ֽڣ�00006E8Fת��ʮ������ 28303 ���й����ʣ�2830.3W  2.8303KW��
	OBJ_POWER		aActivePower     	; 			//A���й����� 	06 00 00 24 BE ��000024BEת��ʮ������ 9406�� A���й�����940.6W 0.9406KW��
	OBJ_POWER		bActivePower     	;				//B���й�����  06 00 00 24 DB ��000024DBת��ʮ������ 9435�� B���й�����943.5W 0.9435KW��
	OBJ_POWER		cActivePower     	; 			//C���й�����  06 00 00 24 F5 ��000024F5ת��ʮ������ 9461�� C���й�����946.1W 0.9461KW��

	OBJ_POWER		totalReactivePower	;			//���޹�����		06 00 00 C0 77 ��0x06 �������� �޷���long���ĸ��ֽڣ�0000C077ת��ʮ������ 49271 ���޹����ʣ�4927.1var  4.9271Kvar��
	OBJ_POWER		aReactivePower			;			//A���޹�����	06 00 00 3F 8A ��00003F8Aת��ʮ������ 16266; A���޹�����1626.6var 1.6266Kvar��
	OBJ_POWER		bReactivePower			;			//B���޹�����	06 00 00 40 5A ��0000405Aת��ʮ������16474; B���޹�����1647.4var 1.6474Kvar��
	OBJ_POWER		cReactivePower			;			//C���޹�����  06 00 00 40 92 ��00004092ת��ʮ������ 16530; C���޹�����1653.0var 1.6530Kvar��

	OBJ_POWER 	totalApparentPower	;			//�����ڹ��� 	06 00 00 DD F6 ��0xDD 0XF6ת��ʮ������ 56822; �����ڹ���5682.2VA 5.6822KVA��
	OBJ_POWER 	aApparentPower			;			//A�����ڹ���	06 00 00 49 66 ��0x49 0x66ת��ʮ������ 18790; A�����ڹ���1879.0VA 1.8790KVA��
	OBJ_POWER 	bApparentPower			;			//B�����ڹ���	06 00 00 4A 29 ��0x4A 0x29ת��ʮ������ 18729; B�����ڹ���1872.9VA 1.8729KVA��
	OBJ_POWER 	cApparentPower			;			//C�����ڹ���	06 00 00 4A 66 ��0x4A 0x66ת��ʮ������ 19046; C�����ڹ���1904.6VA 1.9046KVA��
	
	OBJ_FREQUENCE	gridFrequence			;			//����Ƶ��			12 13 88 ��0x13 0x88ת��ʮ������ 5000; ����Ƶ�� 50.00Hz��С��λ����λ��
	OBJ_FACTOR		totalFactor				;			//�ܹ�������		12 01 F2 ��0x01 0XF2ת��ʮ������ 498���ܹ�������0.498��С��λ����λ��
	OBJ_FACTOR		aFactor						;			//A�๦������	12 01 F4 ��0x01 0XF4ת��ʮ������ 500��A�๦������0.5��С��λ����λ��
	OBJ_FACTOR		bFactor						;			//B�๦������	12 01 F0 ��0x01 0XF0ת��ʮ������ 496��B�๦������0.496��С��λ����λ��
	OBJ_FACTOR		cFactor						;			//C�๦������	12 01 F0 ��0x01 0XF0ת��ʮ������ 496��C�๦������0.496��С��λ����λ��

	OBJ_VoltageAngle	abAngle				;			//B����A���ѹ�н�	12 04 AD ��0x04 0xADת��ʮ������ 1197��B����A���ѹ�н�119.7��С��λ��һλ��
	OBJ_VoltageAngle	acAngle				;			//C����A���ѹ�н�	12 09 61 ��0x09 0x61ת��ʮ������ 2401��C����A���ѹ�н�240.1��С��λ��һλ��

	OBJ_CurrentAngle	aAngle				;			//A���ѹ�����֮��ļн�		12 02 57 ��0x02 0x57ת��ʮ������ 599��A���ѹ�����֮��ļн�59.9��С��λ��һλ��
	OBJ_CurrentAngle	bAngle				;			//B���ѹ�����֮��ļн�		12 02 5A ��0x02 0x5Aת��ʮ������ 602��B���ѹ�����֮��ļн�60.2��С��λ��һλ��
	OBJ_CurrentAngle	cAngle				;			//C���ѹ�����֮��ļн�		12 02 5A ��0x02 0x5Aת��ʮ������ 602��C���ѹ�����֮��ļн�60.2��С��λ��һλ��

	OBJ_ENERGY			activeEnergy 		;	//�й��ܵ���		06 00 00 00 AC ��00 00 00 ACת��ʮ������ 172�� �й��ܵ��� 1.72kwh��С��λ����λ��
	OBJ_ENERGY			reactiveEnergy 	;			//�й������ܵ���		06 00 00 00 00��00 00 00 00ת��ʮ������ 0�� �й������ܵ��� 0kwh��С��λ����λ��

	uint8_t			fcs[2];										//6A 16 7E HDLC FCSУ�� 
	uint8_t			tail;											//���� 7E
}HDLC_FRAME;
	


typedef uint8_t (* pAction)(void);
typedef int8_t	(* pGetVoltage)(PHASE p , uint16_t *pV);
typedef int8_t	(* pGetCurrent)(PHASE p , uint16_t *pC);
typedef int8_t	(* pGetEnergy)(uint32_t *pE);
typedef struct {
		HDLC_FRAME 		*pFrameHDLC;
		pAction 			conn;
		pAction 			handshake;
		pAction				read;
		pAction 			disconn;
		pGetVoltage 	getVoltage;
		pGetCurrent		getCurrent;
		pGetEnergy		getEnergy;
}MTMeter;


class RS485IF
{	private:
	SoftwareSerial SFserial;
	HDLC_FRAME 	*pFrameHDLC = NULL;

	uint16_t getValue16(uint8_t *pStruct);
	uint32_t getValue32(uint8_t *pStruct);

	uint8_t meterConn();
	uint8_t meterHandshake();
	uint8_t meterRead();
	uint8_t meterDisconn();

	int8_t getVoltage(PHASE phase , uint16_t *pVoltage);
	int8_t getCurrent(PHASE phase , uint16_t *pCurrent);
	int8_t getTotalActiveEnergy(uint32_t *pE);

	void meterUART_SendData(uint8_t *txbuffer, size_t length);
	size_t meterUart_receiveData(uint8_t *txbuffer, size_t length);
	public:
	RS485IF(int8_t rxPin, int8_t txPin);

	void loop();
};





#endif /* ENMETER_H */		
