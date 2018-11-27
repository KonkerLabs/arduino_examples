#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#define MEASUREMENT_BUFFER_SIZE 20

#define PROTOCOL_FORMAT "PB:%fkg PL:%fkg T:%fkg\r\n"
#define PROTOCOL_FORMAT2 "B:%fkg PL:%fkg T:%fkg\r\n"

#include "Arduino.h"
#include "debug.h"

/**
 * CLASS MEASUREMENT
 * 
 * holds information about each read taken from the scale 
 */
class Measurement  {
    public:
        unsigned long ts;
        float weight_raw;
        float weight_net;
        float tare;

        // flags of usage
        byte plate;
        byte bag; 
        byte person;

        byte update;

        Measurement();
        Measurement(Measurement*);
        ~Measurement();

        void copy(Measurement*);

        static Measurement* parse(char *buffer, Measurement *obj = NULL); 

        void dump();
        boolean isFlagged();

        void payload(char* aux, int size);
        float weight();

    protected:
        void reset();
};

#endif
