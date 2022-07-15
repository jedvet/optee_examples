
#ifndef __RING_TEE_STORAGE_H__
#define __RING_TEE_STORAGE_H__

/* UUID of the trusted application */
#define TA_RING_TEE_STORAGE_UUID \
		{ 0xdeeacf00, 0xeec0, 0x4f40, \
			{ 0x97, 0x75, 0xd0, 0x32, 0xa5, 0x27, 0x65, 0x99 } }
/*
 * TA_SECURE_STORAGE_CMD_READ_RAW - Create and fill a secure storage file
 * param[0] (memref) ID used the identify the persistent object
 * param[1] (memref) Raw data dumped from the persistent object
 * param[2] unused
 * param[3] unused
 */
#define TA_SECURE_STORAGE_CMD_READ_RAW		0

/*
 * TA_SECURE_STORAGE_CMD_WRITE_RAW - Create and fill a secure storage file
 * param[0] (memref) ID used the identify the persistent object
 * param[1] (memref) Raw data to be writen in the persistent object
 * param[2] unused
 * param[3] unused
 */
#define TA_SECURE_STORAGE_CMD_WRITE_RAW		1

/*
 * TA_SECURE_STORAGE_CMD_DELETE - Delete a persistent object
 * param[0] (memref) ID used the identify the persistent object
 * param[1] unused
 * param[2] unused
 * param[3] unused
 */
#define TA_SECURE_STORAGE_CMD_DELETE		2

#endif /* __RING_TEE_STORAGE_H__ */
