#include "kylin_sane.h"

struct option
 {
   char *name;
   /* has_arg can't be an enum because some compilers complain about
      type mismatches in all the code that assumes it is an int.  */
   int has_arg;
   int *flag;
   int val;
 };
static struct option *all_options;

int main()
{
    const char *devname = 0;

    // 1. initialize SANE
    printf("SANE Init\n");
    init();

    do 
    {
        // 2. get all devices
        const SANE_Device ** device_list = NULL;
        SANE_Status sane_status = 0;
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
    return 0;
}
