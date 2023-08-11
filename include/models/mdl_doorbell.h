#ifndef MODELS_MDL_DOORBELL_H
#define MODELS_MDL_DOORBELL_H

#include <stdint.h>
#include <glib.h>
#include <systemd/sd-bus.h>

#include "models/mdl_unit.h"

#define DOORBELL_SELF_SIGNATURE         "ttysssuuutt"
#define DOORBELL_DATA_SIGNATURE         DOORBELL_SELF_SIGNATURE "a" UNIT_STRUCT_SIGNATURE "a" UNIT_STRUCT_SIGNATURE

typedef union {
    uint8_t config;
    struct {
        uint8_t installationMode    :1;
        uint8_t enableImg           :1;
        uint8_t hiddenSearchBar     :1;
        uint8_t hiddenButtons       :1;
        uint8_t :4;
    };
} mdlDoorbellConfig;

/**
 * @brief Doorbell data parsed for services
 */
typedef struct MdlDoorbellStruct {
    uint64_t id;
    uint64_t buildingId;
    mdlDoorbellConfig config;
    gchar *publicName;
    gchar *address;
    gchar *buildingsNumber;
    uint32_t volume;
    uint32_t brightness;
    uint32_t timeToCloseModal;
    uint64_t dialingSound;
    uint64_t openDoorSound;
    MdlUnitList *business_doors;
    MdlUnitList *general_doors;
} MdlDoorbell;

void doorbell_clean(MdlDoorbell *model);
MdlDoorbell *doorbell_clone(MdlDoorbell *model);
int doorbell_parse_message(MdlDoorbell *model, sd_bus_message *m);

#endif // MODELS_MDL_DOORBELL_H
