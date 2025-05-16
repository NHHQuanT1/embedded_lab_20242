* USER CODE BEGIN Header */
/**
******************************************************************************
* @file           : main.c
* @brief          : Main program body
******************************************************************************
* @attention
*
* Copyright (c) 2025 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tm_stm32f4_mfrc522.h"
#include "stdio.h"
#include "string.h"
#include "SH1106.h"
#include "fonts.h"
/* USER CODE END Includes */
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
struct Time {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t weekday;
	uint8_t day;
	uint8_t month;
	uint8_t year;
};
typedef struct {
   uint8_t cardID[5];
   struct Time timestamp;
   uint8_t isValid; // 1: hợp lệ, 0: bị từ chối
} AccessLog;
/* USER CODE END PTD */
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_CARDS 10
#define MAX_LOGS 100
#define UART_BUFFER_SIZE 256
/* USER CODE END PD */
/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */
/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c3;
SPI_HandleTypeDef hspi4;
UART_HandleTypeDef huart1;
/* USER CODE BEGIN PV */
uint8_t validCardIDs[MAX_CARDS][5] = {
		{0xE3, 0xC4, 0x8C, 0x34, 0x9F}
};
uint8_t validCardCount = 2;  // Số lượng thẻ hợp lệ ban đầu
AccessLog logs[MAX_LOGS];
uint8_t logCount = 0;
uint8_t cardPresent = 0;
uint8_t lastCardID[5] = {0};
// Buffer và biến cho UART
uint8_t uartRxBuffer[UART_BUFFER_SIZE];
uint16_t uartRxIndex = 0;
uint8_t uartRxComplete = 0;
uint8_t uartRxChar;
/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C3_Init(void);
/* USER CODE BEGIN PFP */
uint8_t bcdToDec(uint8_t bcd);
uint8_t decToBcd(uint8_t dec);
void SetDS1307(int sec, int min, int hour, int day, int month, int year, int weekday);
void GetDS1307_1(char buff[]);
void GetDS1307_2(char buff[]);
uint8_t isValidCard(uint8_t* cardID);
void addLog(uint8_t* cardID, uint8_t isValid);
void processUARTCommand(void);
void startUARTReceive(void);
void printCardID(uint8_t* cardID, char* buffer);
/* USER CODE END PFP */
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Hàm chuyển đổi giá trị BCD sang số thập phân
uint8_t bcdToDec(uint8_t bcd) {
	return ((bcd >> 4) * 10) + (bcd & 0x0F);
}
// Hàm chuyển đổi số thập phân sang giá trị BCD
uint8_t decToBcd(uint8_t dec) {
	return ((dec / 10) << 4) | (dec % 10);
}
void SetDS1307(int sec, int min, int hour, int day, int month, int year,
		int weekday) {
	struct Time Set_time;
	Set_time.sec = decToBcd(sec);   // Chuyển đổi sang BCD
	Set_time.min = decToBcd(min);   // Chuyển đổi sang BCD
	Set_time.hour = decToBcd(hour); // Chuyển đổi sang BCD
	Set_time.day = decToBcd(day);   // Chuyển đổi sang BCD
	Set_time.month = decToBcd(month); // Chuyển đổi sang BCD
	Set_time.year = decToBcd(year); // Chuyển đổi sang BCD
	Set_time.weekday = decToBcd(weekday); // Chuyển đổi sang BCD
	//write time to DS1307
	HAL_I2C_Mem_Write(&hi2c3, 0xD0, 0, 1, (uint8_t*) &Set_time, 7, 1000);
}
void GetDS1307_1(char buff[]) {
	struct Time Get_Time;
	//read time back
	HAL_I2C_Mem_Read(&hi2c3, 0xD1, 0, 1, (uint8_t*) &Get_Time, 7, 1000);
	uint8_t sec_dec = bcdToDec(Get_Time.sec);
	uint8_t min_dec = bcdToDec(Get_Time.min);
	uint8_t hour_dec = bcdToDec(Get_Time.hour);
	sprintf(buff, "%02d:%02d:%02d", hour_dec, min_dec, sec_dec);
}
void GetDS1307_2(char buff[]) {
	struct Time Get_Time;
	//read time back
	HAL_I2C_Mem_Read(&hi2c3, 0xD1, 0, 1, (uint8_t*) &Get_Time, 7, 1000);
	uint8_t weekday_dec = bcdToDec(Get_Time.weekday);
	uint8_t day_dec = bcdToDec(Get_Time.day);
	uint8_t month_dec = bcdToDec(Get_Time.month);
	uint8_t year_dec = bcdToDec(Get_Time.year);
	sprintf(buff, "%01d-%02d/%02d/%02d", weekday_dec, day_dec, month_dec,
			year_dec);
}
// Hàm lấy thời gian hiện tại
void getCurrentTime(struct Time* time) {
   HAL_I2C_Mem_Read(&hi2c3, 0xD1, 0, 1, (uint8_t*)time, 7, 1000);
}
// Hàm kiểm tra thẻ có hợp lệ không
uint8_t isValidCard(uint8_t* cardID) {
   for (int i = 0; i < validCardCount; i++) {
       if (memcmp(cardID, validCardIDs[i], 5) == 0) {
           return 1; // Thẻ hợp lệ
       }
   }
   return 0; // Thẻ không hợp lệ
}
// Hàm thêm bản ghi log
void addLog(uint8_t* cardID, uint8_t isValid) {
   if (logCount < MAX_LOGS) {
       memcpy(logs[logCount].cardID, cardID, 5);
       getCurrentTime(&logs[logCount].timestamp);
       logs[logCount].isValid = isValid;
       logCount++;
   } else {
       // Nếu log đầy, xóa log cũ nhất và thêm log mới
       for (int i = 0; i < MAX_LOGS - 1; i++) {
           logs[i] = logs[i + 1];
       }
       memcpy(logs[MAX_LOGS - 1].cardID, cardID, 5);
       getCurrentTime(&logs[MAX_LOGS - 1].timestamp);
       logs[MAX_LOGS - 1].isValid = isValid;
   }
}
// Hàm chuyển đổi ID thẻ thành chuỗi hex
void printCardID(uint8_t* cardID, char* buffer) {
   sprintf(buffer, "%02X%02X%02X%02X%02X",
           cardID[0], cardID[1], cardID[2], cardID[3], cardID[4]);
}
// Hàm xử lý lệnh UART
void processUARTCommand(void) {
   if (!uartRxComplete) return;
   char response[256] = {0};
   // Đảm bảo kết thúc chuỗi
   uartRxBuffer[uartRxIndex] = 0;
   // Chuyển đổi thành chữ hoa để dễ xử lý
   for (int i = 0; i < uartRxIndex; i++) {
       if (uartRxBuffer[i] >= 'a' && uartRxBuffer[i] <= 'z') {
           uartRxBuffer[i] = uartRxBuffer[i] - 'a' + 'A';
       }
   }
   // Xử lý lệnh ADD - thêm thẻ mới
   if (strncmp((char*)uartRxBuffer, "ADD ", 4) == 0) {
       if (validCardCount >= MAX_CARDS) {
           sprintf(response, "ERROR: Danh sach the da day\r\n");
       } else {
           // Kiểm tra độ dài ID thẻ (10 ký tự hex = 5 byte)
           if (strlen((char*)uartRxBuffer + 4) >= 10) {
               uint8_t newCard[5];
               char hexPair[3] = {0};
               // Chuyển đổi chuỗi hex thành bytes
               for (int i = 0; i < 5; i++) {
                   hexPair[0] = uartRxBuffer[4 + i*2];
                   hexPair[1] = uartRxBuffer[5 + i*2];
                   sscanf(hexPair, "%hhX", &newCard[i]);
               }
               // Kiểm tra xem thẻ đã tồn tại chưa
               uint8_t cardExists = 0;
               for (int i = 0; i < validCardCount; i++) {
                   if (memcmp(newCard, validCardIDs[i], 5) == 0) {
                       cardExists = 1;
                       break;
                   }
               }
               if (cardExists) {
                   sprintf(response, "ERROR: The da ton tai trong danh sach\r\n");
               } else {
                   memcpy(validCardIDs[validCardCount], newCard, 5);
                   validCardCount++;
                   sprintf(response, "SUCCESS: Da them the moi vao danh sach\r\n");
               }
           } else {
               sprintf(response, "ERROR: ID the khong hop le\r\n");
           }
       }
   }
   // Xử lý lệnh DELETE - xóa thẻ
   else if (strncmp((char*)uartRxBuffer, "DELETE ", 7) == 0) {
       if (strlen((char*)uartRxBuffer + 7) >= 10) {
           uint8_t cardToDelete[5];
           char hexPair[3] = {0};
           // Chuyển đổi chuỗi hex thành bytes
           for (int i = 0; i < 5; i++) {
               hexPair[0] = uartRxBuffer[7 + i*2];
               hexPair[1] = uartRxBuffer[8 + i*2];
               sscanf(hexPair, "%hhX", &cardToDelete[i]);
           }
           // Tìm và xóa thẻ
           uint8_t found = 0;
           for (int i = 0; i < validCardCount; i++) {
               if (memcmp(cardToDelete, validCardIDs[i], 5) == 0) {
                   // Dịch chuyển các phần tử còn lại
                   for (int j = i; j < validCardCount - 1; j++) {
                       memcpy(validCardIDs[j], validCardIDs[j + 1], 5);
                   }
                   validCardCount--;
                   found = 1;
                   break;
               }
           }
           if (found) {
               sprintf(response, "SUCCESS: Da xoa the khoi danh sach\r\n");
           } else {
               sprintf(response, "ERROR: Khong tim thay the trong danh sach\r\n");
           }
       } else {
           sprintf(response, "ERROR: ID the khong hop le\r\n");
       }
   }
   // Xử lý lệnh LIST - liệt kê danh sách thẻ hợp lệ
   else if (strcmp((char*)uartRxBuffer, "LIST") == 0) {
       if (validCardCount == 0) {
           sprintf(response, "Danh sach the trong\r\n");
       } else {
           char temp[50];
           sprintf(response, "Danh sach the hop le (%d the):\r\n", validCardCount);
           for (int i = 0; i < validCardCount; i++) {
               char cardIDStr[15];
               printCardID(validCardIDs[i], cardIDStr);
               sprintf(temp, "%d. %s\r\n", i + 1, cardIDStr);
               strcat(response, temp);
           }
       }
   }
   // Xử lý lệnh LOG - xem log truy cập
   else if (strcmp((char*)uartRxBuffer, "LOG") == 0) {
       if (logCount == 0) {
           sprintf(response, "Chua co log truy cap nao\r\n");
       } else {
           char temp[100];
           sprintf(response, "Log truy cap (%d ban ghi):\r\n", logCount);
           for (int i = 0; i < logCount; i++) {
               char cardIDStr[15];
               printCardID(logs[i].cardID, cardIDStr);
               struct Time t = logs[i].timestamp;
               sprintf(temp, "%d. %02d:%02d:%02d %02d/%02d/%02d - %s - %s\r\n",
                       i + 1,
                       bcdToDec(t.hour), bcdToDec(t.min), bcdToDec(t.sec),
                       bcdToDec(t.day), bcdToDec(t.month), bcdToDec(t.year),
                       cardIDStr,
                       logs[i].isValid ? "ACCEPT" : "REJECT");
               strcat(response, temp);
           }
       }
   }
   // Xử lý lệnh CLEAR - xóa log
   else if (strcmp((char*)uartRxBuffer, "CLEAR") == 0) {
       logCount = 0;
       sprintf(response, "Da xoa tat ca log\r\n");
   }
   // Xử lý lệnh HELP - hiển thị trợ giúp
   else if (strcmp((char*)uartRxBuffer, "HELP") == 0) {
       sprintf(response, "Danh sach lenh:\r\n"
                         "ADD <ID>: Them the moi vao danh sach\r\n"
                         "DELETE <ID>: Xoa the khoi danh sach\r\n"
                         "LIST: Liet ke danh sach the hop le\r\n"
                         "LOG: Xem log truy cap\r\n"
                         "CLEAR: Xoa tat ca log\r\n"
                         "HELP: Hien thi tro giup\r\n");
   }
   // Lệnh không hợp lệ
   else {
       sprintf(response, "ERROR: Lenh khong hop le. Nhap HELP de xem tro giup\r\n");
   }
   // Gửi phản hồi
   HAL_UART_Transmit(&huart1, (uint8_t*)response, strlen(response), 1000);
   // Reset buffer
   uartRxIndex = 0;
   uartRxComplete = 0;
   // Tiếp tục nhận dữ liệu
   startUARTReceive();
}
// Hàm bắt đầu nhận dữ liệu UART
void startUARTReceive(void) {
   HAL_UART_Receive_IT(&huart1, &uartRxChar, 1);
}
// Callback khi nhận được dữ liệu UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
   if (huart->Instance == USART1) {
       // Nếu nhận được ký tự xuống dòng hoặc buffer đầy
       if (uartRxChar == '\r' || uartRxChar == '\n' || uartRxIndex >= UART_BUFFER_SIZE - 1) {
           uartRxComplete = 1;
       } else {
           uartRxBuffer[uartRxIndex++] = uartRxChar;
       }
       // Tiếp tục nhận nếu chưa hoàn thành
       if (!uartRxComplete) {
           HAL_UART_Receive_IT(&huart1, &uartRxChar, 1);
       }
   }
}
/* USER CODE END 0 */
/**
* @brief  The application entry point.
* @retval int
*/
int main(void) {
	/* USER CODE BEGIN 1 */
	/* USER CODE END 1 */
	/* MCU Configuration--------------------------------------------------------*/
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	/* USER CODE BEGIN Init */
	/* USER CODE END Init */
	/* Configure the system clock */
	SystemClock_Config();
	/* USER CODE BEGIN SysInit */
	/* USER CODE END SysInit */
	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_SPI4_Init();
	MX_USART1_UART_Init();
	MX_I2C3_Init();
	/* USER CODE BEGIN 2 */
	TM_MFRC522_Init();
	SH1106_Init();
	// Hiển thị màn hình khởi động
	SH1106_Clear();
	SH1106_GotoXY(10, 0);
	SH1106_Puts("RFID Access", &Font_11x18, 1);
	SH1106_GotoXY(10, 25);
	SH1106_Puts("Control System", &Font_11x18, 1);
	SH1106_UpdateScreen();
	// Khởi tạo thời gian nếu cần
	// SetDS1307(0, 15, 15, 19, 4, 25, 7);
	// Bắt đầu nhận dữ liệu UART
	startUARTReceive();
	// Thông báo hệ thống đã sẵn sàng
	char welcomeMsg[] = "\r\n===== RFID Access Control System =====\r\n"
	                    "System ready. Enter HELP for commands.\r\n\r\n";
	HAL_UART_Transmit(&huart1, (uint8_t*)welcomeMsg, strlen(welcomeMsg), 1000);
	HAL_Delay(2000);
	// Hiển thị màn hình chính
	SH1106_Clear();
	SH1106_GotoXY(10, 0);
	SH1106_Puts("RFID System", &Font_11x18, 1);
	SH1106_UpdateScreen();
	/* USER CODE END 2 */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */
		/* USER CODE BEGIN 3 */
		uint8_t CardID[5];
		char buff[30];
		char cardIDStr[15];
		// Xử lý lệnh UART nếu có
		processUARTCommand();
		// Kiểm tra thẻ RFID
		if (TM_MFRC522_Check(CardID) == MI_OK) {
		    // Bật LED3 khi phát hiện thẻ
		    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET); // LD3
		    // Nếu là thẻ mới (không phải thẻ đang giữ)
		    if (!cardPresent || memcmp(CardID, lastCardID, 5) != 0) {
		        cardPresent = 1;
		        memcpy(lastCardID, CardID, 5);
		        // Kiểm tra thẻ có hợp lệ không
		        if (isValidCard(CardID)) {
		            // Bật LED4 nếu thẻ hợp lệ
		            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_SET); // LD4
		            // Hiển thị thông báo chào mừng
		            SH1106_Clear();
		            SH1106_GotoXY(25, 10);
		            SH1106_Puts("Welcome", &Font_11x18, 1);
		            // Hiển thị thời gian
		            GetDS1307_1(buff);
		            SH1106_GotoXY(35, 30);
		            SH1106_Puts(buff, &Font_7x10, 1);
		            // Hiển thị ngày tháng
		            GetDS1307_2(buff);
		            SH1106_GotoXY(25, 42);
		            SH1106_Puts(buff, &Font_7x10, 1);
		            // Hiển thị ID thẻ
		            printCardID(CardID, cardIDStr);
		            SH1106_GotoXY(10, 54);
		            SH1106_Puts(cardIDStr, &Font_7x10, 1);
		            SH1106_UpdateScreen();
		            // Lưu log
		            addLog(CardID, 1);
		            // Gửi thông báo qua UART
		            sprintf(buff, "ACCESS GRANTED: %s\r\n", cardIDStr);
		            HAL_UART_Transmit(&huart1, (uint8_t*)buff, strlen(buff), 1000);
		        } else {
		            // Tắt LED4 nếu thẻ không hợp lệ
		            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_RESET); // LD4
		            // Hiển thị thông báo từ chối
		            SH1106_Clear();
		            SH1106_GotoXY(20, 10);
		            SH1106_Puts("Rejected", &Font_11x18, 1);
		            // Hiển thị thời gian
		            GetDS1307_1(buff);
		            SH1106_GotoXY(35, 30);
		            SH1106_Puts(buff, &Font_7x10, 1);
		            // Hiển thị ID thẻ
		            printCardID(CardID, cardIDStr);
		            SH1106_GotoXY(10, 42);
		            SH1106_Puts(cardIDStr, &Font_7x10, 1);
		            SH1106_UpdateScreen();
		            // Lưu log
		            addLog(CardID, 0);
		            // Gửi thông báo qua UART
		            sprintf(buff, "ACCESS DENIED: %s\r\n", cardIDStr);
		            HAL_UART_Transmit(&huart1, (uint8_t*)buff, strlen(buff), 1000);
		        }
		    }
		} else {
		    // Tắt LED3 và LED4 khi không có thẻ
		    if (cardPresent) {
		        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET); // LD3
		        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_RESET); // LD4
		        cardPresent = 0;
		        // Hiển thị màn hình chờ
		        SH1106_Clear();
		        SH1106_GotoXY(10, 0);
		        SH1106_Puts("RFID System", &Font_11x18, 1);
		        // Hiển thị thời gian
		        GetDS1307_1(buff);
		        SH1106_GotoXY(35, 25);
		        SH1106_Puts(buff, &Font_7x10, 1);
		        // Hiển thị ngày tháng
		        GetDS1307_2(buff);
		        SH1106_GotoXY(25, 37);
		        SH1106_Puts(buff, &Font_7x10, 1);
		        SH1106_GotoXY(10, 52);
		        SH1106_Puts("Waiting for card...", &Font_7x10, 1);
		        SH1106_UpdateScreen();
		    }
		}
		HAL_Delay(100);
	}
}
/**
* @brief System Clock Configuration
* @retval None
*/
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 72;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 3;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}
/**
* @brief I2C3 Initialization Function
* @param None
* @retval None
*/
static void MX_I2C3_Init(void) {
	/* USER CODE BEGIN I2C3_Init 0 */
	/* USER CODE END I2C3_Init 0 */
	/* USER CODE BEGIN I2C3_Init 1 */
	/* USER CODE END I2C3_Init 1 */
	hi2c3.Instance = I2C3;
	hi2c3.Init.ClockSpeed = 400000;
	hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c3.Init.OwnAddress1 = 0;
	hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c3.Init.OwnAddress2 = 0;
	hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c3) != HAL_OK) {
		Error_Handler();
	}
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK) {
		Error_Handler();
	}
	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C3_Init 2 */
	/* USER CODE END I2C3_Init 2 */
}
/**
* @brief SPI4 Initialization Function
* @param None
* @retval None
*/
static void MX_SPI4_Init(void) {
	/* USER CODE BEGIN SPI4_Init 0 */
	/* USER CODE END SPI4_Init 0 */
	/* USER CODE BEGIN SPI4_Init 1 */
	/* USER CODE END SPI4_Init 1 */
	/* SPI4 parameter configuration*/
	hspi4.Instance = SPI4;
	hspi4.Init.Mode = SPI_MODE_MASTER;
	hspi4.Init.Direction = SPI_DIRECTION_2LINES;
	hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi4.Init.NSS = SPI_NSS_SOFT;
	hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi4.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi4) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI4_Init 2 */
	/* USER CODE END SPI4_Init 2 */
}
/**
* @brief USART1 Initialization Function
* @param None
* @retval None
*/
static void MX_USART1_UART_Init(void) {
	/* USER CODE BEGIN USART1_Init 0 */
	/* USER CODE END USART1_Init 0 */
	/* USER CODE BEGIN USART1_Init 1 */
	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */
	/* USER CODE END USART1_Init 2 */
}
/**
* @brief GPIO Initialization Function
* @param None
* @retval None
*/
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, NCS_MEMS_SPI_Pin | CSX_Pin | OTG_FS_PSO_Pin,
			GPIO_PIN_RESET);
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(ACP_RST_GPIO_Port, ACP_RST_Pin, GPIO_PIN_RESET);
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, RDX_Pin | WRX_DCX_Pin, GPIO_PIN_RESET);
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOG, LD3_Pin | LD4_Pin, GPIO_PIN_RESET);
	/*Configure GPIO pin : PE4 */
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	/*Configure GPIO pins : NCS_MEMS_SPI_Pin CSX_Pin OTG_FS_PSO_Pin */
	GPIO_InitStruct.Pin = NCS_MEMS_SPI_Pin | CSX_Pin | OTG_FS_PSO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	/*Configure GPIO pins : B1_Pin MEMS_INT1_Pin MEMS_INT2_Pin TP_INT1_Pin */
	GPIO_InitStruct.Pin = B1_Pin | MEMS_INT1_Pin | MEMS_INT2_Pin | TP_INT1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	/*Configure GPIO pin : ACP_RST_Pin */
	GPIO_InitStruct.Pin = ACP_RST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(ACP_RST_GPIO_Port, &GPIO_InitStruct);
	/*Configure GPIO pin : OTG_FS_OC_Pin */
	GPIO_InitStruct.Pin = OTG_FS_OC_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(OTG_FS_OC_GPIO_Port, &GPIO_InitStruct);
	/*Configure GPIO pin : BOOT1_Pin */
	GPIO_InitStruct.Pin = BOOT1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);
	/*Configure GPIO pin : TE_Pin */
	GPIO_InitStruct.Pin = TE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(TE_GPIO_Port, &GPIO_InitStruct);
	/*Configure GPIO pins : RDX_Pin WRX_DCX_Pin */
	GPIO_InitStruct.Pin = RDX_Pin | WRX_DCX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	/*Configure GPIO pins : LD3_Pin LD4_Pin */
	GPIO_InitStruct.Pin = LD3_Pin | LD4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}
/**
* @brief  Period elapsed callback in non blocking mode
* @note   This function is called  when TIM6 interrupt took place, inside
* HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
* a global variable "uwTick" used as application time base.
* @param  htim : TIM handle
* @retval None
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */
	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */
	/* USER CODE END Callback 1 */
}
/**
* @brief  This function is executed in case of error occurrence.
* @retval None
*/
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
 /* USER CODE BEGIN 6 */
 /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
 /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
