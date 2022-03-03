#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>

#define EV_RELEASED 0

struct input_event_and_dev 
{
    int dev;
    struct input_event event;
};

unsigned int  storeInput(char *device, char *device2, struct input_event_and_dev *eventList, int maxSize)
{
	printf("Starting recording, press F4 to stop recording \n");

	// Read the key back from the keyboard buffer
	int fd1 = open(device, O_RDONLY);
	int fd2 = open(device2, O_RDONLY);
	if( fd1 <= 0 || fd2 <= 0 ){
		perror("Error: ");
		if( fd1 > 0 ){
			close(fd1);
		}
		if( fd2 > 0 ){
			close(fd2);
		}
		return 0;
	}
	
	unsigned int index = 0;
	struct pollfd pfds[2];
	pfds[0].fd = fd1;
	pfds[0].events = POLLIN;
	pfds[1].fd = fd2;
	pfds[1].events = POLLIN;

	int exit = 0;
	
	for(;;)
	{
		poll(pfds, 2, -1);
		for( int i = 0; i < 2; i++ ){
			if (pfds[i].revents & POLLIN) {
				if( read(pfds[i].fd, &eventList[index].event, sizeof(struct input_event)) != sizeof(struct input_event))
				{
					printf("Error: \n");
					exit = 1;
					break;
				}

				struct input_event *event = &eventList[index].event;

				if( index+1 >= maxSize || (i == 0 && event->code == KEY_F4))
				{
					exit = 1;
					break;
				}
				
				eventList[index].dev = i;
				index++;
				
				// printf("%ld,%ld: read back scan_code is: %d %c \n",  event->time.tv_sec, event->time.tv_usec, event->value, event->code);
			}
		}
		
		if( exit == 1 ){
			break;
		}
	}
	
	close(fd1);
	close(fd2);
	
	
	return index;
}

void replay(char *device, char *device2, struct input_event_and_dev *eventList, unsigned int eventAmount )
{
	printf("Replay starts.\n");
	int fd[2];
	fd[0] = open(device, O_RDWR|O_SYNC);
	fd[1] = open(device2, O_RDWR|O_SYNC);
	if( fd[0] <= 0 || fd[1] <= 0 ){
		perror("Error: ");
		if( fd[0] > 0 ){
			close(fd[0]);
		}
		if( fd[1] > 0 ){
			close(fd[1]);
		}
		return;
	}	

	struct pollfd pfds[1];
	pfds[0].fd = fd[0];
	pfds[0].events = POLLIN;
	
	
	write(fd[eventList[0].dev], &eventList[0].event, sizeof(struct input_event));
	for( unsigned int i = 1; i < eventAmount; i++ )
	{
		suseconds_t usec = eventList[i].event.time.tv_usec - eventList[i-1].event.time.tv_usec;
		time_t sec = eventList[i].event.time.tv_sec - eventList[i-1].event.time.tv_sec;
		if( usec > 0 )
		{
			usleep(usec);
		}
		else if( usec < 0 )
		{
			usleep(1000000+usec);
			sec -= 1;
		}
		
		for( int j = 0; j < sec; j++ )
		{
			usleep(1000000);
		}

		poll(pfds, 1, 0);
		if (pfds[0].revents & POLLIN) {
			// Keyboard action, abort
			printf("Aborted by keyboard action\n");
			break;
		}
		
		struct input_event* event = &eventList[i].event;
		// printf("Writing dev: %d, type %d, code %d, value %d\n", eventList[i].dev, event->type, event->code, event->value );
		write(fd[eventList[i].dev], event, sizeof(struct input_event));
	}
		
	close(fd[0]);
	close(fd[1]);
	
	printf("Replay ended.\n");
}

int waitKey(char *device)
{
	int fd = 0;
	if(  (fd = open(device, O_RDONLY)) > 0 )
	{
		int exit = 0;
		struct input_event event;
		printf("Press F7 to repeat, F8 to record, F9 to quit.\n");
		for(;;)
		{
			read(fd, &event, sizeof(struct input_event));
			if( event.value == EV_RELEASED && event.code == KEY_F7)
			{
				close(fd);
				return 1;
			}

			if( event.value == EV_RELEASED && event.code == KEY_F9)
			{
				close(fd);
				return 0;
			}
			
			if( event.value == EV_RELEASED && event.code == KEY_F8)
			{
				close(fd);
				return 2;
			}
		}
	}else{
		perror("Error: ");
	}

	return -1;	
}

/**
 * Execute command and return number returned by the output of that command.
 */
int getDev(char *command){
	FILE *fp;
	char output[3];

	fp = popen(command, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		return -1;
	}

	if (fgets(output, sizeof(output), fp) != NULL) {
		pclose(fp);
		return atoi(output);
	}

	pclose(fp);
	return -1;
}

int getMouse(){
	return getDev("ls /dev/input/by-path/ -la | grep -oh \"event-mouse -> ../event[0-9]*\" | grep -oh \"[0-9]*\"");
}

int getKeyboard(){
	return getDev("ls /dev/input/by-path/ -la | grep -oh \"event-kbd -> ../event[0-9]*\" | grep -oh \"[0-9]*\"");
}

int main()
{
	int fd = 0;
	int mouseDev = getMouse();
	int keyboardDev = getKeyboard();
	char device[21];
	char device2[21];
	sprintf(device, "/dev/input/event%d", keyboardDev);
	sprintf(device2, "/dev/input/event%d", mouseDev);
	printf("keyboard: %s\n", device);
	printf("mouse: %s\n", device2);
	int fd1 = 0;
	int maxInputsToCollect = 1000000;
	struct input_event_and_dev *eventList = malloc(sizeof(struct input_event_and_dev)*maxInputsToCollect);
	unsigned int eventAmount = 0;

	for(;;)
	{
		int res = waitKey(device);
		if( res == -1 ){
			printf("Failed to read input device. Only root has read access to input devices.\n");
			break;
		}
		if( res == 0 ){
			break;
		}
		if( res == 2 ){
			eventAmount = storeInput( device, device2, eventList, maxInputsToCollect );
			continue;
		}		

		if( eventAmount == 0 ){
			printf("Record first with F8\n");
		}
		else{
			replay( device, device2, eventList, eventAmount );
		}
		
	}

	free(eventList);
	eventList = 0;
	return 0;
}
