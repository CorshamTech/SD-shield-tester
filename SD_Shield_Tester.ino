//=============================================================================


#include <Arduino.h>
#include <SD.h>
#include <Wire.h>
#include <SPI.h>

// The pins used on the newer SD Shields.

#define NEW_SHIELD_PIN  38
#define OPTION_1_PIN    42
#define OPTION_2_PIN    41
#define OPTION_3_PIN    40
#define OPTION_4_PIN    39
#define TIMER_OUT_PIN   43    //maps to pin 18, CB1

// Pin used by the SD card

#define SD_PIN  53

// Pin with the presence sensor

#define PRESENCE_PIN 19

// Define the pins used for the LEDs

#define RED_LED_PIN  24
#define YELLOW_LED_PIN  23
#define GREEN_LED_PIN  22

// This is the I2C address of the DS3231 RTC.

#define DS3231_I2C_ADDRESS  0x68

// Trying to remember if HIGH or LOW turns the LEDs on or off, so these
// two macros make it easier.

#define LED_OFF  HIGH
#define LED_ON   LOW






int shieldRevision;



//=============================================================================
void setup()
{
        Serial.begin(9600);
        Serial.println("Corsham Tech SD Shield Tester");

        // Find what revision this shield is and set a variable.
        // This value changes what needs to be tested.

        shieldRevision = getShieldRevision();
        Serial.print("This appears to be a revision ");
        Serial.print(shieldRevision);
        Serial.println(" shield.  Verify this is correct.");

        pinMode(SD_PIN, OUTPUT);    // required by SD library

        // Configure the new option pins as inputs with pull-ups
        pinMode(NEW_SHIELD_PIN, INPUT_PULLUP);
        pinMode(OPTION_1_PIN, INPUT_PULLUP);
        pinMode(OPTION_2_PIN, INPUT_PULLUP);
        pinMode(OPTION_3_PIN, INPUT_PULLUP);
        pinMode(OPTION_4_PIN, INPUT_PULLUP);

        pinMode(PRESENCE_PIN, INPUT);   // pin with presence bit

        pinMode(RED_LED_PIN, OUTPUT);
        digitalWrite(RED_LED_PIN, LED_OFF);
        
        pinMode(YELLOW_LED_PIN, OUTPUT);
        digitalWrite(YELLOW_LED_PIN, LED_OFF);
        
        pinMode(GREEN_LED_PIN, OUTPUT);
        digitalWrite(GREEN_LED_PIN, LED_ON);
        
        // Configure new output pins
        pinMode(TIMER_OUT_PIN, OUTPUT);

        SD.begin(SD_PIN);

        Wire.begin();
}



//=============================================================================
void loop()
{
        int key;
        
        printMenu();

        // Now wait for the user to send a command

        do
        {
                key = Serial.read();
        }
        while (!isprint(key));
        
        switch (key)
        {
                case 'r':
                case 'R':
                        toggleLED(RED_LED_PIN, "Red");
                        break;

                case 'y':
                case 'Y':
                        toggleLED(YELLOW_LED_PIN, "Yellow");
                        break;

                case 'g':
                case 'G':
                        toggleLED(GREEN_LED_PIN, "Green");
                        break;

                case 'i':
                case 'I':
                        doInstalledTest();
                        break;

                case 'o':
                case 'O':
                        doShowOptionSwitches();
                        break;

                case 's':
                case 'S':
                        sdAccessTest();
                        break;

                case 'T':
                case 't':
                        doRtcTest();
                        break;
        }
}



//=============================================================================
void printMenu(void)
{
        Serial.println("");
        Serial.println("Options:");
        Serial.println("   R = Toggle red LED");
        Serial.println("   Y = Toggle yellow LED");
        Serial.println("   G = Toggle green LED");

        if (shieldRevision == 2)
        {
                Serial.println("   O = Show option switch settings");
        }
        Serial.println("   I = Show if SD card is installed or not");
        Serial.println("   S = SD card access test");
        Serial.println("   T = RTC test");
}



//=============================================================================
// Given an LED pin number and name, toggle the state and display the
// new state.

void toggleLED(int pin, char *name)
{
        Serial.print(name);
        if (digitalRead(pin))
        {
                Serial.println(" LED is now on");
                digitalWrite(pin, LED_ON);
        }
        else
        {
                Serial.println(" LED is now off");
                digitalWrite(pin, LED_OFF);
        }
}



//=============================================================================
void doInstalledTest(void)
{
        if (digitalRead(PRESENCE_PIN))
        {
                Serial.println("SD is not installed");
        }
        else
        {
                Serial.println("SD is installed");
        }
}



//=============================================================================
void doShowOptionSwitches(void)
{
        Serial.print("Option 1: ");
        Serial.println(digitalRead(OPTION_1_PIN) ? "Off" : "On");

        Serial.print("Option 2: ");
        Serial.println(digitalRead(OPTION_2_PIN) ? "Off" : "On");

        Serial.print("Option 3: ");
        Serial.println(digitalRead(OPTION_3_PIN) ? "Off" : "On");

        Serial.print("Option 4: ");
        Serial.println(digitalRead(OPTION_4_PIN) ? "Off" : "On");
}



//=============================================================================
// This returns the version of the shield.  Currently it only supports old
// (version 1) or new (version 2).  Returns 1 or 2.

int getShieldRevision(void)
{
        if (digitalRead(NEW_SHIELD_PIN))
                return 1;
        else
                return 2;
}



//=============================================================================
// This test to see if we can read the SD card.  It doesn't need to do much,
// just basic access is enough to prove the board is working.

void sdAccessTest(void)
{
        File root;    // root directory

        if (digitalRead(PRESENCE_PIN))
        {
                Serial.println("SD is not installed; can't run test.");
                return;
        }

        root = SD.open("/");
        if (root < 0)
        {
                Serial.println("Could not read the card");
                return;
        }
        Serial.println("Opened the SD, attempting to read files...");

        bool done = true;
        while (done)
        {
                File entry =  root.openNextFile();
                if (! entry)
                {
                        // no more files
                        done = false;
                }
                else
                {
                        Serial.print(entry.name());
                        if (entry.isDirectory())
                        {

                        }
                        else
                        {
                                // files have sizes, directories do not
                                Serial.print("\t\t");
                                Serial.println(entry.size(), DEC);
                        }
                }
                entry.close();
        }
        //SD.close();
        Serial.println("SD card access succeeded");
}



//=============================================================================
void doRtcTest(void)
{
        Wire.beginTransmission(DS3231_I2C_ADDRESS);
        Wire.write(0);    // set DS3231 register pointer to 00h
        Wire.endTransmission();
        Wire.requestFrom(DS3231_I2C_ADDRESS, 1);

        int second = bcdToDec(Wire.read() & 0x7f);
        delay(1900);    // delay just under two seconds

        Wire.beginTransmission(DS3231_I2C_ADDRESS);
        Wire.write(0);    // set DS3231 register pointer to 00h
        Wire.endTransmission();
        Wire.requestFrom(DS3231_I2C_ADDRESS, 1);
        int secondsecond = bcdToDec(Wire.read() & 0x7f);

        if (secondsecond == second + 1)
        {
                Serial.println("RTC is ticking - passed");
        }
        else
        {
                Serial.println("Clock didn't seem to tick, but try again.");
        }
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
        return  (val / 16 * 10) + (val % 16);
}



