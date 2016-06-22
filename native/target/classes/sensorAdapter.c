#include <sensorAdapter.h>
#include <jni.h>
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include "locking.h"

#define MAXTIMINGS 85
static int DHTPIN = 7;
static int dht22_dat[5] = {0,0,0,0,0};

static uint8_t sizecvt(const int read)
{
  /* digitalRead() and friends from wiringpi are defined as returning a value
  < 256. However, they are returned as int() types. This is a safety function */

  if (read > 255 || read < 0)
  {
    printf("Invalid data from wiringPi library\n");
    exit(EXIT_FAILURE);
  }
  return (uint8_t)read;
}

static int read_dht22_dat(float *temperature, float *humidity)
{
  uint8_t laststate = HIGH;
  uint8_t counter = 0;
  uint8_t j = 0, i;

  dht22_dat[0] = dht22_dat[1] = dht22_dat[2] = dht22_dat[3] = dht22_dat[4] = 0;

  // pull pin down for 18 milliseconds
  pinMode(DHTPIN, OUTPUT);
  digitalWrite(DHTPIN, HIGH);
  delay(10);
  digitalWrite(DHTPIN, LOW);
  delay(18);
  // then pull it up for 40 microseconds
  digitalWrite(DHTPIN, HIGH);
  delayMicroseconds(40); 
  // prepare to read the pin
  pinMode(DHTPIN, INPUT);

  // detect change and read data
  for ( i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while (sizecvt(digitalRead(DHTPIN)) == laststate) {
      counter++;
      delayMicroseconds(1);
      if (counter == 255) {
        break;
      }
    }
    laststate = sizecvt(digitalRead(DHTPIN));

    if (counter == 255) break;

    // ignore first 3 transitions
    if ((i >= 4) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      dht22_dat[j/8] <<= 1;
      if (counter > 16)
        dht22_dat[j/8] |= 1;
      j++;
    }
  }

  // check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
  // print it out if data is good
  if ((j >= 40) && 
      (dht22_dat[4] == ((dht22_dat[0] + dht22_dat[1] + dht22_dat[2] + dht22_dat[3]) & 0xFF)) ) {
        float t, h;
        h = (float)dht22_dat[0] * 256 + (float)dht22_dat[1];
        h /= 10;
        t = (float)(dht22_dat[2] & 0x7F)* 256 + (float)dht22_dat[3];
        t /= 10.0;
        if ((dht22_dat[2] & 0x80) != 0)  t *= -1;


    //printf("Humidity = %.2f %% Temperature = %.2f *C \n", h, t );
    *humidity = *h;
    *temperature = *t;
    return 1;
  }
  else
  {
    //printf("Data not good, skip\n");
    return 0;
  }
}


/** 
  * JNI EXCEPTION
  */ 
void throwException(JNIEnv *env, const char *message){
    jclass exception = (*env)->Findclass(env, "com/github/blackanthrax/terracontrol/jni/MeasurementException");
    if (class != NULL) {
        (*env)->ThrowNew(env, class, message);
    }   
    (*env)->DeleteLocalRef(env, class);
}



/**
  * JNI INTERFACE
  */
JNIEXPORT jobject JNICALL Java_com_github_blackanthrax_terracontrol_jni_SensorAdapter_getMeasurement(JNIEnv *env, jobject object){
    
    DHTPIN = //read configs here
        
    int tries = 100;
    float temp;
    float hum;

    while(read_dht22_dat(&temp, &hum) == 0 && tries--){
	delay(1000);
    }

    jclass cls = (*env)->FindClass(env, "com/github/blackanthrax/terracontrol/jni/Measurement");
    return (*env)->NewObject(env, cls,     

}