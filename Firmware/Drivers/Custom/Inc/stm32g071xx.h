/**
 * @file    stm32g071xx.h
 * @brief   Device-specific register definitions for STM32G071xx MCUs.
 *
 * This file contains base addresses, peripheral register structures,
 * peripheral pointers, clock/reset macros, IRQ numbers, and bit definitions
 * used by the bare-metal peripheral drivers in this project.
 *
 * Target board: NUCLEO-G071RB
 * Target MCU:   STM32G071RBTx
 */

#ifndef INC_STM32G071XX_H_
#define INC_STM32G071XX_H_

#include <stdint.h>

/* ------------------------------------------------------------
 * Generic definitions
 * ------------------------------------------------------------ */

#define __vo 				volatile
#define __weak   			 __attribute__((weak))

#define ENABLE              1U
#define DISABLE             0U

#define SET                 ENABLE
#define RESET               DISABLE

#define GPIO_PIN_SET        SET
#define GPIO_PIN_RESET      RESET

#define FLAG_RESET          RESET
#define FLAG_SET            SET

/* ------------------------------------------------------------
 * Processor / NVIC definitions
 * ------------------------------------------------------------ */

/* NVIC ISER registers: enable IRQ lines in the processor by writing 1 to the IRQ bit */
#define NVIC_ISER0 			((__vo uint32_t*)0xE000E100)  // IRQ 0-31
#define NVIC_ISER1 			((__vo uint32_t*)0xE000E104)  // IRQ 32-63
#define NVIC_ISER2			((__vo uint32_t*)0xE000E108)  // IRQ 64-95

/* NVIC ICER registers: disable IRQ lines in the processor by writing 1 to the IRQ bit */
#define NVIC_ICER0			((__vo uint32_t*)0xE000E180)
#define NVIC_ICER1			((__vo uint32_t*)0xE000E184)
#define NVIC_ICER2			((__vo uint32_t*)0xE000E188)

/* NVIC Interrupt Priority Register base address */
#define NVIC_PR_BASE_ADDR	((__vo uint32_t*)0xE000E400UL)

/* STM32G071 implements 3 priority bits, valid priority values: 0-7 */

#define NO_PR_BITS_IMPLEMENTED 		3U

/* IRQ numbers used by the drivers in this project based on the vector table */
#define IRQ_NO_EXTI0_1			5U
#define IRQ_NO_EXTI2_3			6U
#define IRQ_NO_EXTI4_15			7U

#define IRQ_NO_SPI1				25U
#define IRQ_NO_SPI2				26U

#define IRQ_NO_I2C1				23U
#define IRQ_NO_I2C2				24U

#define IRQ_NO_USART1           27U
#define IRQ_NO_USART2			28U
#define IRQ_NO_USART3_4_5_6		29U

/* ------------------------------------------------------------
 * Memory base addresses
 * ------------------------------------------------------------ */

#define FLASH_BASEADDR              0x08000000UL
#define SRAM_BASEADDR               0x20000000UL
#define ROM_BASEADDR                0x1FFF0000UL

/* ------------------------------------------------------------
 * Bus and peripheral base addresses
 * ------------------------------------------------------------ */

#define PERIPH_BASE                 0x40000000UL
#define APB1PERIPH_BASE             PERIPH_BASE
#define APB2PERIPH_BASE             0x40010000UL
#define AHBPERIPH_BASE              0x40020000UL
#define IOPORT_BASEADDR             0x50000000UL

/* GPIO ports are located in the IOPORT memory region on STM32G071 */
#define GPIOA_BASEADDR              (IOPORT_BASEADDR + 0x0000UL)
#define GPIOB_BASEADDR              (IOPORT_BASEADDR + 0x0400UL)
#define GPIOC_BASEADDR              (IOPORT_BASEADDR + 0x0800UL)
#define GPIOD_BASEADDR              (IOPORT_BASEADDR + 0x0C00UL)
#define GPIOE_BASEADDR              (IOPORT_BASEADDR + 0x1000UL)
#define GPIOF_BASEADDR              (IOPORT_BASEADDR + 0x1400UL)

/* AHB peripheral base addresses */
#define DMA1_BASEADDR               (AHBPERIPH_BASE + 0x0000UL)
#define DMAMUX_BASEADDR             (AHBPERIPH_BASE + 0x0800UL)
#define RCC_BASEADDR                (AHBPERIPH_BASE + 0x1000UL)
#define EXTI_BASEADDR               (AHBPERIPH_BASE + 0x1800UL)
#define FLASH_REG_BASEADDR          (AHBPERIPH_BASE + 0x2000UL)
#define CRC_BASEADDR                (AHBPERIPH_BASE + 0x3000UL)
#define RNG_BASEADDR                (AHBPERIPH_BASE + 0x5000UL)
#define AES_BASEADDR                (AHBPERIPH_BASE + 0x6000UL)

