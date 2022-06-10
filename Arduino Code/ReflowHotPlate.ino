#include <LiquidCrystal.h>
#include "max6675.h" //https://github.com/adafruit/MAX6675-library
// Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(12, 11, 6, 5, 4, 3);
int thermoDO = 8; // SO of  MAX6675 module to D8
int thermoCS = 9; // CS of MAX6675 module to D9
int thermoCLK = 10; // SCK of MAX6675 module to D10
int SSRon = 13; //SSR control signal to D13
int button = 2; //Control button pin to D2

//Current and Previous Button State
int buttonState = 0; 
int lastButtonState = 0; 

int stage = 0; //Variable for process stage

//Current and Previous Temperature Values
int CurrTemp = 0;
int OldTemp = 0;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);//Initialising Thermocouple and Library

void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 4);
  lcd.clear();
  pinMode(button, INPUT);
}
void loop()
{
  float t = thermocouple.readCelsius();
  Serial.println(t);
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Temp_C:");
  lcd.setCursor(8, 0);
  lcd.print(t);
  lcd.print((char)223);
  lcd.print("C");
  delay(100);
  buttonState = digitalRead(button);
  lcd.setCursor(0, 1);
  lcd.setCursor(0, 3);
  lcd.print("Stage = ");
  lcd.setCursor(8, 3);
  lcd.print(stage);

  if (buttonState != lastButtonState) 
  {
    if (buttonState == HIGH && stage == 0) 
    {
     stage = 1;//If button is pressed outside the soldering process, the process will be triggered to start
    } 
    else if (buttonState == HIGH && stage != 0)
    {
      stage = 0;//If button is pressed while soldering is ongoing, process will be stopped
    }
  delay(50);
  }
  lastButtonState = buttonState;

  if(t<130 && stage == 1)//Initial ramp to soak, slope controlled at ~2.25K/s
  {
    lcd.setCursor(0, 1);
    t = thermocouple.readCelsius();
    lcd.println("Ramp to Soak");
    lcd.setCursor(0, 2);
    lcd.println("Press to Stop");
    lcd.setCursor(0, 3);
    lcd.print("Stage = ");
    lcd.setCursor(8, 3);
    lcd.print(stage);
    CurrTemp=t;
    if((OldTemp-CurrTemp)<0.25)
    {
      digitalWrite(SSRon, HIGH);
    }
    if((OldTemp-CurrTemp)>0.25)
    {
      digitalWrite(SSRon, LOW);
    }
    OldTemp = CurrTemp;
    
    if (t>115)
    {
      stage = 2;
    }
  }
  
  else if(120<t<160 && stage == 2)// Preheat/Soak stage, slow rise in temperature
  {
    lcd.setCursor(0, 1);
    lcd.println("Preheat/Soak");
    lcd.setCursor(0, 2);
    lcd.println("Press to Stop");
    lcd.setCursor(0, 3);
    lcd.print("Stage = ");
    lcd.setCursor(8, 3);
    lcd.print(stage);
    digitalWrite(SSRon, HIGH);
    delay(100);
    digitalWrite(SSRon, LOW);
    delay(100);
    if (t>160)
    {
      stage = 3;
    }
  }
  
  else if(160<t && stage == 3)//Final ramp to reflow, slope controlled at ~2.25K/s
  {
    lcd.setCursor(0, 1);
    lcd.println("Reflow       ");
    lcd.setCursor(0, 2);
    lcd.println("Press to Stop");
    lcd.setCursor(0, 3);
    lcd.print("Stage = ");
    lcd.setCursor(8, 3);
    lcd.print(stage);
    CurrTemp=t;
    if((OldTemp-CurrTemp)<0.25)
    {
      digitalWrite(SSRon, HIGH);
    }
    if((OldTemp-CurrTemp)>0.25)
    {
      digitalWrite(SSRon, LOW);
    }
    OldTemp = CurrTemp;
    
    if (t>215)
    {
      stage = 4;
    }
  }
  else if(stage == 4)//Power cutoff at 215 degrees Celsius, thermal inertia carries temp to about 240 Celsius
  {
    lcd.setCursor(0, 1);
    lcd.println("Cooldown      ");
    lcd.setCursor(0, 2);
    lcd.println("Press to Reset");
    lcd.setCursor(0, 3);
    lcd.print("Stage = ");
    lcd.setCursor(8, 3);
    lcd.print(stage);
    digitalWrite(SSRon, LOW);
  }
  else if(stage == 0)//Initial/Reset stage, press button to start again
  {
    lcd.setCursor(0, 1);
    lcd.println("Press to start  ");
    lcd.setCursor(0, 2);
    lcd.println("              ");
    lcd.setCursor(0, 3);
    lcd.print("Stage = ");
    lcd.setCursor(8, 3);
    lcd.print(stage);
    digitalWrite(SSRon, LOW);
  }
}

