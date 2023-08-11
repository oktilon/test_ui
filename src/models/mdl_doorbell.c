#include "models/mdl_doorbell.h"

#include "app.h"

void doorbell_clean(MdlDoorbell *model) {
    if(!model) return;

    model->id = 0;
    model->buildingId = 0;
    model->volume = 0;
    model->brightness = 0;
    model->timeToCloseModal = 0;
    model->dialingSound = 0;
    model->openDoorSound = 0;
    model->config.config = 0;

    g_free(model->publicName);
    g_free(model->address);
    g_free(model->buildingsNumber);

    unit_list_clean(model->business_doors);
    g_free(model->business_doors);

    unit_list_clean(model->general_doors);
    g_free(model->general_doors);

    g_free(model);
}

MdlDoorbell *doorbell_clone(MdlDoorbell *model) {
    MdlDoorbell *ret = (MdlDoorbell *) g_malloc0(sizeof(MdlDoorbell));
    if(ret) {
        ret->id = model->id;
        ret->buildingId = model->buildingId;
        ret->config.config = model->config.config;
        ret->publicName = g_strdup_printf("%s", model->publicName);
        ret->address = g_strdup_printf("%s", model->address);
        ret->buildingsNumber = g_strdup_printf("%s", model->buildingsNumber);
        ret->volume = model->volume;
        ret->brightness = model->brightness;
        ret->timeToCloseModal = model->timeToCloseModal;
        ret->dialingSound = model->dialingSound;
        ret->openDoorSound = model->openDoorSound;
        ret->business_doors = unit_list_clone(model->business_doors);
        ret->general_doors = unit_list_clone(model->general_doors);
    }
    return ret;
}

int doorbell_parse_message(MdlDoorbell *mdl, sd_bus_message *msg) {
    // Variable
    int r;

    // Read doorbell self data
    r = sd_bus_message_read(msg, DOORBELL_SELF_SIGNATURE
        , &(mdl->id)
        , &(mdl->buildingId)
        , &(mdl->config.config)
        , &(mdl->publicName)
        , &(mdl->address)
        , &(mdl->buildingsNumber)
        , &(mdl->volume)
        , &(mdl->brightness)
        , &(mdl->timeToCloseModal)
        , &(mdl->dialingSound)
        , &(mdl->openDoorSound)
    );
    if(r < 0) {
        selfLogErr("Read message error(%d): %s", r, strerror(-r));
    }

    // Enter business doors array container
    r = sd_bus_message_enter_container(msg, SD_BUS_TYPE_ARRAY, UNIT_STRUCT_SIGNATURE);
    if(r < 0) {
        // Log error
        selfLogErr("Enter business doors array error(%d): %s", r, strerror(-r));
        return r;
    }

    MdlUnitList *prev = NULL;
    while (sd_bus_message_at_end(msg, FALSE) == 0) {
        MdlUnit *unit = unit_parse_message(msg, FALSE);
        if(unit) {
            prev = unit_list_append(prev, unit);
            if(!mdl->business_doors) {
                mdl->business_doors = prev;
            }
        }
    }

    // Exit business doors array container
    r = sd_bus_message_exit_container(msg);
    if(r < 0) {
        selfLogErr("Close business doors array error(%d): %s", r, strerror(-r));
        return r;
    }


    // Enter general doors array container
    r = sd_bus_message_enter_container(msg, SD_BUS_TYPE_ARRAY, UNIT_STRUCT_SIGNATURE);
    if(r < 0) {
        // Log error
        selfLogErr("Enter general doors array error(%d): %s", r, strerror(-r));
        return r;
    }

    prev = NULL;
    while (sd_bus_message_at_end(msg, FALSE) == 0) {
        MdlUnit *unit = unit_parse_message(msg, FALSE);
        if(unit) {
            prev = unit_list_append(prev, unit);
            if(!mdl->general_doors) {
                mdl->general_doors = prev;
            }
        }
    }

    // Exit general doors array container
    r = sd_bus_message_exit_container(msg);
    if(r < 0) {
        selfLogErr("Close general doors array error(%d): %s", r, strerror(-r));
        return r;
    }


    selfLogInf("Parser finished id=%d, bid=%d, pubName=%s", mdl->id, mdl->buildingId, mdl->publicName);
    return 0;
}