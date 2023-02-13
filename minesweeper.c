#define _POSIX_C_SOURCE 200201L
#include <stdio.h>
#include <stdlib.h> 
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <curses.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

void make_shadow_field(int size, char shadow_field[size][size], int mines_num);
void make_player_field(int size, char player_field[size][size]);
void draw_logo(int x, int y);
void draw_title(int x, int y);
void draw_game_over(int x, int y);
void draw_win(int x, int y);
void draw_rules(int x, int y);
void draw_good_job(int x, int y);
void clear_screen(int start, int end);
void uncover_zero(int size, char shadow_field[][size], char player_field[][size],int a, int b);
void uncover_all(int size, char shadow_field[][size], char player_field[][size],int a, int b);
int check(int x, int y ,int size, char player_field[size][size],char shadow_field[size][size]);
int check_win(int mines_num, int size, char player_field[][size]);

int main (int argc, char *argv[]) {
	//COMMANDLINE ARGUMENTS
	if(argc < 2) {
		printf("There should be 2 arguments:\n1st Player Name & 2nd difficulty\n");
		printf("'e/E' => easy\n'n/N' => normal\n'h/H' => hard\n");
		return 1;
	}
	*argv[2] = tolower(*argv[2]);
	if ( *argv[2] != 'n' && *argv[2] != 'e' && *argv[2] != 'h'){
		printf("Invalid difficulty\n");
		printf("'e/E' => easy\n'n/N' => normal\n'h/H' => hard\n");
		return 1;
	}
	//CONVERTING COMMAND LINE ARGUMENT
	char player_name[20];
	size_t name_size = strlen(argv[1]);
	for ( int i = 0; i < name_size; i++){
		player_name[i] = argv[1][i];
	}
	player_name[name_size] = '\0';

	srand(time(NULL));

	int size = 6, mines_num = 12, quit = 0, play = 0;

	//PREPARING DIFFICULTY
	switch(*argv[2]){
		case 'n':
			size = 11;
			mines_num = 15; 
			break;
		case 'h':
			size = 13;
			mines_num = 18;
			break;
		case 'e':
			size = 9;
			mines_num = 11;
			break;
	}

	char shadow_field[size][size];
	char player_field[size][size];

	initscr();	

	//COLOURS
	start_color();
	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);//TITLE, MENU
	init_pair(5, COLOR_CYAN, COLOR_BLACK);
	init_pair(6, COLOR_WHITE, COLOR_BLACK);
	init_pair(7, COLOR_BLACK, COLOR_WHITE);
	init_pair(8, COLOR_MAGENTA, COLOR_BLACK);//POSITIVE MESSAGE, CURSOR

	cbreak();
	noecho();
	keypad(stdscr,TRUE);
	curs_set(0);
	nodelay(stdscr,TRUE);

	//GAME
	while (quit == 0){ 
		//MAIN MENU
		int input, menu_pos = 0;
		do { //MAIN MENU LOOP
			clear_screen(0,COLS);
			draw_logo(0,0);
			draw_title(LINES-10, COLS/2-72);
			mvprintw(LINES*0.5 - 6,COLS*0.5-3, "START");
			mvprintw(LINES*0.5 - 3,COLS*0.5-3, "QUIT");
			mvprintw(LINES*0.5 + 1,COLS*0.5-3 - 10 , "Press spacebar to confirm");
			draw_rules(LINES*0.05,COLS*0.57);
			switch(menu_pos){//SELECTION HIGHLIGHTING
				case 0:
					attron(COLOR_PAIR(4));
					mvprintw(LINES*0.5 - 6,COLS*0.5-3, "START");
					attroff(COLOR_PAIR(4));
					break;
				case 1:
					attron(COLOR_PAIR(4));
					mvprintw(LINES*0.5 - 3,COLS*0.5-3, "QUIT");
					attroff(COLOR_PAIR(4));
					break;
			}
			refresh();
			input = getch();
			switch(input){//MOVING IN MAIN MENU
				case KEY_UP:
					menu_pos -= 1;
					if (menu_pos < 0){
						menu_pos ++;
					}
					break;
				case KEY_DOWN:
					menu_pos += 1;
					if(menu_pos > 1){
						menu_pos --;
					}
					break;
				case ' ':
					play = (menu_pos == 0)? 1 : 0;
					if (menu_pos == 1){
						endwin();
						return EXIT_SUCCESS;
					}
			}
		
		} while (play == 0);// END OF MAIN MENU
		/////////////////////////////////////////GAMEPLAY////////////////////////////////////////////////
		clear_screen(0,COLS);
		make_shadow_field(size, shadow_field, mines_num);
		make_player_field(size, player_field);
	
		//PLAYER NICK
		mvprintw(LINES/2-2,14, "PLAYER NAME:");
		mvprintw(LINES/2-1, 19 - name_size/2, "%s", player_name);
		//TIME
		struct timespec ts = {
			.tv_sec = 0,
			.tv_nsec = 0.001 * 1000000000L
		};
		ts.tv_nsec = 0.01 * 1000000000L;
		mvprintw(LINES/2+1, 18,"TIME");
	
		//GAME SCREEN
		draw_logo(0,0);
		draw_title(LINES-10, COLS/2-72);
		draw_rules(LINES*0.05,COLS*0.57);
		for ( int i = 0; i < 29; i++){
			//mvprintw(LINES*0.2-1, (COLS*0.55-1) + i*4, "____");
			mvprintw(LINES*0.2 + 14 , (COLS*0.55-1) + i*3, "___");
			mvprintw( (LINES*0.2 +14) - i, COLS*0.55-1 ,"|");
		}
		for ( int i = 0; i < 8; i++){
			mvprintw(LINES/2 - 3, 0+ 5*i, "_____");
			mvprintw(LINES/2 + 3, 0+ 5*i, "_____");
			if(i!= 0 && i!=7){
				mvprintw( (LINES/2-3) + i , 40, "|");
			}
		}
	
		refresh();
	
		//DRAWS THE MINE FIELD
		int a = LINES*0.35-(size/2);
		int b = COLS*0.35-(size/2);
	
		for( int i = 0; i < size + 4 ; i++){
			mvprintw(a-2,(b-4) + 2*i,"__");
			mvprintw(a+size, (b-4) + 2*i, "__");
			if(i != 0 && i != size+3){
				mvprintw((a-2)+i, b-4,"|");
				mvprintw((a-2)+i, b+4+size*2,"|");
			}
		}
		for ( int x = 0; x < size; x++){
			for ( int y = 0; y < size; y++){
				mvprintw(a + x,b + y*2,"|%c",player_field[x][y]);// a + x*2
				printw("|");
				refresh();
				nanosleep(&ts, NULL);
			}
		}
		////////////////////////	

		char player = '?';
		int pl_x  = LINES*0.35 - (size/2);
		int pl_y = COLS*0.35 - (size/2)+1;
		//pl_x and pl_y ==> coordinates where to draw cursor
		int x = 0;
		int y = 0;
		//x and y ==>  position inside played_field
		refresh();
		int arrow;

		double measured_time = 0;
		clock_t t1 = clock();
		clock_t t2;
	
		do{	
			//DISPLAYING TIME
			t2 = clock() - t1;
			measured_time = ((double)t2)/CLOCKS_PER_SEC;
			attron(COLOR_PAIR(7));
			mvprintw(LINES/2+2, 18, "%.2f",measured_time);
			attroff(COLOR_PAIR(7));
			refresh();
	
			char temp = player_field[y][x];//REMEMBERING NUMBER THE CURSOR IS ON RIGHT NOW
			int atr; //remembering which pair was last time
			switch(temp){//COLOUR FOR NUMBERS
				case '1':
					attron(COLOR_PAIR(1));
					atr = 1;
					break;
				case '2':
					attron(COLOR_PAIR(2));
					atr = 2;
					break;
				case '3':
					attron(COLOR_PAIR(3));
					atr = 3;
					break;
				case '4':
					attron(COLOR_PAIR(4));
					atr = 4;
					break;
				case '5':
					attron(COLOR_PAIR(8));
					atr = 8;
					break;
	
				case '0':
					attron(COLOR_PAIR(5));
					atr = 5;
					break;
				case '#':
					attron(COLOR_PAIR(6));
					atr = 6;
					break;
			}
	
			arrow = getch();
			switch(arrow){//MOVING IN MINE FIELD
				case KEY_LEFT:
					x -= 1;
					if ( x>= 0 && x < size){
						pl_y -= 2;
						attron(COLOR_PAIR(atr));
						mvprintw(pl_x,pl_y+2, "%c",temp);
					}
					else{
						x +=1;
					}
					break;
				case KEY_RIGHT:
					x += 1;
					if ( x>= 0 && x < size){
						pl_y += 2;
						attron(COLOR_PAIR(atr));
						mvprintw(pl_x,pl_y-2, "%c",temp);
					}
					else{
						x -=1;
					}
					break;
				case KEY_UP:
					y -= 1;
					if ( y>= 0 && y < size){
						pl_x -= 1;
						attron(COLOR_PAIR(atr));
						mvprintw(pl_x+1,pl_y, "%c",temp);
					}
					else{
						y += 1;
					}
					break;
				case KEY_DOWN:
					y += 1;
					if ( y>= 0 && y < size){
						pl_x += 1;
						attron(COLOR_PAIR(atr));
						mvprintw(pl_x-1,pl_y, "%c",temp);
					}
					else{
						y-= 1;
					}
					break;
				
			
				case ' ': //PLAYER CHOSE A TILE


					if (player_field[y][x]=='1'|| player_field[y][x]=='2'|| player_field[y][x]=='0'||player_field[y][x]=='3'){
						//PLAYER CHOSE ALREADY UNCOVERED TILE
						continue;
					}

					//PLAYER CHOSE CORRECT TILE
					if(check(x,y,size,shadow_field, player_field) == 1){
					
						switch(player_field[y][x]){
							case '1':
								attron(COLOR_PAIR(1));
								atr = 1;
								break;
							case '2':
								attron(COLOR_PAIR(2));
								atr = 2;
								break;
							case '3':
								attron(COLOR_PAIR(3));
								atr = 3;
								break;
							case '4':
								attron(COLOR_PAIR(4));
								atr = 4;
								break;
							case '5':
								attron(COLOR_PAIR(8));
								atr = 8;
								break;
							case '0':
								attron(COLOR_PAIR(5));
								atr = 5;
								break;
	
						}
	
						//A NUMBER WILL BE PRINTED INSIDE THE TILE
	
						mvprintw(pl_x,pl_y,"%c" ,player_field[y][x]);
						if ( player_field[y][x] == '0'){//WILL UNCOVER ALL 0's
							uncover_zero(size, shadow_field, player_field, a ,b);
							refresh();
						}
						clear_screen(LINES-10, COLS);
						draw_good_job(LINES-10, COLS/2-50);
						refresh();
						ts.tv_nsec = 0.75 * 1000000000L;
						nanosleep(&ts, NULL);
						clear_screen(LINES-10, COLS);
						draw_title(LINES-10, COLS/2-72);
						refresh();
					}
					//PLAYER CHOSE TILE WITH MINE
					else{
						//GAME OVER
						attron(COLOR_PAIR(3));
						mvprintw(pl_x,pl_y,"%c" ,player_field[y][x]);
						attroff(COLOR_PAIR(3));
						uncover_all(size, shadow_field, player_field, a, b);	
						refresh();
						clear_screen(LINES-10, COLS);
						draw_game_over(LINES-10,COLS/2-50);
						refresh();
						sleep(3);
						play = 0;
						break;
						//GO BACK TO MAIN MENU
					}	
			}
	
			//mvprintw(30,10,"%c %c", player_field[y][x], shadow_field[y][x]);//CHEATS
			attron(COLOR_PAIR(8));
			mvprintw(pl_x,pl_y, "%c", player);//PRINTS PLAYER CURSOR
			attroff(COLOR_PAIR(8));
			refresh();
	
		}while(arrow != 'q' && arrow != 'Q' && check_win(mines_num, size, player_field) == 0 && play == 1);
	
		//CHECK WIN
		if( check_win(mines_num, size, player_field) == 1){
			clear_screen(LINES-10, COLS);
			draw_win(LINES-10, COLS/2-43);
			refresh();
			sleep(3);

			//SAVE SCORE INTO FILE
			FILE *fp = fopen("score.txt", "a+");
			char diff = *argv[2];
			//measured_time = roundf(measured_time * 100) / 100;
			fprintf(fp,"PLAYER: %s   TIME: %.2f   DIFFICULTY: %c \n", player_name, measured_time, diff);
			fclose(fp);

			//PRINT PREVIOUS SCORES
			FILE *FP = fopen("score.txt", "r");
			char buffer[255];
			int q = 0;
			clear_screen(0,COLS);
			draw_logo(0,0);
			draw_title(LINES-10, COLS/2-72);
			while (fgets(buffer, 255, FP) != NULL){
				mvprintw(LINES*0.1 + q*2,COLS*0.25, "%s", buffer);
				ts.tv_nsec = 0.75 * 1000000000L;
				nanosleep(&ts, NULL);
				refresh();
				q++;
			}
			fclose(fp);
			sleep(5);
		}
		//RETURN TO MAIN MENU
		play = 0;
	}
	//QUITTING THE GAME
	endwin();
	return 0;
}


