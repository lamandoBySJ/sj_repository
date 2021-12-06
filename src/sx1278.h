/**
 *  @brief:  Implementation of a SX1278 radio functions
 *  @author: luk6xff based on SEMTCH code: https://github.com/Lora-net/LoRaMac-node
 *  @email:  luszko@op.pl
 *  @date:   2019-11-15
 */

#ifndef __SX1278_H__
#define __SX1278_H__

#include <stdint.h>
#include <stdbool.h>
#include <mutex>
#include "registers.h"
#include "drivers/Timeout.h"
#include "hal/PinNames.h"
#include "drivers/DigitalIn.h"
#include "drivers/InterruptIn.h"
#include "drivers/DigitalInOut.h"
#include "drivers/DigitalOut.h"

#include "RadioDelegator.h"
#include <atomic>
#include <rtos/rtos.h>
#include "platform_debug.h"

using namespace mbed;
typedef struct
{
    RadioIRQ id;
} message_t;
typedef struct
{
    RadioEvent id;
} mail_radio_event_t;
/**
 * ============================================================================
 * @brief Public defines
 * ============================================================================
 */

/**
 * @brief Radio wake-up time from sleep
 */
#define RADIO_WAKEUP_TIME                           1 // [ms]

/**
 * @brief Sync word for Private LoRa networks
 */
#define LORA_MAC_PRIVATE_SYNCWORD                   0x12

/**
 * @brief Sync word for Public LoRa networks
 */
#define LORA_MAC_PUBLIC_SYNCWORD                    0x34


/**
 * @brief SX1278 definitions
 */
#define XTAL_FREQ                                   32000000
#define FREQ_STEP                                   61.03515625

#define RX_BUFFER_SIZE                              256

/**
 * @brief Constant values need to compute the RSSI value
 */
#define RSSI_OFFSET_LF                              -164.0
#define RSSI_OFFSET_HF                              -157.0

#define RF_MID_BAND_THRESH                          525000000



/**
 * ============================================================================
 * @brief Public datatypes declarations
 * ============================================================================
 */

/**
 * Timeout timer type
 */
typedef enum
{
    RXTimeoutTimer,
    TXTimeoutTimer,
    RXTimeoutSyncWordTimer
} TimeoutTimer_t;


/**
 * Type of the supported board. [RA-01]
 */
typedef enum
{
    BOARD_RA01 = 0,
    BOARD_UNKNOWN
} BoardType_t;

 typedef enum
 {
    LORA_SF6 =  6,  //   64 chips/symbol, SF6 requires an TCXO!
    LORA_SF7 =  7,  //  128 chips/symbol
    LORA_SF8 =  8,  //  256 chips/symbol
    LORA_SF9 =  9,  //  512 chips/symbol
    LORA_SF10 = 10, // 1024 chips/symbol
    LORA_SF11 = 11, // 2048 chips/symbol
    LORA_SF12 = 12, // 4096 chips/symbol
} LoraSpreadingFactor_t;

/**
 * Type of the supported lora bandwidths
 */
typedef enum
{
    LORA_BANDWIDTH_7kHz  = 0, //  7.8 kHz requires TCXO
    LORA_BANDWIDTH_10kHz = 1, // 10.4 kHz requires TCXO
    LORA_BANDWIDTH_15kHz = 2, // 15.6 kHz requires TCXO
    LORA_BANDWIDTH_20kHz = 3, // 20.8 kHz requires TCXO
    LORA_BANDWIDTH_31kHz = 4, // 31.2 kHz requires TCXO
    LORA_BANDWIDTH_41kHz = 5, // 41.4 kHz requires TCXO
    LORA_BANDWIDTH_62kHz = 6, // 62.5 kHz requires TCXO
    LORA_BANDWIDTH_125kHz = 7,
    LORA_BANDWIDTH_250kHz = 8,
    LORA_BANDWIDTH_500kHz = 9,
    LORA_BANDWIDTH_RESERVED = 10,
} LoraBandwidth_t;


typedef enum 
{  // cyclic error coding to perform forward error detection and correction
    LORA_ERROR_CODING_RATE_4_5 = 1,   // 1.25x overhead
    LORA_ERROR_CODING_RATE_4_6 = 2,   // 1.50x overhead
    LORA_ERROR_CODING_RATE_4_7 = 3,   // 1.75x overhead
    LORA_ERROR_CODING_RATE_4_8 = 4,   // 2.00x overhead
} LoraCodingRate_t;


