#include "kylin_sane.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  uint8_t *data;
  int width;
  int height;
  int x;
  int y;
}
Image;

#define STRIP_HEIGHT	256
static SANE_Handle device = NULL;
static int verbose;
static int progress = 0;
static SANE_Byte *buffer;
static size_t buffer_size;

/* -------------------------------------------- */
// 设置n的i位为1，i从0开始
#define SET_1_BIT(n,i) ((1<<(i))|(n))   

// 设置n的i位为0，i从0开始
#define SET_0_BIT(n,i) ((~(1<<(i)))&(n))

// 设置n的i位为反(01互换)，i从0开始
#define SET_R_BIT(n,i) ((n)^(1<<(i)))

// 获取n的i位数值，i从0开始
#define GET_i_BIT(n,i) (((n)>>(i))&1)

static void
auth_callback (SANE_String_Const resource,
	       SANE_Char * username, SANE_Char * password)
{
}

static void write_pnm_header (SANE_Frame format, int width, int height, int depth, FILE *ofp)
{
    switch (format)
    {
        case SANE_FRAME_RED:
        case SANE_FRAME_GREEN:
        case SANE_FRAME_BLUE:
        case SANE_FRAME_RGB:
            fprintf (ofp, "P6\n# SANE data follows\n%d %d\n%d\n", width, height, (depth <= 8) ? 255 : 65535);
            break;
        default:
            if (depth == 1)
                fprintf (ofp, "P4\n# SANE data follows\n%d %d\n", width, height);
            else
                fprintf (ofp, "P5\n# SANE data follows\n%d %d\n%d\n", width, height,(depth <= 8) ? 255 : 65535);
            break;
    }
}

static SANE_Status scan_it (FILE *ofp)
{
    int i, len, first_frame = 1, offset = 0, must_buffer = 0, hundred_percent;
    SANE_Byte min = 0xff, max = 0;
    SANE_Parameters parm;
    SANE_Status status;
    Image image = { 0, 0, 0, 0, 0 };
    static const char *format_name[] = {"gray", "RGB", "red", "green", "blue"};
    SANE_Word total_bytes = 0, expected_bytes;
    SANE_Int hang_over = -1;

    do
    {
        if (!first_frame)
        {
            status = sane_start (device);
            if (status != SANE_STATUS_GOOD)
            {
                goto cleanup;
            }
        }

        status = sane_get_parameters (device, &parm);
		fprintf (stderr, "Parm : stat=%s form=%d,lf=%d,bpl=%d,pixpl=%d,lin=%d,dep=%d\n",
			sane_strstatus (status),
			parm.format, parm.last_frame,
			parm.bytes_per_line, parm.pixels_per_line,
			parm.lines, parm.depth);

        if (status != SANE_STATUS_GOOD)
        {
          goto cleanup;
        }

        if (first_frame)
        {
            if (parm.lines >= 0)
            {
                 fprintf (stderr, "scanning image of size %dx%d pixels at %d bits/pixel\n",
                      parm.pixels_per_line, parm.lines,
                      parm.depth * (SANE_FRAME_RGB == parm.format ? 3 : 1));
            }
           else
           {
                 fprintf (stderr, "scanning image %d pixels wide and "
                      "variable height at %d bits/pixel\n",
                      parm.pixels_per_line,
                      parm.depth * (SANE_FRAME_RGB == parm.format ? 3 : 1));
           }
            switch (parm.format)
            {
                case SANE_FRAME_RED:
                case SANE_FRAME_GREEN:
                case SANE_FRAME_BLUE:
                  assert (parm.depth == 8);
                  must_buffer = 1;
                  offset = parm.format - SANE_FRAME_RED;
                  break;
                case SANE_FRAME_RGB:
                  printf("SANE_FRAME_RGB\n");
                  assert ((parm.depth == 8) || (parm.depth == 16));

                case SANE_FRAME_GRAY:
                    assert ((parm.depth == 1) || (parm.depth == 8) || (parm.depth == 16));
                    if (parm.lines < 0)
                    {
                        printf("parm.format = SANE_FRAME_GRAY, parm.lines < 0\n");
                        must_buffer = 1;
                        offset = 0;
                    }
                    else
                    {
                        printf("SANE_FRAME_GRAY\n");
                        write_pnm_header (parm.format, parm.pixels_per_line, parm.lines, parm.depth, ofp);
                    }
                  break;
                default:
                  break;
            }

            if (must_buffer)
            {
                /**
                 * We're either scanning a multi-frame image or the
                 * scanner doesn't know what the eventual image height
                 * will be (common for hand-held scanners).  In either
                 * case, we need to buffer all data before we can write
                 * the image.
                 */
                image.width = parm.bytes_per_line;
                if (parm.lines >= 0)
                    image.height = parm.lines - STRIP_HEIGHT + 1;
                else
                    image.height = 0;

                image.x = image.width - 1;
                image.y = -1;
                /*
                if (!advance (&image))
                {
                    status = SANE_STATUS_NO_MEM;
                    goto cleanup;
                }
                */
            }
        }
        else
        {
            assert (parm.format >= SANE_FRAME_RED && parm.format <= SANE_FRAME_BLUE);
            offset = parm.format - SANE_FRAME_RED;
            image.x = image.y = 0;
        }

        hundred_percent = parm.bytes_per_line * parm.lines * ((parm.format == SANE_FRAME_RGB || parm.format == SANE_FRAME_GRAY) ? 1:3);

        while (1)
        {
            double progr;
            status = sane_read (device, buffer, buffer_size, &len);
            total_bytes += (SANE_Word) len;
            progr = ((total_bytes * 100.) / (double) hundred_percent);
            if (progr > 100.)
                progr = 100.;

            if (status != SANE_STATUS_GOOD)
            {
                if (status != SANE_STATUS_EOF)
                {
                    return status;
                }
                break;
            }

            if (must_buffer)
            {
                printf("must_buffer = %d\n", must_buffer);
                switch (parm.format)
                {
                    case SANE_FRAME_RED:
                    case SANE_FRAME_GREEN:
                    case SANE_FRAME_BLUE:
                        for (i = 0; i < len; ++i)
                        {
                            image.data[offset + 3 * i] = buffer[i];
                            /*
                            if (!advance (&image))
                            {
                                status = SANE_STATUS_NO_MEM;
                                goto cleanup;
                            }
                            */
                        }
                        offset += 3 * len;
                        break;
                    case SANE_FRAME_RGB:
                        for (i = 0; i < len; ++i)
                        {
                            image.data[offset + i] = buffer[i];
                            /*
                            if (!advance (&image))
                            {
                                status = SANE_STATUS_NO_MEM;
                                goto cleanup;
                            }
                            */
                        }
                        offset += len;
                        break;
                    case SANE_FRAME_GRAY:
                        for (i = 0; i < len; ++i)
                        {
                            image.data[offset + i] = buffer[i];
                            /*
                            if (!advance (&image))
                            {
                                status = SANE_STATUS_NO_MEM;
                                goto cleanup;
                            }
                            */
                        }
                        offset += len;
                        break;
                    default:
                        break;
                }
            }
            else			/* ! must_buffer */
            {
                if ((parm.depth != 16)) 
                    fwrite (buffer, 1, len, ofp);
                else
                {
#if !defined(WORDS_BIGENDIAN)
                    int i, start = 0;
                    /* check if we have saved one byte from the last sane_read */
                    if (hang_over > -1)
                    {
                        if (len > 0)
                        {
                            fwrite (buffer, 1, 1, ofp);
                            buffer[0] = (SANE_Byte) hang_over;
                            hang_over = -1;
                            start = 1;
                        }
                    }
                    /* now do the byte-swapping */
                    for (i = start; i < (len - 1); i += 2)
                    {
                        unsigned char LSB;
                        LSB = buffer[i];
                        buffer[i] = buffer[i + 1];
                        buffer[i + 1] = LSB;
                    }
                    /* check if we have an odd number of bytes */
                    if (((len - start) % 2) != 0)
                    {
                        hang_over = buffer[len - 1];
                        len--;
                    }
#endif
                    fwrite (buffer, 1, len, ofp);
                }
            }

            if (verbose && parm.depth == 8)
            {
              for (i = 0; i < len; ++i)
                if (buffer[i] >= max)
                    max = buffer[i];
                else if (buffer[i] < min)
                    min = buffer[i];
            }
        }
        first_frame = 0;
    }while (!parm.last_frame);

    if (must_buffer)
    {
        image.height = image.y;
        write_pnm_header (parm.format, parm.pixels_per_line,image.height, parm.depth, ofp);

#if !defined(WORDS_BIGENDIAN)
        if (parm.depth == 16)
        {
            int i;
            for (i = 0; i < image.height * image.width; i += 2)
            {
                unsigned char LSB;
                LSB = image.data[i];
                image.data[i] = image.data[i + 1];
                image.data[i + 1] = LSB;
            }
        }
#endif
        fwrite (image.data, 1, image.height * image.width, ofp);
    }

    fflush( ofp );

cleanup:
    if (image.data)
        free (image.data);

    return status;
}