/* APB1 peripherals */
#define TIM2_BASEADDR               (APB1PERIPH_BASE + 0x0000UL)
#define TIM3_BASEADDR               (APB1PERIPH_BASE + 0x0400UL)
#define TIM4_BASEADDR               (APB1PERIPH_BASE + 0x0800UL)
#define TIM6_BASEADDR               (APB1PERIPH_BASE + 0x1000UL)
#define TIM7_BASEADDR               (APB1PERIPH_BASE + 0x1400UL)
#define TIM14_BASEADDR              (APB1PERIPH_BASE + 0x2000UL)

#define RTCAPB_BASEADDR             (APB1PERIPH_BASE + 0x2800UL)
#define WWDG_BASEADDR               (APB1PERIPH_BASE + 0x2C00UL)
#define IWDG_BASEADDR               (APB1PERIPH_BASE + 0x3000UL)

#define SPI2_BASEADDR               (APB1PERIPH_BASE + 0x3800UL)
#define SPI3_BASEADDR               (APB1PERIPH_BASE + 0x3C00UL)

#define USART2_BASEADDR             (APB1PERIPH_BASE + 0x4400UL)
#define USART3_BASEADDR             (APB1PERIPH_BASE + 0x4800UL)
#define USART4_BASEADDR             (APB1PERIPH_BASE + 0x4C00UL)
#define USART5_BASEADDR             (APB1PERIPH_BASE + 0x5000UL)

#define I2C1_BASEADDR               (APB1PERIPH_BASE + 0x5400UL)
#define I2C2_BASEADDR               (APB1PERIPH_BASE + 0x5800UL)

#define USB_BASEADDR                (APB1PERIPH_BASE + 0x5C00UL)
#define CRS_BASEADDR                (APB1PERIPH_BASE + 0x6000UL)

#define FDCAN1_BASEADDR             (APB1PERIPH_BASE + 0x6400UL)
#define FDCAN2_BASEADDR             (APB1PERIPH_BASE + 0x6800UL)
#define FDCAN_BASEADDR              FDCAN1_BASEADDR

#define PWR_BASEADDR                (APB1PERIPH_BASE + 0x7000UL)
#define DAC1_BASEADDR               (APB1PERIPH_BASE + 0x7400UL)
#define DAC_BASEADDR                DAC1_BASEADDR

#define CEC_BASEADDR                (APB1PERIPH_BASE + 0x7800UL)
#define LPTIM1_BASEADDR             (APB1PERIPH_BASE + 0x7C00UL)

#define LPUART2_BASEADDR            (APB1PERIPH_BASE + 0x8400UL)
#define I2C3_BASEADDR               (APB1PERIPH_BASE + 0x8800UL)
#define LPTIM2_BASEADDR             (APB1PERIPH_BASE + 0x9400UL)

#define UCPD1_BASEADDR              (APB1PERIPH_BASE + 0xA000UL)
#define UCPD2_BASEADDR              (APB1PERIPH_BASE + 0xA400UL)
#define TAMP_BASEADDR               (APB1PERIPH_BASE + 0xB000UL)

/* APB2 peripherals */
#define SYSCFG_BASEADDR             (APB2PERIPH_BASE + 0x0000UL)
#define COMP_BASEADDR               (APB2PERIPH_BASE + 0x0200UL)
#define ADC_BASEADDR                (APB2PERIPH_BASE + 0x2400UL)
#define TIM1_BASEADDR               (APB2PERIPH_BASE + 0x2C00UL)
#define SPI1_BASEADDR               (APB2PERIPH_BASE + 0x3000UL)
#define USART1_BASEADDR             (APB2PERIPH_BASE + 0x3800UL)
#define USART6_BASEADDR             (APB2PERIPH_BASE + 0x3C00UL)
#define TIM15_BASEADDR              (APB2PERIPH_BASE + 0x4000UL)
#define TIM16_BASEADDR              (APB2PERIPH_BASE + 0x4400UL)
#define TIM17_BASEADDR              (APB2PERIPH_BASE + 0x4800UL)

#define SYSTICK_BASEADDR            0xE000E010UL
#define SysTick                     ((SysTick_RegDef_t *)SYSTICK_BASEADDR)

/* ------------------------------------------------------------
 * Peripheral register definition structures
 * ------------------------------------------------------------ */

