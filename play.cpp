#include<iostream>
#include<cstring>
#include <conio.h>
#include<thread>
using namespace std;
class game {
	int** table;
	int tsize,curpX=0,curpY=0,curp=1,mmf=1;;
	void draw(int dis=1) {
		clrscr();
		for (int i = 0; i < tsize; i++) {

			
			for (int k = 0; k < i; k++) {
				cout << " ";
				if (i != 0)
					cout << " ";
			}
			for (int j = 0; j < tsize; j++) {
				if(dis!=1&&i == curpX && j== curpY)
					cout << "0 ";
				else if (table[i][j] == 1)
					cout << "X ";
				else if (table[i][j] == 2)
					cout << "O ";	
				else
					cout << ". ";				
					
				if (j != tsize-1)
					cout << "- ";

			}
			cout <<"\n";
			if (i != tsize-1) {
				for (int k = 0; k < i; k++) {
					cout << " ";
					if (i != 0)
						cout << " ";
				}
				for (int j = 0; j < tsize; j++) {
					cout << " \\";
					if (j != tsize-1)
					cout << " /";

			}
			cout << "\n";
		}
			
		}
	}
void movements(){
	

	while(mmf==1){
	char c = getch();
	if(c=='e'||c=='E'){
		mmf=0;
	}
	else if(c=='w'||c=='W')
		curpX=(--curpX)<0?tsize-1:curpX;		
	else if(c=='s'||c=='S')
		curpX=++curpX%tsize;		
	else if(c=='d'||c=='D')	
		curpY=++curpY%tsize;
	else if(c=='a'||c=='A')
		curpY=(--curpY)<0?tsize-1:curpY;	
	else if(c=='\n'){
		if(curp==1&&table[curpX][curpY]==0){
			table[curpX][curpY]=1;
			curp=2;
		}
		else if(curp==2&&table[curpX][curpY]==0){
			table[curpX][curpY]=2;
			curp=1;

		}
		draw();
	}		
	}
}
public:
	game(int num) {
		tsize=num;
		table = new int* [tsize];
		for(int i=0;i<tsize;i++){
			table[i] = new int[tsize];
			memset(table[i], 0, sizeof(int) *  tsize);
		}
		
		thread t([&](){
			int flip=0;
			while(mmf){
				draw(flip);
				flip=++flip%2;
				sleep(1);
			}
		});
		movements();
		t.join();
		
		draw();

	}
	
};
int main() {
	game a(7);
	return 0;
}