#include <Arduino_FreeRTOS.h>
#define THRESHOLD 100
#define RELAY_DELAY 0
#define PWM_DELAY 0
#define fCoeff_P 0.3
#define fCoeff_I 0.4
#define fCoeff_D 2.8
int iTarget = 80;
int iPWM = 128;
float fP_error = 0.0;
float fI_error = 0.0;
float fD_error = 0.0;
float fP_error_previous = 0.0;
bool flg = true;
bool previous_flag = true;
void TaskRelay(void *pvParameters);
void TaskPWM(void *pvParameters);
void TaskDisplay(void *pvParameters);
void setup()
{
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    Serial.begin(115200);
    xTaskCreate(
        TaskRelay, "TaskRelay" // A name just for humans
        ,
        128 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,
        NULL, 3 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,
        NULL);
    xTaskCreate(
        TaskPWM, "TaskPWM", 128 // Stack size
        ,
        NULL, 1 // Priority
        ,
        NULL);
    xTaskCreate(
        TaskDisplay, "TaskDisplay", 128 // Stack size
        ,
        NULL, 1 // Priority
        ,
        NULL);
}
void loop() {}
void TaskRelay(void *pvParameters)
{ // Relay control
    (void)pvParameters;
    for (;;)
    {
        if (iPWM < THRESHOLD - 1)
            flg = true;
        if (iPWM > THRESHOLD)
            flg = false;
        switch (flg)
        {
        case true:
            digitalWrite(4, HIGH);
            if (previous_flag == false)
            {
                vTaskDelay(RELAY_DELAY / portTICK_PERIOD_MS);
                ;
                previous_flag = true;
                // Serial.println("RELAY ON Control");
            }
            break;
        case false:
            digitalWrite(4, LOW);
            if (previous_flag == true)
            {
                vTaskDelay(RELAY_DELAY / portTICK_PERIOD_MS);
                previous_flag = false;
                // Serial.println("RELAY OFF Control");
            }
            break;
        }
        vTaskDelay(1);
    }
}
void TaskPWM(void *pvParameters)
{ // PWM control
    (void)pvParameters;
    for (;;)
    {
        int iMonitor = analogRead(A1);
        fP_error = fCoeff_P * (float)(iMonitor - iTarget) / 1.5;
        fI_error += fCoeff_I * fP_error;
        fD_error = fCoeff_D * (fP_error - fP_error_previous);
        fP_error_previous = fP_error;
        iPWM -= (int)(fP_error + fI_error + fD_error);
        if (iPWM > 255)
            iPWM = 255;
        if (iPWM < 0)
            iPWM = 0;
        analogWrite(3, iPWM);
        // Serial.println("PWM Control");
        vTaskDelay(PWM_DELAY / portTICK_PERIOD_MS);
        vTaskDelay(1);
    }
}
void TaskDisplay(void *pvParameters)
{ // PWM control
    (void)pvParameters;
    for (;;)
    {
        // Serial.println("D");
        vTaskDelay(1);
    }
}