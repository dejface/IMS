#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <getopt.h>
#include <algorithm>

using namespace std;

#define ROWS 10
#define COLUMNS 20

int infected = 10;
bool masks = false;
bool separation = false;
int capacity = ROWS * COLUMNS;


int processArgs(int argc, char **argv){
    int opt;
    char* endptr = NULL;
    opterr = 0;
    while ((opt = getopt(argc, argv, "i:ms")) != -1) {
        switch (opt) {
        case 'i': // infected count, default 10
            infected = (int)strtol(optarg, &endptr, 10);
            if (*endptr) {
                fprintf(stderr, "Wrong infected (-i) value!\n");
                return 1;
            }
            break;
        case 'm': // people are wearing masks
            masks = true;
            break;
        case 's': // reduced capacity to half (separation)
            separation = true;
            break;
        case ':':
            break;

        default:
            if (optopt == 105) break;
            fprintf(stderr, "Wrong parameter!\n");
            return 1;
        }
    }
    return 0;
}

void modifyCinema(int cinema[][COLUMNS]){
    for (int i = 0; i < ROWS; i++){
        for (int j = 0; j < COLUMNS; j++){
            if (((j % 2 == 0) && (i % 2 == 0))){
                cinema[i][j] = -1;
            } else if (((j % 2 == 1) && (i % 2 == 1))){
                cinema[i][j] = -1;
            }
        }
    }
}

int main(int argc, char **argv){
    int code = 0;
    if (code = processArgs(argc, argv)) return code;

    int cinema[ROWS][COLUMNS];
    memset(cinema, 0, sizeof(cinema[0][0]) * ROWS * COLUMNS);
    srand((unsigned int)time(NULL));
    if (separation) {
        capacity /= 2;
        if (capacity < infected){
            fprintf(stderr,"Number of infected is higher than capacity of cinema.\n");
            return 1;
        }
        modifyCinema(cinema);
    }

    int count = infected;
    while (count != 0){
        int x = rand()%ROWS;
        int y = rand()%COLUMNS;
        if (cinema[x][y] == 0) {
            cinema[x][y] = 1;
        } else {
            count++;
        }
        count--;
    }
    for (int i = 0; i < ROWS; ++i){
        for (int j = 0; j < COLUMNS; ++j){
            cout << cinema[i][j] << ' ';
        }
        cout << endl;
    }


    return 0;
}