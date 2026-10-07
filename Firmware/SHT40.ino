#include <Wire.h>
#include <Adafruit_SHT4x.h>

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

void setup()
{
  Serial.begin(115200);

  Wire.begin(PA10, PA9);

  if (!sht4.begin())
  {
    Serial.println("Couldn't find SHT40");
    return;
  } 

  Serial.println("SHT40 Found");

  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  sht4.setHeater(SHT4X_NO_HEATER);
}

void loop()
{
  sensors_event_t humidity, temp;

  if (!sht4.getEvent(&humidity, &temp))
  {
    Serial.println("Read failed");
    delay(100);
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.print(" °C\tHumidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  delay(300);
}