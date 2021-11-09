#include "../DiskManager/PageId.h"
#include "../FileManager/RelationInfo.h"

#ifndef CODE_RELATIONMANAGER_CREATERELATIONCOMMAND_H_
#define CODE_RELATIONMANAGER_CREATERELATIONCOMMAND_H_

typedef struct _RelCom{
	char *name;
	uint32_t nbCol;
	char **colNames;
	ColType *colTypes;
    uint32_t size;
    uint32_t *colOffset;
    PageId headerPage;
    uint32_t slotCount;

} RelationCommand;



#endif /* CODE_RELATIONMANAGER_CREATERELATIONCOMMAND_H_ */