typedef struct
{
    __vo uint32_t MODER;      /* GPIO port mode register,                         offset 0x00 */
    __vo uint32_t OTYPER;     /* GPIO port output type register,                  offset 0x04 */
    __vo uint32_t OSPEEDR;    /* GPIO port output speed register,                 offset 0x08 */
    __vo uint32_t PUPDR;      /* GPIO port pull-up/pull-down register,            offset 0x0C */
    __vo uint32_t IDR;        /* GPIO port input data register,                   offset 0x10 */
    __vo uint32_t ODR;        /* GPIO port output data register,                  offset 0x14 */
    __vo uint32_t BSRR;       /* GPIO port bit set/reset register,                offset 0x18 */
    __vo uint32_t LCKR;       /* GPIO port configuration lock register,           offset 0x1C */
    __vo uint32_t AFR[2];     /* AFR[0] = AFRL offset 0x20, AFR[1] = AFRH 0x24 */
    __vo uint32_t BRR;        /* GPIO port bit reset register,                    offset 0x28 */
} GPIO_RegDef_t;

typedef struct
{
    __vo uint32_t CR;         /* RCC clock control register,                       offset 0x00 */
    __vo uint32_t ICSCR;      /* RCC internal clock sources calibration register,  offset 0x04 */
    __vo uint32_t CFGR;       /* RCC clock configuration register,                 offset 0x08 */
    __vo uint32_t PLLCFGR;    /* RCC PLL configuration register,                   offset 0x0C */
    uint32_t      RESERVED0;  /* Reserved,                                         offset 0x10 */
    __vo uint32_t CRRCR;      /* RCC clock recovery RC register,                   offset 0x14 */
    __vo uint32_t CIER;       /* RCC clock interrupt enable register,              offset 0x18 */
    __vo uint32_t CIFR;       /* RCC clock interrupt flag register,                offset 0x1C */
    __vo uint32_t CICR;       /* RCC clock interrupt clear register,               offset 0x20 */
    __vo uint32_t IOPRSTR;    /* RCC I/O port reset register,                      offset 0x24 */
    __vo uint32_t AHBRSTR;    /* RCC AHB peripheral reset register,                offset 0x28 */
    __vo uint32_t APBRSTR1;   /* RCC APB peripheral reset register 1,              offset 0x2C */
    __vo uint32_t APBRSTR2;   /* RCC APB peripheral reset register 2,              offset 0x30 */
    __vo uint32_t IOPENR;     /* RCC I/O port clock enable register,               offset 0x34 */
    __vo uint32_t AHBENR;     /* RCC AHB peripheral clock enable register,         offset 0x38 */
    __vo uint32_t APBENR1;    /* RCC APB peripheral clock enable register 1,       offset 0x3C */
    __vo uint32_t APBENR2;    /* RCC APB peripheral clock enable register 2,       offset 0x40 */
    __vo uint32_t IOPSMENR;   /* RCC I/O port clock enable in Sleep mode register, offset 0x44 */
    __vo uint32_t AHBSMENR;   /* RCC AHB clock enable in Sleep/Stop mode register, offset 0x48 */
    __vo uint32_t APBSMENR1;  /* RCC APB clock enable in Sleep/Stop mode register1,offset 0x4C */
    __vo uint32_t APBSMENR2;  /* RCC APB clock enable in Sleep/Stop mode register2,offset 0x50 */
    __vo uint32_t CCIPR;      /* RCC peripherals independent clock config register,offset 0x54 */
    __vo uint32_t CCIPR2;     /* RCC peripherals independent clock config register2,offset 0x58 */
    __vo uint32_t BDCR;       /* RCC RTC domain control register,                  offset 0x5C */
    __vo uint32_t CSR;        /* RCC control/status register,                      offset 0x60 */
} RCC_RegDef_t;

typedef struct
{
    __vo uint32_t RTSR1;      /* Rising trigger selection register 1,      offset 0x00 */
    __vo uint32_t FTSR1;      /* Falling trigger selection register 1,     offset 0x04 */
    __vo uint32_t SWIER1;     /* Software interrupt event register 1,      offset 0x08 */
    __vo uint32_t RPR1;       /* Rising pending register 1,                offset 0x0C */
    __vo uint32_t FPR1;       /* Falling pending register 1,               offset 0x10 */

    uint32_t RESERVED0[5];    /* Reserved area, offsets 0x14 - 0x24 */

    __vo uint32_t RTSR2;      /* Rising trigger selection register 2,      offset 0x28 */
    __vo uint32_t FTSR2;      /* Falling trigger selection register 2,     offset 0x2C */
    __vo uint32_t SWIER2;     /* Software interrupt event register 2,      offset 0x30 */
    __vo uint32_t RPR2;       /* Rising pending register 2,                offset 0x34 */
    __vo uint32_t FPR2;       /* Falling pending register 2,               offset 0x38 */

    uint32_t RESERVED1[9];    /* Reserved area, offsets 0x3C - 0x5C */

    __vo uint32_t EXTICR[4];  /* EXTI external interrupt selection registers,
                                EXTICR[0] = EXTI_EXTICR1, offset 0x60
                                EXTICR[1] = EXTI_EXTICR2, offset 0x64
                                EXTICR[2] = EXTI_EXTICR3, offset 0x68
                                EXTICR[3] = EXTI_EXTICR4, offset 0x6C */

    uint32_t RESERVED2[4];    /* Reserved area, offsets 0x70 - 0x7C */

    __vo uint32_t IMR1;       /* Interrupt mask register 1,                offset 0x80 */
    __vo uint32_t EMR1;       /* Event mask register 1,                    offset 0x84 */

    uint32_t RESERVED3[2];    /* Reserved area, offsets 0x88 - 0x8C */

    __vo uint32_t IMR2;       /* Interrupt mask register 2,                offset 0x90 */
    __vo uint32_t EMR2;       /* Event mask register 2,                    offset 0x94 */

} EXTI_RegDef_t;

