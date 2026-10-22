#include "bldc.h"
#include "tim.h"
#include "hall.h"

/**
这里面要写的内容：
1，获取霍尔状态
2，根据霍尔状态选择打开某个通道
3，写六个通道的函数
特别注意，这里面就按照wvu走 顺序就是123，wvu

**/
uint8_t current_state;
uint8_t last_state;
uint8_t dir=1;
uint8_t hall_fault_flag=0;


uint8_t Hallsensor_Get_State(uint8_t motor_id) 
{ 
    uint8_t State ; 
    State  = 0; 
    if(motor_id == MOTOR_1 ) 
    { 
        if(HALLW_READ()!=  GPIO_PIN_RESET)  /* 霍尔1传感器状态获取，也就是W相 */ 
        { 
            State |= 0x01U; 
        } 
        if(HALLV_READ()!=  GPIO_PIN_RESET)/* 霍尔2传感器状态获取，也就是V相 */ 
        { 
            State |= 0x02U; 
        } 
        if(HALLU_READ()!=  GPIO_PIN_RESET)  /* 霍尔3传感器状态获取，也就是U相 */ 
        { 
            State |= 0x04U; 
        } 
    } 
    return State; 
} 


 
void Start_Motor1(void)
{
	 
	//启动CH1和CH1N互补PWM
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
 
}

