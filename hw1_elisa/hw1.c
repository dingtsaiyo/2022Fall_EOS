#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define SEG_BUF_SIZE 8
#define LED_BUF_SIZE 10
#define PATH_SIZE 20

int confirm[9][2] = {0};
int fd = 0, n = 0;
char buf_seg[SEG_BUF_SIZE];
char buf_led[LED_BUF_SIZE];
char seg_for_c[10][7]= {
    "0000001",
    "1001111",
    "0010010",
    "0000110",
    "1001100",
    "0100100",
    "0100000",
    "0001111",
    "0000000",
    "0000100"
};


// char *path[9] = {"/dev/led0", "/dev/led1", "/dev/led2", "/dev/led3", 
//                  "/dev/led4", "/dev/led5", "/dev/led6", "/dev/led7", "/dev/led8"};


int getComfirm(int area, char ms){
    int cases;
    cases = ms == 'm' ? 0 : 1;
    return confirm[area][cases];
}

void setComfirm(int area, char ms, int value){
    int cases;
    cases = ms == 'm' ? 0 : 1;
    confirm[area][cases] += value;
}

int sumComfirm(int regoin){
    return (getComfirm(regoin, 'm') + getComfirm(regoin, 's'));
}

void blank(){
    printf("\n");
}

void ledUP(int region, char blink){
    char mems[10];
    memset(mems, '0', 9);
    mems[9]='\0';
    int i = 0, test = 0;
    if(region > 9){
        while(i < 9){
            if(sumComfirm(i) > 0){
                buf_led[i]='1';
            }else{
                buf_led[i]='0';
            }
            i++;
        }
        buf_led[9]='\0';
        write(fd, buf_led, 10);
    }else{
        memset(buf_led,'0', 9);
        buf_led[9]='\0';
        buf_led[region]='1';
        while(i < 6){
            write(fd, buf_led, 10);
            usleep(500000);
            if(blink =='b'){
                write(fd, mems, 10);
                usleep(500000);
            }
            i++;
        }
    }
    printf("write to driver for LED: %s with size: %ld\n", buf_led, sizeof(buf_led));
    
}

void segBlink(int turns, int num[]){
    int count = 0;
    while(count < turns){
        strncpy(buf_seg, seg_for_c[num[count]], 7);
        buf_seg[7]='\0';
        write(fd, buf_seg, 8);
        printf("write to driver for SEG: %s with size: %ld\n", buf_seg, sizeof(buf_seg));
        usleep(1000000);
        count++;
    }
}

void segUP(int number){
    int sp_number[3]={0};
    if(number < 10){
        sp_number[0] = number;
        segBlink(1, sp_number);
    }else if(number < 100){
        sp_number[1] = number%10;
        sp_number[0] = number/10;
        segBlink(2, sp_number);
    }else if(number < 1000){
        sp_number[2] = number%100;
        sp_number[1] = (number%100-sp_number[0])/10;
        sp_number[0] = number/100;
        segBlink(3, sp_number);
    }else{
        printf("wrong in segUP, no correct number");
    }
}

void showCases(){
    //lightUp --> regoin have confirm cases
    ledUP(10, 'n');
    int area_index = 0, confirm_case;
    while(area_index < 9){
        confirm_case = sumComfirm(area_index);
        printf("%d : %d\n", area_index, confirm_case);
        area_index++;
    }
    blank();
}

int optionOne(){
    int search, total = 0, sum = 0;
    char next;

    showCases();
    while(total < 9){
        sum += sumComfirm(total);
        total++;
    }
    printf("sum = %d\n", sum);
    segUP(sum);
    blank();
    scanf(" %d", &search);

    //lightUp --> 0.5 for 3 sec
    ledUP(search, 'b');
    //segUp --> confirm cases
    segUP(sumComfirm(search));

    printf("Mild: %d\n", getComfirm(search, 'm'));
    printf("Severe: %d\n", getComfirm(search, 's'));
    scanf(" %c", &next);
    if(next == 'q'){
        return 0;
    }
    else{
        return optionOne();
    }
}

int optionTwo(){

    int report, rep_int, rep_case;
    char rep_ms, next;
    showCases();
    printf("Area (0-8):");
    scanf(" %d", &report);
    printf("Mild or Server ('m' or 's'): ");
    scanf(" %c", &rep_ms);
    printf("The number of confirmed case: ");
    scanf(" %d", &rep_case);
    blank();
    setComfirm(report, rep_ms, rep_case);

    //lightUp --> selected regoin
    ledUP(report, 'n');
    //segUp --> cases of the regoin
    segUP(sumComfirm(report));
    scanf(" %c", &next);
    if(next == 'e'){
        return 0;
    }
    else{ 
        return optionTwo();
    }
}


int main(void){
    int option;
    char path[PATH_SIZE];
    char mem_sev[8], mem_nin[10];

    snprintf(path, sizeof(path), "/dev/etx_device");
    fd = open(path, O_WRONLY);
    if (fd < 0) {
		perror("Error opening");
		exit(EXIT_FAILURE);
	}

    while(1){
        printf("1. Confirmed case\n2. Reporting system\n3. Exit\n");
        scanf("%d", &option);

        while(option == 1){
            if (!optionOne()){
                blank();
                break;
            }
        }
        while(option == 2){
            if (!optionTwo()){
                blank();
                break;
            }            
        }
        //lightUp --> off
        //segUp --> off
        if(option == 3)break;
    }
    memset(mem_sev, '0', 7);
    memset(mem_nin, '0', 9);
    mem_sev[7]='\0';
    mem_nin[9]='\0';
    write(fd, mem_sev, sizeof(mem_sev));
    write(fd, mem_nin, sizeof(mem_nin));
    close(fd);

    return EXIT_SUCCESS;
} 