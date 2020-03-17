#include "kylin_sane.h"

int main()
{
    // 1. initialize SANE
    printf("Init\n");
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

        // 3. open a device
        printf("Open a device\n");
        SANE_Handle sane_handle = NULL;
        SANE_Device *device = (SANE_Device *)*device_list;
        if (!device) 
        {
            printf("No device connected\n");
            break;
        }

        if (sane_status = open_device(device, &sane_handle))
        {
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