void Stop_Motor1(void)
{
	 
	  /* 关闭半桥芯片输出 */ 

    /* 关闭PWM输出 */ 
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    /* 上下桥臂全部关断 */ 
		htim1.Instance->CCR1 = 0;
		htim1.Instance->CCR2 = 0;
		htim1.Instance->CCR3 = 0;
	 
		HAL_GPIO_WritePin(M1_UL_PORT,M1_UL_PIN,GPIO_PIN_RESET); 
		HAL_GPIO_WritePin(M1_VL_PORT,M1_VL_PIN,GPIO_PIN_RESET); 
		HAL_GPIO_WritePin(M1_WL_PORT,M1_WL_PIN,GPIO_PIN_RESET); 
	 
}

 /* 六步换相控制函数:
上下桥臂的导通情况，共6种，也称为6步换向 
*/ 
void M1_UH_WL(void)
{
	htim1.Instance->CCR1 = 1050; //四分之一
	htim1.Instance->CCR2 = 0;
	htim1.Instance->CCR3 = 0;
  HAL_GPIO_WritePin(M1_UL_PORT,M1_UL_PIN,GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(M1_VL_PORT,M1_VL_PIN,GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(M1_WL_PORT,M1_WL_PIN,GPIO_PIN_SET); 
	
}
void M1_UH_VL(void)
{
	htim1.Instance->CCR1 = 1050; //四分之一
	htim1.Instance->CCR2 = 0;
	htim1.Instance->CCR3 = 0;
  HAL_GPIO_WritePin(M1_UL_PORT,M1_UL_PIN,GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(M1_WL_PORT,M1_WL_PIN,GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(M1_VL_PORT,M1_VL_PIN,GPIO_PIN_SET); 
	
}
void M1_WH_VL(void)
{
	htim1.Instance->CCR1 = 0; //四分之一
	htim1.Instance->CCR2 = 0;
	htim1.Instance->CCR3 = 1050;
  HAL_GPIO_WritePin(M1_UL_PORT,M1_UL_PIN,GPIO_PIN_RESET); 

	HAL_GPIO_WritePin(M1_WL_PORT,M1_WL_PIN,GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(M1_VL_PORT,M1_VL_PIN,GPIO_PIN_SET); 
	
}
void M1_WH_UL(void)
{
	htim1.Instance->CCR1 = 0; //四分之一
	htim1.Instance->CCR2 = 0;
	htim1.Instance->CCR3 = 1050;
   
	HAL_GPIO_WritePin(M1_VL_PORT,M1_VL_PIN,GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(M1_WL_PORT,M1_WL_PIN,GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(M1_UL_PORT,M1_UL_PIN,GPIO_PIN_SET);
	
}
void M1_VH_UL(void)
{
	htim1.Instance->CCR1 = 0; //四分之一
	htim1.Instance->CCR2 = 1050;
	htim1.Instance->CCR3 = 0;
  
	HAL_GPIO_WritePin(M1_VL_PORT,M1_VL_PIN,GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(M1_WL_PORT,M1_WL_PIN,GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(M1_UL_PORT,M1_UL_PIN,GPIO_PIN_SET); 
	
}
void M1_VH_WL(void)
{
	htim1.Instance->CCR1 = 0; //四分之一
	htim1.Instance->CCR2 = 1050;
	htim1.Instance->CCR3 = 0;
  HAL_GPIO_WritePin(M1_UL_PORT,M1_UL_PIN,GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(M1_VL_PORT,M1_VL_PIN,GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(M1_WL_PORT,M1_WL_PIN,GPIO_PIN_SET); 
	
}


uint8_t BLDC_Process(void)
{
    current_state = Hallsensor_Get_State(1);

    printf("hall_state:%d\r\n", current_state);

    /* 1. 无效霍尔状态：000 或 111，立即停机 */
    if ((current_state == 0) || (current_state == 7))
    {
        Stop_Motor1();
        last_state = current_state;   // 记录无效态，便于下次上电/恢复时重新对齐
			  hall_fault_flag =1;
        return current_state;
    }
		if(hall_fault_flag==1)
		{
			return current_state;   // 锁定期间不换相
		}

    /* 2. 首次运行（last_state 未初始化）或上次是无效态：只记录，不换相 */
    if ((last_state == 0) || (last_state == 7))
    {
        last_state = current_state;
        return current_state;
    }

    /* 3. 判断本次跳变是否合法 */
    uint8_t step_ok = 0;

    if (dir == 1)   /* 正转：4→6→2→3→1→5→4 */
    {
        if ((last_state == 4 && current_state == 6) ||
            (last_state == 6 && current_state == 2) ||
            (last_state == 2 && current_state == 3) ||
            (last_state == 3 && current_state == 1) ||
            (last_state == 1 && current_state == 5) ||
            (last_state == 5 && current_state == 4))
        {
            step_ok = 1;
        }
    }
    else            /* 反转：4→5→1→3→2→6→4 */
    {
        if ((last_state == 4 && current_state == 5) ||
            (last_state == 5 && current_state == 1) ||
            (last_state == 1 && current_state == 3) ||
            (last_state == 3 && current_state == 2) ||
            (last_state == 2 && current_state == 6) ||
            (last_state == 6 && current_state == 4))
        {
            step_ok = 1;
        }
    }

    /* 4. 跳变不合法：可能是丢步、抖动、干扰，停机保护 */
    if (!step_ok)
    {
        Stop_Motor1();
        last_state = current_state;
        return current_state;
    }

    /* 5. 跳变合法，执行换相 */
    switch (current_state)
    {
        case 4:   // WVU = 001
            M1_WH_UL();   // W+ U-
            break;

        case 5:   // WVU = 101
            M1_WH_VL();   // W+ V-
            break;

        case 1:   // WVU = 100
            M1_UH_VL();   // U+ V-
            break;

        case 3:   // WVU = 110
            M1_UH_WL();   // U+ W-
            break;

        case 2:   // WVU = 010
            M1_VH_WL();   // V+ W-
            break;

        case 6:   // WVU = 011
            M1_VH_UL();   // V+ U-
            break;

        default:
            Stop_Motor1();
            break;
    }

    last_state = current_state;
    return current_state;
}




void BLDC_Init(void)
{
	htim1.Instance->CCR1 = 0;
	htim1.Instance->CCR2 = 0;
	htim1.Instance->CCR3 = 0;
	
  Start_Motor1();
}