typedef struct
{
    __vo uint32_t CR1;        /* Control register 1,        offset 0x00 */
    __vo uint32_t CR2;        /* Control register 2,        offset 0x04 */
    __vo uint32_t SR;         /* Status register,           offset 0x08 */
    __vo uint32_t DR;         /* Data register,             offset 0x0C */
    __vo uint32_t CRCPR;      /* CRC polynomial register,   offset 0x10 */
    __vo uint32_t RXCRCR;     /* RX CRC register,           offset 0x14 */
    __vo uint32_t TXCRCR;     /* TX CRC register,           offset 0x18 */
    __vo uint32_t I2SCFGR;    /* I2S configuration register,offset 0x1C */
    __vo uint32_t I2SPR;      /* I2S prescaler register,    offset 0x20 */
} SPI_RegDef_t;

typedef struct
{
    __vo uint32_t CR1;        /* Control register 1,              offset 0x00 */
    __vo uint32_t CR2;        /* Control register 2,              offset 0x04 */
    __vo uint32_t OAR1;       /* Own address register 1,          offset 0x08 */
    __vo uint32_t OAR2;       /* Own address register 2,          offset 0x0C */
    __vo uint32_t TIMINGR;    /* Timing register,                 offset 0x10 */
    __vo uint32_t TIMEOUTR;   /* Timeout register,                offset 0x14 */
    __vo uint32_t ISR;        /* Interrupt and status register,   offset 0x18 */
    __vo uint32_t ICR;        /* Interrupt clear register,        offset 0x1C */
    __vo uint32_t PECR;       /* PEC register,                    offset 0x20 */
    __vo uint32_t RXDR;       /* Receive data register,           offset 0x24 */
    __vo uint32_t TXDR;       /* Transmit data register,          offset 0x28 */
} I2C_RegDef_t;

typedef struct
{
    __vo uint32_t CR1;        /* Control register 1,      offset 0x00 */
    __vo uint32_t CR2;        /* Control register 2,      offset 0x04 */
    __vo uint32_t CR3;        /* Control register 3,      offset 0x08 */
    __vo uint32_t BRR;        /* Baud rate register,      offset 0x0C */
    __vo uint32_t GTPR;       /* Guard time/prescaler,    offset 0x10 */
    __vo uint32_t RTOR;       /* Receiver timeout register,offset 0x14 */
    __vo uint32_t RQR;        /* Request register,        offset 0x18 */
    __vo uint32_t ISR;        /* Interrupt/status register,offset 0x1C */
    __vo uint32_t ICR;        /* Interrupt clear register,offset 0x20 */
    __vo uint32_t RDR;        /* Receive data register,   offset 0x24 */
    __vo uint32_t TDR;        /* Transmit data register,  offset 0x28 */
    __vo uint32_t PRESC;      /* Prescaler register,      offset 0x2C */
} USART_RegDef_t;