void make_shadow_field(int size,char shadow_field[size][size], int mines_num){
	//GENERATE MINES POSITIONS
	int mines_pos[2*mines_num];
	for ( int i = 0; i < 2*mines_num; i++){
		int pos = rand() % size;
		mines_pos[i] = pos;
		//printf("%d",mines_pos[i]);
	}
	
	//FILL FIELD
	for ( int i = 0; i < size; i ++){
		
		for ( int j = 0; j < size; j ++){
			
			shadow_field[i][j] = 'X';

		}	
	}

	//PLACE MINES
	for ( int i = 0; i < mines_num; i ++){
		int x = mines_pos[i*2];
		int y = mines_pos[i*2+1];

		shadow_field[x][y] = '*';
	}
	//PLACE NUMBERS
	for ( int i = 0; i < size; i++){
		
		for ( int j = 0; j < size; j++){
			
			if (shadow_field[i][j] != '*'){
				//FIND NUM OF MINES
				int sum = 0;

				if ( j == 0){//IF AT THE LEFT EDGE
					for ( int m = 0; m < 2; m++){//ROW BELOW
						if ( shadow_field[i+1][j+m] == '*'){
							sum += 1;
						}
					}
					for ( int m = 0; m < 2; m++){//ROW ABOVE
						if ( shadow_field[i-1][j+m] == '*'){
							sum += 1;
						}
					}
					if ( shadow_field[i][j+1] == '*'){//NEXT
						sum += 1;
					}
				shadow_field[i][j] = sum + '0';
				continue;
				}
				if ( j == size-1){//IF AT THE RIGHT EDGE
					for ( int m = 0; m < 2; m++){//ROW BELOW
						if ( shadow_field[i+1][j-m] == '*'){
							sum += 1;
						}
					}
					for ( int m = 0; m < 2; m++){//ROW ABOVE
						if ( shadow_field[i-1][j-m] == '*'){
							sum += 1;
						}
					}
					if ( shadow_field[i][j-1] == '*'){//PREVIOUS
						sum += 1;
					}
				shadow_field[i][j] = sum + '0';
				continue;
				}




				//IF NOT ON THE EDGE
				for ( int m = 0; m < 3; m++){//ROW BELOW
					if ( shadow_field[i+1][j-1+m] == '*'){
						sum += 1;
					}
				}
				for ( int m = 0; m < 3; m++){//ROW ABOVE
					if ( shadow_field[i-1][j-1+m] == '*'){
						sum += 1;
					}
				}
				if ( shadow_field[i][j-1] == '*'){//PREVIOUS
						sum += 1;
				}
				if ( shadow_field[i][j+1] == '*'){//NEXT
						sum += 1;
				}
				shadow_field[i][j] = sum + '0';

			}
		}
	}

}

