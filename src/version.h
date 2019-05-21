#ifndef _VERSION__H
#define _VERSION__H

#define MAJOR_VERSION 0x02
#define MINOR_VERSION 0x01

#ifdef __cplusplus
class Version {
    public:
        const char *get_build(void) const;
        const char *get_build_date(void) const;
};
#endif

#endif
