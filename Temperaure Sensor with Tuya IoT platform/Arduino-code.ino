#include <TuyaWifi.h>
#include <SoftwareSerial.h>

#include <Wire.h>
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

TuyaWifi my_device;
SoftwareSerial DebugSerial(8,9);

/* Current LED status */
unsigned char led_state = 0;
/* Connect network button pin */
int wifi_key_pin = 7;

/* Data point define */
#define ObjectTemp  101
#define AmbientTemp  102

/* Current device DP values */

long x= 0;
long y= 0;

/* Stores all DPs and their types. PS: array[][0]:dpid, array[][1]:dp type. 
 *                                     dp type(TuyaDefs.h) : DP_TYPE_RAW, DP_TYPE_BOOL, DP_TYPE_VALUE, DP_TYPE_STRING, DP_TYPE_ENUM, DP_TYPE_BITMAP
*/
unsigned char dp_array[][2] = {
   
    {ObjectTemp , DP_TYPE_VALUE},
    {AmbientTemp, DP_TYPE_VALUE},
};

unsigned char pid[] = {"7y4c8gqh8xw3c8bf"};
unsigned char mcu_ver[] = {"1.0.0"};

/* last time */
unsigned long last_time = 0;

void setup()

{
    mlx.begin();   
    Serial.begin(9600);
    DebugSerial.begin(9600);
    //Initialize led port, turn off led.
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    //Initialize networking keys.
    pinMode(wifi_key_pin, INPUT_PULLUP);
    //Enter the PID and MCU software version
    my_device.init(pid, mcu_ver);
    //incoming all DPs and their types array, DP numbers
    my_device.set_dp_cmd_total(dp_array, 2);  
   //register DP download processing callback function
    my_device.dp_process_func_register(dp_process); 
    //register upload all DP callback function
    my_device.dp_update_all_func_register(dp_update_all);

    last_time = millis();
}

void loop()
{
    my_device.uart_service();  
    //Enter the connection network mode when Pin7 is pressed.
    if (digitalRead(wifi_key_pin) == LOW) {
        delay(80);
        if (digitalRead(wifi_key_pin) == LOW) {
            my_device.mcu_set_wifi_mode(SMART_CONFIG);
        }
    }
  /* LED blinks when network is being connected */
    if ((my_device.mcu_get_wifi_work_state() != WIFI_LOW_POWER) && (my_device.mcu_get_wifi_work_state() != WIFI_CONN_CLOUD) && (my_device.mcu_get_wifi_work_state() != WIFI_SATE_UNKNOW)) {
        if (millis() - last_time >= 500) {
            last_time = millis();
            /* Toggle current LED status */
            if (led_state == LOW) {
                led_state = HIGH;
            } else {
                led_state = LOW;
            }

            digitalWrite(LED_BUILTIN, led_state);
        }
    }

  if ((my_device.mcu_get_wifi_work_state() == WIFI_CONNECTED) || (my_device.mcu_get_wifi_work_state() == WIFI_CONN_CLOUD)) 
   {
    x=mlx.readObjectTempC();
    y=mlx.readAmbientTempC();
    my_device.mcu_dp_update(ObjectTemp, x, 1);
    my_device.mcu_dp_update(AmbientTemp, y, 1);
   }
    delay(10);
}
unsigned char dp_process(unsigned char dpid,const unsigned char value[], unsigned short length)
{
  /* all DP only report */
  return SUCCESS;
}
void dp_update_all(void)
{
   my_device.mcu_dp_update(ObjectTemp, x, 1);
   my_device.mcu_dp_update(AmbientTemp, y, 1);
}