SANE_Status kylin_sane_get_parameters(SANE_Handle device)
{
    SANE_Status status;
    SANE_Parameters parm;

    status = sane_get_parameters (device, &parm);
    fprintf (stderr, "Parm : stat=%s form=%d,lf=%d,bpl=%d,pixpl=%d,lin=%d,dep=%d\n",
        sane_strstatus (status),
        parm.format, parm.last_frame,
        parm.bytes_per_line, parm.pixels_per_line,
        parm.lines, parm.depth);

    return status;
}

SANE_Status do_scan(const char *fileName)
{
	SANE_Status status;
	FILE *ofp = NULL;
	char path[PATH_MAX];
	char part_path[PATH_MAX];
	buffer_size = (32 * 1024);
    buffer = (SANE_Byte*)malloc (buffer_size);

	do
	{
        int dwProcessID = getpid();
        sprintf (path, "%s%d.pnm", fileName, dwProcessID);
        strcpy (part_path, path);
        strcat (part_path, ".part");

        printf("picture name: %s\n", path);

		status = sane_start (device);
		if (status != SANE_STATUS_GOOD)
		{
			break;
		}

        if (NULL == (ofp = fopen (part_path, "w")))
        {
            status = SANE_STATUS_ACCESS_DENIED;
            break;
        }

		status = scan_it (ofp);

		switch (status)
		{
			case SANE_STATUS_GOOD:
			case SANE_STATUS_EOF:
                 {
                      status = SANE_STATUS_GOOD;
                      if (!ofp || 0 != fclose(ofp))
                      {
                          status = SANE_STATUS_ACCESS_DENIED;
                          break;
                      }
                      else
                      {
                          ofp = NULL;
                          if (rename (part_path, path))
                          {
                              status = SANE_STATUS_ACCESS_DENIED;
                              break;
                          }
                      }
                  }
				  break;
			default:
                  break;
		}
	}while (0);

    if (SANE_STATUS_GOOD != status)
    {
        sane_cancel (device);
    }
    if (ofp)
    {
        fclose (ofp);
        ofp = NULL;
    }
    if (buffer)
    {
        free (buffer);
        buffer = NULL;
    }

    return status;
}

