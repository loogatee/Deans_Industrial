#ifndef INCLUDE_GPIO_CDC3_H_
#define INCLUDE_GPIO_CDC3_H_

/**
 * Copyright 2019 john reed
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not  use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations  under the License.
 */

#include "stm32f4xx_gpio.h"

/* Private defines -----------------------------------------------------------*/
#define DI3_Pin GPIO_Pin_2
#define DI3_GPIO_Port GPIOE
#define DI4_Pin GPIO_Pin_3
#define DI4_GPIO_Port GPIOE
#define DI5_Pin GPIO_Pin_4
#define DI5_GPIO_Port GPIOE
#define DI6_Pin GPIO_Pin_5
#define DI6_GPIO_Port GPIOE
#define DI7_Pin GPIO_Pin_6
#define DI7_GPIO_Port GPIOE
#define SYS_BYPASS_Pin GPIO_Pin_13
#define SYS_BYPASS_GPIO_Port GPIOC
#define _24VACIN_1_1_Pin GPIO_Pin_0
#define _24VACIN_1_1_GPIO_Port GPIOC
#define _24VACIN_1_2_Pin GPIO_Pin_1
#define _24VACIN_1_2_GPIO_Port GPIOC
#define _24VACIN_1_3_Pin GPIO_Pin_2
#define _24VACIN_1_3_GPIO_Port GPIOC
#define _24VACIN_1_4_Pin GPIO_Pin_3
#define _24VACIN_1_4_GPIO_Port GPIOC
#define CTRL_P_CTS_Pin GPIO_Pin_0
#define CTRL_P_CTS_GPIO_Port GPIOA
#define CTRL_P_RTS_Pin GPIO_Pin_1
#define CTRL_P_RTS_GPIO_Port GPIOA
#define CTRL_P_TX_Pin GPIO_Pin_2
#define CTRL_P_TX_GPIO_Port GPIOA
#define CTRL_P_RX_Pin GPIO_Pin_3
#define CTRL_P_RX_GPIO_Port GPIOA
#define RPUMP_ON_Pin GPIO_Pin_4
#define RPUMP_ON_GPIO_Port GPIOA
#define LCD_LED_PWM_Pin GPIO_Pin_5
#define LCD_LED_PWM_GPIO_Port GPIOA
#define SVALVE_ON_Pin GPIO_Pin_6
#define SVALVE_ON_GPIO_Port GPIOA
#define WPUMP_ON_Pin GPIO_Pin_7
#define WPUMP_ON_GPIO_Port GPIOA
#define _24VACIN_2_1_Pin GPIO_Pin_4
#define _24VACIN_2_1_GPIO_Port GPIOC
#define _24VACIN_2_2_Pin GPIO_Pin_5
#define _24VACIN_2_2_GPIO_Port GPIOC
#define MUXBIT_0_Pin GPIO_Pin_0
#define MUXBIT_0_GPIO_Port GPIOB
#define MUXBIT_1_Pin GPIO_Pin_1
#define MUXBIT_1_GPIO_Port GPIOB
#define MUXBIT_2_Pin GPIO_Pin_2
#define MUXBIT_2_GPIO_Port GPIOB
#define DI8_Pin GPIO_Pin_7
#define DI8_GPIO_Port GPIOE
#define DI9_RPC_FAULT_Pin GPIO_Pin_8
#define DI9_RPC_FAULT_GPIO_Port GPIOE
#define DO1_Pin GPIO_Pin_9
#define DO1_GPIO_Port GPIOE
#define DO2_Pin GPIO_Pin_10
#define DO2_GPIO_Port GPIOE
#define DO3_Pin GPIO_Pin_11
#define DO3_GPIO_Port GPIOE
#define DO4_Pin GPIO_Pin_12
#define DO4_GPIO_Port GPIOE
#define DO5_RPC_ON_Pin GPIO_Pin_13
#define DO5_RPC_ON_GPIO_Port GPIOE
#define DO6_RPC_PWM_Pin GPIO_Pin_14
#define DO6_RPC_PWM_GPIO_Port GPIOE
#define HB_LED_Pin GPIO_Pin_15
#define HB_LED_GPIO_Port GPIOE
#define _24VACOUT_1_2_Pin GPIO_Pin_8
#define _24VACOUT_1_2_GPIO_Port GPIOD
#define _24VACOUT_2_1_Pin GPIO_Pin_9
#define _24VACOUT_2_1_GPIO_Port GPIOD
#define _24VACOUT_2_2_Pin GPIO_Pin_10
#define _24VACOUT_2_2_GPIO_Port GPIOD
#define _24VACOUT_3_1_Pin GPIO_Pin_11
#define _24VACOUT_3_1_GPIO_Port GPIOD
#define _24VACOUT_3_2_Pin GPIO_Pin_12
#define _24VACOUT_3_2_GPIO_Port GPIOD
#define _24VACOUT_4_1_Pin GPIO_Pin_13
#define _24VACOUT_4_1_GPIO_Port GPIOD
#define _24VACOUT_4_2_Pin GPIO_Pin_14
#define _24VACOUT_4_2_GPIO_Port GPIOD
#define _24VACOUT_CC_Pin GPIO_Pin_15
#define _24VACOUT_CC_GPIO_Port GPIOD
#define _24VACIN_2_3_Pin GPIO_Pin_6
#define _24VACIN_2_3_GPIO_Port GPIOC
#define _24VACIN_2_4_Pin GPIO_Pin_7
#define _24VACIN_2_4_GPIO_Port GPIOC
#define _24VACIN_3_1_Pin GPIO_Pin_8
#define _24VACIN_3_1_GPIO_Port GPIOC
#define _24VACIN_3_2_Pin GPIO_Pin_9
#define _24VACIN_3_2_GPIO_Port GPIOC
#define PM_IRQ_Pin GPIO_Pin_8
#define PM_IRQ_GPIO_Port GPIOA
#define TX_CONSOLE_Pin GPIO_Pin_9
#define TX_CONSOLE_GPIO_Port GPIOA
#define RX_CONSOLE_Pin GPIO_Pin_10
#define RX_CONSOLE_GPIO_Port GPIOA
#define FAN2_ON_Pin GPIO_Pin_11
#define FAN2_ON_GPIO_Port GPIOA
#define FAN1_ON_Pin GPIO_Pin_12
#define FAN1_ON_GPIO_Port GPIOA
#define _24VACIN_3_3_Pin GPIO_Pin_10
#define _24VACIN_3_3_GPIO_Port GPIOC
#define _24VACIN_3_4_Pin GPIO_Pin_11
#define _24VACIN_3_4_GPIO_Port GPIOC
#define SYS_OFF_Pin GPIO_Pin_12
#define SYS_OFF_GPIO_Port GPIOC
#define STEPPER_Reset_Pin GPIO_Pin_0
#define STEPPER_Reset_GPIO_Port GPIOD
#define STEPPER_Sleep_Pin GPIO_Pin_1
#define STEPPER_Sleep_GPIO_Port GPIOD
#define STEPPER_Disable_Pin GPIO_Pin_2
#define STEPPER_Disable_GPIO_Port GPIOD
#define STEPPER_Step_Pin GPIO_Pin_3
#define STEPPER_Step_GPIO_Port GPIOD
#define STEPPER_Dir_Pin GPIO_Pin_4
#define STEPPER_Dir_GPIO_Port GPIOD
#define PM_RESET_Pin GPIO_Pin_5
#define PM_RESET_GPIO_Port GPIOD
#define CTRL_to_COMMS_Pin GPIO_Pin_6
#define CTRL_to_COMMS_GPIO_Port GPIOD
#define _24VACOUT_1_1_Pin GPIO_Pin_7
#define _24VACOUT_1_1_GPIO_Port GPIOD
#define COMMS_to_CTRL_Pin GPIO_Pin_5
#define COMMS_to_CTRL_GPIO_Port GPIOB
#define MUX1_EN_Pin GPIO_Pin_8
#define MUX1_EN_GPIO_Port GPIOB
#define MUX2_EN_Pin GPIO_Pin_9
#define MUX2_EN_GPIO_Port GPIOB
#define DI1_Pin GPIO_Pin_0
#define DI1_GPIO_Port GPIOE
#define DI2_Pin GPIO_Pin_1
#define DI2_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */


#endif /* INCLUDE_GPIO_CDC3_H_ */
