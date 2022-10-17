
//#include "board_api.h"
#include <esp_log.h>
#include "stdlib.h"
#include "enmMeter.h"
#include "emChargePoint.hpp"

#define METER_RESPONSE_SIZE_MAX	138
#define METER_RESPONSD_TIMEOUT	5000
#define METER_FRAME_BEGIN				0x7E
#define METER_FRAME_END					0x7E

uint8_t meterReceiveBuffer[METER_RESPONSE_SIZE_MAX] ={0,};

//connect
uint8_t snrmRequest[9] = { 0x7E, 0xA0, 0x07, 0x03, 0x23, 0x93, 0xBF, 0x32, 0x7E };
#define SNRM_RESPONSE_SIZE	32
uint8_t SNRM_respone_model[32]={0x7E,0xA0,0x1E,0x23,0x03,0x73,0x7B,0xCF,0x81,0x80,0x12,0x05,0x01,
0x80,0x06,0x01,0x80,0x07,0x04,0x00,0x00,0x00,0x01,0x08,0x04,0x00,0x00,0x00,0x01,0x53,0x3B,0x7E};

uint8_t handshakeRequest[74] = { 0x7E, 0xA0, 0x48, 0x03, 0x23, 0x10, 0x62, 0x20, 0xE6,
		0xE6, 0x00, 0x60, 0x3A, 0x80, 0x02, 0x02, 0x84, 0xA1, 0x09, 0x06, 0x07,
		0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01, 0x8A, 0x02, 0x07, 0x80, 0x8B,
		0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x01, 0xAC, 0x0A, 0x80, 0x08,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0xBE, 0x10, 0x04, 0x0E,
		0x01, 0x00, 0x00, 0x00, 0x06, 0x5F, 0x1F, 0x04, 0x00, 0xFF, 0xFF, 0xFF,
		0x28, 0x00, 0x68, 0xF4, 0x7E };
#define HANDSHAKE_RESPONSE_SIZE	57
		
uint8_t dataRequest[27] = { 0x7E, 0xA0, 0x19, 0x03, 0x23, 0x32, 0xDF, 0xEB, 0xE6,
		0xE6, 0x00, 0xC0, 0x01, 0x81, 0x00, 0x01, 0x01, 0x00, 0x01, 0x07, 0x00,
		0xFE, 0x02, 0x00, 0xED, 0x0E, 0x7E };

#define DATAREAD_RESPONSE_SIZE	138
		
//disconnect		
uint8_t disconnectRequest[9] = { 0x7E, 0xA0, 0x07, 0x03, 0x23, 0x53, 0xB3, 0xF4,0x7E };
#define DISCONNECT_RESPONSE_SIZE	32

#define TAG_METER	"METER "

//MTMeter mtMeter ;

/*
MTMeter mtMeter = {
	NULL,
	meterConn,
	meterHandshake,
	meterRead,
	meterDisconn,
	getVoltage,
	getCurrent,
	getTotalActiveEnergy
};
*/
RS485IF::RS485IF(int8_t rxPin, int8_t txPin){
	this->SFserial.begin(9600,SWSERIAL_8N1,rxPin,txPin,false,138);
	this->SFserial.setTimeout(METER_RESPONSD_TIMEOUT);
}

 uint8_t RS485IF::meterConn(){
	//meterUart__RxBufferReset();
	//uint8_t meterReceiveBuffer[METER_RESPONSE_SIZE_MAX] ={0,};
	meterUART_SendData(snrmRequest , sizeof(snrmRequest));
	uint8_t rxLen = meterUart_receiveData(meterReceiveBuffer , SNRM_RESPONSE_SIZE);
	if(!rxLen){
		ESP_LOGW(TAG_METER,"Meter connect timeout!(%d)\r\n" , rxLen);
	} else {
		ESP_LOGD(TAG_METER,"Connect RX=%d-->\r\n" , rxLen);
		ESP_LOG_BUFFER_HEX(TAG_METER,meterReceiveBuffer , rxLen);
	}	
	return rxLen;	
}


 uint8_t RS485IF::meterHandshake(){
	//meterUart__RxBufferReset();
	//uint8_t meterReceiveBuffer[METER_RESPONSE_SIZE_MAX] ={0,};
	meterUART_SendData(handshakeRequest , sizeof(handshakeRequest));
	uint8_t rxLen = meterUart_receiveData(meterReceiveBuffer , HANDSHAKE_RESPONSE_SIZE );
	if(!rxLen){
		ESP_LOGW(TAG_METER,"Meter handshake timeout!(%d)\r\n" , rxLen);
	} else {
		ESP_LOGD(TAG_METER,"Handshake RX=%d-->\r\n" , rxLen);
		ESP_LOG_BUFFER_HEX(TAG_METER,meterReceiveBuffer , rxLen);
	}	
	return rxLen;
}

 uint8_t RS485IF::meterRead(){
	//meterUart__RxBufferReset();
	//uint8_t meterReceiveBuffer[METER_RESPONSE_SIZE_MAX] ={0,};
	meterUART_SendData(dataRequest , sizeof(dataRequest));
	uint8_t rxLen = meterUart_receiveData(meterReceiveBuffer , DATAREAD_RESPONSE_SIZE );
	if(!rxLen){
		ESP_LOGW(TAG_METER,"Meter read timeout!(%d)\r\n" , rxLen);
		pFrameHDLC = NULL;
	} else {
		ESP_LOGD(TAG_METER,"Data RX=%d-->\r\n" , rxLen);
		ESP_LOG_BUFFER_HEX(TAG_METER,meterReceiveBuffer , rxLen);
		//if frame check OK
		pFrameHDLC = (HDLC_FRAME *)meterReceiveBuffer;
		uint16_t va,vb,vc;
		getVoltage(phaseA ,&va);
		getVoltage(phaseB ,&vb);
		getVoltage(phaseC ,&vc);
		ESP_LOGD(TAG_METER,"Votage---->[A:%u(0x%04x)] [B:%u(0x%04x)] [C:%u(0x%04x)]\r\n" , va ,va, vb,vb, vc,vc);
		uint16_t ca,cb,cc;
		getCurrent(phaseA,&ca);
		getCurrent(phaseB,&cb);
		getCurrent(phaseC,&cc);
		ESP_LOGD(TAG_METER,"VCurrent---->[A:%u(0x%04x)] [B:%u(0x%04x)] [C:%u(0x%04x)]\r\n" , ca,ca, cb,cb, cc,cc);
		uint32_t tae;
		getTotalActiveEnergy(&tae);
		ESP_LOGD(TAG_METER,"Total ActiveEnergy---->%lu(0x%08x)\r\n" , tae,tae);
		
	}	
	return rxLen;
}

 uint8_t RS485IF::meterDisconn(){
	//meterUart__RxBufferReset();
	//uint8_t meterReceiveBuffer[METER_RESPONSE_SIZE_MAX] ={0,};
	meterUART_SendData(disconnectRequest , sizeof(disconnectRequest));
	uint8_t rxLen = meterUart_receiveData(meterReceiveBuffer , DISCONNECT_RESPONSE_SIZE );
	if(!rxLen){
		ESP_LOGW(TAG_METER,"Meter connect timeout!(%d)\r\n" , rxLen);
	} else {
		ESP_LOGD(TAG_METER,"Disconnect RX=%d-->\r\n" , rxLen);
		ESP_LOG_BUFFER_HEX(TAG_METER,meterReceiveBuffer , rxLen);
	}	
	return rxLen;	

}

