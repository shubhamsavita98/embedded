/**********************************************************************************
* File Name:   main.c
*
* Description: Code for Basic Oscilloscope implementation; ADC and DMA HAL drivers
*              are configured to sample input voltage and the sampled input
*              voltage is displayed on the UART. Better Serial Plotter is used
*              to control time/amplitude divisions, analysis and visualization
*              of the waveforms.
*
* Related Document:
* https://infineon.github.io/psoc6pdl/pdl_api_reference_manual/html/index.html
*
***********************************************************************************
* Copyright 2020-2023, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
***********************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"


/*  ADC Macros */
#define VPLUS_CHANNEL_0  (P10_0)
#define MICRO_TO_MILLI_CONV_RATIO        (1000u)
#define ACQUISITION_TIME_NS              (100u)
#define ADC_SCAN_DELAY_MS                (1u)

/*  DMA Macros */
#define DMA_HW    DMAC
#define DMA_CHANNEL             (0u)
#define DST_BUFFER_SIZE 2000
#define BUFFER_SIZE     2000
#define WS_NUM_DESCRIPTORS (2)

/*****************************************************************************/

cyhal_adc_t adc_obj;
cyhal_adc_channel_t adc_chan_0_obj;

uint16_t dstBuffer[DST_BUFFER_SIZE];
uint16_t adcBuffer[BUFFER_SIZE];
volatile uint32_t adcBufferIndex = 0;
static cy_stc_dmac_descriptor_t WSDescriptors[WS_NUM_DESCRIPTORS];

static void ws_dmac_init(void)
{
	cy_rslt_t result;

    const cy_stc_dmac_descriptor_config_t WS_DMA_Descriptors_config =
    {
		.retrigger       = CY_DMAC_RETRIG_IM,
		.interruptType   = CY_DMAC_DESCR_CHAIN,
		.triggerOutType  = CY_DMAC_1ELEMENT,
		.channelState    = CY_DMAC_CHANNEL_ENABLED,
		.triggerInType   = CY_DMAC_1ELEMENT,
		.dataSize        = CY_DMAC_BYTE,
		.srcTransferSize = CY_DMAC_TRANSFER_SIZE_DATA,
		.dstTransferSize = CY_DMAC_TRANSFER_SIZE_DATA,
		.descriptorType  = CY_DMAC_1D_TRANSFER,
		.srcAddress      = NULL,
		.dstAddress      = NULL,
		.srcXincrement   = 1L,
		.dstXincrement   = 0L,
		.xCount          = BUFFER_SIZE,
		.srcYincrement   = 0L,
		.dstYincrement   = 0L,
		.yCount          = 1UL,
		.nextDescriptor  = 0
    };

    for(unsigned int i=0;i<WS_NUM_DESCRIPTORS;i++)
    {
        Cy_DMAC_Descriptor_Init(&WSDescriptors[i], &WS_DMA_Descriptors_config);
        Cy_DMAC_Descriptor_SetSrcAddress(&WSDescriptors[i], (void *)adcBuffer);
        Cy_DMAC_Descriptor_SetDstAddress(&WSDescriptors[i], (void *)&dstBuffer[i * DST_BUFFER_SIZE]);
        Cy_DMAC_Descriptor_SetXloopDataCount(&WSDescriptors[i],BUFFER_SIZE); // the last
        if (i < (WS_NUM_DESCRIPTORS  - 1)) {
        Cy_DMAC_Descriptor_SetNextDescriptor(&WSDescriptors[i],&WSDescriptors[i+1]);}
        else {
                   Cy_DMAC_Descriptor_SetNextDescriptor(&WSDescriptors[i], NULL);
               }
    }

    /* DMA channel configuration */
    const cy_stc_dmac_channel_config_t channelConfig = {
		.descriptor  = &WSDescriptors[0],
		.priority    = 3,
		.enable      = false,
    };

    result = Cy_DMAC_Channel_Init(DMA_HW, DMA_CHANNEL, &channelConfig);

    if (result != CY_DMA_SUCCESS) {
            printf("DMA Initialization failed...\r\n\n");
            CY_ASSERT(0);
        }
    printf("DMA initialized successfully.\r\n\n");

    Cy_DMAC_Channel_Enable(DMA_HW,DMA_CHANNEL);
}

/*******************************************************************************
 * Function Name: adc_init
 *******************************************************************************
 *
 * Summary:
 *  ADC single channel initialization function. This function initializes and
 *  configures channel 0 of ADC.
 *
 *******************************************************************************/
void adc_init(void) {

	/* Variable to capture return value of functions */
    cy_rslt_t result;

    /* Initialize ADC. The ADC block which can connect to the channel 0 input pin is selected */
    result = cyhal_adc_init(&adc_obj, VPLUS_CHANNEL_0, NULL);
    if (result != CY_RSLT_SUCCESS) {
        printf("ADC initialization failed. Error: %ld\n", (long unsigned int)result);
        CY_ASSERT(0);
    }

    /* ADC channel configuration */
    const cyhal_adc_channel_config_t channel_config = {
        .enable_averaging = true,
        .min_acquisition_ns = ACQUISITION_TIME_NS,
        .enabled = true
    };

    /* Initialize a channel 0 and configure it to scan the channel 0 input pin in single ended mode. */
    result  = cyhal_adc_channel_init_diff(&adc_chan_0_obj, &adc_obj, VPLUS_CHANNEL_0,
                                          CYHAL_ADC_VNEG, &channel_config);

    if (result != CY_RSLT_SUCCESS) {
        printf("ADC single ended channel initialization failed. Error: %ld\n", (long unsigned int)result);
        CY_ASSERT(0);
    }
    printf("ADC initialized successfully.\r\n\n");
}


/*******************************************************************************
 * Function Name: adc_process
 *******************************************************************************
 *
 * Summary:
 *  ADC single channel process function. This function reads the input voltage,
 *  store it in the buffer and sends to serial prints the input voltage on plotter
 *  and UART every few samples.
 *
 *******************************************************************************/
void adc_process(void) {
    int32_t adc_result_0 = 0;
    adc_result_0 = cyhal_adc_read_uv(&adc_chan_0_obj) / MICRO_TO_MILLI_CONV_RATIO;

    // Store the ADC value in the buffer
    adcBuffer[adcBufferIndex++] = adc_result_0;
    if (adcBufferIndex >= BUFFER_SIZE) {
        adcBufferIndex = 0;
    }
	// Send ADC buffer to serial plotter every few samples
	if (adcBufferIndex % 20 == 1) {
		printf("Data: %u\r\n", (unsigned int)adcBuffer[(adcBufferIndex - 1) % BUFFER_SIZE]);
	}
	cyhal_system_delay_ms(ADC_SCAN_DELAY_MS);
}


int main(void){
    cy_rslt_t result;

    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT(0);
    }

    __enable_irq();

    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT(0);
    }

    printf("\x1b[2J\x1b[;H");
    printf("Data Acquisition Started..\r\n\n");

	adc_init();
	ws_dmac_init();

	for(;;){
		adc_process();
	}

}