typedef struct{
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_RegDef_t;

typedef struct
{
    __vo uint32_t ISR;        /* ADC interrupt and status register,       offset 0x00 */
    __vo uint32_t IER;        /* ADC interrupt enable register,           offset 0x04 */
    __vo uint32_t CR;         /* ADC control register,                    offset 0x08 */
    __vo uint32_t CFGR1;      /* ADC configuration register 1,            offset 0x0C */
    __vo uint32_t CFGR2;      /* ADC configuration register 2,            offset 0x10 */
    __vo uint32_t SMPR;       /* ADC sampling time register,              offset 0x14 */

    uint32_t RESERVED0[2];    /* Reserved,                                offsets 0x18-0x1C */

    __vo uint32_t TR;         /* ADC watchdog threshold register,         offset 0x20 */

    uint32_t RESERVED1;       /* Reserved,                                offset 0x24 */

    __vo uint32_t CHSELR;     /* ADC channel selection register,          offset 0x28 */

    uint32_t RESERVED2[5];    /* Reserved,                                offsets 0x2C-0x3C */

    __vo uint32_t DR;         /* ADC data register,                       offset 0x40 */

} ADC_RegDef_t;

/* ------------------------------------------------------------
 * Peripheral definitions
 * ------------------------------------------------------------ */

#define GPIOA                       ((GPIO_RegDef_t*) GPIOA_BASEADDR)
#define GPIOB                       ((GPIO_RegDef_t*) GPIOB_BASEADDR)
#define GPIOC                       ((GPIO_RegDef_t*) GPIOC_BASEADDR)
#define GPIOD                       ((GPIO_RegDef_t*) GPIOD_BASEADDR)
#define GPIOE                       ((GPIO_RegDef_t*) GPIOE_BASEADDR)
#define GPIOF                       ((GPIO_RegDef_t*) GPIOF_BASEADDR)

#define RCC                         ((RCC_RegDef_t*) RCC_BASEADDR)
#define EXTI    					((EXTI_RegDef_t*) EXTI_BASEADDR)

#define SPI1						((SPI_RegDef_t*)SPI1_BASEADDR)
#define SPI2						((SPI_RegDef_t*)SPI2_BASEADDR)
#define SPI3						((SPI_RegDef_t*)SPI3_BASEADDR)

#define I2C1    					((I2C_RegDef_t*)I2C1_BASEADDR)
#define I2C2    					((I2C_RegDef_t*)I2C2_BASEADDR)

#define USART1    					((USART_RegDef_t*)USART1_BASEADDR)
#define USART2    					((USART_RegDef_t*)USART2_BASEADDR)
#define USART3    					((USART_RegDef_t*)USART3_BASEADDR)
#define USART4    					((USART_RegDef_t*)USART4_BASEADDR)
#define USART5    					((USART_RegDef_t*)USART5_BASEADDR)
#define USART6    					((USART_RegDef_t*)USART6_BASEADDR)

#define ADC                         ((ADC_RegDef_t*) ADC_BASEADDR)

/* ------------------------------------------------------------
 * Clock enable macros
 * ------------------------------------------------------------ */

#define GPIOA_PCLK_EN()             (RCC->IOPENR |= (1U << 0))
#define GPIOB_PCLK_EN()             (RCC->IOPENR |= (1U << 1))
#define GPIOC_PCLK_EN()             (RCC->IOPENR |= (1U << 2))
#define GPIOD_PCLK_EN()             (RCC->IOPENR |= (1U << 3))
#define GPIOE_PCLK_EN()             (RCC->IOPENR |= (1U << 4))
#define GPIOF_PCLK_EN()             (RCC->IOPENR |= (1U << 5))

#define TIM2_PCLK_EN()              (RCC->APBENR1 |= (1U << 0))
#define TIM3_PCLK_EN()              (RCC->APBENR1 |= (1U << 1))
#define TIM4_PCLK_EN()              (RCC->APBENR1 |= (1U << 2))
#define TIM6_PCLK_EN()              (RCC->APBENR1 |= (1U << 4))
#define TIM7_PCLK_EN()              (RCC->APBENR1 |= (1U << 5))
#define USART5_PCLK_EN()            (RCC->APBENR1 |= (1U << 8))
#define USART6_PCLK_EN()            (RCC->APBENR1 |= (1U << 9))
#define SPI2_PCLK_EN()              (RCC->APBENR1 |= (1U << 14))
#define SPI3_PCLK_EN()              (RCC->APBENR1 |= (1U << 15))
#define USART2_PCLK_EN()            (RCC->APBENR1 |= (1U << 17))
#define USART3_PCLK_EN()            (RCC->APBENR1 |= (1U << 18))
#define USART4_PCLK_EN()            (RCC->APBENR1 |= (1U << 19))
#define LPUART1_PCLK_EN()           (RCC->APBENR1 |= (1U << 20))
#define I2C1_PCLK_EN()              (RCC->APBENR1 |= (1U << 21))
#define I2C2_PCLK_EN()              (RCC->APBENR1 |= (1U << 22))
#define I2C3_PCLK_EN()              (RCC->APBENR1 |= (1U << 23))
#define PWR_PCLK_EN()               (RCC->APBENR1 |= (1U << 28))
#define DAC1_PCLK_EN()              (RCC->APBENR1 |= (1U << 29))

#define SYSCFG_PCLK_EN()            (RCC->APBENR2 |= (1U << 0))
#define TIM1_PCLK_EN()              (RCC->APBENR2 |= (1U << 11))
#define SPI1_PCLK_EN()              (RCC->APBENR2 |= (1U << 12))
#define USART1_PCLK_EN()            (RCC->APBENR2 |= (1U << 14))
#define TIM14_PCLK_EN()             (RCC->APBENR2 |= (1U << 15))
#define TIM15_PCLK_EN()             (RCC->APBENR2 |= (1U << 16))
#define TIM16_PCLK_EN()             (RCC->APBENR2 |= (1U << 17))
#define TIM17_PCLK_EN()             (RCC->APBENR2 |= (1U << 18))
#define ADC_PCLK_EN()               (RCC->APBENR2 |= (1U << 20))

/* ------------------------------------------------------------
 * Clock disable macros
 * ------------------------------------------------------------ */

#define GPIOA_PCLK_DI()             (RCC->IOPENR &= ~(1U << 0))
#define GPIOB_PCLK_DI()             (RCC->IOPENR &= ~(1U << 1))
#define GPIOC_PCLK_DI()             (RCC->IOPENR &= ~(1U << 2))
#define GPIOD_PCLK_DI()             (RCC->IOPENR &= ~(1U << 3))
#define GPIOE_PCLK_DI()             (RCC->IOPENR &= ~(1U << 4))
#define GPIOF_PCLK_DI()             (RCC->IOPENR &= ~(1U << 5))

#define SPI1_PCLK_DI()              (RCC->APBENR2 &= ~(1U << 12))
#define SPI2_PCLK_DI()              (RCC->APBENR1 &= ~(1U << 14))
#define SPI3_PCLK_DI()              (RCC->APBENR1 &= ~(1U << 15))

#define I2C1_PCLK_DI()    			(RCC->APBENR1 &= ~(1U << 21))
#define I2C2_PCLK_DI()    			(RCC->APBENR1 &= ~(1U << 22))

#define USART1_PCLK_DI()    		(RCC->APBENR2 &= ~(1U << 14))
#define USART2_PCLK_DI()    		(RCC->APBENR1 &= ~(1U << 17))
#define USART3_PCLK_DI()    		(RCC->APBENR1 &= ~(1U << 18))
#define USART4_PCLK_DI()    		(RCC->APBENR1 &= ~(1U << 19))
#define USART5_PCLK_DI()    		(RCC->APBENR1 &= ~(1U << 8))
#define USART6_PCLK_DI()    		(RCC->APBENR1 &= ~(1U << 9))

#define ADC_PCLK_DI()               (RCC->APBENR2 &= ~(1U << 20))

/* ------------------------------------------------------------
 * Peripheral reset macros
 * ------------------------------------------------------------ */

#define GPIOA_REG_RESET()		do { RCC->IOPRSTR |=  (1U << 0); RCC->IOPRSTR &= ~(1U << 0); } while(0)
#define GPIOB_REG_RESET()		do { RCC->IOPRSTR |=  (1U << 1); RCC->IOPRSTR &= ~(1U << 1); } while(0)
#define GPIOC_REG_RESET()		do { RCC->IOPRSTR |=  (1U << 2); RCC->IOPRSTR &= ~(1U << 2); } while(0)
#define GPIOD_REG_RESET()		do { RCC->IOPRSTR |=  (1U << 3); RCC->IOPRSTR &= ~(1U << 3); } while(0)
#define GPIOE_REG_RESET()		do { RCC->IOPRSTR |=  (1U << 4); RCC->IOPRSTR &= ~(1U << 4); } while(0)
#define GPIOF_REG_RESET()		do { RCC->IOPRSTR |=  (1U << 5); RCC->IOPRSTR &= ~(1U << 5); } while(0)

#define SPI1_REG_RESET()		do { RCC->APBRSTR2 |=  (1U << 12); RCC->APBRSTR2 &= ~(1U << 12); } while(0)
#define SPI2_REG_RESET()		do { RCC->APBRSTR1 |=  (1U << 14); RCC->APBRSTR1 &= ~(1U << 14); } while(0)
#define SPI3_REG_RESET()		do { RCC->APBRSTR1 |=  (1U << 15); RCC->APBRSTR1 &= ~(1U << 15); } while(0)

#define I2C1_REG_RESET()		do { RCC->APBRSTR1 |=  (1U << 21); RCC->APBRSTR1 &= ~(1U << 21); } while(0)
#define I2C2_REG_RESET()		do { RCC->APBRSTR1 |=  (1U << 22); RCC->APBRSTR1 &= ~(1U << 22); } while(0)

#define USART1_REG_RESET()		do { RCC->APBRSTR2 |= (1U << 14); RCC->APBRSTR2 &= ~(1U << 14); } while(0)
#define USART2_REG_RESET()		do { RCC->APBRSTR1 |= (1U << 17); RCC->APBRSTR1 &= ~(1U << 17); } while(0)
#define USART3_REG_RESET()		do { RCC->APBRSTR1 |= (1U << 18); RCC->APBRSTR1 &= ~(1U << 18); } while(0)
#define USART4_REG_RESET()		do { RCC->APBRSTR1 |= (1U << 19); RCC->APBRSTR1 &= ~(1U << 19); } while(0)
#define USART5_REG_RESET()		do { RCC->APBRSTR1 |= (1U << 8);  RCC->APBRSTR1 &= ~(1U << 8);  } while(0)
#define USART6_REG_RESET()		do { RCC->APBRSTR1 |= (1U << 9);  RCC->APBRSTR1 &= ~(1U << 9);  } while(0)

/* ------------------------------------------------------------
 * Returns port code for given GPIOx base address
 * ------------------------------------------------------------ */

#define GPIO_BASEADDR_TO_CODE(x)   ( \
        (x == GPIOA) ? 0 : \
        (x == GPIOB) ? 1 : \
        (x == GPIOC) ? 2 : \
        (x == GPIOD) ? 3 : \
        (x == GPIOE) ? 4 : \
        (x == GPIOF) ? 5 : 0 )

