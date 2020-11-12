#ifndef __SERVOPUBLICACCESS_H
#define __SERVOPUBLICACCESS_H

// addresses used for public data access
#define servo_checksum              CHECKSUM("servo")

struct pad_servo {
    int name;
    bool state;
    float value;
};

#endif // __SERVOPUBLICACCESS_H