void make_player_field(int size, char player_field[][size]){
	//PREPARES player_field TO BE USED
	for ( int i = 0; i < size; i ++){
		
		for ( int j = 0; j < size; j ++){
			
			player_field[i][j] = '#';

		}	
	}
}

int check(int x, int y ,int size, char player_field[size][size],char shadow_field[size][size]){
	//CHECKS IF THERE IS MINE AT GIVEN POSITION
	if (player_field[y][x] != '*'){
		shadow_field[y][x] = player_field[y][x];
		return 1; 
	}
	shadow_field[y][x] = player_field[y][x];
	return 0;
}
void uncover_zero(int size, char shadow_field[][size], char player_field[][size], int a, int b){
	//FINDS '0' and changes player_field
	for ( int i = 0; i < size; i++){
		for ( int j = 0; j < size; j++){
			if (shadow_field[i][j] == '0'){
				player_field[i][j] = shadow_field[i][j];
	
			}
		}
	}
	//REFRESH SCREEN WITH '0'
	attron(COLOR_PAIR(5));
	for ( int y = 0; y < size; y++){
		for ( int x = 0; x < size; x++){
			if (player_field[y][x] == '0'){
				mvprintw(a + y,b+1 + x*2,"%c",player_field[y][x]);
			}
		}
	}
	attroff(COLOR_PAIR(5));
}

