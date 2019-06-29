/**
 * main.c
 */
#include <DSP28x_Project.h>
#include <f2802x_common/include/clk.h>
#include <f2802x_common/include/gpio.h>
#include <f2802x_common/include/pll.h>
#include <f2802x_common/include/wdog.h>
#include <f2802x_common/include/sci.h>

#ifdef _FLASH
memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif

WDOG_Handle myWDog;
CLK_Handle myClk;
PLL_Handle myPll;
GPIO_Handle myGpio;
SCI_Handle mySci;

bool ledState = false;

void GeneralSetup();
void SciSetup();
void Loop();

void SciMsg(char *msg);

int main(void){
    GeneralSetup();
    SciSetup();

    GPIO_setMode(myGpio, GPIO_Number_0, GPIO_0_Mode_GeneralPurpose);
    GPIO_setMode(myGpio, GPIO_Number_1, GPIO_1_Mode_GeneralPurpose);

    GPIO_setDirection(myGpio, GPIO_Number_0, GPIO_Direction_Output);
    GPIO_setDirection(myGpio, GPIO_Number_1, GPIO_Direction_Output);

    GPIO_setHigh(myGpio, GPIO_Number_0);
    GPIO_setHigh(myGpio, GPIO_Number_1);

    Loop();
}

void Loop(){
    while(1){
           char data = SCI_getDataBlocking(mySci);
           switch (data){
               case '2':
                   GPIO_setHigh(myGpio, GPIO_Number_0);
                   //SciMsg("El LED 0 muere\r\n");
                   ledState = false;
                   break;
               case '1':
                   GPIO_setLow(myGpio, GPIO_Number_0);
                   //SciMsg("El LED 0 vive\r\n");
                   ledState = true;
                   break;
               case '4':
                   GPIO_setHigh(myGpio, GPIO_Number_1);
                   //SciMsg("El LED 1 muere\r\n");
                   break;
               case '3':
                   GPIO_setLow(myGpio, GPIO_Number_1);
                   //SciMsg("El LED 1 vive\r\n");
                   break;
               case 'c':
                   if(ledState){
                       SCI_putDataBlocking(mySci, 'Y');
                   }else{
                       SCI_putDataBlocking(mySci, 'N');
                   }
                   break;
           }
       }
}

void SciSetup(){
    mySci = SCI_init((void *)SCIA_BASE_ADDR, sizeof(SCI_Obj));

    GPIO_setMode(myGpio, GPIO_Number_28, GPIO_28_Mode_SCIRXDA);
    GPIO_setMode(myGpio, GPIO_Number_29, GPIO_29_Mode_SCITXDA);

    GPIO_setPullUp(myGpio, GPIO_Number_28, GPIO_PullUp_Enable);
    GPIO_setPullUp(myGpio, GPIO_Number_29, GPIO_PullUp_Disable);
    GPIO_setQualification(myGpio, GPIO_Number_28, GPIO_Qual_ASync);

    CLK_enableSciaClock(myClk);

    SCI_disableParity(mySci);
    SCI_setNumStopBits(mySci, SCI_NumStopBits_One);
    SCI_setCharLength(mySci, SCI_CharLength_8_Bits);
    SCI_setBaudRate(mySci, SCI_BaudRate_9_6_kBaud);
    SCI_setPriority(mySci, SCI_Priority_FreeRun);

    SCI_enableTx(mySci);
    SCI_enableRx(mySci);
    SCI_enable(mySci);
}

void GeneralSetup(){
    myWDog = WDOG_init((void *)WDOG_BASE_ADDR, sizeof(WDOG_Obj));
    WDOG_disable(myWDog);

    myClk = CLK_init((void *)CLK_BASE_ADDR, sizeof(CLK_Obj));
    CLK_setOscSrc(myClk, CLK_OscSrc_Internal);

    myPll = PLL_init((void *)PLL_BASE_ADDR, sizeof(PLL_Obj));
    PLL_setup(myPll, PLL_Multiplier_12, PLL_DivideSelect_ClkIn_by_2);

    myGpio = GPIO_init((void *)GPIO_BASE_ADDR, sizeof(GPIO_Obj));
}

void SciMsg(char *msg){
    while(*msg != '\0'){
        SCI_putDataBlocking(mySci, *msg);
        ++msg;
    }
}