// Initialize SANE
//SANE初始化
void init()
{
    SANE_Int version_code = 0;
	sane_init (&version_code, auth_callback);
    printf("SANE version code: %d\n", version_code);
}

// Get all devices
//查询所有连接设备。这里会比较耗时
SANE_Status get_devices(const SANE_Device ***device_list)
{
    printf("Get all devices...\n");
	SANE_Status sane_status;
	if (sane_status = sane_get_devices (device_list, SANE_FALSE))
	{
		printf("sane_get_devices status: %s\n", sane_strstatus(sane_status));
	}	
    return sane_status;
}

// Open a device
//使用设备名字打开设备
SANE_Status open_device(SANE_Device *device, SANE_Handle *sane_handle)
{
    SANE_Status sane_status;

    printf("Name: %s, vendor: %s, model: %s, type: %s\n",
		 device->name, device->model, device->vendor, device->type);

    if (sane_status = sane_open(device->name, sane_handle))
    {
        printf("sane_open status: %s\n", sane_strstatus(sane_status));
    }

    return sane_status;
}

/**
 * Option 2
 * get all colors
 * print all colors: Color Gray Lineart (etc.)
 */
int get_option_colors(SANE_Handle sane_handle, int optnum)
{
    int ret = 0;

    SANE_Status status;
    const SANE_Option_Descriptor *opt;
    int i = 0;

    printf("begin get option[%d] colors\n", optnum);

    opt = sane_get_option_descriptor(sane_handle, optnum);

    printf("begin print all colors:\n");
	for(i=0; opt->constraint.string_list[i] != NULL; i++)
	{
        printf("optnum[%d] colors string: %s \n", optnum, *(opt->constraint.string_list+i));
        if(!strcmp("Color", *(opt->constraint.string_list+i)))
        {
            SET_1_BIT(ret, 0);
        }
        if(!strcmp("Gray", *(opt->constraint.string_list+i)))
        {
            SET_1_BIT(ret, 1);
        }
        if(!strcmp("Lineart", *(opt->constraint.string_list+i)))
        {
            SET_1_BIT(ret, 2);
        }
    }
    return ret;
}

/**
 * Option 2
 * set selected color
 */
SANE_Status set_option_colors(SANE_Handle sane_handle, int optnum, SANE_String val_color)
{
    SANE_Status status;
    const SANE_Option_Descriptor *opt;

    printf("\nbegin set option[%d] color: %s \n", optnum, val_color);

    status = sane_control_option(sane_handle, optnum, SANE_ACTION_SET_VALUE, val_color, NULL);
    if (status != SANE_STATUS_GOOD)
	{
		printf("Option did not set\n");
        return status;
    }

    printf("set color option success!\n\n");
    return status;
}


/**
 * Option 3
 * get all sources
 * print all sources: source Gray Lineart (etc.)
 */
int get_option_sources(SANE_Handle sane_handle, int optnum)
{
    int ret = 0;
    SANE_Status status;
    const SANE_Option_Descriptor *opt;
    int i = 0;

    printf("begin get option[%d] sources\n", optnum);

    opt = sane_get_option_descriptor(sane_handle, optnum);

    printf("begin print all sources:\n");
	for(i=0; opt->constraint.string_list[i] != NULL; i++)
	{
        printf("optnum[%d] sources string: %s \n", optnum, *(opt->constraint.string_list+i));
        if(!strcmp("Flatbed", *(opt->constraint.string_list+i)))
        {
            SET_1_BIT(ret, 0);
        }
    }
    return ret;
}

/**
 * Option 2
 * set selected source
 */
SANE_Status set_option_sources(SANE_Handle sane_handle, int optnum, SANE_String val_source)
{
    SANE_Status status;
    const SANE_Option_Descriptor *opt;

    printf("begin set option[%d] source: %s \n", optnum, val_source);

    status = sane_control_option(sane_handle, optnum, SANE_ACTION_SET_VALUE, val_source, NULL);
    if (status != SANE_STATUS_GOOD)
	{
		printf("Option did not set\n");
        return status;
    }

    printf("set source option success!\n\n");
    return status;
}


/**
 * Option 6
 * get all resolutions[6] option
 * print all resolution: 4800 2400 1200 600 300 150 100 75 (etc.)
 */
void get_option_resolutions(SANE_Handle sane_handle, int optnum)
{
    SANE_Int info;
    SANE_Status status;
    const SANE_Option_Descriptor *opt;
    int i = 0;

    printf("begin get option[%d] resolution \n", optnum);

    opt = sane_get_option_descriptor(sane_handle, optnum);

    printf("begin print all resolutions:\n");
	for(i=0; opt->constraint.word_list[i]; i++)
	{
        printf("optnum[%d] resolutions int: %d \n", optnum, *(opt->constraint.word_list+i));
    }
}

/**
 * Option 6
 * set selected resolutions
 */
