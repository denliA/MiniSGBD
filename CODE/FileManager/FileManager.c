/*
 * FileManager.c
 *
 *  Created on: 12 oct. 2021
 *      Author: ij07411
 */

PageId readPageIdFromPageBuffer(uint8_t *buff, uint8_t first){
	void *debut = buff + sizeof(PageId)*first;
	PageId *ptr = (PageId*)debut;
	return *ptr;
}
