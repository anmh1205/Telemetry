#include <TELEMETRY.h>

// Telemetry struct
TELEMETRY_S telemetry;

void Update_TM(void)
{
    /*----------------------------------Get value----------------------------------*/
    double IMU_Yaw = read(IMU);             // Replace with actual IMU Yaw reading
    double Encoder_X_Loc = read(Encoder_X); // Replace with actual Encoder X location reading
    double Encoder_Y_Loc = read(Encoder_Y); // Replace with actual Encoder Y location reading

    /*----------------------------------Set value----------------------------------*/
    TM_SetDataField(&telemetry, 0, IMU.Yaw);
    TM_SetDataField(&telemetry, 1, Encoder_X_Loc);
    TM_SetDataField(&telemetry, 2, Encoder_Y_Loc);

    /*----------------------------------Add publish data command to publish data----------------------------------*/
    TM_PublishData(&telemetry);
}

/*----------------------------------Timer Handle------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /*----------------------------------10ms TIMER INTERRUPT----------------------------------*/
    if (htim->Instance == TIM17) //  example: TIM17 for 10ms
    {
        /*----------------------------------Update Telemetry----------------------------------*/
        Update_TM();
        TM_Process(&telemetry);
    }
}

int main(void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MPU Configuration--------------------------------------------------------*/
    MPU_Config();

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
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_FDCAN1_Init();
    MX_I2C1_Init();
    MX_UART4_Init();
    MX_UART5_Init();
    MX_UART7_Init();
    MX_UART8_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();
    MX_USART6_UART_Init();
    MX_TIM1_Init();
    MX_TIM4_Init();
    MX_TIM5_Init();
    MX_TIM17_Init();
    MX_TIM16_Init();
    /* USER CODE BEGIN 2 */

    /*----------------------------------INIT TELEMETRY----------------------------------*/
    TM_Init(&telemetry, &huart2);
    TM_SetNodeName(&telemetry, "Swerve_Robot");

    /*----------------------------------ID ASSIGN---------------------------------------*/
    TM_SetIdAssign(&telemetry, 0, "imu");
    TM_SetIdAssign(&telemetry, 1, "encoder_x");
    TM_SetIdAssign(&telemetry, 2, "encoder_y");
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}