/* ------------------------------------------------------------
 * SPI bit position definitions
 * ------------------------------------------------------------ */

#define SPI_CR1_CPHA        0U
#define SPI_CR1_CPOL        1U
#define SPI_CR1_MSTR        2U
#define SPI_CR1_BR          3U
#define SPI_CR1_SPE         6U
#define SPI_CR1_LSBFIRST    7U
#define SPI_CR1_SSI         8U
#define SPI_CR1_SSM         9U
#define SPI_CR1_RXONLY      10U
#define SPI_CR1_CRCEN       13U
#define SPI_CR1_BIDIOE      14U
#define SPI_CR1_BIDIMODE    15U

#define SPI_CR2_RXDMAEN     0U
#define SPI_CR2_TXDMAEN     1U
#define SPI_CR2_SSOE        2U
#define SPI_CR2_NSSP        3U
#define SPI_CR2_FRF         4U
#define SPI_CR2_ERRIE       5U
#define SPI_CR2_RXNEIE      6U
#define SPI_CR2_TXEIE       7U
#define SPI_CR2_DS          8U
#define SPI_CR2_FRXTH       12U
#define SPI_CR2_LDMA_RX     13U
#define SPI_CR2_LDMA_TX     14U

#define SPI_SR_RXNE         0U
#define SPI_SR_TXE          1U
#define SPI_SR_CRCERR       4U
#define SPI_SR_MODF         5U
#define SPI_SR_OVR          6U
#define SPI_SR_BSY          7U
#define SPI_SR_FRE          8U
#define SPI_SR_FRLVL        9U
#define SPI_SR_FTLVL        11U

