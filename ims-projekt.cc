#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <getopt.h>
#include <algorithm>
#include <time.h>
#include <math.h>

using namespace std;

#define ROWS 10
#define COLUMNS 20
#define FORBIDDEN_SITTING -1
#define FORBIDDEN_SITTING_INFECTED -2
#define HEALTHY 0
#define INFECTED 1
#define NEW_INFECTED 2


int infected = 10;
int ventilation = 1;    // no ventilation, just natural air transmission (~0.35) 
bool masks = false;
bool separation = false;
bool extreme_separation = false;
bool effective_mask = false;
int capacity = ROWS * COLUMNS;


int processArgs(int argc, char **argv){
    int opt;
    char* endptr = NULL;
    opterr = 0;
    while ((opt = getopt(argc, argv, "i:mesxv:")) != -1) {
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
        case 'e':
            effective_mask = true;
            break;
        case 's': // reduced capacity to half (separation)
            separation = true;
            break;
        case 'x': // reduced capacity to quarter (separation)
            extreme_separation = true;
            break;
        case 'v':
            ventilation = (int)strtol(optarg, &endptr, 10);
            if (*endptr) {
                fprintf(stderr, "Wrong ventilation (-v) value!\n");
                return 1;
            }
            if (ventilation > 4 || ventilation < 1) {
                fprintf(stderr, "Ventilation level should be from 1 to 4.\n");
                return 1;
            }
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


void modifyCinemaExtreme(int cinema[][COLUMNS]){
    for (int i = 0; i < ROWS; i++){
        for (int j = 0; j < COLUMNS; j++){
            if ((j % 4 != 0) && (i % 2 == 0)){
                cinema[i][j] = FORBIDDEN_SITTING;
            } else if ((j % 4 != 2) && (i % 2 == 1)){
                cinema[i][j] = FORBIDDEN_SITTING;
            } 
        }
    }
}



/* 
If there is infected or new_infected neighbours returns either state HEALTHY or NEW_INFECTED
*/
int newState(int actual_state, int infectedcount, int new_infectedcount, int model_time, int dist_from_infected){
    if (actual_state == INFECTED || actual_state == NEW_INFECTED){
        return actual_state;
    }
    if (dist_from_infected == 0){
        dist_from_infected = 1;
    }
    if (infectedcount > 0 || new_infectedcount > 0){
        // random numbers between 0 and 100
        float rand_prob = ((float)rand()/(float)(RAND_MAX)) * 100.0;    
        // Prna - chance of getting infection by single RNA copy
        float P_rna = 0.0022;      
        // the number of viral copies inhaled and deposited in the airways (D50 rule is 316)
        double viral_copies = 0;
        // Ri - infection risk
        double Ri = 0.0;
        if (infectedcount > 0){
            viral_copies = 20 * model_time * infectedcount; 
            Ri = (1 - pow((1 - P_rna), viral_copies)) * 100;         
        } else {
            viral_copies = 20 * model_time * (new_infectedcount /2);  
            Ri = (1 - pow((1 - P_rna), viral_copies)) / 2 * 100;  //3 feet rule https://www.upi.com/Health_News/2020/06/01/At-least-3-feet-of-social-distancing-likely-reduces-COVID-19-spread-study-confirms/7431591040809/     
            // Ri *= 1/dist_from_infected; // longer distance, lower risk --toto sposobovalo najma to blbnutie
        }
        if (masks){
            Ri *= 0.4 * 0.7; // 60% effectiveness in blocking viral particles, 30% filtering  
        } 
        if (ventilation == 1){
            Ri *= 1; 
        } else if (ventilation == 2){
            Ri *= 0.4;  // value from study https://www.mdpi.com/1660-4601/17/21/8114/htm // depends on quality of ventilation
        } else if (ventilation == 3){
            Ri *= 0.2;  // better ventilation
        } else if (ventilation == 4){
            Ri *= 0.1;  // HEPA filtration
        }
        if (effective_mask){
            Ri *= 0.01; 
        }
        // CALCULATOR: https://www.mpic.de/4747361/risk-calculator?en
        // when empty seats contain enough viral copies in the area to get infected
        if (actual_state == FORBIDDEN_SITTING || actual_state == FORBIDDEN_SITTING_INFECTED){
            if (rand_prob < Ri){
                return FORBIDDEN_SITTING_INFECTED;
            } else {
                return FORBIDDEN_SITTING;
            }  
        }
        if (rand_prob < Ri && dist_from_infected < 5){
            return NEW_INFECTED;
        } else {
            return HEALTHY;
        }
    }
    return actual_state;
}


int infectedDistance(int cinema[][COLUMNS], int row, int column){
    int distance = 0;
    int index = 4;
    for (int r = row - 4; r <= row + 4; r++){
        for (int c = column - 4; c <= column + 4; c++){
            if (r < 0 || r >= ROWS) {    //ak je aktualne spracovany riadok zaporny, neriesime
                continue;
            } else if (c < 0 || c  >= COLUMNS) { //ak je aktualne spracovany stlpec zaporny, neriesime
                continue;
            }
            if (r == row && c == column) continue;  //ak je to centrum gridu, neriesime
            if (cinema[r][c] == INFECTED){  // ak sa dostal az sem a je infekcny, pocitame
                distance = index;
            }
        }
        index--;
    }
    return abs(distance);
}


/* zisti stav aktualne spracovanej bunky, napocita infikovanych '1' a vyplni stav bunky do 
   new_cinema */
void getStateOfNeighbors(int cinema[][COLUMNS], int new_cinema[][COLUMNS], int row, int column, bool isLast, int model_time){
    int infectedCount = 0;
    int new_infectedcount = 0;
    for (int r = row - 1; r <= row + 1; r++){
        for (int c = column - 1; c <= column + 1; c++){
            if (r < 0 || r >= ROWS) {    //ak je aktualne spracovany riadok zaporny, neriesime
                continue;
            } else if (c < 0 || c  >= COLUMNS) { //ak je aktualne spracovany stlpec zaporny, neriesime
                continue;
            }
            if (r == row && c == column) continue;  //ak je to centrum 3x3 gridu, neriesime
            if (cinema[r][c] == INFECTED){  // ak sa dostal az sem a je infekcny, pocitame
                infectedCount++;
            }
            if (cinema[r][c] == NEW_INFECTED || cinema[r][c] == FORBIDDEN_SITTING_INFECTED){  // ak sa dostal az sem a bol novy nakazeny, pocitame
                new_infectedcount++;
            }
        }
    }

    int dist_from_infected = infectedDistance(cinema, row, column);
    //toto treba modifikovat
    if (infectedCount > 0 || new_infectedcount > 0) {   // pocet infekcnych je viac ako 1
        // if (cinema[row][column] != FORBIDDEN_SITTING)   // neni tam zakazane sediet
            new_cinema[row][column] = newState(cinema[row][column], infectedCount, new_infectedcount, model_time, dist_from_infected);    //dame na to miesto infikovaneho
    } else {
        if (cinema[row][column] == INFECTED){
            new_cinema[row][column] = INFECTED;
        }
        else if (cinema[row][column] == HEALTHY){
            new_cinema[row][column] = HEALTHY;
        }
        else if (cinema[row][column] == NEW_INFECTED){
            new_cinema[row][column] = NEW_INFECTED;
        }
    }

    // ak je to posledna bunka 
    if (isLast){
        memset(cinema, 0, sizeof(cinema[0][0]) * ROWS * COLUMNS);   //vynulujeme kino
        if (separation) {
            // ak je zadany -s treba opat upravit kino
            modifyCinema(cinema);
        }
        if (extreme_separation) {
            // ak je zadany -x treba opat upravit kino
            modifyCinemaExtreme(cinema);
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
        if (extreme_separation){
            modifyCinemaExtreme(new_cinema);
        }
    }
}



int main(int argc, char **argv){
    int pseudoTimer = 20;   // deklaracia pseudo casovaca
    int code = 0;
    srand((unsigned int)time(NULL));
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
    // ak je zadany prepinac s
    if (extreme_separation) {
        capacity /= 4;  // kapacita sa redukuje na polovicu
        if (capacity < infected){
            fprintf(stderr,"Number of infected is higher than capacity of cinema.\n");
            return 1;
        }
        modifyCinemaExtreme(cinema);       // vyplni sa striedave sedenie
        modifyCinemaExtreme(new_cinema);
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
        int nic = 0;
        static int gen_index = 1;
        cout << "GENERACIA " << gen_index << endl;
        for (int i = 0; i < ROWS; ++i){
            for (int j = 0; j < COLUMNS; ++j){
                if (cinema[i][j] == 1) ic++;
                if (cinema[i][j] == 2) nic++;
                cout << cinema[i][j] << ' ';
            }
            cout << endl;
        }
        cout << "POCET INFIKOVANYCH: " << ic << endl;
        cout << "POCET NOVO INFIKOVANYCH: " << nic << endl;
        for (int i = 0; i < ROWS; i++){
            for (int j = 0; j < COLUMNS; j++){
                // ak sme na poslednej bunke, flag je true
                if ((i == ROWS - 1) && (j == COLUMNS - 1)){
                    isLast = true;
                }
                getStateOfNeighbors(cinema, new_cinema, i, j, isLast, gen_index);
            }
        }
        
        isLast = false;
        pseudoTimer--;
        gen_index++;
    }


    return 0;
}