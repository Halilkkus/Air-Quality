/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"                /* STANDART KUTUPHANELER  */
#include "stdlib.h"
#include "string.h"
#include "fonts.h"                     /* yazı tipi                  */
#include "ssd1306.h"                   /* PM 2.5 PM 10 sensör kütüp  */
#include "ens160.h"                    /* VOC ve CO2 sensör kütüp    */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;      //ADC1 donanım birimini tanımlar

I2C_HandleTypeDef hi2c1;      //I2C1 donanım birimini tanımlar

TIM_HandleTypeDef htim1;      //Timer 1 donanım birimi tanımlar
TIM_HandleTypeDef htim6;      //Timer 1 donanım birimi tanımlar

UART_HandleTypeDef huart1;    //UART1 donanım birimi tanımlar
UART_HandleTypeDef huart2;    //UART2 donanım birimi tanımlar

/* USER CODE BEGIN PV */

uint8_t data[50];           // 50 eleman ve 8 -bit veri
uint16_t size = 0;          // 2-bit size degislen baslangıc deger 0
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);        // SystemClock fonksiyonu tanımlar
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);          /* Yapılandırmak ve başlatma fonk. */
static void MX_ADC1_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

#define DHT22_PORT GPIOE               /* Baglantıları tanımlama */
#define DHT22_PIN GPIO_PIN_10          /* Baglantıları tanımlama */

uint8_t hum1, hum2, tempC1, tempC2, SUM, CHECK;   // degiskenler tanımlanır, her degisken 8-bit veri turu
uint32_t pMillis, cMillis;                        // 32-bit veri türünde önceki zaman ve şuan ki zaman tanımlanır
float temp_Celsius = 0;                          // Sıcaklık tanımlanır Celcius ile
float temp_Fahrenheit = 0;                       // Sıcaklık tanımlanır Fahrenheit ile
float Humidity = 0;                               // Nem tanımlanır
uint8_t hum_integral, hum_decimal, tempC_integral, tempC_decimal,
		tempF_integral, tempF_decimal;                 // Değerlerin tam ve ondalık sayıları alınıcak şekilde tanımlanır
char string[15];

void microDelay(uint16_t delay) {
	__HAL_TIM_SET_COUNTER(&htim1, 0);              //TIM1 periferinin sayacını sıfırlar
	while (__HAL_TIM_GET_COUNTER(&htim1) < delay)  //Belirtilen gecikme süresi
		;
}

uint8_t DHT22_Start(void) {                 // DHT22 okuma başlatma
	uint8_t Response = 0;                   // kontrol 0 başarılı 1 başarısız
	GPIO_InitTypeDef GPIO_InitStructPrivate = { 0 };       //GPIO pinlerinin konfigürasyon ayarları belirleme
	GPIO_InitStructPrivate.Pin = DHT22_PIN;                //GPIO pinine DHT22_PIN değerini atar
	GPIO_InitStructPrivate.Mode = GPIO_MODE_OUTPUT_PP;     //GPIO pininin çıkış modunda çalışması olarak tanımlama
	GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;    //GPIO pininin düşük hızda çalışacağı belirtilir
	GPIO_InitStructPrivate.Pull = GPIO_NOPULL;             //pinin çekme direnci olmadığını ifade eder
	HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStructPrivate);    //HAL kütüphanesi pin işlemlerini gerçekleştirir  (çekme, hız vb)
	HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, 0);           //GPIO pini üzerinde 0  çıkış sinyali verir
	microDelay(1300);                                      //1.3 saniye gecikme
	HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, 1);           //GPIO pini üzerinde 1 çıkış sinyali verir
	microDelay(30);                                        //30 milisaniye gecikme
	GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;         //GPIO pininin giriş modunda çaılşması olarak tanımlama
	GPIO_InitStructPrivate.Pull = GPIO_PULLUP;             //yüksek seviyede bir cekme direnci olusturur
	HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStructPrivate);    //HAL kütüphanesi pin işlemlerini gerçekleştirir (çekme, hız vb)
	microDelay(40);                                        //40 milisaniye gecikme
	if (!(HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN))) {   // Eğer pin düşük seviyede (0) ise başlangıç işaretinin alındığı anlamına gelir ve içine girer
		microDelay(80);                                   //80 milisaniye gecikme
		if ((HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN)))   /*Pin yüksek seviyede (1) ise, veri alımı devam ediyor demektir
			Response = 1;                                   ve Response değişkeni 1'e atanır */
	}
	pMillis = HAL_GetTick();                               //(milisaniye cinsinden) değerini atar
	cMillis = HAL_GetTick();
	while ((HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN)) && pMillis + 2 > cMillis) {  //DHT22_PIN'in yüksek seviyede (1) olduğunu ve süre sınırlamasının henüz geçmediğini kontrol eder. Bu süre sınırlaması pMillis + 2 değerine göre belirlenir
		cMillis = HAL_GetTick();          //her döngüde zamanı günceller
	}
	return Response; //DHT22 sensöründen yanıt alınınca 1, alınamazsa 0 değerini döndürür
}

