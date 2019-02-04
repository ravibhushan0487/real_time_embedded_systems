

#define __EEPROM_24FC256_DEFS_H__

#define EEPROM_24FC256_RDID_VALUE		(0x00ef4015)
#define EEPROM_24FC256_MAX_LEN_REG_CMD      (6)
#define EEPROM_24FC256_OPCODE_LEN           (1)
#define EEPROM_24FC256_ADDRESS_WIDTH        (3)
#define EEPROM_24FC256_LEN_CMD_ADDRESS      (4)
#define EEPROM_24FC256_LEN_CMD_AND_ID       (4)

/* relevant status register bits */
#define EEPROM_24FC256_WIP_BIT         (0x1 << 0)
#define EEPROM_24FC256_WEL_BIT         (0x1 << 1)
#define EEPROM_24FC256_SRWD_BIT        (0x1 << 7)
#define EEPROM_24FC256_TB_BIT          (0x1 << 3)
#define EEPROM_24FC256_SR_BP_OFFSET    (2)

/* relevant security register bits */
#define EEPROM_24FC256_SECR_WPSEL_BIT  (0x1 << 7)
#define EEPROM_24FC256_SECR_EFAIL_BIT  (0x1 << 6)
#define EEPROM_24FC256_SECR_PFAIL_BIT  (0x1 << 5)

/* supported erase size */
#define EEPROM_24FC256_SECTOR_SIZE     (0x1000)
#define EEPROM_24FC256_BLOCK32K_SIZE   (0x8000)
#define EEPROM_24FC256_BLOCK_SIZE      (0x10000)

#define EEPROM_24FC256_SECTOR_MASK     (0xFFF)

/* ID comands */
#define EEPROM_24FC256_CMD_RDID        0x9F
#define EEPROM_24FC256_CMD_RES         0xAB
#define EEPROM_24FC256_CMD_REMS        0x90
#define EEPROM_24FC256_CMD_QPIID       0xAF
#define EEPROM_24FC256_CMD_UNID        0x4B

/*Register comands */
#define EEPROM_24FC256_CMD_WRSR        0x01
#define EEPROM_24FC256_CMD_RDSR        0x05
#define EEPROM_24FC256_CMD_RDSR2       0x35
#define EEPROM_24FC256_CMD_WRSCUR      0x2F
#define EEPROM_24FC256_CMD_RDSCUR      0x48

/* READ comands */
#define EEPROM_24FC256_CMD_READ        0x03
#define EEPROM_24FC256_CMD_2READ       0xBB
#define EEPROM_24FC256_CMD_4READ       0xEB
#define EEPROM_24FC256_CMD_FASTREAD    0x0B
#define EEPROM_24FC256_CMD_DREAD       0x3B
#define EEPROM_24FC256_CMD_QREAD       0x6B
#define EEPROM_24FC256_CMD_RDSFDP      0x5A

/* Program comands */
#define EEPROM_24FC256_CMD_WREN        0x06
#define EEPROM_24FC256_CMD_WRDI        0x04
#define EEPROM_24FC256_CMD_PP          0x02
#define EEPROM_24FC256_CMD_4PP         0x32
#define EEPROM_24FC256_CMD_WRENVSR     0x50

/* Erase comands */
#define EEPROM_24FC256_CMD_SE          0x20
#define EEPROM_24FC256_CMD_BE32K       0x52
#define EEPROM_24FC256_CMD_BE          0xD8
#define EEPROM_24FC256_CMD_CE          0x60

/* Mode setting comands */
#define EEPROM_24FC256_CMD_DP          0xB9
#define EEPROM_24FC256_CMD_RDP         0xAB

/* Reset comands */
#define EEPROM_24FC256_CMD_RSTEN       0x66
#define EEPROM_24FC256_CMD_RST         0x99
#define EEPROM_24FC256_CMD_RSTQIO      0xF5

/* Security comands */
#define EEPROM_24FC256_CMD_ERSR        0x44
#define EEPROM_24FC256_CMD_PRSR        0x42

/* Suspend/Resume comands */
#define EEPROM_24FC256_CMD_PGM_ERS_S   0x75
#define EEPROM_24FC256_CMD_PGM_ERS_R   0x7A

#define EEPROM_24FC256_CMD_NOP         0x00
