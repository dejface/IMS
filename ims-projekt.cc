#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <getopt.h>
#include <algorithm>

using namespace std;

#define ROWS 10
#define COLUMNS 20
#define FORBIDDEN_SITTING -1
#define HEALTHY 0
#define INFECTED 1


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
                cinema[i][j] = FORBIDDEN_SITTING;
            } else if (((j % 2 == 1) && (i % 2 == 1))){
                cinema[i][j] = FORBIDDEN_SITTING;
            }
        }
    }
}

/* zisti stav aktualne spracovanej bunky, napocita infikovanych '1' a vyplni stav bunky do 
   new_cinema */

void getStateOfNeighbors(int cinema[][COLUMNS], int new_cinema[][COLUMNS], int row, int column, bool isLast){
    int infectedCount = 0;
    for (int r = row - 1; r <= row + 1; r++){
        for (int c = column - 1; c <= column + 1; c++){
            if (r < 0) {    //ak je aktualne spracovany riadok zaporny, neriesime
                continue;
            } else if (c < 0) { //ak je aktualne spracovany stlpec zaporny, neriesime
                continue;
            }
            if (r == row && c == column) continue;  //ak je to centrum 3x3 gridu, neriesime
            if (cinema[r][c] == INFECTED){  // ak sa dostal az sem a je infekcny, pocitame
                infectedCount++;
            }
        }
    }
    //toto treba modifikovat
    if (infectedCount > 1) {   // pocet infekcnych je viac ako 1
        if (cinema[row][column] != FORBIDDEN_SITTING)   // neni tam zakazane sediet
            new_cinema[row][column] = 1;    //dame na to miesto infikovaneho
    } else {
        if (cinema[row][column] == INFECTED){
            new_cinema[row][column] = 1;
        }
        else if (cinema[row][column] != FORBIDDEN_SITTING)
            new_cinema[row][column] = 0;
    }

    // ak je to posledna bunka 
    if (isLast){
        memset(cinema, 0, sizeof(cinema[0][0]) * ROWS * COLUMNS);   //vynulujeme kino
        if (separation) {
            // ak je zadany -s treba opat upravit kino
            modifyCinema(cinema);
        }
        for (int i = 0; i < ROWS; i++){
            for (int j = 0; j < COLUMNS; j++){
                cinema[i][j] = new_cinema[i][j];    //skopirujeme prvky z new_cinema do cinema
            }
        }
        memset(new_cinema, 0, sizeof(new_cinema[0][0]) * ROWS * COLUMNS);   //vynulujeme new_cinema
        if (separation){
            modifyCinema(new_cinema);
        }
    }
}

int main(int argc, char **argv){
    int pseudoTimer = 20;   // deklaracia pseudo casovaca
    int code = 0;
    if (code = processArgs(argc, argv)) return code;

    int cinema[ROWS][COLUMNS];
    int new_cinema[ROWS][COLUMNS];  // toto bude stav v dalsom kroku
    memset(cinema, 0, sizeof(cinema[0][0]) * ROWS * COLUMNS);
    memset(new_cinema, 0, sizeof(new_cinema[0][0]) * ROWS * COLUMNS);  
    srand((unsigned int)time(NULL));

    // ak je zadany prepinac s
    if (separation) {
        capacity /= 2;  // kapacita sa redukuje na polovicu
        if (capacity < infected){
            fprintf(stderr,"Number of infected is higher than capacity of cinema.\n");
            return 1;
        }
        modifyCinema(cinema);       // vyplni sa striedave sedenie
        modifyCinema(new_cinema);
    }
    bool isLast = false;    //flag na to aby som rozpoznal kedy sa spracuva posledna bunka generacie
    int count = infected;

    //naplnenie kina random infikovanymi na zaklade prepinaca -i, implicitne je to 10
    while (count != 0){
        int x = rand()%ROWS;
        int y = rand()%COLUMNS;
        if (cinema[x][y] == HEALTHY) {
            cinema[x][y] = INFECTED;
        } else {
            count++;
        }
        count--;
    }

    while (pseudoTimer){
        // toto je tu na debug, vypis stavu v kine
        int ic = 0;
        cout << "GENERACIA " << pseudoTimer << endl;
        for (int i = 0; i < ROWS; ++i){
            for (int j = 0; j < COLUMNS; ++j){
                if (cinema[i][j] == 1) ic++;
                cout << cinema[i][j] << ' ';
            }
            cout << endl;
        }
        cout << "POCET INFIKOVANYCH: " << ic << endl;
        for (int i = 0; i < ROWS; i++){
            for (int j = 0; j < COLUMNS; j++){
                // ak sme na poslednej bunke, flag je true
                if ((i == ROWS - 1) && (j == COLUMNS - 1)){
                    isLast = true;
                }
                getStateOfNeighbors(cinema, new_cinema, i, j, isLast);
            }
        }
        isLast = false;
        pseudoTimer--;
    }


    return 0;
}