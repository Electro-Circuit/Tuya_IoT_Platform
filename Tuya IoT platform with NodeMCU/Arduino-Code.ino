#include <TuyaWifi.h>
#include <SoftwareSerial.h>

TuyaWifi my_device;
/* Current LED status */
unsigned char led_state = 0;

unsigned char led_state1 = 0;
unsigned char led_state2 = 0;
/* Connect network button pin */
int key_pin = 7;

int led1=2;
int led2=3;
/* Data point define */
#define DPID_SWITCH1 101
#define DPID_SWITCH2 102

unsigned char dp_array[][2] =
{
  {DPID_SWITCH1, DP_TYPE_BOOL},
  {DPID_SWITCH2, DP_TYPE_BOOL},
};

unsigned char pid[] = {"160z1sanoqsribbr"};
unsigned char mcu_ver[] = {"3.1.4"};

/* last time */
unsigned long last_time = 0;

void setup() 
{
  // Serial.begin(9600);
  Serial.begin(9600);

  //Initialize led port, turn off led.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(led1, OUTPUT);
  digitalWrite(led1, LOW);
  pinMode(led2, OUTPUT);
  digitalWrite(led2, LOW);
  
  //Initialize networking keys.
  pinMode(key_pin, INPUT_PULLUP);

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
  if (digitalRead(key_pin) == LOW) {
    delay(80);
    if (digitalRead(key_pin) == LOW) {
      my_device.mcu_set_wifi_mode(SMART_CONFIG);
    }
  }
  /* LED blinks when network is being connected */
  if ((my_device.mcu_get_wifi_work_state() != WIFI_LOW_POWER) && (my_device.mcu_get_wifi_work_state() != WIFI_CONN_CLOUD) && (my_device.mcu_get_wifi_work_state() != WIFI_SATE_UNKNOW)) {
    if (millis()- last_time >= 500) {
      last_time = millis();

      if (led_state == LOW) {
        led_state = HIGH;
      } else {
        led_state = LOW;
      }
      digitalWrite(LED_BUILTIN, led_state);
    }
  }

  delay(10);
}

unsigned char dp_process(unsigned char dpid,const unsigned char value[], unsigned short length)
{
  switch(dpid) {
    case DPID_SWITCH1:
      led_state1 = my_device.mcu_get_dp_download_data(dpid, value, length); /* Get the value of the down DP command */
      if (led_state1) {
        //Turn on
        digitalWrite(led1, HIGH);
      } else {
        //Turn off
        digitalWrite(led1, LOW);
      }
      //Status changes should be reported.
      my_device.mcu_dp_update(dpid, value, length);
    break;
    case DPID_SWITCH2:
      led_state2 = my_device.mcu_get_dp_download_data(dpid, value, length); /* Get the value of the down DP command */
      if (led_state2) {
        //Turn on
        digitalWrite(led2, HIGH);
      } else {
        //Turn off
        digitalWrite(led2, LOW);
      }
      //Status changes should be reported.
      my_device.mcu_dp_update(dpid, value, length);
    break;

    default:break;
  }
  return SUCCESS;
}

void dp_update_all(void)
{
  my_device.mcu_dp_update(DPID_SWITCH1, led_state1, 1);
  my_device.mcu_dp_update(DPID_SWITCH2, led_state2, 1);
}