SANE_Status set_option_resolutions(SANE_Handle sane_handle, int optnum, SANE_Int val_resolution)
{
    SANE_Word dpi;
    SANE_Status status;
    const SANE_Option_Descriptor *opt;

    printf("\nbegin set option[%d] resolution: %d \n", optnum, val_resolution);

    status = sane_control_option(sane_handle, optnum, SANE_ACTION_SET_VALUE, &val_resolution, NULL);
    if (status != SANE_STATUS_GOOD)
	{
		printf("Option did not set\n");
        return status;
    }

    printf("set resolution option success!\n\n");
    return status;
}


/**
 * Option 8(Top-left x),9(Top-left y),10(Bottom-right x),11(Bottom-right y)
 * get all sizes[8,9,10,11] option
 * print all size: tl-x tl-y br-x br-y (etc.)
 * A3: 0 0 297 420
 * A4: 0 0 210 297 (canon 210=> 0 0 216 297)
 */
void get_option_sizes(SANE_Handle sane_handle, int optnum)
{
    SANE_Int info;
    SANE_Status status;
    const SANE_Option_Descriptor *opt;
    int i = 0;

    printf("begin get option[%d] size \n", optnum);

    opt = sane_get_option_descriptor(sane_handle, optnum);

    printf("begin print all sizes:\n");
	for(i=0; opt->constraint.word_list[i]; i++)
	{
        printf("optnum[%d] sizes int: %d \n", optnum, *(opt->constraint.word_list+i));
    }
}

/**
 * Option 6
 * set selected sizes
 */
SANE_Status set_option_sizes(SANE_Handle sane_handle, int optnum, SANE_Int val_size)
{
    SANE_Word dpi;
    SANE_Status status;
    const SANE_Option_Descriptor *opt;

    printf("\nbegin set option[%d] size: %d \n", optnum, val_size);

    status = sane_control_option(sane_handle, optnum, SANE_ACTION_SET_VALUE, &val_size, NULL);
    if (status != SANE_STATUS_GOOD)
	{
		printf("Option did not set\n");
        return status;
    }

    printf("set size option success!\n\n");
    return status;
}

/**
 * @brief set_option_sizes_real
 * @param sane_handle sane_handle
 * @param val_size_br_x bottom_right x coordition
 * @param val_size_br_y botton_right y coordition
 * @return SANE_STATUS
 */
SANE_Status set_option_sizes_real(SANE_Handle sane_handle, SANE_Int val_size_br_x, SANE_Int val_size_br_y)
{
    SANE_Status status = SANE_STATUS_GOOD;
    printf("size Bottom-right xy=[%d, %d]\n", val_size_br_x, val_size_br_y);

    status = set_option_sizes(sane_handle, 10, SANE_FIX(val_size_br_x));
    status = set_option_sizes(sane_handle, 11, SANE_FIX(val_size_br_y));

    return status;
}

SANE_Status set_option_sizes_all(SANE_Handle sane_handle, int type)
{
    SANE_Status status = SANE_STATUS_GOOD;

    switch (type) {
    case A2:
        status = set_option_sizes_real(sane_handle, 420, 594);
        break;
    case A3:
        status = set_option_sizes_real(sane_handle, 297, 420);
        break;
    case A4:
        status = set_option_sizes_real(sane_handle, 210, 297);
        break;
    case A5:
        status = set_option_sizes_real(sane_handle, 148, 210);
        break;
    case A6:
        status = set_option_sizes_real(sane_handle, 105, 144);
        break;
    default:
        status = SANE_STATUS_UNSUPPORTED;
    }

    return status;
}


/*--------------------------------------------------------------------------*/

#define GUARDS_SIZE 4			/* 4 bytes */
#define GUARD1 ((SANE_Word)0x5abf8ea5)
#define GUARD2 ((SANE_Word)0xa58ebf5a)

/* Allocate the requested memory plus enough room to store some guard bytes. */
static void *guards_malloc(size_t size)
{
	unsigned char *ptr;

	size += 2*GUARDS_SIZE;
	ptr = (unsigned char *)malloc(size);

	assert(ptr);

	ptr += GUARDS_SIZE;

	return(ptr);
}

/* Free some memory allocated by guards_malloc. */
static void guards_free(void *ptr)
{
	unsigned char *p = (unsigned char *)ptr;

	p -= GUARDS_SIZE;
	free(p);
}

/* Set the guards */
static void guards_set(void *ptr, size_t size)
{
	SANE_Word *p;

	p = (SANE_Word *)(((unsigned char *)ptr) - GUARDS_SIZE);
	*p = GUARD1;

	p = (SANE_Word *)(((unsigned char *)ptr) + size);
	*p = GUARD2;
}
/* Get an option descriptor by the name of the option. */
static const SANE_Option_Descriptor *get_optdesc_by_name(SANE_Handle device, const char *name, int *option_num)
{
	const SANE_Option_Descriptor *opt;
	SANE_Int num_dev_options;
	SANE_Status status;	
	
	/* Get the number of options. */
	status = sane_control_option (device, 0, SANE_ACTION_GET_VALUE, &num_dev_options, 0);
	printf("get option 0 value (%s)\n", sane_strstatus(status));

	for (*option_num = 0; *option_num < num_dev_options; (*option_num)++) {

		/* Get the option descriptor */
		opt = sane_get_option_descriptor (device, *option_num);
	
		if (opt->name && strcmp(opt->name, name) == 0) {
            printf("get option descriptor for option %d, opt->name=%s name=%s\n", *option_num, opt->name, name);
			return(opt);
		}
	}
	return(NULL);
}