typedef enum 
{
    RF_FREQUENCY_433_0 = 433000000,
    RF_FREQUENCY_470_0 = 470000000, // Hz
    RF_FREQUENCY_434_0 = 434000000, // Hz
    RF_FREQUENCY_868_0 = 868000000, // Hz
    RF_FREQUENCY_868_1 = 868100000, // Hz
    RF_FREQUENCY_868_3 = 868300000, // Hz
    RF_FREQUENCY_868_5 = 868500000, // Hz
} RfFrequency_t;


/**
 * @brief Radio driver callback functions.
 *        Reporting functions for upper layers.
 *        The radio driver reports various vital events to the upper controlling layers
 *        using callback functions provided by the upper layers at the initialization
 *        phase.
 */
typedef struct
{
    /**
     * @brief Callback when Transmission is done.
     */
    void (*TxDone)(void);

    /**
     * @brief Callback when Transmission is timed out.
     */
    void (*TxTimeout)(void);

    /**
     * @brief Rx Done callback prototype.
     *
     * @param [IN] payload Received buffer pointer
     * @param [IN] size    Received buffer size
     * @param [IN] rssi    RSSI value computed while receiving the frame [dBm]
     * @param [IN] snr     Raw SNR value given by the radio hardware
     *                     FSK : N/A (set to 0)
     *                     LoRa: SNR value in dB
     */
    void (*RxDone)(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
    
    /**
     * @brief Callback when Reception is timed out.
     */
    void (*RxTimeout)(void);

    /**
     * @brief Callback when Reception ends up in error.
     */
    void (*RxError)(void);

    /**
     * @brief  FHSS Change Channel callback prototype.
     *
     * @param [IN] currentChannel   Index number of the current channel
     */
    void (*FhssChangeChannel)(uint8_t currentChannel);

    /**
     * @brief CAD Done callback prototype.
     *
     * @param [IN] channelDetected    True, if Channel activity detected.
     */
    void (*CadDone) (bool channelActivityDetected);
    void (*CadDetected) ();
} RadioEvents_t;



/**
 * @brief Radio driver internal state machine states definition
 * Helps identify current state of the transceiver.
 */
typedef enum
{
    /** @brief IDLE state.
     * Radio is in idle state.
     */
    RF_IDLE = 0,

    /** @brief RX state.
     * Radio is receiving.
     */
    RF_RX_RUNNING,

    /** @brief TX state.
     * Radio is transmitting.
     */
    RF_TX_RUNNING,

    /** @brief CAD state.
     * Radio is detecting channel activity.
     */
    RF_CAD,
} RadioState_t;

/** 
 * @brief Type of modem [LORA/FSK]
 */
typedef enum
{
    /** 
     * @brief FSK operation mode.
     * Radio is using FSK modulation.
     */
    MODEM_FSK = 0,

    /** 
     * @brief LoRa operation mode.
     * Radio is using LoRa modulation.
     */
    MODEM_LORA
} RadioModems_t;

/**
 * @brief  Radio FSK modem parameters
 * Parameters encompassing FSK modulation.
 */
typedef struct
{
    /**
     * @brief Transmit power.
     */
    int8_t   Power;

    /**
     * @brief Frequency deviation.
     */
    uint32_t Fdev;

    /**
     * @brief Modulation bandwidth.
     */
    uint32_t Bandwidth;

    /**
     * @brief Automated frequency correction bandwidth.
     */
    uint32_t BandwidthAfc;

    /**
     * @brief Data rate (SF).
     */
    uint32_t Datarate;

    /**
     * @brief Expected preamble length.
     */
    uint16_t PreambleLen;

    /**
     * @brief This flag turns on if the TX data size is fixed.
     */
    bool     FixLen;

    /**
     * @brief Size of outgoing data.
     */
    uint8_t  PayloadLen;

    /**
     * @brief Turn CRC on/off.
     */
    bool     CrcOn;

    /** @deprecated
     * @brief Does not apply to FSK. Will be removed.
     */
    bool     IqInverted;

    /**
     * @brief Turn continuous reception mode (such as Class C mode) on/off.
     */
    bool     RxContinuous;

    /**
     * @brief Timeout value in milliseconds (ms) after which the radio driver reports
     * a timeout if the radio was unable to transmit.
     */
    uint32_t TxTimeout;

    /**
     * @brief Timeout value in symbols (symb) after which the radio driver reports a timeout
     * if the radio did not receive a Preamble.
     */
    uint32_t RxSingleTimeout;
} RadioFskSettings_t;

/**
 * @brief Radio FSK packet handler state
 * Contains information about an FSK packet and various metadata.
 */
typedef struct
{
    /**
     * @brief Set to true (1) when a Preamble is detected, otherwise false (0).
     */
    uint8_t  PreambleDetected;

    /**
     * @brief Set to true (1) when a SyncWord is detected, otherwise false (0).
     */
    uint8_t  SyncWordDetected;

    /**
     * @brief Storage for RSSI value of the received signal.
     */
    int8_t   RssiValue;

    /**
     * @brief Automated frequency correction value.
     */
    int32_t  AfcValue;

    /**
     * @brief LNA gain value (dbm).
     */
    uint8_t  RxGain;

    /**
     * @brief Size of the received data in bytes.
     */
    uint16_t Size;

    /**
     * @brief Keeps track of number of bytes already read from the RX FIFO.
     */
    uint16_t NbBytes;

    /**
     * @brief Stores the FIFO threshold value.
     */
    uint8_t  FifoThresh;

    /**
     * @brief Defines the size of a chunk of outgoing buffer written to
     * the FIFO at a unit time. For example, if the size of the data exceeds the FIFO
     * limit, a certain sized chunk is written to the FIFO. Later, a FIFO-level
     * interrupt enables writing of the remaining data to the FIFO chunk by chunk until
     * transmission is complete.
     */
    uint8_t  ChunkSize;
} RadioFskPacketHandler_t;

/**
 * @brief Radio LoRa modem parameters
 * Parameters encompassing LoRa modulation.
 */
typedef struct
{
    /**
     * @brief Transmit power.
     */
    int8_t   Power;

    /**
     * @brief Modulation bandwidth.
     */
    uint32_t Bandwidth;

    /**
     * @brief Data rate (SF).
     */
    uint32_t Datarate;

    /**
     * @brief Turn low data rate optimization on/off.
     */
    bool     LowDatarateOptimize;

    /**
     * @brief Error correction code rate.
     */
    uint8_t  Coderate;

    /**
     * @brief Preamble length in symbols.
     */
    uint16_t PreambleLen;

    /**
     * @brief Set to true if the outgoing payload length is fixed.
     */
    bool     FixLen;

    /**
     * @brief Size of outgoing payload.
     */
    uint8_t  PayloadLen;

    /**
     * @brief Turn CRC on/off.
     */
    bool     CrcOn;

    /**
     * @brief Turn frequency hopping on/off.
     */
    bool     FreqHopOn;

    /**
     * @brief Number of symbols between two frequency hops.
     */
    uint8_t  HopPeriod;

    /**
     * @brief Turn IQ inversion on/off. Usually, the end device sends an IQ inverted
     * signal, and the base stations do not invert. We recommended sending an
     * IQ inverted signal from the device side, so any transmissions from the
     * base stations do not interfere with end device transmission.
     */
    bool     IqInverted;

    /**
     * @brief Turn continuous reception mode (such as in Class C) on/off.
     */
    bool     RxContinuous;

    /**
     * @brief Timeout in milliseconds (ms) after which the radio driver reports an error
     * if the radio was unable to transmit.
     */
    uint32_t TxTimeout;

    /**
     * @brief Change the network mode to Public or Private.
     */
    bool     PublicNetwork;
} RadioLoRaSettings_t;

/**
 * Radio LoRa packet handler state
 */
typedef struct
{
    /**
     * @brief Signal-to-noise ratio of a received packet.
     */
    int8_t SnrValue;

    /**
     * @brief RSSI value in dBm for the received packet.
     */
    int16_t RssiValue;

    /**
     * @brief Size of the transmitted or received packet.
     */
    uint8_t Size;
} RadioLoRaPacketHandler_t;


/** 
 *  @brief Global radio settings.
 *  Contains settings for the overall transceiver operation.
 */
typedef struct
{
    /**
     * @brief Current state of the radio, such as RF_IDLE.
     */
    RadioState_t               State;
    
    /**
     * @brief Current modem operation, such as LORA or FSK.
     */
    RadioModems_t            Modem;

    /**
     * @brief Current channel of operation.
     */
    uint32_t                 Channel;

    /**
     * @brief Settings for FSK modem part.
     */
    RadioFskSettings_t       Fsk;

    /**
     * @brief FSK packet and meta data.
     */
    RadioFskPacketHandler_t  FskPacketHandler;

    /**
     * @brief Settings for LoRa modem part.
     */
    RadioLoRaSettings_t      LoRa;

    /**
     * @brief LoRa packet and metadata.
     */
    RadioLoRaPacketHandler_t LoRaPacketHandler;
} RadioSettings_t;


/**
 * @brief FSK and LORA bandwidth definition
 */
typedef struct
{
    uint32_t bandwidth;
    uint8_t  RegValue;
} BandwidthMap_t;

/**
 * @brief Radio registers definition
 */
typedef struct
{
    RadioModems_t   Modem;
    uint8_t     Addr;
    uint8_t     Value;
} RadioRegisters_t;





/**
 * ============================================================================
 * @brief Public functions prototypes
 * ============================================================================
 */

/** 
 * @brief Actual implementation of a SX1278 radio
 */
 
//-------------------------------------------------------------------------
//                        Radio functions
//-------------------------------------------------------------------------
/**
 * @brief Initializes the radio
 *
 * @param [IN] events Structure containing the driver callback functions
 */
class RadioInterface
{
public:
virtual ~RadioInterface()=default;
virtual    void TxDone(void)=0;
virtual    void TxTimeout(void)=0;
virtual   void RxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)=0;
virtual    void RxTimeout(void)=0;
virtual    void RxError(void)=0;
virtual    void FhssChangeChannel(uint8_t currentChannel)=0;
virtual    void CadDone(bool channelActivityDetected)=0;
virtual    void CadDetected ()=0;
};


