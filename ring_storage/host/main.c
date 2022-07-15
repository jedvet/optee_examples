
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* TA API: UUID and command IDs */
#include <ring_tee_storage_ta.h>

#define TEST_OBJECT_SIZE	5000
#define TEE_OBJ_ID          "ring-tee-file"

/* TEE resources */
struct ta_priv {
	TEEC_Context ctx;
	TEEC_Session sess;
};

struct app_opts {
    char *str;
    bool show_help;
    bool tee_read;
    bool tee_write;
    bool tee_delete;
};

static void app_usage(char *argv0)
{
    printf("(c) ring ukraine - tee storage\n\n");
    printf("built:\n\t%s %s\n", __DATE__, __TIME__);
    printf("usage:\n");
    printf("\t%s -h\n", argv0);
}

static int app_parse_opts(int argc, char *argv[], struct app_opts *opts)
{
    int opt;

    memset(opts, 0, sizeof(struct app_opts));

    while ((opt = getopt(argc, argv, "dhrw:")) != -1) {
        switch (opt) {
        case 'r':
            opts->tee_read = true;
            break;

        case 'w':
            opts->tee_write = true;
            opts->str = optarg;
            break;

        case 'd':
            opts->tee_delete = true;
            break;

        case 'h':
            opts->show_help = true;
            break;

        default:
            fprintf(stderr, "try '%s -h' for more options.\n", argv[0]);
            return -1;
        }
    }
}

void open_tee_session(struct ta_priv *priv)
{
	TEEC_UUID uuid = TA_RING_TEE_STORAGE_UUID;
	uint32_t origin;
	TEEC_Result res;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &priv->ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x\n", res);

	/* Open a session with the TA */
	res = TEEC_OpenSession(&priv->ctx, &priv->sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x\n",
			res, origin);
}

void close_tee_session(struct ta_priv *priv)
{
	TEEC_CloseSession(&priv->sess);
	TEEC_FinalizeContext(&priv->ctx);
}

TEEC_Result read_secure_object(struct ta_priv *priv, char *id,
			char *data, size_t data_len)
{
	TEEC_Operation op;
	TEEC_Result res;
    uint32_t origin;
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_OUTPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	op.params[1].tmpref.buffer = data;
	op.params[1].tmpref.size = data_len;

	res = TEEC_InvokeCommand(&priv->sess,
				 TA_SECURE_STORAGE_CMD_READ_RAW,
				 &op, &origin);
	switch (res) {
	case TEEC_SUCCESS:
	case TEEC_ERROR_SHORT_BUFFER:
	case TEEC_ERROR_ITEM_NOT_FOUND:
		break;
	default:
		printf("Command READ_RAW failed: 0x%x / %u\n", res, origin);
	}

	return res;
}

TEEC_Result write_secure_object(struct ta_priv *priv, char *id,
			char *data, size_t data_len)
{
    TEEC_Operation op;
    TEEC_Result res;
    uint32_t origin; 
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	op.params[1].tmpref.buffer = data;
	op.params[1].tmpref.size = data_len;

	res = TEEC_InvokeCommand(&priv->sess,
				 TA_SECURE_STORAGE_CMD_WRITE_RAW,
				 &op, &origin);
	if (res != TEEC_SUCCESS)
		printf("Command WRITE_RAW failed: 0x%x / %u\n", res, origin);

	switch (res) {
	case TEEC_SUCCESS:
		break;
	default:
		printf("Command WRITE_RAW failed: 0x%x / %u\n", res, origin);
	}

	return res;
}

TEEC_Result delete_secure_object(struct ta_priv *priv, char *id)
{
	TEEC_Operation op;
	TEEC_Result res;
    uint32_t origin; 
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_NONE, TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	res = TEEC_InvokeCommand(&priv->sess,
				 TA_SECURE_STORAGE_CMD_DELETE,
				 &op, &origin);

	switch (res) {
	case TEEC_SUCCESS:
	case TEEC_ERROR_ITEM_NOT_FOUND:
		break;
	default:
		printf("Command DELETE failed: 0x%x / %u\n", res, origin);
	}

	return res;
}


int main(int argc, char *argv[])
{
	struct ta_priv ctx;
    struct app_opts opts;
	char obj_id[] = TEE_OBJ_ID;		    /* string identification for the object */
	char read_data[TEST_OBJECT_SIZE];
	TEEC_Result res;

    app_parse_opts(argc, argv, &opts);

    if (opts.show_help || !argc) {
        app_usage(argv[0]);
        return EXIT_SUCCESS;
    }

	open_tee_session(&ctx);

    /* read string */
    if (opts.tee_read) {
        memset(read_data, 0, sizeof(read_data));
        res = read_secure_object(&ctx, obj_id, read_data, sizeof(read_data));
        if (res == TEEC_SUCCESS) {
            printf("%s", read_data);
        }
        else if (res == TEEC_ERROR_ITEM_NOT_FOUND) {
            printf("[err]: object not found in TA secure storage\n");
        }
        else {
            errx(1, "[err]: unexpected status when reading an object : 0x%x\n", res);
        }
    }

    /* write string */
    if (opts.tee_write) {
        res = write_secure_object(&ctx, obj_id, opts.str, strlen(opts.str));
        if (res != TEEC_SUCCESS)
        	errx(1, "[err]: failed to write an object in the secure storage\n");
    }

    /* delete the file */
    if (opts.tee_delete) {
        res = delete_secure_object(&ctx, obj_id);
    	if (res != TEEC_SUCCESS)
    		errx(1, "[err]: failed to delete the object: 0x%x\n", res);
    }

	close_tee_session(&ctx);
	return res;
}