uint8_t DHT22_Read(void) {           /*x ve y adında iki uint8_t türünde değişken tanımlar */
	uint8_t x, y;    //x değişkeni, bitlerin sırasını temsil etmek için kullanılırken, y değişkeni okunan veriyi tutmak için kullanılır.
	for (x = 0; x < 8; x++) {    // DHT22 sensöründen 8 bit veri okumak için kullanılır. x değeri 0'dan başlayarak 7'ye kadar artırır
		pMillis = HAL_GetTick();            //(milisaniye cinsinden) değerini atar
		cMillis = HAL_GetTick();
		while (!(HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN))   //veri okumak için pinin düşük seviyede (0) olmasını bekler
				&& pMillis + 2 > cMillis) {   //süre sınırlaması pMillis + 2
			cMillis = HAL_GetTick();
		}
		microDelay(40);         //40 milisaniye gecikme
		if (!(HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN)))
			y &= ~(1 << (7 - x));   //pinin (0) olup olmadığını kontrol eder, 0 ise y bitini temizler
		else
			y |= (1 << (7 - x));  //pin (1) ise, y değişkeninin ilgili bitini ayarlar 1 yapar
		pMillis = HAL_GetTick();    //mevcut zamanı (cMillis) ve son zaman damgasını (pMillis) günceller.
		cMillis = HAL_GetTick();
		while ((HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN)) //(1) olduğunu ve süre sınırlamasının henüz geçmediğini kontrol eder
				&& pMillis + 2 > cMillis) {  // Pinin 0 a düşmesini bekler
			cMillis = HAL_GetTick();
		}
	}
	return y;  //y değişkenini döndürerek DHT22 sensöründen okunan 8 bit veriyi döndürür
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
	SystemClock_Config();     //clock ları tekrardan aktifleştiriyor

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();        //GPIO ve clock ları aktifleştiriliyor
	MX_I2C1_Init();
	MX_ADC1_Init();
	MX_TIM6_Init();
	MX_TIM1_Init();
	MX_USART2_UART_Init();     // usart1 modülü aktifleştiriliyor
	MX_USART1_UART_Init();     // usart2 modülü aktifleştiriliyor
	/* USER CODE BEGIN 2 */

	HAL_TIM_Base_Start(&htim1);    //zamanlama işlemlerini başlar

	SSD1306_Init();  // başlatma
	ENSE160_Init();  // başlatma

	// bazı dizileri yazdıralım

	SSD1306_GotoXY(1, 5);                      //başlangıç koordinatları
	SSD1306_Puts("    AIR ", &Font_11x18, 1);  // yazı tipi ve renkle ekrana yazar
	SSD1306_GotoXY(2, 30);                     //başlangıç koordinatları
	SSD1306_Puts("QUALITY :)", &Font_11x18, 1);  // yazı tipi ve renkle ekrana yazar
	SSD1306_UpdateScreen(); //display

	HAL_Delay(1000);   //1 saniye gecikme

	SSD1306_ScrollRight(0, 7);  //ekranda 7 piksel sağa kaydırma
	HAL_Delay(1000);  // 1 saniye gecikme

	SSD1306_ScrollLeft(0, 7);  //ekranda 7 piksel sola kaydırma
	HAL_Delay(1000);  // 1 saniye gecikme

	SSD1306_Stopscroll();   //ekranda kaydırma durdurma
	SSD1306_Clear();        // ekranı temizler
	float temp, hum = 0;    //sıcaklık nem değerlerini tutar 0 dan başlar
	uint16_t pm25 = 0;      //PM2.5 partikül madde ölçümünü tutar
	uint16_t pm10 = 0;      //PM10 partikül madde ölçümünü tutar
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		if (DHT22_Start()) {    // Sensörden gelen değerleri başlatır
			hum1 = DHT22_Read();  //DHT22 sensöründen bir byte nem verisi okur ve hum1 değişkenine atar
			hum2 = DHT22_Read();  //DHT22 sensöründen bir byte nem verisi okur ve hum2 değişkenine atar
			tempC1 = DHT22_Read();  //DHT22 sensöründen bir byte sıcaklık verisi okur ve tempC1 değişkenine atar
			tempC2 = DHT22_Read();  //DHT22 sensöründen bir byte sıcaklık verisi okur ve tempC2 değişkenine atar
			SUM = DHT22_Read();  //DHT22 sensöründen bir byte daha veri okur ve SUM değişkenine atar, bütünlügü kontrol ermek icin
			CHECK = hum1 + hum2 + tempC1 + tempC2;  //Okunan verilerin toplamını hesaplar ve CHECK değişkenine atar
			if (CHECK == SUM) {  //değerler doğru geliyor ise
				if (tempC1 > 127) {    //sıcaklık 127 den kücük ise
					temp_Celsius = (float) tempC2 / 10 * (-1);  //n değerleri hesaplar
				} else {
					temp_Celsius = (float) ((tempC1 << 8) | tempC2) / 10;  //p değerleri hesaplar
				}

				temp_Fahrenheit = temp_Celsius * 9 / 5 + 32;   //Sıcaklık değerini Fahrenheit'a dönüştürür

				Humidity = (float) ((hum1 << 8) | hum2) / 10;  //nem değerini hesaplar ve Humidity değişkenine atar

				SSD1306_GotoXY(4, 3); //metnin başlangıç koordinatlarını belirler
				hum_integral = Humidity;  //degisken atama
				hum_decimal = Humidity * 10 - hum_integral * 10;  // nem değerlerinin 10dalık kısmı hesaplar
				sprintf(string, "%d.%d%%RH |", hum_integral, hum_decimal);  // nem değerini yazar
				SSD1306_Puts(string, &Font_7x10, 1);     // değeri ekrana yazdırır

				SSD1306_GotoXY(68, 3);  //başlangıç koordinatlarını belirler
				if (temp_Celsius < 0) {  // sıcaklık 0'dan küçük ise
					tempC_integral = temp_Celsius * (-1); //negatif işaret verir
					tempC_decimal = temp_Celsius * (-10) - tempC_integral * 10;  //ondalık kısmı hesaplar
					sprintf(string, "-%d.%d'C ", tempC_integral,
							tempC_decimal);    //sıcaklıgı yazdırır
				} else {
					tempC_integral = temp_Celsius;
					tempC_decimal = temp_Celsius * 10 - tempC_integral * 10; //ondalık kısmı hesaplar
					sprintf(string, "%d.%d'C ", tempC_integral,
							tempC_decimal);        // degerleri yazdırır
				}
				SSD1306_Puts(string, &Font_7x10, 1);  // ekrana degerleri yazdırır

				uint16_t aqi = getAQI();   // AQI (Air Quality Index) değerini aqi değişkenine alır
				HAL_Delay(100);            //100 milisaniye gecikme
				uint16_t voc = getTVOC();  //TVOC (Total Volatile Organic Compounds) değerini voc değişkenine alır
				HAL_Delay(100);            //100 milisaniye gecikme
				uint16_t eco = getECO2();  //CO2 (Equivalent CO2) değerini eco değişkenine alır
				HAL_Delay(100);            //100 milisaniye gecikme
				if (aqi > 0) {             //AQI değeri 0'dan büyükse
					SSD1306_GotoXY(4,14);                               //başlangıç koordinatlarını belirler
					sprintf(string, "AQI=%d TVOC=%d ", aqi, voc);       //degerleri yazdırır
					SSD1306_Puts(string, &Font_7x10, 1);                //VOC ve AQI stringi SSD1306 ekranına yazdırır
					SSD1306_GotoXY(4, 25);                              //başlangıç koordinatlarını belirler
					sprintf(string, "ECO2=%d ", eco);                   //CO2 stringi SSD1306 ekranına yazdırır
					SSD1306_Puts(string, &Font_7x10, 1);                //başlangıç koordinatlarını belirler
				}



				uint8_t MSG2[11] = { };                        //11 elemanlı boş dizi
				HAL_UART_Receive(&huart1, MSG2, 11, 500);      //UART üzerinden 11 byte'lık veri alır ve MSG2 dizisine kaydeder

				if (MSG2[1] == 170 && MSG2[2] == 192 && MSG2[10] == 171) {
					pm25 = MSG2[3];               //MSG2 dizisinin 3. elemanını pm25 değişkenine atar
					pm25 += (MSG2[4] << 8);       //MSG2 dizisinin 4. elemanını 8 bit sola kaydırır ve pm25 değişkenine ekler
					pm10 = MSG2[5];               //MSG2 dizisinin 5. elemanını pm10 değişkenine atar
					pm10 += (MSG2[6] << 8);       //MSG2 dizisinin 6. elemanını 8 bit sola kaydırır ve pm10 değişkenine ekler
					pm25 = pm25 / 10;             //sonuclar 10 a bölünür
					pm10 = pm10 / 10;             //sonuclar 10 a bölünür
				}
				uint8_t MSG[100] = { '\0' };  //100 elemanlı, tüm elemanları '\0' (null) karakteri ile başlatılmış bir dizi oluşturur
				sprintf(MSG,"AQI=%d&TEMP=%d&HUM=%d&VOC=%d&ECO=%d&PM25=%d&PM10=%d",aqi, tempC_integral, hum_integral, voc, eco, pm25,pm10);  //değerleri yazdırır
				SSD1306_GotoXY(4, 37);      //başlangıç koordinatlarını belirler
				sprintf(string, "PM2.5=%d ug/m3 ", pm25);   //pm2.5 deger atar
				SSD1306_Puts(string, &Font_7x10, 1);    //ekrana yazdırır
				SSD1306_GotoXY(4, 49);                  //başlangıç koordinatlarını belirler
				sprintf(string, "PM10 =%d ug/m3 ", pm10);   //pm10 deger atar
				SSD1306_Puts(string, &Font_7x10, 1);        //ekrana yazdırır
				HAL_UART_Transmit(&huart2, MSG, sizeof(MSG), 100);   //MSG dizisini UART üzerinden ileterek gönderir

				SSD1306_UpdateScreen();           //SSD1306 ekranını günceller

				//	HAL_UART_Transmit(&huart1, MSG2, sizeof(MSG2), 100);
			}
		}
		HAL_Delay(1000);  //1 saniye gecikme
	}
	/* USER CODE END 3 */
}

