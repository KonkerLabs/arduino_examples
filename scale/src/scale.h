#ifndef SCALE_H
#define SCALE_H

#include "measurement.h"
#include <SoftwareSerial.h>
#include <debug.h>

#define NUMBER_SECONDS_PER_PLATE 2
#define DATA_BUFFER_SIZE 255

#define HEARTBEAT_TIMEOUT_MS      60000   // 1 min
#define PERSON_WEIGHT_THRESHOLD   30      // KG to consider a person measured
#define BAG_LOWER_BOUND_LIMIT     1       // KG to consider that the weight is a bag

#define STATUS_SENDING_DATA 0x10
#define STATUS_CONNECTION_ERROR 0x11

#define TIMEOUT_READING_FROM_SCALE 60000 // 1 minute

//#define DEBUG

/**
 * CLASS SCALE
 * 
 * the scale class handles all operations a SCALE can have, including managing
 * to save some buffered information of the previous reads 
 */
class Scale {
    public: 
        Measurement* measurements[MEASUREMENT_BUFFER_SIZE];
        
        Scale();
        Scale(int _rx, int _tx);
        ~Scale();

        int add(Measurement* m);
        Measurement* getCurrent();
        void begin();
        int read();
        void dump(char* buffer, int size);
        int getState();

    private:

        char _buffer[DATA_BUFFER_SIZE];
        int state = 0;
        int index = -1;       
        int samples = 0;
        int _restartCounter = 0;
        String _channel = String("weight");

        Measurement* _mx;

        SoftwareSerial* serial_conn = NULL; // (D7,D8,true,256); // RX, TX, inverse, buffersize 
        unsigned long _lastUpdate; // last ts when the data was sent to the platform
        unsigned long _lastReadFromDevice = 0; 


        int getSampleBefore(Measurement* m, unsigned long seconds);
        int getNext(int ix);
        int getPrev(int ix);
        int process(char *buffer);

        void blink(int sec);

};


#endif