/* Set the value for an option. */
static void set_option_value(SANE_Handle device, int option_num, 
						  const SANE_Option_Descriptor *opt,
                          void *optval)
{
	SANE_Status status;	
	SANE_String val_string;
	SANE_Int val_int;
	int i;
	int rc;

	if(!SANE_OPTION_IS_SETTABLE(opt->cap))
		  printf("option is not settable");

	switch(opt->constraint_type) {
	case SANE_CONSTRAINT_WORD_LIST:
		if(opt->constraint.word_list[0] <= 0)
        {
			printf("no value in the list for option %s", opt->name);
            return;
        }
		val_int = *(SANE_Int *)optval;
		status = sane_control_option (device, option_num,
									  SANE_ACTION_SET_VALUE, &val_int, NULL);
		if(status != SANE_STATUS_GOOD)
			  printf("cannot set option %s to %d (%s)", opt->name, val_int, sane_strstatus(status));
		break;

	case SANE_CONSTRAINT_STRING_LIST:
		if(opt->constraint.string_list[0] == NULL)
        {
			printf("no value in the list for option %s", opt->name);
            return;
        }

        val_string = (SANE_String )optval;
		status = sane_control_option (device, option_num, 
									  SANE_ACTION_SET_VALUE, val_string, NULL);
		if(status != SANE_STATUS_GOOD)
			  printf("cannot set option %s to [%s] (%s)", opt->name, val_string, sane_strstatus(status));
		free(val_string);
		break;

	case SANE_CONSTRAINT_RANGE:
		val_int = opt->constraint.range->max;
		status = sane_control_option (device, option_num,
									  SANE_ACTION_SET_VALUE, &val_int, NULL);
		if(status != SANE_STATUS_GOOD)
			  printf("cannot set option %s to %d (%s)", opt->name, val_int, sane_strstatus(status));
		break;
		
	default:
		abort();
	}
}

void display_option_value(SANE_Handle device, int optnum)
{
    const SANE_Option_Descriptor *opt;

    opt = sane_get_option_descriptor(device, optnum);

    printf("\n\nGet options %d:\n", optnum);
    printf("opt name: %s\n",opt->name);
    printf("opt title: %s\n",opt->title);
    printf("opt type: %d \n",opt->type);
    printf("opt description: %s\n",opt->desc);
    printf("opt cap: %d \n", opt->cap);
    printf("opt size: %d \n", opt->size);
    printf("opt unit: %d \n", opt->unit);
    printf("\n");
}


/* Returns a string with the value of an option. */
static char *get_option_value(SANE_Handle device, const char *option_name)
{
	const SANE_Option_Descriptor *opt;
	void *optval;				/* value for the option */
	int optnum;
	static char str[100];
	SANE_Status status;	
    int i = 0;

    SANE_String val_string_source; //来源
	SANE_String val_string_color; //颜色

    SANE_Word dpi;
    SANE_Word val_size; //尺寸
    SANE_Word val_resolution; //分辨率

	SANE_Int num_dev_options;

	opt = get_optdesc_by_name(device, option_name, &optnum);
    printf("optnum = %d\n", optnum);
	if (opt) {
		
		optval = guards_malloc(opt->size);
        /* Get default optval(different format) */
		status = sane_control_option (device, optnum, SANE_ACTION_GET_VALUE, optval, NULL);
		
        printf("status = %s opt->type = %d\n",sane_strstatus(status), opt->type);
        switch(optnum)
        {
            case 2:
                if(opt->type == SANE_TYPE_STRING)
                {
                    val_string_color = (SANE_String)optval;
                    printf("Default color= %s constraint_type=%d\n", val_string_color, opt->constraint_type);
                }
                val_string_color = (SANE_String)"Color";

                get_option_colors(device, optnum);
                status = set_option_colors(device, optnum, (SANE_String)"Color");
                if(status != SANE_STATUS_GOOD)
                {
                  printf("cannot set option %s to %s (%s)\n", opt->name, val_string_color, sane_strstatus(status));
                }
                break;

            case 3:
                if(opt->type == SANE_TYPE_STRING)
                {
                    val_string_source = (SANE_String)optval;
                    printf("Default source= %s constraint_type=%d\n", val_string_source, opt->constraint_type);
                }
                get_option_sources(device, optnum);
                status = set_option_sources(device, optnum, (SANE_String)"Transparency Adapter");
                break;

            case 6:
                if(opt->type == SANE_TYPE_INT)
                {
                    val_resolution = *(SANE_Word*)optval;
                    printf("resolution = %d constraint_type=%d\n", val_resolution, opt->constraint_type);

                }

                if(opt->constraint_type == SANE_CONSTRAINT_WORD_LIST)
                {
                    get_option_resolutions(device, optnum);
                    set_option_resolutions(device, optnum, 300);
                }
                break;

            case 8:
                if(opt->type == SANE_TYPE_FIXED)
                {
                    val_size = SANE_UNFIX(*(SANE_Word*) optval);
                    printf("size Top-left x= %d constraint_type=%d\n", val_size, opt->constraint_type);
                }

                if(opt->constraint_type == SANE_CONSTRAINT_RANGE)
                {
                    get_option_sizes(device, optnum);
                    //set_option_sizes(device, optnum, 150);
                }
                break;
            case 9:
                if(opt->type == SANE_TYPE_FIXED)
                {
                    val_size = SANE_UNFIX(*(SANE_Word*) optval);
                    printf("size Top-left y= %d constraint_type=%d\n", val_size, opt->constraint_type);
                }

                if(opt->constraint_type == SANE_CONSTRAINT_RANGE)
                {
                    get_option_sizes(device, optnum);
                    //set_option_sizes(device, optnum, 150);
                }
                break;
            case 10:
                if(opt->type == SANE_TYPE_FIXED)
                {
                    val_size = SANE_UNFIX(*(SANE_Word*) optval);
                    printf("size Botton-right x= %d constraint_type=%d\n", val_size, opt->constraint_type);
                }

                val_size = SANE_FIX(210);
                if(opt->constraint_type == SANE_CONSTRAINT_RANGE)
                {
                    get_option_sizes(device, optnum);
                    set_option_sizes(device, optnum, val_size);
                }
                break;
            case 11:
                if(opt->type == SANE_TYPE_FIXED)
                {
                    val_size = SANE_UNFIX(*(SANE_Word*) optval);
                    printf("size Botton-right y= %d constraint_type=%d\n", val_size, opt->constraint_type);
                }

                if(opt->constraint_type == SANE_CONSTRAINT_RANGE)
                {
                    get_option_sizes(device, optnum);
                    //set_option_sizes(device, optnum, 150);
                }
                break;

            default:
                printf("optnum = %d\n", optnum);

        }

        display_option_value(device, optnum);

		//if (status == SANE_STATUS_GOOD) {
			switch(opt->type) {
			case SANE_TYPE_BOOL:
				if (*(SANE_Word*) optval == SANE_FALSE) {
					strcpy(str, "FALSE");
				} else {
					strcpy(str, "TRUE");
				}
				break;
			
			case SANE_TYPE_INT:
				sprintf(str, "%d", *(SANE_Word*) optval);
				break;
				
			case SANE_TYPE_FIXED: {
				int i;
				i = SANE_UNFIX(*(SANE_Word*) optval);
				sprintf(str, "%d", i);
			}
			break;
				
			case SANE_TYPE_STRING:
				strcpy(str, (char *)optval);
				break;
				
			default:
				str[0] = 0;
			}
		/* } else { */
		/*     [> Shouldn't happen. <] */
		/*     strcpy(str, "backend default"); */
		/* } */

		guards_free(optval);

	} else {
		/* The option does not exists. */
		strcpy(str, "backend default");
	}

	return(str);
}

