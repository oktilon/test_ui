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

    mdl_unit_list_clean(model->business_doors);
    g_free(model->business_doors);

    mdl_unit_list_clean(model->general_doors);
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
        ret->business_doors = mdl_unit_list_clone(model->business_doors);
        ret->general_doors = mdl_unit_list_clone(model->general_doors);
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
}