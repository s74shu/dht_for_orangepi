#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>         
#include <sys/stat.h>
#include <fcntl.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1 
#define MAX_TIMINGS 85
#define DHT_PIN 7

int data[5] = { 0, 0, 0, 0, 0 };
int fd[2];
FILE *th_file;
char *f_name = "/tmp/tmp_hmd.json";

//daemonize process
static void daemonize(void)
{
    pid_t pid, sid;
    int fds;
    /* already a daemon */
    if ( getppid() == 1 ) return;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)  
    {   
        exit(EXIT_FAILURE);
    }   

    if (pid > 0)  
    {   
        exit(EXIT_SUCCESS); /*Killing the Parent Process*/
    }   

    /* At this point we are executing as the child process */

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0)  
    {
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory. */
    if ((chdir("/")) < 0)
    {
        exit(EXIT_FAILURE);
    }


    fds = open("/dev/null",O_RDWR, 0);

    if (fds != -1)
    {
        dup2 (fds, STDIN_FILENO);
        dup2 (fds, STDOUT_FILENO);
        dup2 (fds, STDERR_FILENO);

        if (fds > 2)
        {
            close (fds);
        }
    }

    /*resettign File Creation Mask */
    umask(022);
}

void read_dht_data()
{
    uint8_t laststate    = HIGH;
    uint8_t counter        = 0;
    uint8_t j            = 0, i;
 
    data[0] = data[1] = data[2] = data[3] = data[4] = 0;
 
    /* pull pin down for 18 milliseconds */
    pinMode( DHT_PIN, OUTPUT );
    digitalWrite( DHT_PIN, LOW );
    delay( 18 );
 
    /* prepare to read the pin */
    pinMode( DHT_PIN, INPUT );
 
    /* detect change and read data */
    for ( i = 0; i < MAX_TIMINGS; i++ )
    {
        counter = 0;
        while ( digitalRead( DHT_PIN ) == laststate )
        {
            counter++;
            delayMicroseconds( 1 );
            if ( counter == 255 )
            {
                break;
            }
        }
        laststate = digitalRead( DHT_PIN );
 
        if ( counter == 255 )
            break;
 
        /* ignore first 3 transitions */
        if ( (i >= 4) && (i % 2 == 0) )
        {
            /* shove each bit into the storage bytes */
            data[j / 8] <<= 1;
            if ( counter > 16 )
                data[j / 8] |= 1;
            j++;
        }
    }
 
    /*
     * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
     * print it out if data is good
     */
 
    if ( (j >= 40) &&
         (data[4] == ( (data[0] + data[1] + data[2] + data[3]) & 0xFF) ) )
    {
        float h = (float)((data[0] << 8) + data[1]) / 10;
        if ( h > 100 )
        {
            h = data[0];    // for DHT11
        }
        float c = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
        if ( c > 125 )
        {
            c = data[2];    // for DHT11
        }
        if ( data[2] & 0x80 )
        {
            c = -c;
        }
	//float f = c * 1.8f + 32;
        printf( "Humidity = %.1f %% Temperature = %.1f *C\n" /*(%.1f *F)\n"*/, h, c/*, f*/ );
        th_file = fopen(f_name, "w");
	fprintf(th_file, "{\"temperature\":%.1f, \"humidity\":%.1f}", c,h);
        fclose(th_file);
    }
    else  {
       printf( "Data not good, skip\n" );
    }
}
 
int main( void )
{
    //printf( "DHT22 temperature/humidity test\n" );

    if ( wiringPiSetup() == -1 )
        exit( 1 );

    daemonize();
 
    while ( 1 )
    {
        read_dht_data();
        //delay( 2000 ); /* wait 2 seconds before next read */
        sleep(2);
    }

    return(0);
}