#define SPI_RXNE_FLAG		(1U << SPI_SR_RXNE)
#define SPI_TXE_FLAG		(1U << SPI_SR_TXE)
#define SPI_BSY_FLAG		(1U << SPI_SR_BSY)

/* ------------------------------------------------------------
 * I2C bit position definitions
 * ------------------------------------------------------------ */

#define I2C_CR1_PE                  0U
#define I2C_CR1_TXIE                1U
#define I2C_CR1_RXIE                2U
#define I2C_CR1_ADDRIE              3U
#define I2C_CR1_NACKIE              4U
#define I2C_CR1_STOPIE              5U
#define I2C_CR1_TCIE                6U
#define I2C_CR1_ERRIE               7U
#define I2C_CR1_DNF                 8U
#define I2C_CR1_ANFOFF              12U
#define I2C_CR1_NOSTRETCH           17U

#define I2C_CR2_SADD                0U
#define I2C_CR2_RD_WRN              10U
#define I2C_CR2_ADD10               11U
#define I2C_CR2_START               13U
#define I2C_CR2_STOP                14U
#define I2C_CR2_NACK                15U
#define I2C_CR2_NBYTES              16U
#define I2C_CR2_RELOAD              24U
#define I2C_CR2_AUTOEND             25U

#define I2C_ISR_TXE                 0U
#define I2C_ISR_TXIS                1U
#define I2C_ISR_RXNE                2U
#define I2C_ISR_ADDR                3U
#define I2C_ISR_NACKF               4U
#define I2C_ISR_STOPF               5U
#define I2C_ISR_TC                  6U
#define I2C_ISR_TCR                 7U
#define I2C_ISR_BERR                8U
#define I2C_ISR_ARLO                9U
#define I2C_ISR_OVR                 10U
#define I2C_ISR_TIMEOUT             12U
#define I2C_ISR_BUSY                15U

#define I2C_ICR_ADDRCF              3U
#define I2C_ICR_NACKCF              4U
#define I2C_ICR_STOPCF              5U
#define I2C_ICR_BERRCF              8U
#define I2C_ICR_ARLOCF              9U
#define I2C_ICR_OVRCF               10U

#define I2C_OAR1_OA1                0U
#define I2C_OAR1_OA1MODE            10U
#define I2C_OAR1_OA1EN              15U

#define I2C_TIMINGR_SCLL            0U
#define I2C_TIMINGR_SCLH            8U
#define I2C_TIMINGR_SDADEL          16U
#define I2C_TIMINGR_SCLDEL          20U
#define I2C_TIMINGR_PRESC           28U