void uncover_all(int size, char shadow_field[][size], char player_field[][size],int a, int b){
	//SOLVES WHOLE PLAYER_FIELD
	for ( int i = 0; i < size; i++){
		for ( int j = 0; j < size; j++){
			player_field[i][j] = shadow_field[i][j];
		}
	}
	//REFRESH SCREEN WITH SOLVED FIELD
	for ( int y = 0; y < size; y++){
		for ( int x = 0; x < size; x++){
			switch(player_field[y][x]){
				case '1':
					attron(COLOR_PAIR(1));
					break;
				case '2':
					attron(COLOR_PAIR(2));
					break;
				case '3':
					attron(COLOR_PAIR(3));
					break;
				case '4':
					attron(COLOR_PAIR(4));
					break;
				case '0':
					attron(COLOR_PAIR(5));
					break;
				case '*':
					attron(COLOR_PAIR(3));
					break;
				case '5':
					attron(COLOR_PAIR(8));
					break;
			}


			mvprintw(a + y,b+1 + x*2,"%c",player_field[y][x]);
		}
	}

}
void draw_logo(int x, int y){
	char logo[19][41] = {
	{"@&5!:.           !J5@P!           .:7P@@"},
	{"G^            .::&5:&@#::.            ~B"},
	{":    .!7^  ^7              7^  ^7!.    :"},
	{"    JB@YYPY                   5JY@BJ    "},
	{"    Y#@&B                      &#@#Y    "},
	{"     !@Y                        @@!     "},
	{"    ^!G                          @J:    "},
	{"  ..BP                            @G    "},
	{"^?5Y&G          Created by         &PPJ^"},
	{"@GJY&P       Christian Feliks      &GG#@"},
	{"^5##@B             2022            &##5^"},
	{"    G@&                           @G    "},
	{"    :J@&                         @J:    "},
	{"     ^5&@                       &5^     "},
	{"      :B@@                     @B:      "},
	{"     !@GB@@                   @BG@!     "},
	{":    :7B&#7!                 7#&B7:    ^"},
	{"B~     ...    ...#@&JB#:..    ...     ~#"},
	{"@@P!:.           !P@BY!           .:7P@@"},
	};

	for ( int i = 0; i < 19; i ++){
		mvprintw(x+i,y,"%s",logo[i]);
	}
}
void draw_title(int x, int y){
	char title[7][144] = {
		{"        :::   :::   ::::::::::: ::::    ::: ::::::::::           ::::::::  :::       ::: :::::::::: :::::::::: :::::::::  :::::::::: ::::::::: "},
		{"      :+:+: :+:+:      :+:     :+:+:   :+: :+:                 :+:    :+: :+:       :+: :+:        :+:        :+:    :+: :+:        :+:    :+: "},
		{"    +:+ +:+:+ +:+     +:+     :+:+:+  +:+ +:+                 +:+        +:+       +:+ +:+        +:+        +:+    +:+ +:+        +:+    +:+  "},
		{"   +#+  +:+  +#+     +#+     +#+ +:+ +#+ +#++:++#            +#++:++#++ +#+  +:+  +#+ +#++:++#   +#++:++#   +#++:++#+  +#++:++#   +#++:++#:    "},
		{"  +#+       +#+     +#+     +#+  +#+#+# +#+                        +#+ +#+ +#+#+ +#+ +#+        +#+        +#+        +#+        +#+    +#+    "},
		{" #+#       #+#     #+#     #+#   #+#+# #+#                 #+#    #+#  #+#+# #+#+#  #+#        #+#        #+#        #+#        #+#    #+#     "},
		{"###       ### ########### ###    #### ##########           ########    ###   ###   ########## ########## ###        ########## ###    ###      "},
	};
	attron(COLOR_PAIR(4));
	for ( int i = 0; i < 7; i ++){
		mvprintw(x+i,y,"%s",title[i]);
	}
	attroff(COLOR_PAIR(4));

}
void draw_game_over(int x, int y){
	char text[7][100] = {
		{" ::::::::      :::     ::::    ::::  ::::::::::        ::::::::  :::     ::: :::::::::: :::::::::  "},
		{":+:    :+:   :+: :+:   +:+:+: :+:+:+ :+:              :+:    :+: :+:     :+: :+:        :+:    :+: "},
		{"+:+         +:+   +:+  +:+ +:+:+ +:+ +:+              +:+    +:+ +:+     +:+ +:+        +:+    +:+ "},
		{":#:        +#++:++#++: +#+  +:+  +#+ +#++:++#         +#+    +:+ +#+     +:+ +#++:++#   +#++:++#:  "},
		{"+#+   +#+# +#+     +#+ +#+       +#+ +#+              +#+    +#+  +#+   +#+  +#+        +#+    +#+ "},
		{"#+#    #+# #+#     #+# #+#       #+# #+#              #+#    #+#   #+#+#+#   #+#        #+#    #+# "},
		{" ########  ###     ### ###       ### ##########        ########      ###     ########## ###    ### "},
	};
	attron(COLOR_PAIR(3));
	for ( int i = 0; i < 7; i ++){
		mvprintw(x+i,y,"%s",text[i]);
	}
	attroff(COLOR_PAIR(3));

}
void clear_screen(int start, int end){
	//start => where to begin clearing
	//end => where to stop clearing
	for (int i = start; i < LINES; i++){
		for ( int j = 0; j < end; j++){
			mvprintw(i,j," ");
		}	
	}

}
int check_win(int mines_num, int size, char player_field[][size]){
	int sum =0 ;//=> sum of correct guesses
	for ( int i = 0; i < size; i ++){
		for ( int j = 0; j < size; j++){
			if (player_field[i][j] != '#'){
				sum++;
			}
		}
	}

	if ( sum == (size*size - mines_num)){
		return 1;
	}
	return 0;
}
void draw_win(int x, int y){
	char text[7][88] = {
		{":::   :::  ::::::::  :::    :::       :::       ::: ::::::::::: ::::    :::       ::: "},
		{":+:   :+: :+:    :+: :+:    :+:       :+:       :+:     :+:     :+:+:   :+:       :+: "},
		{" +:+ +:+  +:+    +:+ +:+    +:+       +:+       +:+     +:+     :+:+:+  +:+       +:+ "},
		{"  +#++:   +#+    +:+ +#+    +:+       +#+  +:+  +#+     +#+     +#+ +:+ +#+       +#+ "},
		{"   +#+    +#+    +#+ +#+    +#+       +#+ +#+#+ +#+     +#+     +#+  +#+#+#       +#+ "},
		{"   #+#    #+#    #+# #+#    #+#        #+#+# #+#+#      #+#     #+#   #+#+#           "},
		{"   ###     ########   ########          ###   ###   ########### ###    ####       ### "},
	};
	attron(COLOR_PAIR(2));
	for ( int i = 0; i < 7; i ++){
		mvprintw(x+i,y,"%s",text[i]);
	}
	attroff(COLOR_PAIR(2));
}
void draw_rules(int x, int y){
	char text[16][90] = {
		{"--------RULES-----------"},
		{"Minesweeper is a game where mines are hidden in a grid of tiles."},
		{"Safe squares have numbers telling you how many mines touch the square."},
		{"You can use the number clues to solve the game by opening all of the safe squares."},
		{"If you pick tile with a mine you lose the game!"},
		{"You win if you manage to choose all the tiles that don't include mines."},
		{"If you manage to find '0' once, the game will uncover the rest of them for you."},
		{"Good Luck !"},
		{""},
		{"--------LABELING--------"},
		{"'#' ==> HIDDEN TILE"},
		{"'?' ==> CURRENT POSITION OF CURSOR"},
		{"'*' ==> MINE"},
		{""},
		{"--------CONTROLS--------"},
		{"Press 'q/Q' to quit. Move by arrows. Chose a tile by pressing spacebar."},

	};
	//attron(COLOR_PAIR(4));
	for ( int i = 0; i < 16; i ++){
		mvprintw(x+i,y,"%s",text[i]);
	}
	//attroff(COLOR_PAIR(4));
}
void draw_good_job(int x, int y){
	char text1[7][114] = {
		{":::    ::: :::::::::: :::::::::: :::::::::        ::::::::::: :::::::::::       :::    ::: :::::::::        ::: "},
		{":+:   :+:  :+:        :+:        :+:    :+:           :+:         :+:           :+:    :+: :+:    :+:       :+: "},
		{"+:+  +:+   +:+        +:+        +:+    +:+           +:+         +:+           +:+    +:+ +:+    +:+       +:+ "},
		{"+#++:++    +#++:++#   +#++:++#   +#++:++#+            +#+         +#+           +#+    +:+ +#++:++#+        +#+ "},
		{"+#+  +#+   +#+        +#+        +#+                  +#+         +#+           +#+    +#+ +#+              +#+ "},
		{"#+#   #+#  #+#        #+#        #+#                  #+#         #+#           #+#    #+# #+#                  "},
		{"###    ### ########## ########## ###              ###########     ###            ########  ###              ### "},
	};

	char text2[7][110] = {	
		{":::     ::: :::::::::: :::::::::  :::   :::       ::::    ::: ::::::::::: ::::::::  ::::::::::       ::: "},
		{":+:     :+: :+:        :+:    :+: :+:   :+:       :+:+:   :+:     :+:    :+:    :+: :+:              :+: "},
		{"+:+     +:+ +:+        +:+    +:+  +:+ +:+        :+:+:+  +:+     +:+    +:+        +:+              +:+ "},
		{"+#+     +:+ +#++:++#   +#++:++#:    +#++:         +#+ +:+ +#+     +#+    +#+        +#++:++#         +#+ "},
		{" +#+   +#+  +#+        +#+    +#+    +#+          +#+  +#+#+#     +#+    +#+        +#+              +#+ "},
		{"  #+#+#+#   #+#        #+#    #+#    #+#          #+#   #+#+#     #+#    #+#    #+# #+#                  "},
		{"    ###     ########## ###    ###    ###          ###    #### ########### ########  ##########       ### "},
	};
	
	char text3[7][100] = {	
		{" ::::::::   ::::::::   ::::::::  :::::::::        ::::::::::: ::::::::  :::::::::        ::: "},
		{":+:    :+: :+:    :+: :+:    :+: :+:    :+:           :+:    :+:    :+: :+:    :+:       :+: "},
		{"+:+        +:+    +:+ +:+    +:+ +:+    +:+           +:+    +:+    +:+ +:+    +:+       +:+ "},
		{":#:        +#+    +:+ +#+    +:+ +#+    +:+           +#+    +#+    +:+ +#++:++#+        +#+ "},
		{"+#+   +#+# +#+    +#+ +#+    +#+ +#+    +#+           +#+    +#+    +#+ +#+    +#+       +#+ "},
		{"#+#    #+# #+#    #+# #+#    #+# #+#    #+#       #+# #+#    #+#    #+# #+#    #+#           "},
		{" ########   ########   ########  #########         #####      ########  #########        ### "},
	};

	char text4[7][110] = {
		{":::       ::: :::::::::: :::        :::              :::::::::   ::::::::  ::::    ::: ::::::::::       ::: "},	
		{":+:       :+: :+:        :+:        :+:              :+:    :+: :+:    :+: :+:+:   :+: :+:              :+: "},	
		{"+:+       +:+ +:+        +:+        +:+              +:+    +:+ +:+    +:+ :+:+:+  +:+ +:+              +:+ "},	
		{"+#+  +:+  +#+ +#++:++#   +#+        +#+              +#+    +:+ +#+    +:+ +#+ +:+ +#+ +#++:++#         +#+ "},	
		{"+#+ +#+#+ +#+ +#+        +#+        +#+              +#+    +#+ +#+    +#+ +#+  +#+#+# +#+              +#+ "},	
		{" #+#+# #+#+#  #+#        #+#        #+#              #+#    #+# #+#    #+# #+#   #+#+# #+#                  "},	
		{"  ###   ###   ########## ########## ##########       #########   ########  ###    #### ##########       ### "},	
	};
	int random = rand () % 4 + 1;
	attron(COLOR_PAIR(8));
	switch(random){
		case 1:
			for ( int i = 0; i < 7; i ++){
				mvprintw(x+i,y,"%s",text1[i]);
			}
			break;
		case 2:
			for ( int i = 0; i < 7; i ++){
				mvprintw(x+i,y,"%s",text2[i]);
			}
			break;
		case 3:
			for ( int i = 0; i < 7; i ++){
				mvprintw(x+i,y,"%s",text3[i]);
			}
			break;
		case 4:
			for ( int i = 0; i < 7; i ++){
				mvprintw(x+i,y,"%s",text4[i]);
			}
			break;

	}
	attroff(COLOR_PAIR(8));
}