class Radio : public  OnTxRunningListener, public  OnRxRunningListener
{
public:
Radio()=delete;
Radio(RadioInterface* objectInterface):_thisObject(objectInterface)
    
{   
    //_thisObject = std::make_shared<RadioInterface>(objectInterface);
    dioIrq[0] = &Radio::OnDio0IrqEnqueue;
    dioIrq[1] = &Radio::OnDio1IrqEnqueue;
    dioIrq[2] = &Radio::OnDio2IrqEnqueue;
    dioIrq[3] = &Radio::OnDio3IrqEnqueue;
    dioIrq[4] = &Radio::OnDio4IrqEnqueue;
    dioIrq[5] = &Radio::OnDio5IrqEnqueue;
    _paSelect=true;
}

/**
 * @brief Hardware IO IRQ callback function definition
 */
using DioIrqHandler = void (Radio::*)(void);

/**
 * @brief Timeout timer callback function definition
 */
using timeoutFuncPtr = void (Radio::*)(void);
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void AttachIrqHandlers(DioIrqHandler *irqHandlers)
{
    dio0->rise(mbed::callback(this,irqHandlers[0]));
    dio1->rise(mbed::callback(this,irqHandlers[1]));
    dio2->rise(mbed::callback(this,irqHandlers[2]));
    //dio3->rise(mbed::callback(this,irqHandlers[3]));
    //dio4->rise(mbed::callback(this,irqHandlers[4]));
}

bool ModuleInit();

/**
 * @brief Initializes the radio registers
 */
void SX1278RadioRegistersInit();


/**
 * Return current radio status
 *
 * @param status Radio status. [RF_IDLE, RX_RUNNING, TX_RUNNING]
 */
RadioState_t SX1278GetStatus(void);

/**
 * @brief Configures the SX1278 with the given modem
 *
 * @param [IN] modem Modem to be used [0: FSK, 1: LoRa] 
 */
void SX1278SetModem(RadioModems_t modem);

/**
 * @brief Sets the channel frequency
 *
 * @param [IN] freq         Channel RF frequency
 */
void SX1278SetChannel(uint32_t freq);

/**
 * @brief Sets the channels configuration
 *
 * @param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
 * @param [IN] freq       Channel RF frequency
 * @param [IN] rssiThresh RSSI threshold
 * @param [IN] maxCarrierSenseTime Max time while the RSSI is measured
 *
 * @retval isFree         [true: Channel is free, false: Channel is not free]
 */
bool SX1278IsChannelFree(RadioModems_t modem, uint32_t freq, int16_t rssiThresh, uint32_t maxCarrierSenseTime);

/**
 * @brief Generates a 32 bits random value based on the RSSI readings
 *
 * @note This function sets the radio in LoRa modem mode and disables
 *         all interrupts.
 *         After calling this function either SX1278SetRxConfig or
 *         SX1278SetTxConfig functions must be called.
 *
 * @retval randomValue    32 bits random value
 */
uint32_t SX1278Random(void);

/**
 * @brief Sets the reception parameters
 *
 * @note When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported
 *
 * @param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
 * @param [IN] bandwidth    Sets the bandwidth
 *                          FSK : >= 2600 and <= 250000 Hz
 *                          LoRa: [0: 125 kHz, 1: 250 kHz,
 *                                 2: 500 kHz, 3: Reserved]
 * @param [IN] datarate     Sets the Datarate
 *                          FSK : 600..300000 bits/s
 *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
 *                                10: 1024, 11: 2048, 12: 4096  chips]
 * @param [IN] coderate     Sets the coding rate (LoRa only)
 *                          FSK : N/A (set to 0)
 *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * @param [IN] bandwidthAfc Sets the AFC Bandwidth (FSK only)
 *                          FSK : >= 2600 and <= 250000 Hz
 *                          LoRa: N/A (set to 0)
 * @param [IN] preambleLen  Sets the Preamble length (LoRa only)
 *                          FSK : N/A (set to 0)
 *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
 * @param [IN] symbTimeout  Sets the RxSingle timeout value
 *                          FSK : timeout number of bytes
 *                          LoRa: timeout in symbols
 * @param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
 * @param [IN] payloadLen   Sets payload length when fixed lenght is used
 * @param [IN] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
 * @param [IN] freqHopOn    Enables disables the intra-packet frequency hopping
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: OFF, 1: ON]
 * @param [IN] hopPeriod    Number of symbols between each hop
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: Number of symbols
 * @param [IN] iqInverted   Inverts IQ signals (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: not inverted, 1: inverted]
 * @param [IN] rxContinuous Sets the reception in continuous mode
 *                          [false: single mode, true: continuous mode]
 */
void SX1278SetRxConfig (RadioModems_t modem, uint32_t bandwidth,
                            uint32_t datarate, uint8_t coderate,
                            uint32_t bandwidthAfc, uint16_t preambleLen,
                            uint16_t symbTimeout, bool fixLen,
                            uint8_t payloadLen,
                            bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                            bool iqInverted, bool rxContinuous);
/**
 * @brief Sets the transmission parameters
 *
 * @param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
 * @param [IN] power        Sets the output power [dBm]
 * @param [IN] fdev         Sets the frequency deviation (FSK only)
 *                          FSK : [Hz]
 *                          LoRa: 0
 * @param [IN] bandwidth    Sets the bandwidth (LoRa only)
 *                          FSK : 0
 *                          LoRa: [0: 125 kHz, 1: 250 kHz,
 *                                 2: 500 kHz, 3: Reserved]
 * @param [IN] datarate     Sets the Datarate
 *                          FSK : 600..300000 bits/s
 *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
 *                                10: 1024, 11: 2048, 12: 4096  chips]
 * @param [IN] coderate     Sets the coding rate (LoRa only)
 *                          FSK : N/A (set to 0)
 *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * @param [IN] preambleLen  Sets the preamble length
 * @param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
 * @param [IN] crcOn        Enables disables the CRC [0: OFF, 1: ON]
 * @param [IN] freqHopOn    Enables disables the intra-packet frequency hopping  [0: OFF, 1: ON] (LoRa only)
 * @param [IN] hopPeriod    Number of symbols bewteen each hop (LoRa only)
 * @param [IN] iqInverted   Inverts IQ signals (LoRa only)
 *                          FSK : N/A (set to 0)
 *                          LoRa: [0: not inverted, 1: inverted]
 * @param [IN] timeout      Transmission timeout [ms]
 */
void SX1278SetTxConfig(RadioModems_t modem, int8_t power, uint32_t fdev,
                            uint32_t bandwidth, uint32_t datarate,
                            uint8_t coderate, uint16_t preambleLen,
                            bool fixLen, bool crcOn, bool freqHopOn,
                            uint8_t hopPeriod, bool iqInverted, uint32_t timeout);

/**
 * @brief Computes the packet time on air for the given payload
 *
 * @note Can only be called once SetRxConfig or SetTxConfig have been called
 *
 * @param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
 * @param [IN] pktLen     Packet payload length
 *
 * @retval airTime        Computed airTime (ms) for the given packet payload length
 */
uint32_t SX1278GetTimeOnAir(RadioModems_t modem, uint8_t pktLen);

/**
 * @brief Sends the buffer of size. Prepares the packet to be sent and sets
 *        the radio in transmission
 *
 * @param [IN]: buffer     Buffer pointer
 * @param [IN]: size       Buffer size
 */
void SX1278Send(uint8_t *buffer, uint8_t size);

/**
 * @brief Sets the radio in sleep mode
 */
void SX1278SetSleep(void);

/**
 * @brief Sets the radio in standby mode
 */
void SX1278SetStandby(void);

/**
 * @brief Sets the radio in CAD mode (starts a Channel Activity Detection)
 */
void SX1278StartCad(void);

/**
 * @brief Sets the radio in reception mode for the given time
 * @param [IN] timeout Reception timeout [ms]
 *                     [0: continuous, others timeout]
 */
void SX1278SetRx(uint32_t timeout=osWaitForever);

/**
 * @brief Sets the radio in transmission mode for the given time
 * @param [IN] timeout Transmission timeout [ms]
 *                     [0: continuous, others timeout]
 */
void SX1278SetTx(uint32_t timeout);

/**
 * @brief Sets the radio in continuous wave transmission mode
 *
 * @param [IN]: freq       Channel RF frequency
 * @param [IN]: power      Sets the output power [dBm]
 * @param [IN]: time       Transmission mode timeout [s]
 */
void SX1278SetTxContinuousWave(uint32_t freq, int8_t power, uint16_t time);

/**
 * @brief Reads the current RSSI value
 *
 * @retval rssiValue Current RSSI value in [dBm]
 */
int16_t SX1278GetRssi(RadioModems_t modem);

/**
 * @brief Reads the current frequency error
 *
 * @retval frequency error value in [Hz]
 */
int32_t SX1278GetFrequencyError(RadioModems_t modem);

/**
 * @brief Sets the maximum payload length.
 *
 * @param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
 * @param [IN] max        Maximum payload length in bytes
 */
void SX1278SetMaxPayloadLength(RadioModems_t modem, uint8_t max);

/**
 * @brief Sets the network to public or private. Updates the sync byte.
 *
 * @note Applies to LoRa modem only
 *
 * @param [IN] enable if true, it enables a public network
 */
void SX1278SetPublicNetwork(bool enable);

/**
 * @brief Gets the time required for the board plus radio to get out of sleep.[ms]
 *
 * @retval Time Radio plus board wakeup time in ms.
 */
uint32_t SX1278GetWakeupTime(void);

/**
 * @brief Sets the SX1278 operating mode
 *
 * @param [IN] opMode New operating mode
 */
void SX1278SetOpMode(uint8_t opMode);

/**
 * @brief Sets the radio output power.
 *
 * @param [IN] power Sets the RF output power
 */
void SX1278SetRfTxPower(int8_t power);

/**
 * @brief Gets the PA selection configuration
 *
 * @param [IN] channel Channel frequency in Hz
 * @retval PaSelect RegPaConfig PaSelect value
 */
uint8_t SX1278GetPaSelect(uint32_t channel=525000000);
void SX1278SetPaSelect(bool PABOOST)
{
    _paSelect=PABOOST;
}
/**
 * @brief Checks if the given RF frequency is supported by the hardware
 *
 * @param [IN] frequency RF frequency to be checked
 * @retval isSupported [true: supported, false: unsupported]
 */
bool SX1278CheckRfFrequency(uint32_t frequency);

/**
 * @brief Writes the radio register at the specified address
 *
 * @param [IN]: addr Register address
 * @param [IN]: data New register value
 */
void SX1278Write (uint8_t addr, uint8_t data);

/**
 * @brief Reads the radio register at the specified address
 *
 * @param [IN]: addr Register address
 * @retval data Register value
 */
uint8_t SX1278Read(uint8_t addr);

/**
 * @brief Writes the buffer contents to the SX1278 FIFO
 *
 * @param [IN] buffer Buffer containing data to be put on the FIFO.
 * @param [IN] size Number of bytes to be written to the FIFO
 */
void SX1278WriteFifo(uint8_t *buffer, uint8_t size);

/**
 * @brief Reads the contents of the SX1278 FIFO
 *
 * @param [OUT] buffer Buffer where to copy the FIFO read data.
 * @param [IN] size Number of bytes to be read from the FIFO
 */
void SX1278ReadFifo(uint8_t *buffer, uint8_t size);

void RxChainCalibration(void);
/**
 * Returns the known FSK bandwidth registers value
 *
 * @param [IN] bandwidth Bandwidth value in Hz
 * @retval regValue Bandwidth register value.
 */
uint8_t GetFskBandwidthRegValue(uint32_t bandwidth);
/**
 * Returns the known LORA bandwidth registers value
 *
 * @param [IN] bandwidth Bandwidth value in Hz
 * @retval regValue Bandwidth register value.
 */
 uint8_t GetLoRaBandwidthRegValue(uint32_t bandwidth);

void OnDio0Irq();
void OnDio1Irq();
void OnDio2Irq();
void OnDio3Irq();
void OnDio4Irq();
void OnDio5Irq();

void OnDio0IrqEnqueue(){
   message_t* message = _pool.alloc();
   message->id= RadioIRQ::DIO0;
    _queue.try_put_from_isr(message);
}
void OnDio1IrqEnqueue(){
   message_t* message = _pool.alloc();
   message->id= RadioIRQ::DIO1;
    _queue.try_put_from_isr(message);
}
void OnDio2IrqEnqueue(){
   message_t* message = _pool.alloc();
  message->id= RadioIRQ::DIO2;
    _queue.try_put_from_isr(message);
}
void OnDio3IrqEnqueue(){
   message_t* message = _pool.alloc();
  message->id= RadioIRQ::DIO3;
    _queue.try_put_from_isr(message);
}
void OnDio4IrqEnqueue(){
   message_t* message = _pool.alloc();
 message->id= RadioIRQ::DIO4;
 if(_queue.full()){
     return;
 }else{
      _queue.try_put_from_isr(message);
 }
}
void OnDio5IrqEnqueue(){
   message_t* message = _pool.alloc();
   message->id= RadioIRQ::DIO5;
    _queue.try_put_from_isr(message);
}
bool  RxBusy(){
    return _queue.count()!=0;
}
RadioIRQ receive(uint32_t millis=osWaitForever){
    irq=RadioIRQ::NONE;
    osEvent evt = _queue.get(millis);
    if(evt.status == osEventMessage){
            message_t* message = (message_t*)evt.value.p;
            irq = message->id;
            switch(irq)
            {
                case RadioIRQ::DIO0:OnDio0Irq();break;
                case RadioIRQ::DIO1:OnDio1Irq();break;
                case RadioIRQ::DIO2:OnDio2Irq();break;
                case RadioIRQ::DIO3:OnDio3Irq();break;
                case RadioIRQ::DIO4:OnDio4Irq();break;
               // case RadioIRQ::DIO5:OnDio5Irq();break;
                default:   irq=RadioIRQ::NONE;break;
            }
            _pool.free(message);
    }
    return irq;
}
/*
void RadioEventPoll(RadioEvent id){
    mail_radio_event_t* message = _mail.alloc();
    message->id= id;
    _mail.put(message);
}
RadioEvent RadioEventHandler(uint32_t millis=osWaitForever){
    RadioEvent id =RadioEvent::RF_IDLE;
    osEvent evt = _mail.get(millis);
    if(evt.status == osEventMail){
        mail_radio_event_t* message =(mail_radio_event_t*)evt.value.p;
        id = message->id;
        _mail.free(message);
    }
    return id;
}*/
//-------------------------------------------------------------------------
//  Board/Platform relative (HW dependent) functions, 
//  Must be implemented in folder platform. 
//-------------------------------------------------------------------------

/**
 * @brief Initializes the radio I/Os pins interface
 */
virtual void IoInit(PinName _mosi, PinName _miso, PinName _sclk, PinName _nss,
            PinName _dio0, PinName _dio1, PinName _dio2, PinName _dio3, PinName _dio4, PinName _dio5,
            PinName _reset)=0;

/**
 * @brief De-initializes the radio I/Os pins interface. 
 *
 * @note Useful when going in MCU lowpower modes
 */
virtual void IoDeInit(void)=0;

/**
 * @brief Initializes DIO IRQ handlers
 *
 * @param [IN] irqHandlers Array containing the IRQ callback functions
 */
//virtual void IoIrqInit(DioIrqHandler *irqHandlers)=0;

/**
 * @brief Resets the SX1278
 */
//virtual void Reset(void)=0;
//-----------------------------------------------------------------------------
void Reset(void)
{
    //reset->output();
    *reset = 0;
    DelayMs(20);
   // reset->input();
    *reset = 1;
    DelayMs(50);
}
/**
 * @brief Writes multiple radio registers starting at address
 *
 * @param [IN] addr   First Radio register address
 * @param [IN] buffer Buffer containing the new register's values
 * @param [IN] size   Number of registers to be written
 */
virtual void WriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)=0;