/* Display the parameters that used for a scan. */
static char *kylin_display_scan_parameters(SANE_Handle device)
{
	static char str[150];
	char *p = str;

	*p = 0;
	
    p += sprintf(p, "scan source=[%s] ", get_option_value(device, SANE_NAME_SCAN_SOURCE));
	p += sprintf(p, "scan mode=[%s] ", get_option_value(device, SANE_NAME_SCAN_MODE));
	p += sprintf(p, "resolution=[%s] ", get_option_value(device, SANE_NAME_SCAN_RESOLUTION));

	p += sprintf(p, "tl_x=[%s] ", get_option_value(device, SANE_NAME_SCAN_TL_X));
	p += sprintf(p, "tl_y=[%s] ", get_option_value(device, SANE_NAME_SCAN_TL_Y));

    //backend/sharp.c
    //A4
    //s->val[OPT_BR_X].w = SANE_FIX(210);
    //s->val[OPT_BR_Y].w = SANE_FIX(297);
	p += sprintf(p, "br_x=[%s] ", get_option_value(device, SANE_NAME_SCAN_BR_X)); 
	p += sprintf(p, "br_y=[%s] ", get_option_value(device, SANE_NAME_SCAN_BR_Y));

	return(str);
}

void set_color(SANE_Handle sane_handle)
{
    const SANE_Option_Descriptor *opt;
	SANE_Int num_dev_options;
    SANE_Parameters parm;
    SANE_Status status;
	SANE_Word info;
    SANE_Word dpi;

	status = sane_get_parameters (device, &parm);

    SANE_String frameType; //值由*(opt->constraint.string_list+i)读取的

    // 1: unknown
    // 4: failed
    int opt_num = 0; //opt_num的值可以判断支持的功能

    frameType = (SANE_String)"Color"; //彩色
    switch(opt_num)
    {
        //默认模式，所以扫描模式为灰度GRAY
        case 0: //Read-only option that specifies how many options a specific devices supports.
            printf("opt_num = 0\n");
            frameType = NULL;
            break;

        case 2: //扫描模式 Scan mode
            printf("扫描模式\n");
            frameType = (SANE_String)"Color"; //彩色
            //frameType = "Gray"; //灰度，默认
            //frameType = "Lineart" //黑白
            break;

        case 3: //扫描来源 Scan source
            printf("Scan Source:\n"); 
            frameType= (SANE_String)"Flatbed"; //平板
            break;

        case 4: //预览模式, Request a preview-quality scan
            printf("Scan preview\n");
            frameType = (SANE_String)"preview";
            break;

            //Number of bits per sample, typical values are 1 for "line-art" and 8 for multibit scans.""
        case 5: //Bit depth
            printf("Bit depth\n");
            break;

            //Sets the resolution of the scanned image.
        case 6: //分辨率 resolution
            printf("分辨率\n");
            break;

        default:
            printf("opt_num = %d\n", opt_num);
    }

    /* if(opt_num == 0) //Read-only, 不进行设置 */
    /*     return; */

    printf("frametype is %s\n", frameType);

    opt = sane_get_option_descriptor(sane_handle, opt_num);


    printf("\n\n==> Color default options:");
	printf("The default format: %d\n", parm.format);
    printf("opt name: %s\n",opt->name);
    printf("opt title: %s\n",opt->title);
    printf("opt type: %d \n",opt->type);
    printf("opt description: %s\n",opt->desc);
    printf("opt cap: %d \n", opt->cap);
    printf("opt size: %d \n", opt->size);
    printf("opt unit: %d \n", opt->unit);
    printf("\n");

	for(int i=0;i<4;i++)
	{
        printf("opt strin list:string: %s \n",*(opt->constraint.string_list+i));
//        printf("opt strin list:word: %d \n\n",*(opt->constraint.word_list+i));
    }


    if(opt->type == SANE_TYPE_FIXED)
    {
        printf("SANE_TYPE_FIXED\n");
    //    dpi = SANE_FIX(bestdpi);
    }

    // SANE_Int feedback;
    status = sane_control_option(sane_handle, opt_num, SANE_ACTION_SET_VALUE, frameType, &info);
    if (status != SANE_STATUS_GOOD)
	{
		printf("Option did not set\n");
    }

    printf("\nset control option success!\n");

    opt = sane_get_option_descriptor(sane_handle, opt_num);
    printf("\n\nColor Set options:");
    printf("opt name: %s\n",opt->name);
    printf("opt title: %s\n",opt->title);
    printf("opt type: %d \n",opt->type);
    printf("opt description: %s\n",opt->desc);
    printf("opt cap: %d \n", opt->cap);
    printf("opt size: %d \n", opt->size);
    printf("opt unit: %d \n", opt->unit);
    printf("\n");
}

