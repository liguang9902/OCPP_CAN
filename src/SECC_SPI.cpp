#include "SECC_SPI.hpp"
//#include "esp32-hal-spi.h"
//#include "driver\spi_master.h"
//#include "string.h"
#include "Arduino.h"
//#include "SPI.h"

#include "ProtocolEVSE.hpp"


SECC_SPIClass::SECC_SPIClass(uint8_t spi_bus)
    :_spi_num(spi_bus)
    ,_spi(NULL)
    ,_use_hw_ss(false)
    ,_sck(-1)
    ,_miso(-1)
    ,_mosi(-1)
    ,_ss(-1)
    ,_div(0)
    ,_freq(1000000)
    ,_inTransaction(false)
{}
int SECC_SPIClass::available(void)
{
    return 1 ;
}

int SECC_SPIClass::peek(void)
{
    return -1 ;
}

void  SECC_SPIClass::flush(void)
{
    
}

#define PIN_NUM_CS      15
/*esp_err_t SECC_SPIClass::spi_write(spi_device_handle_t spi, uint8_t *data, uint8_t len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return 0;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction

    gpio_set_level(PIN_NUM_CS, 0);

    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.

    gpio_set_level(PIN_NUM_CS, 1);
    return ret;
}*/


void SECC_SPIClass::SPIsend( uint8_t *data,size_t length){
    static const int spiClk = 20000000;
    uint8_t RBuff;
    this->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_NUM_CS, LOW); //pull SS slow to prep other end for transfer
  
    /*char c;
    for (const char *da=data.c_str();c=*da;da++){
        spi->transfer(c);
    }*/
    /*for (size_t i = 0; i < length; i++)
    {
        RBuff =  this->transfer(*data);
        *data++;
    }*/
     //this->transfer(*data);
     
    this->transfer(data,length);
    digitalWrite(PIN_NUM_CS, HIGH); //pull ss high to signify end of data transfer
    this->endTransaction();
    //return RBuff;
}

size_t  SECC_SPIClass::SPIrev(uint8_t *data,size_t length){
    static const int spiClk = 20000000;
    uint8_t ifRxBuffer[256] = {
      0,
    };
    this->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_NUM_CS, LOW); //pull SS slow to prep other end for transfer
  
    for (size_t i = 0; i < length; i++)
    {
        ifRxBuffer[i] =  this->transfer(*data);
        *data++;
    }
     //this->transfer(*data);
    this->transfer(data,length);
    digitalWrite(PIN_NUM_CS, HIGH); //pull ss high to signify end of data transfer
    this->endTransaction();
    return sizeof(ifRxBuffer)/sizeof(ifRxBuffer[0]);
}
TransferData SECC_SPIClass::SPItransfer( uint8_t *data,size_t length){
    TransferData  transferdata;
    static const int spiClk = 20000000;
    //uint8_t RBuff[]={0,};
    this->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_NUM_CS, LOW); //pull SS slow to prep other end for transfer
  
    /*char c;
    for (const char *da=data.c_str();c=*da;da++){
        spi->transfer(c);
    }*/
    for (size_t i = 0; i < length; i++)
    {
        transferdata.data[i] =  this->transfer(*data++);
        //*data++;
    }
    using namespace std;
    PacketResponse *repacket = (PacketResponse *)transferdata.data;
    ESP_LOGI(TAG,"CommandID:%s",repacket->CommandID);
    //this->transfer(data,length);
    digitalWrite(PIN_NUM_CS, HIGH); //pull ss high to signify end of data transfer
    this->endTransaction();
    transferdata.size = sizeof(transferdata.data)/sizeof(transferdata.data[0]);
    return transferdata;
    //return RBuff;
}

void SECC_SPIClass::SPItransfer(uint8_t *data,uint8_t *rxdata,size_t length){


    static const int spiClk = 20000000;

    this->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_NUM_CS, LOW); //pull SS slow to prep other end for transfer
    this->transferBytes(data,rxdata,length);

    digitalWrite(PIN_NUM_CS, HIGH); //pull ss high to signify end of data transfer
    this->endTransaction();

}

SECC_SPIClass SECC_SPI(VSPI);