#include "models/mdl_unit.h"

void mdl_unit_list_clean(MdlUnitList *units) {
    MdlUnitList *it = units;
    while(it) {
        if(it->unit) {
            mdl_unit_clean(it->unit);
        }
        it = it->next;
    }
}

void mdl_unit_clean(MdlUnit *unit) {
    if(!unit) return;

    g_free(unit->imgUrl);
    g_free(unit->header);
    g_free(unit->text);
    mdl_unit_list_clean(unit->members);
    g_free(unit->members);

    g_free(unit);
}

MdlUnit *mdl_unit_clone(MdlUnit *unit) {
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
    ret->members = mdl_unit_list_clone(unit->members);

    return ret;
}

MdlUnitList *mdl_unit_list_clone(MdlUnitList *units) {
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
        cur->unit = mdl_unit_clone(it->unit);
        it = it->next;
        prev = cur;
    }

    return ret;
}