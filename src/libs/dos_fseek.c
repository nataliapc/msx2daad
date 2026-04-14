#include "dos.h"
#include "utils.h"


uint32_t fseek (uint32_t offset, char origin)
{
    FCB *fcb = (FCB*)SYSFCB;

    if (origin==SEEK_SET) {
        fcb->rndRecord = offset;
    } else
    if (origin==SEEK_CUR) {
        fcb->rndRecord += offset;
    } else if (origin==SEEK_END) {
        fcb->rndRecord = fcb->fileSize + offset;
    } else {
        return -1L;   // Devuelve error en vez de die()
    }
    return fcb->rndRecord;
}
