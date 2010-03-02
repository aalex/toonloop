/* 
 * g++ -lporttime -lportmidi -lasound -o toonmidi toonmidi.cpp
 */

#include "portmidi.h"
#include "porttime.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 0
#define DRIVER_INFO NULL
#define TIME_PROC ((long (*)(void *)) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

#define STRING_MAX 80 /* used for console input */

long latency = 0;

/* read a number from console */
/**/
int get_number()
{
    char line[STRING_MAX];
    int n = 0, i;
    while (n != 1) {
        n = scanf("%d", &i);
        fgets(line, STRING_MAX, stdin);

    }
    return i;
}

/*
 * We want PortMidi to close Midi ports automatically in the event of a
 * crash because Windows does not (and this may cause an OS crash)
 */
void main_test_input() {
    PmStream * midi;
    PmError status, length;
    PmEvent buffer[1];
    printf("type input number: ");
    int i = get_number();
    /* It is recommended to start timer before Midi; otherwise, PortMidi may
       start the timer with its (default) parameters
     */
    TIME_START;
    /* open input device */
    Pm_OpenInput(&midi, 
                 i,
                 DRIVER_INFO, 
                 INPUT_BUFFER_SIZE, 
                 TIME_PROC, 
                 TIME_INFO);
    printf("Midi Input opened.\n");
    Pm_SetFilter(midi, PM_FILT_ACTIVE | PM_FILT_CLOCK);
    /* empty the buffer after setting filter, just in case anything
       got through */
    while (Pm_Poll(midi)) {
        Pm_Read(midi, buffer, 1);
    }
    /* now start paying attention to messages */
    while (true) {
        status = Pm_Poll(midi);
        if (status == TRUE) {
            length = Pm_Read(midi, buffer, 1);
            if (length > 0) {
                printf("Got message: time %ld, status: %2lx data1: %2lx data2: %2lx\n",
                       buffer[0].timestamp,
                       Pm_MessageStatus(buffer[0].message),
                       Pm_MessageData1(buffer[0].message),
                       Pm_MessageData2(buffer[0].message));
            } else {
                printf("ERROR: MIDI message with bad length.\n");
                assert(0);
            }
        }
    }
    /* close device (this not explicitly needed in most implementations) */
    printf("ready to close...");
    Pm_Close(midi);
    printf("done closing...");
}

int main(int argc, char *argv[])
{
    int i = 0, n = 0;
    char line[STRING_MAX];
    int test_input = 1;

    /* list device information */
    for (i = 0; i < Pm_CountDevices(); i++) {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if (((test_input) & info->input)) {
            printf("%d: %s, %s", i, info->interf, info->name);
            if (info->input) printf(" (input)");
            printf("\n");
        }
    }
    
	if (test_input) {
        main_test_input();
    }
    
    printf("finished portMidi test...");
    return 0;
}
