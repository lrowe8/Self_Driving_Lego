#include <Servo.h>
#include <LiquidCrystal.h>

// Adding this comment

#define TURN_PIN      6
#define MOTOR_PIN     5
#define LEFT_SIGNAL   13
#define RIGHT_SIGNAL  12

#define LEFT_ON       0x1
#define BRAKE_ON      0x2
#define RIGHT_ON      0x4

#define LCD_RS        11
#define LCD_EN        10
#define LCD_D4        7
#define LCD_D5        4
#define LCD_D6        3
#define LCD_D7        2
#define LCD_V0        8

#define DAMPENING     .96
#define BLINK_RATE    125
#define CENTER        112

unsigned char lights_on = 0x00;
unsigned char lights_status = 0x00;
unsigned long blinkMark = 0;

Servo drive;
Servo turn;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void setup() {

  lcd.begin(16, 2);
  pinMode(LEFT_SIGNAL, OUTPUT);
  pinMode(RIGHT_SIGNAL, OUTPUT);
  pinMode(LCD_V0, OUTPUT);

  drive.attach(MOTOR_PIN);
  turn.attach(TURN_PIN);

  digitalWrite(LEFT_SIGNAL, LOW);
  digitalWrite(RIGHT_SIGNAL, LOW);
  digitalWrite(LCD_V0, LOW);

  drive.write(50);
  turn.write(CENTER);
}

void loop() {
  
  float leftDistance = (float)analogRead(A2) * (5.0 / 1023.0);
  float centerDistance = (float)analogRead(A1) * (5.0 / 1023.0);
  float rightDistance = (float)analogRead(A0) * (5.0 / 1023.0);

  BrakeLightControl(leftDistance, centerDistance, rightDistance);
  DecideOnTurn(leftDistance, centerDistance, rightDistance);
  SetSpeed(leftDistance, centerDistance, rightDistance);

  lcd.setCursor(0, 1);
  lcd.print("ST");
  lcd.print(lights_status, HEX);
  lcd.print(" ON");
  lcd.print(lights_on, HEX);
}

void BrakeLightControl(long left, long center, long right)
{
  if((lights_status & BRAKE_ON) || (lights_status & LEFT_ON))
  {    
    if(lights_status & LEFT_ON)
    {
      if((millis() - blinkMark) >= BLINK_RATE)
      {
        if(lights_on & LEFT_ON)
        {
          digitalWrite(LEFT_SIGNAL, LOW);
          lights_on &= (~LEFT_ON);
        }
        else
        {
          digitalWrite(LEFT_SIGNAL, HIGH);
          lights_on |= LEFT_ON;
        }

        blinkMark = millis();
      }
    }
    else
    {
      digitalWrite(LEFT_SIGNAL, HIGH);
      lights_on |= LEFT_ON;
    }
  }
  
  if((lights_status & BRAKE_ON) || (lights_status & RIGHT_ON))
  {
    if(lights_status & RIGHT_ON)
    {
      if((millis() - blinkMark) >= BLINK_RATE)
      {
        if(lights_on & RIGHT_ON)
        {
          digitalWrite(RIGHT_SIGNAL, LOW);
          lights_on &= (~RIGHT_ON);
        }
        else
        {
          digitalWrite(RIGHT_SIGNAL, HIGH);
          lights_on |= RIGHT_ON;
        }

        blinkMark = millis();
      }
    }
    else
    {
      digitalWrite(RIGHT_SIGNAL, HIGH);
      lights_on |= RIGHT_ON;
    }
  }

  if(lights_status == 0x00)
  {
    digitalWrite(LEFT_SIGNAL, LOW);
    digitalWrite(RIGHT_SIGNAL, LOW);
    lights_on = 0x00; 
    blinkMark = 0;
  }
}

void DecideOnTurn(float left, float center, float right)
{
  // Right - 150, Center - 110, Left - 70
  int angleOffset = 0;

  if(center > 1.5 && center <= 3.0)
  {
    center -= 1.5;

    if(left > right)
      angleOffset += (((float)center / 1.5) * 40.0) * 1.5;
    else if(right >= left)
      angleOffset -= (((float)center / 1.5) * 40.0) * 1.5;
  }
  if(left > 1.5 && left <= 3.0)
  {
    left -= 1.5;
    angleOffset += (((float)left / 1.5) * 40.0) * DAMPENING;
  }
  
  if(right > 1.5 && right <= 3.0)
  {
    right -= 1.5;
    angleOffset -= (((float)right / 1.5) * 40.0) * DAMPENING;
  }

  if(angleOffset >= 38)
    angleOffset = 38;
  else if(angleOffset <= -38)
    angleOffset = -38;

  lcd.setCursor(0, 0);
  lcd.print("Turn:         ");
  lcd.setCursor(6, 0);
  
  if(angleOffset > 0)
  {
    lights_status |= RIGHT_ON;
    lights_status &= (~LEFT_ON);
  }
  else if(angleOffset < 0)
  {
    lights_status |= LEFT_ON;
    lights_status &= (~RIGHT_ON);
  }
  else
  {
    lights_status &= (~LEFT_ON);
    lights_status &= (~RIGHT_ON);
  }

  lcd.print(angleOffset);

  turn.write(CENTER + angleOffset);
  
}

void SetSpeed(float left, float center, float right)
{
  float temp = left;

  if(right < temp)
    temp = right;
  if(center < temp)
    temp = center;

  if(center > 2.3)
  {
    drive.write(50);
    lights_status |= BRAKE_ON;
  }
  else if(center >= 1.6)
  {
    drive.write(65);
    lights_status |= BRAKE_ON;
  }
  else if(center < 1.6)
  {
    drive.write(66);
    lights_status &= (~BRAKE_ON);
  }
}

