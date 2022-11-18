#include <stdio.h>
#include <stdlib.h>     // exit()
#include <fcntl.h>      // open()
#include <string.h>
#include <unistd.h>         /* Symbolic Constants */
#include <sys/time.h>
#include <sys/types.h>      /* Primitive System Data Types */
#include <sys/wait.h>       /* Wait for Process Termination */
#include <pthread.h>

#define LED_NUM 9
#define SEG_NUM 7
#define BUF_SIZE 10
#define PATH_SIZE 20

int fd_led[LED_NUM];
int fd_seg[SEG_NUM];

unsigned int total_case_buffer[BUF_SIZE] = {0};
unsigned int total_case_digit_num = 0;

unsigned int region_case_buffer[BUF_SIZE] = {0};
unsigned int region_case_digit_num = 0;
unsigned int region_cases[LED_NUM] = {0};

/* device functions */
void set_one_led(int fd, int signal);
void set_all_led(int signal);
void set_seg(int signal);
void update_total_case_buffer(unsigned int total_case);
void update_region_case_buffer(unsigned int region_case);

/* thread functions */
void *set_seg_blink(void *threadid);

/* choice of main menu */
char menu_choice;
int chosen_area = -1;

/* Main function */
void main(int argc, char **argv)
{
    char led[LED_NUM][PATH_SIZE];
    char seg[SEG_NUM][PATH_SIZE];
    char buf[BUF_SIZE];

    unsigned int mild_case[LED_NUM] = {0};
    unsigned int severe_case[LED_NUM] = {0};
    unsigned int total_case = 0;
    unsigned int region_case;

    int status;
    int i = 0;

    int rc;
    pthread_t thread;

    /* Link the path of LED devices and open all devices*/
    char path[PATH_SIZE] = "/dev/LED_0";
    // LEDs (9 GPIO)
    while (i < LED_NUM) {
        path[9] = i + 48;
        snprintf(led[i], sizeof(led[i]), path);
        
        fd_led[i] = open(led[i], O_RDWR);
        if (fd_led[i] < 0) {
            printf("Error opening /dev/LED_%d\n", i);
            exit(EXIT_FAILURE);
        }
        i++;
    }

    // 7-segment display (7 GPIO)
    i = 0;
    strcpy(path, "/dev/SEG_0");
    while (i < SEG_NUM) {
        path[9] = i + 48;
        snprintf(seg[i], sizeof(seg[i]), path);
        
        fd_seg[i] = open(seg[i], O_RDWR);
        if (fd_seg < 0) {
            printf("Error opening /dev/SEG_%d\n", i);
            exit(EXIT_FAILURE);
        }
        i++;
    }
    set_seg(0); // initially set to 0

    /* create a new thread */
    rc = pthread_create(&thread, NULL, set_seg_blink, (void *)0);
    if (rc){
        printf("ERROR; pthread_create() returns %d\n", rc);
        exit(EXIT_FAILURE);
    }

    /* Menu */
    char degree, ch;
    unsigned int area, number;
    while (1) {
        printf("================= Main Menu =================\n");
        printf("1. Confirmed case\n");
        printf("2. Reporting system\n");
        printf("3. Exit\n");
        scanf(" %c", &menu_choice);

        if (menu_choice == '1') {
            /* 1. Confirmed case */
            while (1) {
                printf("================= List of Confirmed Cases =================\n");
                i = 0;
                total_case = 0;
                while (i < LED_NUM) {
                    region_case = mild_case[i]+severe_case[i];
                    
                    // If the region i has confirmed cases, then turn on the corresponding LED
                    set_one_led(fd_led[i], region_case > 0);
                    
                    // Print the overall confirmed cases
                    printf("%d : %d\n", i, region_case);
                    i++;
                }
                scanf(" %c", &ch);

                if (ch == 'q')  break;
                else if (ch >= 48 && ch < LED_NUM+48){
                    // Trun all LEDs off
                    set_all_led(0);

                    // Switch the boolean value
                    chosen_area = ch-48;
                    update_region_case_buffer(region_cases[chosen_area]);

                    // Print confirmed cases of specific region in detail
                    printf("Mild : %d\n", mild_case[chosen_area]);
                    printf("Severe : %d\n", severe_case[chosen_area]);
                    
                    // Blink the specific LED for 3 seconds (switch on/off every 0.5 seconds)
                    set_one_led(fd_led[chosen_area], 1);
                    usleep(1000 * 500);   // 0.5 sec                     
                    set_one_led(fd_led[chosen_area], 0);
                    usleep(1000 * 500);   // 0.5 sec 
                    set_one_led(fd_led[chosen_area], 1);
                    usleep(1000 * 500);   // 0.5 sec 
                    set_one_led(fd_led[chosen_area], 0);
                    usleep(1000 * 500);   // 0.5 sec 
                    set_one_led(fd_led[chosen_area], 1);
                    usleep(1000 * 500);   // 0.5 sec
                    set_one_led(fd_led[chosen_area], 0);
                    usleep(1000 * 500);   // 0.5 sec

                    // Turn on the specific LED
                    set_one_led(fd_led[chosen_area], 1);
                    scanf(" %c", &ch);

                    chosen_area = -1;
                }
                else {
                    printf("Input should be 0-%d or q!!!\n\n", LED_NUM-1);
                }
            }
        } else if (menu_choice == '2') {
            /* 2. Reporting system */
            while (1) {
                printf("================= Reporting system =================\n");
                printf("Area : ");
                scanf("%d", &area);
                while (area < 0 || area > LED_NUM-1) {
                    printf("Input should be 0-%d!!!\n", LED_NUM-1);
                    printf("Area : ");
                    scanf("%d", &area);
                }
                
                printf("Mild or Severe : ");
                scanf(" %c", &degree);
                while (degree != 'm' && degree != 's') {
                    printf("Input should be m or s!!!\n");
                    printf("Mild or Severe : ");
                    scanf(" %c", &degree);
                }

                printf("The number of confirmed case : ");
                scanf("%d", &number);
                while (number < 0) {
                    printf("Input should be 0 or positive integer!!\n");
                    printf("The number of confirmed case : ");
                    scanf("%d", &number);
                }

                if (degree == 'm') 
                    mild_case[area] += number;
                else if (degree == 's')
                    severe_case[area] += number;
                total_case += number;
                update_total_case_buffer(total_case);
                
                region_cases[area] += number;
                
                // Turn on the specific LED
                if (mild_case[area] + severe_case[area] > 0)
                    set_one_led(fd_led[area], 1);

                printf("Press c to continue reporting or e to exit the system : ");
                scanf(" %c", &ch);
                while (ch != 'c' && ch != 'e') {    
                    printf("Input should be c or e!!!\n");
                    printf("Press c to continue reporting or e to exit the system : ");
                    scanf(" %c", &ch);            
                }
                if (ch == 'e') {
                    printf("\n");
                    break;
                }
            }
        }
        else if (menu_choice != '3') {
            /* Others: undefined input */
            printf("Input should be 1-3!!!\n\n");
        }

        if (menu_choice == '3')  {
            set_all_led(0);
            set_seg(-1);
            break;
        }
            
    }

    pthread_exit(NULL);     // main function exit
}