/**
 * @brief Reads multiple radio registers starting at address
 *
 * @param [IN] addr First Radio register address
 * @param [OUT] buffer Buffer where to copy the registers data
 * @param [IN] size Number of registers to be read
 */
virtual void ReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)=0;

/*
    * @brief The the Timeout for a given Timer.
    */
 void SetTimeout(TimeoutTimer_t timer, timeoutFuncPtr, int timeout_ms);

/*
    * @brief A simple ms sleep
    */
virtual void DelayMs(int ms)=0;

void OnTimeoutIrq();

void onRxRunning() override
{
   // Serial.println("onRxRunning:RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR ");
    receive(RadioListener::_timeout);
    _mtx.unlock();
}
void onTxRunning() override
{
   // Serial.println("onTxRunning:TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT ");
    SetTimeout(TXTimeoutTimer, &Radio::OnTimeoutIrq,RadioListener::_timeout);
    while ((SX1278Read(REG_LR_IRQFLAGS) & RFLR_IRQFLAGS_TXDONE_MASK) == 0) {
        yield();
    }
    
    receive(RadioListener::_timeout);
    SetTimeout(TXTimeoutTimer, nullptr, 0);
    SX1278Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_TXDONE_MASK);
    _mtx.unlock();
}
void onRadioListenerChanged() override{
    if(settings.State == RF_TX_RUNNING){
        this->OnTxRunningListener::onRadioListenerChanged();
    }else{
       this->OnRxRunningListener::onRadioListenerChanged();
    }
}
void wake(){

  SX1278Read(REG_LR_IRQFLAGS);
}
bool Cad(){
    uint8_t cad= (SX1278Read(REG_OPMODE) & RF_OPMODE_MASK) ;
    return cad == 0x07;
}
void clear(){
  //  if ((SX1278Read(REG_LR_IRQFLAGS) & RFLR_IRQFLAGS_CADDETECTED) == RFLR_IRQFLAGS_CADDETECTED)
  //  {
      // Clear Irq
       SX1278Write(REG_LR_IRQFLAGS,RFLR_IRQFLAGS_CADDETECTED | RFLR_IRQFLAGS_CADDONE);
               
  //  }
}
void query(){
     debug("SX1278Read(REG_LR_IRQFLAGS):%x\n",SX1278Read(REG_LR_IRQFLAGS));
}

