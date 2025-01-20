int high_scores();
void Flash_ReadArray(uint32_t address, uint64_t *data, uint32_t length);
HAL_StatusTypeDef Flash_WriteArray(uint32_t address, uint64_t *data, uint32_t length);

#define FLASH_USER_START_ADDR   0x08080000