/**
 *
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {           //sistem saatini yapılandırır
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };      //RCC_OscInitTypeDef ve RCC_ClkInitTypeDef yapıları tanımlanır.
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Ana dahili regülatör çıkış voltajını yapılandırın
	*/
	__HAL_RCC_PWR_CLK_ENABLE(); //Güç yönetimi için RCC modülü etkinleştirilir.
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** RCC Osilatörlerini belirtilen parametrelere göre başlatır

	* RCC_OscInitTypeDef yapısında.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE; //
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();   //Yapılandırma işlemi başarılı değilse Error_Handler() fonksiyonu çağrılır
	}
	/** CPU, AHB ve APB veri yolu saatlerini başlatır
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();   //Yapılandırma işlemi başarılı değilse Error_Handler() fonksiyonu çağrılır
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */
	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_7;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 400000;             //iletişim hızını belirler ve burada 400000 olarak ayarlanır (400 kHz)
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;     //%50 görev döngüsü oranı
	hi2c1.Init.OwnAddress1 = 0;                 //kendi adresini kullanmayacak
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;          //7 bitlik adresleme modu (I2C_ADDRESSINGMODE_7BIT) kullanılır
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;         //çift adresleme modunu devredışı
	hi2c1.Init.OwnAddress2 = 0;                                   // ikinci adres kullanılmayacak
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;         //genel çağrı modu devredışı
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;             //saat gerilimi gerginlik modu devredışı
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {                    //yapılandırma başarılı değilse error_handler çağrılır
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {        //IM1 birimini yapılandırır

	/* KULLANICI KODU BEGIN TIM1_Init 0 */

	/* KULLANICI KODU END TIM1_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };  // Zamanlayıcının saat kaynağını belirtir
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* KULLANICI KODU BEGIN TIM1_Init 1 */

	/* KULLANICI KODU END TIM1_Init 1 */
	htim1.Instance = TIM1;             //htim1 yapı değişkeninin Instance üye değişkeni, TIM1 zamanlayıcısının örneğini belirtir.
	htim1.Init.Prescaler = 168;        //TIM1 zamanlayıcısının ön bölücü (prescaler) değerini 168 olarak ayarlar. Bu değer, zamanlayıcının giriş frekansını bölerek saat döngüsünün süresini ayarlar.
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP; //Bu, zamanlayıcının saymayı başlangıç değerinden artarak yapacağı anlamına gelir.
	htim1.Init.Period = 0xffff - 1;              //zamanlayıcının tam bir döngü yapması için gereken sayım değerini belirtir.
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;  //Zamanlayıcının saat bölme değerini 1 olarak ayarlar
	htim1.Init.RepetitionCounter = 0;    //Zamanlayıcının tekrarlama sayıcısını 0 olarak ayarlar
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; //Zamanlayıcının otomatik yeniden yükleme (auto-reload) ön yükleme işlemini devre dışı bırakır
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {       //hata durumunda Error_Handler() işlevi çağrılır
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;  //zamanlayıcının kendi dahili saat kaynağını kullanacağını belirtir.
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) { // //hata durumunda Error_Handler() işlevi çağrılır
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;  //Ana zamanlayıcının çıkış tetikleyici ayarını sıfırlamak
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE; //zamanlayıcıların bir ana-köle ilişkisi içinde çalışmasını engeller
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) //İşlevin dönüş değeri kontrol edilir ve hata durumunda Error_Handler() işlevi çağrılır
			!= HAL_OK) {
		Error_Handler();
	}
	/* KULLANICI KODU BEGIN TIM1_Init 2 */

	/* KULLANICI KODU END TIM1_Init 2 */

}