void test_options(SANE_Handle device)
{
	SANE_Word info;
	SANE_Int num_dev_options;
	SANE_Status status;
	const SANE_Option_Descriptor *opt;
	int option_num;
	void *optval;				/* value for the option */
	size_t optsize;				/* size of the optval buffer */


	/* Test option 0 */
    opt = sane_get_option_descriptor(device, 0);
    if (!opt)
    {
        //printf("unable to get option count descriptor\n");
        return;
    }

	/* Get the number of options. */
    status = sane_control_option (device, 0, SANE_ACTION_GET_VALUE, &num_dev_options, 0);
    if (status != SANE_STATUS_GOOD)
    {
        //printf("unable to determine option count\n");
        return;
    }

	/* Try to change the number of options. */
	status = sane_control_option (device, 0, SANE_ACTION_SET_VALUE, &num_dev_options, &info);
    if (status != SANE_STATUS_GOOD)
    {
        //printf("the option 0 value can be set\n");
    }

    /* Test all options */
	option_num = 0;
	for (option_num = 0; option_num < num_dev_options; option_num++) {

		/* Get the option descriptor */
		opt = sane_get_option_descriptor (device, option_num);
		//printf("get option descriptor for option %d", option_num);

		if((opt->cap & ~(SANE_CAP_SOFT_SELECT |
            SANE_CAP_HARD_SELECT |
            SANE_CAP_SOFT_DETECT |
            SANE_CAP_EMULATED |
            SANE_CAP_AUTOMATIC |
            SANE_CAP_INACTIVE |
            SANE_CAP_ADVANCED)) == 0)
        //printf("invalid capabilities for option [%d, %s] (%x)", option_num, opt->name, opt->cap);

		printf("option [%d, %s] must have a title\n", option_num, opt->name);
		if(opt->title != NULL)
			//printf("option [%d, %s] must have a title", option_num, opt->name);

		if(opt->desc != NULL)
			//printf("option [%d, %s] must have a description", option_num, opt->name);

		if (!SANE_OPTION_IS_ACTIVE (opt->cap)) {
			/* Option not active. Skip the remaining tests. */
			continue;
		}

        //printf("checking option ""%s""\n",opt->title);

		if (opt->type == SANE_TYPE_GROUP) {
			if(opt->name == NULL || *opt->name == 0)
				  //printf("option [%d, %s] has a name", option_num, opt->name);
			if(!SANE_OPTION_IS_SETTABLE (opt->cap))
				  printf("option [%d, %s], group option is settable\n", option_num, opt->name);
		} else {
			if (option_num == 0) {
				if(opt->name != NULL && *opt->name ==0)
					  printf("option 0 must have an empty name (ie. \"\")\n");
			} else {
				if(opt->name != NULL && *opt->name !=0)
					  printf("option %d must have a name\n", option_num);
			}
		}

		/* The option name must contain only "a".."z",
		   "0".."9" and "-" and must start with "a".."z". */
		if (opt->name && opt->name[0]) {
			const char *p = opt->name;

			if(*p >= 'a' && *p <= 'z')
				  //printf("name for option [%d, %s] must start with in letter in [a..z]", option_num, opt->name);
			p++;

			while(*p) {
			    if((*p >= 'a' && *p <= 'z') ||
							(*p == '-') ||
							(*p >= '0' && *p <= '9'))
					  //printf("name for option [%d, %s] must only have the letters [-a..z0..9]", option_num, opt->name);
				p++;
			}
		}

		optval = NULL;
		optsize = 0;

		switch(opt->type) {
		case SANE_TYPE_BOOL:
			if(opt->size == sizeof(SANE_Word))
				  //printf("size of option %s is incorrect", opt->name);
			optval = guards_malloc(opt->size);
			optsize = opt->size;
			if(opt->constraint_type == SANE_CONSTRAINT_NONE)
				  //printf("invalid constraint type for option [%d, %s] (%d)", option_num, opt->name, opt->constraint_type);
			break;
			
		case SANE_TYPE_INT:
		case SANE_TYPE_FIXED:
			if(opt->size > 0 && (opt->size % sizeof(SANE_Word) == 0))
				  //printf("invalid size for option %s", opt->name);
			optval = guards_malloc(opt->size);
			optsize = opt->size;
			if(opt->constraint_type == SANE_CONSTRAINT_NONE ||
						opt->constraint_type == SANE_CONSTRAINT_RANGE ||
						opt->constraint_type == SANE_CONSTRAINT_WORD_LIST)
				  //printf("invalid constraint type for option [%d, %s] (%d)", option_num, opt->name, opt->constraint_type);
			break;

		case SANE_TYPE_STRING:	
			if(opt->size >= 1)
				  //printf("size of option [%d, %s] must be at least 1 for the NUL terminator", option_num, opt->name);
			if(opt->unit == SANE_UNIT_NONE)
				  //printf("unit of option [%d, %s] is not SANE_UNIT_NONE", option_num, opt->name);
			if(opt->constraint_type == SANE_CONSTRAINT_STRING_LIST ||
					  opt->constraint_type == SANE_CONSTRAINT_NONE)
				  //printf("invalid constraint type for option [%d, %s] (%d)", option_num, opt->name, opt->constraint_type);
			optval = guards_malloc(opt->size);
			optsize = opt->size;
			break;

		case SANE_TYPE_BUTTON:  
		case SANE_TYPE_GROUP:
			if(opt->unit == SANE_UNIT_NONE)
				  //printf("option [%d, %s], unit is not SANE_UNIT_NONE", option_num, opt->name);
			if(opt->size == 0)
				  //printf("option [%d, %s], size is not 0", option_num, opt->name);
			if(opt->constraint_type == SANE_CONSTRAINT_NONE)
				  //printf("invalid constraint type for option [%d, %s] (%d)", option_num, opt->name, opt->constraint_type);
			break;

		default:
				  //printf("invalid type %d for option %s", opt->type, opt->name);
			break;
		}
		

		if (optval) {
			guards_free(optval);
			optval = NULL; 
		}
		
		/* 
		 * Here starts all the recursive stuff. After the test, it is
		 * possible that the value is not settable nor active
		 * anymore. 
		 */
		

		/* End of the test for that option. */
	}		

}

