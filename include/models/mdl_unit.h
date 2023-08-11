#ifndef MODELS_MDL_UNIT_H
#define MODELS_MDL_UNIT_H

#include <stdint.h>
#include <glib.h>
#include <systemd/sd-bus.h>

#define UNIT_BASE_SIGNATURE             "tttttysssyyuu"
#define UNIT_MEMBER_STRUCT_SIGNATURE    "(" UNIT_BASE_SIGNATURE ")"
#define UNIT_DATA_SIGNATURE             UNIT_BASE_SIGNATURE "a" UNIT_MEMBER_STRUCT_SIGNATURE
#define UNIT_STRUCT_SIGNATURE           "(" UNIT_DATA_SIGNATURE ")"

/**
 * @brief Union to use bits for boolean
 */
typedef union {
    uint8_t config;
    struct {
        uint8_t disabled         :1;
        uint8_t disabledSchedule :1;
        uint8_t disableSequence  :1;
    };
} mdlUnitConfig;

typedef union {
    uint8_t fullWeek;
    struct {
        uint8_t sunday      :1;
        uint8_t monday      :1;
        uint8_t tuesday     :1;
        uint8_t wednesday   :1;
        uint8_t thursday    :1;
        uint8_t friday      :1;
        uint8_t saturday    :1;
    };
} weekDays;

typedef struct MdlUnitListStruct {
    struct MdlUnitStruct *unit;
    struct MdlUnitListStruct *next;
} MdlUnitList;

/**
 * @brief Door data class
 */
typedef struct MdlUnitStruct {
    uint64_t roomId;
    uint64_t userId;
    uint64_t number;
    uint64_t sequentialNumber;
    uint64_t callTimeOut;
    uint8_t type;
    gchar *imgUrl;
    gchar *header;
    gchar *text;
    mdlUnitConfig unitConfig;
    weekDays daysOfWeek;
    /**
     * @brief Time of a day in seconds
     * (00:00:00-23:59:59) = (0 - 86400)
     */
    uint32_t startTime;
    /**
     * @brief time of a day in seconds
     * (00:00:00-23:59:59) = (0 - 86400)
     */
    uint32_t endTime;
    MdlUnitList *members;
} MdlUnit;

void unit_clean(MdlUnit *unit);
void unit_list_clean(MdlUnitList *units);
MdlUnit *unit_clone(MdlUnit *unit);
MdlUnitList *unit_list_clone(MdlUnitList *units);
MdlUnitList *unit_list_append(MdlUnitList *units, MdlUnit *unit);
MdlUnit *unit_parse_message(sd_bus_message *msg, int isMember);

#endif // MODELS_MDL_UNIT_H