void setDelegateObject(RadioInterface** obj){
    this->_objectInterface=*obj;
}
void destoryDelegateObject(){
    if(this->_objectInterface!=nullptr){
        delete this->_objectInterface;
        this->_objectInterface=nullptr;
    }
}
void dumpCallbacks(){
    if(this->_objectInterface!=nullptr){
    this->_objectInterface->TxDone();
    this->_objectInterface->TxTimeout();
    this->_objectInterface->RxTimeout();
    this->_objectInterface->CadDone(true);
    this->_objectInterface->CadDetected();
    }
}
DioIrqHandler dioIrq[6];// = {&Radio::OnDio0Irq, &Radio::OnDio1Irq, &Radio::OnDio2Irq, &Radio::OnDio3Irq,&Radio::OnDio4Irq, NULL };

/**
 * Radio settings
 */
RadioSettings_t settings;
uint32_t SX1278TimeOnAir(RadioModems_t modem, uint8_t pktLen);
RadioIRQ irq;
Timeout txTimeoutTimer;
Timeout rxTimeoutTimer;
Timeout rxTimeoutSyncWord;
Timeout _CCATimeout;

    /**
     * SX1278 DIO pins
     */
    InterruptIn* dio0;
    InterruptIn* dio1;
    InterruptIn* dio2; 
    InterruptIn* dio3;
    InterruptIn* dio4;
    DigitalIn* dio5;
       /**
     * SX1278 Reset pin
     */
    DigitalOut* reset;
    rtos::Mutex _mtx;
    rtos::Queue<message_t,32> _queue;
    rtos::MemoryPool<message_t,32> _pool;   

    //rtos::Mail<mail_radio_event_t,32> _mail;
    std::atomic<bool> _paSelect;

    //static RadioEvents_t* RadioEvents;
    //mbed::Callback<void()> _onTxDone=nullptr;
    //mbed::Callback<void()> _onTxTimeout=nullptr;
    //mbed::Callback<void()> _onRxTimeout=nullptr;
    //mbed::Callback<void(uint8_t*, uint16_t, int16_t, int8_t)> _onRxDone=nullptr;
   // mbed::Callback<void()> _onRxError=nullptr;
    //mbed::Callback<void(uint8_t)> _onFhssChangeChannel=nullptr;
    //mbed::Callback<void(bool)> _onCadDone=nullptr;
    //mbed::Callback<void()> _onCadDetected=nullptr;
    RadioInterface* _objectInterface=nullptr;
    std::shared_ptr<RadioInterface> _thisObject;
};

#endif // __SX1278_H__

