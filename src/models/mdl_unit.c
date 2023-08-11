#include "models/mdl_unit.h"

#include "app.h"

void unit_list_clean(MdlUnitList *units) {
    MdlUnitList *it = units;
    while(it) {
        if(it->unit) {
            unit_clean(it->unit);
        }
        it = it->next;
    }
}

void unit_clean(MdlUnit *unit) {
    if(!unit) return;

    g_free(unit->imgUrl);
    g_free(unit->header);
    g_free(unit->text);
    unit_list_clean(unit->members);
    g_free(unit->members);

    g_free(unit);
}

MdlUnit *unit_clone(MdlUnit *unit) {
    MdlUnit *ret = (MdlUnit *) g_malloc0(sizeof(MdlUnit));
    ret->roomId = unit->roomId;
    ret->userId = unit->userId;
    ret->number = unit->number;
    ret->sequentialNumber = unit->sequentialNumber;
    ret->callTimeOut = unit->callTimeOut;
    ret->type = unit->type;
    ret->imgUrl = g_strdup_printf("%s", unit->imgUrl);
    ret->header = g_strdup_printf("%s", unit->header);
    ret->text = g_strdup_printf("%s", unit->text);
    ret->unitConfig.config = unit->unitConfig.config;
    ret->daysOfWeek.fullWeek = unit->daysOfWeek.fullWeek;
    ret->startTime = unit->startTime;
    ret->endTime = unit->endTime;
    ret->members = unit_list_clone(unit->members);

    return ret;
}

MdlUnitList *unit_list_clone(MdlUnitList *units) {
    gsize count = 0;
    MdlUnitList *ret = NULL,
                *prev = NULL,
                *cur,
                *it = units;
    while(it) {
        cur = (MdlUnitList *) g_malloc0_n(++count, sizeof(MdlUnitList));
        if(prev) {
            prev->next = ret;
        }
        if(!ret) {
            ret = cur;
        }
        cur->unit = unit_clone(it->unit);
        it = it->next;
        prev = cur;
    }

    return ret;
}

MdlUnitList *unit_list_append(MdlUnitList *list, MdlUnit *unit) {
    MdlUnitList *ret = (MdlUnitList *) g_malloc0(sizeof(MdlUnitList));
    ret->unit = unit;
    if(list) {
        list->next = ret;
    }
    return ret;
}

MdlUnit *unit_parse_message(sd_bus_message *msg, int isMember) {
    // Variable
    int r;
    const char *signature = UNIT_DATA_SIGNATURE;

    // Prepare Unit signature
    if(isMember) {
        signature = UNIT_BASE_SIGNATURE;
    }

    // Create business doors array container
    r = sd_bus_message_enter_container(msg, SD_BUS_TYPE_STRUCT, signature);
    if(r < 0) {
        // Log error
        selfLogErr("Enter unit struct container error(%d): %s", r, strerror(-r));
        return NULL;
    }

    MdlUnit *mdl = (MdlUnit*) g_malloc0(sizeof(MdlUnit));
    if(!mdl) {
        // Log error
        selfLogErr("Allocate unit struct error(%d): %m", errno);
        return NULL;
    }

    // Read doorbell self data
    r = sd_bus_message_read(msg, UNIT_BASE_SIGNATURE
        , &(mdl->roomId)
        , &(mdl->userId)
        , &(mdl->number)
        , &(mdl->sequentialNumber)
        , &(mdl->callTimeOut)
        , &(mdl->type)
        , &(mdl->imgUrl)
        , &(mdl->header)
        , &(mdl->text)
        , &(mdl->unitConfig.config)
        , &(mdl->daysOfWeek.fullWeek)
        , &(mdl->startTime)
        , &(mdl->endTime)
    );
    if(r < 0) {
        selfLogErr("Read message error(%d): %s", r, strerror(-r));
    }

    if(!isMember) {
        // Enter members array container
        r = sd_bus_message_enter_container(msg, SD_BUS_TYPE_ARRAY, UNIT_MEMBER_STRUCT_SIGNATURE);
        if(r < 0) {
            // Log error
            selfLogErr("Enter unit members array error(%d): %s", r, strerror(-r));
            return NULL;
        }

        MdlUnitList *prev = NULL;
        while (sd_bus_message_at_end(msg, FALSE) == 0) {
            MdlUnit *unit = unit_parse_message(msg, TRUE);
            if(unit) {
                prev = unit_list_append(prev, unit);
                if(!mdl->members) {
                    mdl->members = prev;
                }
            }
        }

        // Exit members array container
        r = sd_bus_message_exit_container(msg);
        if(r < 0) {
            selfLogErr("Exit unit members array error(%d): %s", r, strerror(-r));
            return NULL;
        }

    }

    // Exit memebers array container
    r = sd_bus_message_exit_container(msg);
    if(r < 0) {
        selfLogErr("Close business doors array error(%d): %s", r, strerror(-r));
        return NULL;
    }


    selfLogInf("Door parsed roomId=%d, hdr=%s, txt=%s", mdl->roomId, mdl->header, mdl->text);
    return mdl;
}