#define I2C_TXE_FLAG                (1U << I2C_ISR_TXE)
#define I2C_TXIS_FLAG               (1U << I2C_ISR_TXIS)
#define I2C_RXNE_FLAG               (1U << I2C_ISR_RXNE)
#define I2C_ADDR_FLAG               (1U << I2C_ISR_ADDR)
#define I2C_NACKF_FLAG              (1U << I2C_ISR_NACKF)
#define I2C_STOPF_FLAG              (1U << I2C_ISR_STOPF)
#define I2C_TC_FLAG                 (1U << I2C_ISR_TC)
#define I2C_TCR_FLAG                (1U << I2C_ISR_TCR)
#define I2C_BERR_FLAG               (1U << I2C_ISR_BERR)
#define I2C_ARLO_FLAG               (1U << I2C_ISR_ARLO)
#define I2C_OVR_FLAG                (1U << I2C_ISR_OVR)
#define I2C_TIMEOUT_FLAG            (1U << I2C_ISR_TIMEOUT)
#define I2C_BUSY_FLAG               (1U << I2C_ISR_BUSY)

/* ------------------------------------------------------------
 * USART bit position definitions
 * ------------------------------------------------------------ */
#define USART_CR1_UE                0U
#define USART_CR1_RE                2U
#define USART_CR1_TE                3U
#define USART_CR1_IDLEIE            4U
#define USART_CR1_RXNEIE            5U
#define USART_CR1_TCIE              6U
#define USART_CR1_TXEIE             7U
#define USART_CR1_PS                9U
#define USART_CR1_PCE               10U
#define USART_CR1_M0                12U
#define USART_CR1_OVER8             15U
#define USART_CR1_M1                28U

#define USART_CR2_STOP              12U

#define USART_CR3_RTSE              8U
#define USART_CR3_CTSE              9U

#define USART_BRR_DIV_FRACTION      0U
#define USART_BRR_DIV_MANTISSA      4U

#define USART_ISR_PE                0U
#define USART_ISR_FE                1U
#define USART_ISR_NE                2U
#define USART_ISR_ORE               3U
#define USART_ISR_IDLE              4U
#define USART_ISR_RXNE              5U
#define USART_ISR_TC                6U
#define USART_ISR_TXE               7U

#define USART_ICR_PECF              0U
#define USART_ICR_FECF              1U
#define USART_ICR_NECF              2U
#define USART_ICR_ORECF             3U
#define USART_ICR_IDLECF            4U
#define USART_ICR_TCCF              6U

#define USART_PE_FLAG               (1U << USART_ISR_PE)
#define USART_FE_FLAG               (1U << USART_ISR_FE)
#define USART_NE_FLAG               (1U << USART_ISR_NE)
#define USART_ORE_FLAG              (1U << USART_ISR_ORE)
#define USART_IDLE_FLAG             (1U << USART_ISR_IDLE)
#define USART_RXNE_FLAG             (1U << USART_ISR_RXNE)
#define USART_TC_FLAG               (1U << USART_ISR_TC)
#define USART_TXE_FLAG              (1U << USART_ISR_TXE)

/* USART software states */
#define USART_READY                 0U
#define USART_BUSY_IN_RX            1U
#define USART_BUSY_IN_TX            2U

/* ------------------------------------------------------------
 * ADC bit position definitions
 * ------------------------------------------------------------ */

/* ADC_ISR bits */
#define ADC_ISR_ADRDY               0U
#define ADC_ISR_EOC                 2U
#define ADC_ISR_EOS                 3U
#define ADC_ISR_OVR                 4U
#define ADC_ISR_EOCAL               11U
#define ADC_ISR_CCRDY               13U

/* ADC_CR bits */
#define ADC_CR_ADEN                 0U
#define ADC_CR_ADDIS                1U
#define ADC_CR_ADSTART              2U
#define ADC_CR_ADSTP                4U
#define ADC_CR_ADVREGEN             28U
#define ADC_CR_ADCAL                31U

/* ADC_CFGR1 bits */
#define ADC_CFGR1_RES               3U
#define ADC_CFGR1_ALIGN             5U
#define ADC_CFGR1_CONT              13U

/* ADC_SMPR bits */
#define ADC_SMPR_SMP1               0U

/* ADC_CHSELR bits */
#define ADC_CHSELR_CHSEL0           0U

/* ADC flags */
#define ADC_ADRDY_FLAG              (1U << ADC_ISR_ADRDY)
#define ADC_EOC_FLAG                (1U << ADC_ISR_EOC)
#define ADC_EOS_FLAG                (1U << ADC_ISR_EOS)
#define ADC_EOCAL_FLAG              (1U << ADC_ISR_EOCAL)
#define ADC_CCRDY_FLAG              (1U << ADC_ISR_CCRDY)

/* ------------------------------------------------------------
 * RCC bit position definitions
 * ------------------------------------------------------------ */

#define RCC_CFGR_SW          0U
#define RCC_CFGR_SWS         3U
#define RCC_CFGR_HPRE        8U
#define RCC_CFGR_PPRE        12U

#endif /* INC_STM32G071XX_H_ */