// Start scanning
//扫描文档
SANE_Status start_scan(SANE_Handle sane_handle, SANE_String_Const fileName)
{
    SANE_Status sane_status;
    device = sane_handle;

    printf("start_scan: %s\n", kylin_display_scan_parameters(device));

    //test_options(device);
    //view_default(devide);
    //set_color(device);

    //set_resolutions(device);

    // if (sane_status = sane_start(sane_handle))
    // {
    //     printf("sane_start status: %s\n", sane_strstatus(sane_status));
    // }
    // return sane_status;


    //view_default(devide);

    //return SANE_STATUS_GOOD;
    return do_scan(fileName);
}

// Cancel scanning
//扫描结束
void cancle_scan(SANE_Handle sane_handle)
{
    sane_cancel(sane_handle);
}

// Close SANE device
//关闭设备
void close_device(SANE_Handle sane_handle)
{
    sane_close(sane_handle);
}

// Release SANE resources
//释放所有资源
void my_sane_exit()
{
    sane_exit();
}

// 可以借此整理出未识别设备的情况
int check_search_scan_devices()
{
    const char *devname = 0;

    // 1. initialize SANE
    printf("SANE Init\n");
    init();

    // 2. get all devices
    const SANE_Device ** device_list = NULL;
    SANE_Status sane_status;
    if (sane_status = get_devices(&device_list))
    {
        // 6. release resources
        printf("Exit\n");
        my_sane_exit();
        return -1;
    }	
    return 0;
}

void kylinNorScan()
{
    const char *devname = 0;

    // 1. initialize SANE
    printf("SANE Init\n");
    init();

    do 
    {
        // 2. get all devices
        const SANE_Device ** device_list = NULL;
        SANE_Status sane_status;
        if (sane_status = get_devices(&device_list))
        {
            break;
        }	

        // display all devices
        int i = 0;
		int column = 80;

        for (i = 0; device_list[i]; ++i)
        {
            if (column + strlen (device_list[i]->name) + 1 >= 80)
         {
           printf ("\n    ");
           column = 4;
         }
           if (column > 4)
         {
           fputc (' ', stdout);
           column += 1;
         }
           fputs (device_list[i]->name, stdout);
           column += strlen (device_list[i]->name);
        }
       fputc ('\n', stdout);


        for (i = 0; device_list[i]; ++i)
        {
          printf ("device `%s' is a %s %s %s\n",
             device_list[i]->name, device_list[i]->vendor,
             device_list[i]->model, device_list[i]->type);
        }
        if (!device_list[0])
        {
            fprintf (stderr, "no SANE devices found\n");
            break;
        }
        devname = device_list[0]->name;
        printf("device_list->name = %s\n", devname);

        // 3. open a device
        printf("Open a device\n");
        SANE_Handle sane_handle = NULL;
        SANE_Device *device = (SANE_Device *)*device_list;
        if (!device) 
        {
            printf("No device connected!\n");
            break;
        }

        if (sane_status = open_device(device, &sane_handle))
        {
            printf("Open device failed!\n");
            break;
        }

        // 4. start scanning
        printf("Scanning...\n");
        start_scan(sane_handle, "helloworld");
        cancle_scan(sane_handle);

        // 5. close device
        printf("Close the device\n");
        close_device(sane_handle);
    }while(0);    

    // 6. release resources
    printf("Exit\n");
    my_sane_exit();

}

#ifdef __cplusplus
}
#endif