//--------------------------------------------------------------------------------------------------------------------------------------
uint16_t RS485IF::getValue16(uint8_t *pStruct){
	return (uint16_t)((pStruct[1]<<8) | pStruct[2]) ;
}

uint32_t RS485IF::getValue32(uint8_t *pStruct){
	return (uint32_t)( (pStruct[1]<<24) | (pStruct[2]<<16) | (pStruct[3]<<8) | pStruct[4]) ;	
}

/*
	@brief HDLC frame check
@return:
 0: correct
-1: error head
-2: error tail
-3:	error size
-4:	
*/
int32_t frameCheck(void){
	return 0;
}

int8_t RS485IF::getVoltage(PHASE phase , uint16_t *pVoltage){
	if(pFrameHDLC == NULL)
		return -1;
	//other check
	switch(phase){
		case phaseA:
			*pVoltage = getValue16( (uint8_t *)(&(pFrameHDLC->aVoltage)) );	
			break;
		case phaseB:
			*pVoltage = getValue16( (uint8_t *)(&(pFrameHDLC->bVoltage)) );
			break;
		case phaseC:
			*pVoltage = getValue16( (uint8_t *)(&(pFrameHDLC->cVoltage)) );
			break;
	}
	return 0;
}


int8_t RS485IF::getCurrent(PHASE phase , uint16_t *pCurrent){
	if(pFrameHDLC == NULL)
		return -1;
	//other check
	switch(phase){
		case phaseA:
			*pCurrent = getValue16( (uint8_t *)(&(pFrameHDLC->aCurrent)) );	
			break;
		case phaseB:
			*pCurrent = getValue16( (uint8_t *)(&(pFrameHDLC->bCurrent)) );
			break;
		case phaseC:
			*pCurrent = getValue16( (uint8_t *)(&(pFrameHDLC->cCurrent)) );
			break;
	}
	return 0;
}

int8_t RS485IF::getTotalActiveEnergy(uint32_t *pE){
	if(pFrameHDLC == NULL)
		return -1;
	*pE = getValue32( (uint8_t *)(&(pFrameHDLC->activeEnergy)) );
}

//int DE_RE=32;
void RS485IF::meterUART_SendData(uint8_t *txbuffer, size_t length){
	digitalWrite(32,HIGH); 
    SFserial.write(txbuffer,length);
    digitalWrite(32,LOW);
	delay(800);
}

size_t RS485IF::meterUart_receiveData(uint8_t *txbuffer, size_t length){
	uint8_t rxbyte = 0;
	
	if(SFserial.available()) 
      {
        rxbyte =  SFserial.readBytes(txbuffer,length);
		return rxbyte;
      }  
	
}

void RS485IF::loop(){
	uint8_t rxlen= 0;

	rxlen = meterConn();
	
	if(rxlen == SNRM_RESPONSE_SIZE)
	{	//delay(500);
		rxlen=meterHandshake();
		if (rxlen == HANDSHAKE_RESPONSE_SIZE)
		{
			//delay(500);
			rxlen = meterRead();//等待时间很久
							
		}	
	}	
	meterDisconn();
}