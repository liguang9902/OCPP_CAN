#ifndef SECC_SPI_H_INCLUDED
#define SECC_SPI_H_INCLUDED

//#include "driver\spi_master.h"
#include "string.h"
#include "SPI.h"

/*class SPISettings
{
public:
    SPISettings() :_clock(1000000), _bitOrder(SPI_MSBFIRST), _dataMode(SPI_MODE0) {}
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) :_clock(clock), _bitOrder(bitOrder), _dataMode(dataMode) {}
    uint32_t _clock;
    uint8_t  _bitOrder;
    uint8_t  _dataMode;
};*/

struct TransferData{
    uint8_t  data[256]={0,};
    size_t   size;
};

#define SPI_HAS_TRANSACTION
class SECC_SPIClass: public SPIClass
{
private:
    int8_t _spi_num;
    spi_t * _spi;
    bool _use_hw_ss;
    int8_t _sck;
    int8_t _miso;
    int8_t _mosi;
    int8_t _ss;
    uint32_t _div;
    uint32_t _freq;
    bool _inTransaction;
    
    //TransferData  transferdata;
public:
    SECC_SPIClass(uint8_t spi_bus=HSPI);

    //void begin(int8_t sck=-1, int8_t miso=-1, int8_t mosi=-1, int8_t ss=-1);
    //void end();


    int available(void);
    //int availableForWrite(void);
    int peek(void);
    int read(void);
    void flush(void);
    //void flush( bool txOnly);
    //size_t write(uint8_t);
    //size_t write(const uint8_t *buffer, size_t size);
    void  SPIsend( uint8_t *data,size_t length);
    size_t  SPIrev(uint8_t *data,size_t length);
    TransferData SPItransfer(uint8_t *data,size_t length);
    void SPItransfer(uint8_t *data,uint8_t *rxdata,size_t length);
    //esp_err_t spi_write(spi_device_handle_t spi, uint8_t *data, uint8_t  len);

    //spi_t * bus(){ return _spi; }
};

extern SECC_SPIClass SECC_SPI;

#endif