/* function of thread */
void *set_seg_blink(void *threadid) {
    long tid  = (long)threadid;
    int seg_num = 0;
    int timer_index = 0;

    while (menu_choice != '3') {
        if (chosen_area == -1) {
            /* NOT checking the specific area */
            if (timer_index >= total_case_digit_num)
                timer_index = 0;    // to avoid runtime error
            
            if (total_case_digit_num > 0) {
                seg_num = total_case_buffer[total_case_digit_num - timer_index - 1];
                set_seg(seg_num);
                timer_index = (timer_index + 1) % total_case_digit_num;
            }
        } else if (chosen_area >= 0 && chosen_area < LED_NUM) {
            /* checking the specific area */
            if (timer_index >= region_case_digit_num)
                timer_index = 0;    // to avoid runtime error
            
            if (region_case_digit_num > 0) {
                seg_num = region_case_buffer[region_case_digit_num - timer_index - 1];
                set_seg(seg_num);
                timer_index = (timer_index + 1) % region_case_digit_num;
            }
        }

        sleep(1);
    }

    pthread_exit(NULL);
}


/* Switch single LED */
void set_one_led(int fd, int signal) {
    char buf[BUF_SIZE];
    buf[0] = signal + 48;
    buf[1] = '\0';
    if (write(fd, buf, sizeof(buf)) < 0) {
        perror("[set_one_led()] Cannot write fd_led");
        exit(EXIT_FAILURE);
    }
}

/* Switch all LEDs */
void set_all_led(int signal) {
    char buf[BUF_SIZE];
    buf[0] = signal + 48;
    buf[1] = '\0';
    
    int i = 0;
    while (i < LED_NUM)
        if (write(fd_led[i++], buf, sizeof(buf)) < 0) {
            perror("[set_all_led()]Cannot write fd_led");
            exit(EXIT_FAILURE);
    }
}

/* Switch 7-segment displayer */
void set_seg(int signal) {
    char seg_list[11][SEG_NUM] = {
        "0000001",      // 0
        "1001111",      // 1
        "0010010",      // 2
        "0000110",      // 3
        "1001100",      // 4
        "0100100",      // 5
        "0100000",      // 6
        "0001111",      // 7
        "0000000",      // 8
        "0000100",      // 9
        "1111111",      // -1
    };

    char buf[BUF_SIZE];
    int i = 0;
    while (i < SEG_NUM) {
        if (signal == -1)
            buf[0] = seg_list[10][i];
        else
            buf[0] = seg_list[signal][i];
        buf[1] = '\0';
        if (write(fd_seg[i], buf, sizeof(buf)) < 0) {
            perror("[set_seg()] Cannot write fd_seg");
            exit(EXIT_FAILURE);
        }
        i++;
    }
}

/* Update the total case number buffer */
void update_total_case_buffer(unsigned int total_case) {
    total_case_digit_num = 0;

    // Update the total_case_buffer with the new confirmed cases
    while (total_case > 0) {
        total_case_buffer[total_case_digit_num++] = total_case % 10;
        total_case /= 10;
    }
    
    //printf("[update_total_case_buffer()] Update completed!!! total_case_digit_num = %d\n", total_case_digit_num);
}

/* Update the region case number buffer of the specific area */
void update_region_case_buffer(unsigned int region_case) {
    region_case_digit_num = 0;

    // Update the total_case_buffer with the new confirmed cases
    while (region_case > 0) {
        region_case_buffer[region_case_digit_num++] = region_case % 10;
        region_case /= 10;
    }
}