/**
 * @brief TIM6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM6_Init(void) {

	/* KULLANICI KODU BEGIN TIM6_Init 0 */

	/* KULLANICI KODU END TIM6_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* KULLANICI KODU BEGIN TIM6_Init 1 */

	/* KULLANICI KODU END TIM6_Init 1 */
	htim6.Instance = TIM6;    //TIM6'nin kullanılacağı belirtilir.
	htim6.Init.Prescaler = 84;  //zamanlayıcının giriş frekansını bölerek saat döngüsünün süresini ayarlar
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP; //zamanlayıcının saymayı başlangıç değerinden artarak yapacağı anlamına gelir
	htim6.Init.Period = 0xffff - 1; //zamanlayıcının tam bir döngü yapması için gereken sayım değerini belirtir.
	htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; ////Zamanlayıcının otomatik yeniden yükleme (auto-reload) ön yükleme işlemini devre dışı bırakır
	if (HAL_TIM_Base_Init(&htim6) != HAL_OK) { //İşlevin dönüş değeri kontrol edilir ve hata durumunda Error_Handler() işlevi çağrılır
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;  //Ana zamanlayıcının çıkış tetikleyici ayarını sıfırlamak
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE; //zamanlayıcıların bir ana-köle ilişkisi içinde çalışmasını engeller
	if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}

	/* KULLANICI KODU BEGIN TIM6_Init 2 */

	/* KULLANICI KODU END TIM6_Init 2 */

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
	huart1.Instance = USART1;      //USART1'nin kullanılacağı belirtilir.
	huart1.Init.BaudRate = 9600;  //İletişim hızı (baud hızı) 9600 olarak ayarlanır
	huart1.Init.WordLength = UART_WORDLENGTH_8B; //Veri bit uzunluğu 8 bit olarak ayarlanır
	huart1.Init.StopBits = UART_STOPBITS_1; //her bir veri iletiminde 1 durma biti kullanıldığını gösterir
	huart1.Init.Parity = UART_PARITY_NONE; //verinin doğrulama için çiftlik bitiyle işaretlenmediğini gösterir
	huart1.Init.Mode = UART_MODE_TX_RX;    //USART1'nin hem veri alıcı (RX) hem de veri gönderici (TX) olarak çalışacağı belirtilir
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE; //Donanım akış kontrolü devre dışı bırakılır
	huart1.Init.OverSampling = UART_OVERSAMPLING_16; //Örnekleme (oversampling) modu 16 olarak ayarlanır
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* KULLANICI KODU BEGIN USART1_Init 2 */

	/* KULLANICI KODU END USART1_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;    //USART2'nin kullanılacağı belirtilir.
	huart2.Init.BaudRate = 115200;     //İletişim hızı (baud hızı) 115200 olarak ayarlanır
	huart2.Init.WordLength = UART_WORDLENGTH_8B;  //Veri bit uzunluğu 8 bit olarak ayarlanır
	huart2.Init.StopBits = UART_STOPBITS_1;       //her bir veri iletiminde 1 durma biti kullanıldığını gösterir
	huart2.Init.Parity = UART_PARITY_NONE;         //verinin doğrulama için çiftlik bitiyle işaretlenmediğini gösterir
	huart2.Init.Mode = UART_MODE_TX_RX;             //USART2'nin hem veri alıcı (RX) hem de veri gönderici (TX) olarak çalışacağı belirtilir
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;     //Donanım akış kontrolü devre dışı bırakılır
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;  //Örnekleme (oversampling) modu 16 olarak ayarlanır
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* KULLANICI KODU BEGIN USART2_Init 2 */

	/* KULLANICI KODU END USART2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };     //GPIO yapılandırması için oluşturulur

	/* GPIO Bağlantı Noktaları Saati Etkinleştir */
	__HAL_RCC_GPIOC_CLK_ENABLE();    //GPIOC portunun saat sinyali etkinleştirilir
	__HAL_RCC_GPIOH_CLK_ENABLE();    //GPIOH portunun saat sinyali etkinleştirilir
	__HAL_RCC_GPIOA_CLK_ENABLE();    //GPIOA portunun saat sinyali etkinleştirilir
	__HAL_RCC_GPIOE_CLK_ENABLE();    //GPIOE portunun saat sinyali etkinleştirilir
	__HAL_RCC_GPIOD_CLK_ENABLE();    //GPIOD portunun saat sinyali etkinleştirilir
	__HAL_RCC_GPIOB_CLK_ENABLE();    //GPIOB portunun saat sinyali etkinleştirilir

	/*GPIO pin Çıkış Düzeyini Yapılandır */
	HAL_GPIO_WritePin(DHT22_GPIO_Port, DHT22_Pin, GPIO_PIN_RESET); //DHT22_GPIO_Port üzerindeki DHT22_Pin'i GPIO_PIN_RESET seviyesine çeker

	/*GPIO pin Çıkış Düzeyini Yapılandır */
	HAL_GPIO_WritePin(GPIOD, LD4_Pin | LD3_Pin | LD5_Pin | LD6_Pin, // RESET seviyesine çeker
			GPIO_PIN_RESET);

	/*GPIO pinini yapılandır: B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING; //B1 pininin yükselen kenar olayına tepki vereceğini gösterir
	GPIO_InitStruct.Pull = GPIO_NOPULL; //B1 pininin harici bir direnç kullanmadan çalışacağını gösterir
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct); //yapılandırma ayarlarını kullanarak gerçekleştirilir

	/*GPIO pinini yapılandır: DHT22_Pin */
	GPIO_InitStruct.Pin = DHT22_Pin;  //DHT22 pininin yapılandırılacağını gösterir
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; //DHT22 pininin açık drenaj (open-drain) çıkış modunda çalışacağını gösterir
	GPIO_InitStruct.Pull = GPIO_NOPULL;  //harici direnc kullanmayacak
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; //DHT22 pininin düşük frekanslı bir çıkış hızıyla çalışacak
	HAL_GPIO_Init(DHT22_GPIO_Port, &GPIO_InitStruct);

	/*GPIO pinlerini yapılandır: LD4_Pin LD3_Pin LD5_Pin LD6_Pin */
	GPIO_InitStruct.Pin = LD4_Pin | LD3_Pin | LD5_Pin | LD6_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //LD4, LD3, LD5 ve LD6 pinlerinin push-pull çıkış modunda çalışacak
	GPIO_InitStruct.Pull = GPIO_NOPULL;   //harici direnc kullanmayacak
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; //LD4, LD3, LD5 ve LD6 pinlerinin düşük frekanslı bir çıkış hızıyla çalışacak
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* KULLANICI KODU 4 BAŞLANGIÇ */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
}
/* KULLANICI KODU 4 SONU */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* KULLANICI KODU BEGIN Error_Handler_Debug */
	/* Kullanıcı, HAL hatası dönüş durumunu bildirmek için kendi uygulamasını ekleyebilir */
	__disable_irq();
	while (1) {
	}
	/* KULLANICI KODU END Error_Handler_Debug */
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
