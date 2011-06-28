/**
 * Sends "1" (ASCII 49) over serial when we press button
 * and "0" (ASCII 48) when released.
 * 
 * Possible baudrates:
 * 300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 
 * 57600, or 115200
 */

const long BAUDRATE = 19200;
const long DEBOUNCE = 10; // milliseconds
const int BUTTON_PIN = 7;

byte current_state;
byte previous_state = 0;

void setup()
{
    pinMode(BUTTON_PIN, INPUT);
    digitalWrite(BUTTON_PIN, HIGH); // Use an internal pull-up resistor
    Serial.begin(BAUDRATE);
}

void loop()
{
    current_state = digitalRead(BUTTON_PIN);
  
    if (current_state != previous_state)
    {
        // Inversion due to the pull-up
        if (current_state)
            Serial.println("0");
        else
            Serial.println("1");
        previous_state = current_state;  
    }
  
    delay(DEBOUNCE);
}

