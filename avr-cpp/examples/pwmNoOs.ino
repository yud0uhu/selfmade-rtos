#define THRESHOLD 100
#define RELAY_TIME 3
int iTarget = 100;
int iPWM = 128;
bool flg = true;
bool previous_flag = true;
void setup()
{
    // Serial.begin(115200);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
}
void loop()
{
    int iMonitor = analogRead(A1);
    iPWM = iPWM - (int)((float)(iMonitor - iTarget) / 3);
    if (iPWM > 255)
        iPWM = 255;
    if (iPWM < 0)
        iPWM = 0;
    analogWrite(3, iPWM);
    // Serial.println("PWM Control");
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
            delay(RELAY_TIME);
            previous_flag = true;
            // Serial.println("RELAY ON Control");
        }
        break;
    case false:
        digitalWrite(4, LOW);
        if (previous_flag == true)
        {
            delay(RELAY_TIME);
            previous_flag = false;
            // Serial.println("RELAY OFF Control");
        }
        break;
    }
}