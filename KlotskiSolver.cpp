#include <SDL2/SDL.h>
#include <stdio.h>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <bits/stdc++.h>
using namespace std;

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;
int fcount = 0;
int mousestate = 0;
SDL_Point lastm = {0,0};
SDL_Rect bframe;
static const int ep = 2;

bool init();
void initBlocks();

//#define FULLSCREEN_FLAG SDL_WINDOW_FULLSCREEN_DESKTOP
#define FULLSCREEN_FLAG 0

enum bType {hor,ver,ssq,lsq};
struct block {
	SDL_Rect R;
	bType type;

	void rotate() {
		if (type != hor && type != ver) return;
		type = (type==hor)?ver:hor;
		swap(R.w,R.h);
	}
};

#define NBLOCKS 10
block B[NBLOCKS];
block* dragged = NULL;
block* findBlock(int x, int y);
void close();
SDL_Window* gWindow = 0;
SDL_Renderer* gRenderer = 0;

bool init(){
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL_Init failed.  Error: %s\n", SDL_GetError());
		return false;
	}
	if(!SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1")) {
		printf("Warning: vsync hint didn't work.\n");
	}

	gWindow = SDL_CreateWindow("Sliding block puzzle solver",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH, SCREEN_HEIGHT,SDL_WINDOW_SHOWN|FULLSCREEN_FLAG);
	if(!gWindow) {
		printf("Failed to create main window. SDL Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_GetWindowSize(gWindow, &SCREEN_WIDTH, &SCREEN_HEIGHT);
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(!gRenderer) {
		printf("Failed to create renderer. SDL Error: %s\n", SDL_GetError());
		return false;
	}
	SDL_SetRenderDrawBlendMode(gRenderer,SDL_BLENDMODE_BLEND);

	initBlocks();
	return true;
}

void initBlocks(){
	int& W = SCREEN_WIDTH;
	int& H = SCREEN_HEIGHT;
	int h = H*3/4;
	int w = 4*h/5;
	int u = h/5-2*ep;
	int mw = (W-w)/2;
	int mh = (H-h)/2;

	bframe.x = (W-w)/2;
	bframe.y = (H-h)/2;
	bframe.w = w;
	bframe.h = h;

	for (size_t i = 0; i < 5; i++) {
		B[i].R.x = (mw-2*u)/2;
		B[i].R.y = mh + (i+1)*(u/5) + i*u;
		B[i].R.w = 2*(u+ep);
		B[i].R.h = u;
		B[i].type = hor;
	}
	B[4].R.x = mw+ep;
	B[4].R.y = mh+ep;
	B[4].R.w = 2*(u+ep);
	B[4].R.h = u;
	B[4].type = hor;

	for (size_t i = 0; i < 4; i++) {
		B[i+5].R.x = (W+w)/2 + (mw-2*u)/2 + (i%2)*(u+u/5);
		B[i+5].R.y = mh + ((i/2)+1)*(u/5) + (i/2)*u;
		B[i+5].R.w = u;
		B[i+5].R.h = u;
		B[i+5].type = ssq;
	}

	B[9].R.x = B[5].R.x + u/10;
	B[9].R.y = B[7].R.y + u + 2*u/5;
	B[9].R.w = 2*(u+ep);
	B[9].R.h = 2*(u+ep);
	B[9].type = lsq;
}

void drawBlocks(){
	// rectangles
	SDL_SetRenderDrawColor(gRenderer, 0x43, 0x4c, 0x5e, 0xff);
	for (size_t i = 0; i < 5; i++) {
		SDL_RenderFillRect(gRenderer,&B[i].R);
	}
	// small squares
	SDL_SetRenderDrawColor(gRenderer, 0x5e, 0x81, 0xac, 0xff);
	for (size_t i = 5; i < 9; i++) {
		SDL_RenderFillRect(gRenderer,&B[i].R);
	}
	// large square
	SDL_SetRenderDrawColor(gRenderer, 0xa3, 0xbe, 0x8c, 0xff);
	SDL_RenderFillRect(gRenderer,&B[9].R);
}

block* findBlock(int x, int y){
	for (int i = NBLOCKS-1; i >= 0; i--) {
		if (B[i].R.x <= x && x <= B[i].R.x + B[i].R.w && B[i].R.y <= y && y <= B[i].R.y + B[i].R.h) {
			return (B+i);
		}
	}
	return NULL;
}

void close(){
	SDL_DestroyRenderer(gRenderer); gRenderer = NULL;
	SDL_DestroyWindow(gWindow); gWindow = NULL;
	SDL_Quit();
}

void render(){
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(gRenderer);

	int& W = SCREEN_WIDTH;
	int& H = SCREEN_HEIGHT;
	int w = bframe.w;
	int h = bframe.h;
	SDL_SetRenderDrawColor(gRenderer, 0x39, 0x39, 0x39, 0xff);
	SDL_RenderDrawRect(gRenderer, &bframe);
	SDL_Rect rframe(bframe);
	int e = 3;
	rframe.x -= e;
	rframe.y -= e;
	rframe.w += 2*e;
	rframe.h += 2*e;
	SDL_RenderDrawRect(gRenderer, &rframe);

	SDL_Point p1,p2;
	SDL_SetRenderDrawColor(gRenderer, 0x19, 0x19, 0x1a, 0xff);
	p1.x = (W-w)/2;
	p1.y = (H-h)/2;
	p2.x = p1.x;
	p2.y = p1.y + h;
	for (size_t i = 1; i < 4; i++) {
		p1.x += w/4;
		p2.x += w/4;
		SDL_RenderDrawLine(gRenderer,p1.x,p1.y,p2.x,p2.y);
	}
	p1.x = (W-w)/2;
	p1.y = (H-h)/2;
	p2.x = p1.x + w;
	p2.y = p1.y;
	for (size_t i = 1; i < 5; i++) {
		p1.y += h/5;
		p2.y += h/5;
		SDL_RenderDrawLine(gRenderer,p1.x,p1.y,p2.x,p2.y);
	}
	SDL_SetRenderDrawColor(gRenderer, 0xd8, 0xde, 0xe9, 0x7f);
	SDL_Rect goal = {bframe.x + w/4 + ep, bframe.y + 3*h/5 + ep, w/2 - 2*ep, 2*h/5 - 2*ep};
	SDL_RenderDrawRect(gRenderer,&goal);

	drawBlocks();
	SDL_RenderPresent(gRenderer);
}

void snap(block* b){
	assert(b != NULL);
	int x = b->R.x - bframe.x;
	int y = b->R.y - bframe.y;
	int uw = bframe.w/4;
	int uh = bframe.h/5;
	int i = (y+uh/2)/uh;
	int j = (x+uw/2)/uw;
	if (0 <= i && i < 5 && 0 <= j && j < 4) {
		b->R.x = bframe.x + j*uw + ep;
		b->R.y = bframe.y + i*uh + ep;
	}
}

// Started editing from here.

string inverse(string s) { 
	/*	The way I have set up my program is to use strings to represent block configuration 
		therefore if I take the index+1 I access the block to the right but if I do the 
		inverse which swap the order so that index+1 actual gives me down one unit.
	*/
	// assume: s = "0056 1178 2299 3399 44XX";
	// s-prime sp = "01234 01234 5799X 6899X";
	string sp = s;
	sp[0] = s[0];
	sp[1] = s[4];
	sp[2] = s[8];
	sp[3] = s[12];
	sp[4] = s[16];
	sp[5] = s[1];
	sp[6] = s[5];
	sp[7] = s[9];
	sp[8] = s[13];
	sp[9] = s[17];
	sp[10] = s[2];
	sp[11] = s[6];
	sp[12] = s[10];
	sp[13] = s[14];
	sp[14] = s[18];
	sp[15] = s[3];
	sp[16] = s[7];
	sp[17] = s[11];
	sp[18] = s[15];
	sp[19] = s[19];
	return sp;
}

string reverse(string s) { 
	// Reverses the effects of the inverse back to original.
	string sp = s;
	sp[0] = s[0];
	sp[1] = s[5];
	sp[2] = s[10];
	sp[3] = s[15];
	sp[4] = s[1];
	sp[5] = s[6];
	sp[6] = s[11];
	sp[7] = s[16];
	sp[8] = s[2];
	sp[9] = s[7];
	sp[10] = s[12];
	sp[11] = s[17];
	sp[12] = s[3];
	sp[13] = s[8];
	sp[14] = s[13];
	sp[15] = s[18];
	sp[16] = s[4];
	sp[17] = s[9];
	sp[18] = s[14];
	sp[19] = s[19];
	return sp;
}

string setstring() {
	/* 	The main purpose of this function is to take any block configuration
		from the sdl screen and turn it into strings to that it can be manupulated
	*/
	string str = "XXXXXXXXXXXXXXXXXXXX";
	int x = bframe.w/4;
	int y = bframe.h/5;
	int& H = SCREEN_HEIGHT;
	int h = H*3/4;
	int u = h/5-2*ep;

	int Bpos = (((B[0].R.x - bframe.x-ep)/x)+((B[0].R.y - bframe.y-ep)/y)*4);
	if (B[0].R.w == 2*(u+ep)) {str.replace(Bpos,2,"00");}
	else {str.replace(Bpos,1,"0"); str.replace(Bpos+4,1,"0");}

	Bpos = (((B[1].R.x - bframe.x-ep)/x)+((B[1].R.y - bframe.y-ep)/y)*4);
	if (B[1].R.w == 2*(u+ep)) {str.replace(Bpos,2,"11");}
	else {str.replace(Bpos,1,"1"); str.replace(Bpos+4,1,"1");}

	Bpos = (((B[2].R.x - bframe.x-ep)/x)+((B[2].R.y - bframe.y-ep)/y)*4);
	if (B[2].R.w == 2*(u+ep)) {str.replace(Bpos,2,"22");}
	else {str.replace(Bpos,1,"2"); str.replace(Bpos+4,1,"2");}

	Bpos = (((B[3].R.x - bframe.x-ep)/x)+((B[3].R.y - bframe.y-ep)/y)*4);
	if (B[3].R.w == 2*(u+ep)) {str.replace(Bpos,2,"33");}
	else {str.replace(Bpos,1,"3"); str.replace(Bpos+4,1,"3");}

	Bpos = (((B[4].R.x - bframe.x-ep)/x)+((B[4].R.y - bframe.y-ep)/y)*4);
	if (B[4].R.w == 2*(u+ep)) {str.replace(Bpos,2,"44");}
	else {str.replace(Bpos,1,"4"); str.replace(Bpos+4,1,"4");}

	str.replace((((B[5].R.x - bframe.x-ep)/x)+((B[5].R.y - bframe.y-ep)/y)*4),1,"5");
	str.replace((((B[6].R.x - bframe.x-ep)/x)+((B[6].R.y - bframe.y-ep)/y)*4),1,"6");
	str.replace((((B[7].R.x - bframe.x-ep)/x)+((B[7].R.y - bframe.y-ep)/y)*4),1,"7");
	str.replace((((B[8].R.x - bframe.x-ep)/x)+((B[8].R.y - bframe.y-ep)/y)*4),1,"8");

	str.replace((((B[9].R.x - bframe.x-ep)/x)+((B[9].R.y - bframe.y-ep)/y)*4),2,"99");
	str.replace((((B[9].R.x - bframe.x-ep)/x)+((B[9].R.y - bframe.y-ep)/y)*4)+4,2,"99");

	return str;
}

void setboard(string s) {
	/* 	This function takes a string an set the block in the screen in the 
		corrent configuration/order.
	*/
	string sp = inverse(s);
	int x = bframe.w/4;
	int y = bframe.h/5;
	int& H = SCREEN_HEIGHT;
	int h = H*3/4;
	int u = h/5-2*ep;

	for (int i = 0; i < 5; i++) {
		B[i].R.w = u;
		B[i].R.h = 2*(u+ep);
		B[i].type = ver;
	}

	B[0].R.x = (s.find_first_of("0") % 4)*x + bframe.x+ep;
	B[0].R.y = (sp.find_first_of("0") % 5)*y + bframe.y+ep;
	if (s.find("00") < 19) {
		B[0].type = hor;
		swap(B[0].R.w,B[0].R.h);
	}

	B[1].R.x = (s.find_first_of("1") % 4)*x + bframe.x+ep;
	B[1].R.y = (sp.find_first_of("1") % 5)*y + bframe.y+ep;
	if (s.find("11") < 19) {
		B[1].type = hor;
		swap(B[1].R.w,B[1].R.h);
	}

	B[2].R.x = (s.find_first_of("2") % 4)*x + bframe.x+ep;
	B[2].R.y = (sp.find_first_of("2") % 5)*y + bframe.y+ep;
	if (s.find("22") < 19) {
		B[2].type = hor;
		swap(B[2].R.w,B[2].R.h);
	}

	B[3].R.x = (s.find_first_of("3") % 4)*x + bframe.x+ep;
	B[3].R.y = (sp.find_first_of("3") % 5)*y + bframe.y+ep;
	if (s.find("33") < 19) {
		B[3].type = hor;
		swap(B[3].R.w,B[3].R.h);
	}

	B[4].R.x = (s.find_first_of("4") % 4)*x + bframe.x+ep;
	B[4].R.y = (sp.find_first_of("4") % 5)*y + bframe.y+ep;
	if (s.find("44") < 19) {
		B[4].type = hor;
		swap(B[4].R.w,B[4].R.h);
	}

	B[5].R.x = (s.find_first_of("5") % 4)*x + bframe.x+ep;
	B[5].R.y = (sp.find_first_of("5") % 5)*y + bframe.y+ep;
	B[6].R.x = (s.find_first_of("6") % 4)*x + bframe.x+ep;
	B[6].R.y = (sp.find_first_of("6") % 5)*y + bframe.y+ep;
	B[7].R.x = (s.find_first_of("7") % 4)*x + bframe.x+ep;
	B[7].R.y = (sp.find_first_of("7") % 5)*y + bframe.y+ep;
	B[8].R.x = (s.find_first_of("8") % 4)*x + bframe.x+ep;
	B[8].R.y = (sp.find_first_of("8") % 5)*y + bframe.y+ep;
	B[9].R.x = (s.find_first_of("9") % 4)*x + bframe.x+ep;
	B[9].R.y = (sp.find_first_of("9") % 5)*y + bframe.y+ep;
}

vector<string> possiblemove(string s) {
	/*	this function take one string configuraton, follows the written rules, and
		produces other string which are possible moves that the blocks can take.
	*/
	size_t a,b,c,ap,bp;
	string v,sp,temp;
	vector<string> vect;
	sp = inverse(s);

	a = s.find_first_of("X");
	b = s.find_last_of("X");
	ap = sp.find_first_of("X");
	bp = sp.find_last_of("X");
	if (s[b]==s[a+1]){c = 2;}//hor
	else if (s[b]==s[a+4]){c = 1;}//ver
	else {c = 0;}//seperate
	
	/*	this is done in two parts first, a string is created that has codes which 
		are 3 units long which state 
		1. which block (0-9)
		2. direction (left/right/up/down)
		3. refrence to which empty space (a = first empty, b = second empty, c = both)
		having this step ensures that if one block has several movement possibilities
		then all of them are taken into consideration.
	*/

    if ((a%4)<3) {if (a+1 == s.find("5") || a+1 == s.find("6") || a+1 == s.find("7") || a+1 == s.find("8")) {v.push_back(s[a+1]); v.push_back('l'); v.push_back('a');}}
    if ((a%4)>0) {if (a-1 == s.find("5") || a-1 == s.find("6") || a-1 == s.find("7") || a-1 == s.find("8")) {v.push_back(s[a-1]); v.push_back('r'); v.push_back('a');}}
    if ((b%4)<3) {if (b+1 == s.find("5") || b+1 == s.find("6") || b+1 == s.find("7") || b+1 == s.find("8")) {v.push_back(s[b+1]); v.push_back('l'); v.push_back('b');}}
    if ((b%4)>0) {if (b-1 == s.find("5") || b-1 == s.find("6") || b-1 == s.find("7") || b-1 == s.find("8")) {v.push_back(s[b-1]); v.push_back('r'); v.push_back('b');}}

    if ((a%4)<2) {if (a+1 == s.find("00") || a+1 == s.find("11") || a+1 == s.find("22") || a+1 == s.find("33") || a+1 == s.find("44")) {v.push_back(s[a+1]); v.push_back('l'); v.push_back('a');}}
    if ((a%4)>1) {if (a-2 == s.find("00") || a-2 == s.find("11") || a-2 == s.find("22") || a-2 == s.find("33") || a-2 == s.find("44")) {v.push_back(s[a-2]); v.push_back('r'); v.push_back('a');}}
    if ((b%4)<2) {if (b+1 == s.find("00") || b+1 == s.find("11") || b+1 == s.find("22") || b+1 == s.find("33") || b+1 == s.find("44")) {v.push_back(s[b+1]); v.push_back('l'); v.push_back('b');}}
    if ((b%4)>1) {if (b-2 == s.find("00") || b-2 == s.find("11") || b-2 == s.find("22") || b-2 == s.find("33") || b+2 == s.find("44")) {v.push_back(s[b-2]); v.push_back('r'); v.push_back('b');}}

    if ((ap%5)<4) {if (ap+1 == sp.find("5") || ap+1 == sp.find("6") || ap+1 == sp.find("7") || ap+1 == sp.find("8")) {v.push_back(sp[ap+1]); v.push_back('d'); v.push_back('a');}}
    if ((ap%5)>0) {if (ap-1 == sp.find("5") || ap-1 == sp.find("6") || ap-1 == sp.find("7") || ap-1 == sp.find("8")) {v.push_back(sp[ap-1]); v.push_back('u'); v.push_back('a');}}
    if ((bp%5)<4) {if (bp+1 == sp.find("5") || bp+1 == sp.find("6") || bp+1 == sp.find("7") || bp+1 == sp.find("8")) {v.push_back(sp[bp+1]); v.push_back('d'); v.push_back('b');}}
    if ((bp%5)>0) {if (bp-1 == sp.find("5") || bp-1 == sp.find("6") || bp-1 == sp.find("7") || bp-1 == sp.find("8")) {v.push_back(sp[bp-1]); v.push_back('u'); v.push_back('b');}}

    if ((ap%5)<3) {if (ap+1 == sp.find("00") || ap+1 == sp.find("11") || ap+1 == sp.find("22") || ap+1 == sp.find("33") || ap+1 == sp.find("44")) {v.push_back(sp[ap+1]); v.push_back('d'); v.push_back('a');}}
    if ((ap%5)>1) {if (ap-2 == sp.find("00") || ap-2 == sp.find("11") || ap-2 == sp.find("22") || ap-2 == sp.find("33") || ap-2 == sp.find("44")) {v.push_back(sp[ap-2]); v.push_back('u'); v.push_back('a');}}
    if ((bp%5)<3) {if (bp+1 == sp.find("00") || bp+1 == sp.find("11") || bp+1 == sp.find("22") || bp+1 == sp.find("33") || bp+1 == sp.find("44")) {v.push_back(sp[bp+1]); v.push_back('d'); v.push_back('b');}}
    if ((bp%5)>1) {if (bp-2 == sp.find("00") || bp-2 == sp.find("11") || bp-2 == sp.find("22") || bp-2 == sp.find("33") || bp+2 == sp.find("44")) {v.push_back(sp[bp-2]); v.push_back('u'); v.push_back('b');}}

	if (c==1) {//ver
		if ((a%4)<3) {if (s[a+1] == s[b+1]) {v.push_back(s[a+1]); v.push_back('l'); v.push_back('c');}}
		if ((a%4)>0) {if (s[a-1] == s[b-1]) {v.push_back(s[a-1]); v.push_back('r'); v.push_back('c');}}
	}
	if (c==2) {//hor
		if ((ap%5)<4) {if (sp[ap+1] == sp[bp+1]) {v.push_back(sp[ap+1]); v.push_back('u'); v.push_back('c');}}
		if ((ap%5)>0) {if (sp[ap-1] == sp[bp-1]) {v.push_back(sp[ap-1]); v.push_back('d'); v.push_back('c');}}
	}

//	vect.push_back(v); // for testing purposes so i can which codes are sent from step 1

	/*	the next step simply takes the code and create new strings representing each
		possible move, and adds it to a vector which is then returned.
	*/	

	while (!v.empty()) {
		temp = s;
	if (v[0] == '0' || v[0] == '1' || v[0] == '2' || v[0] == '3' || v[0] == '4') {
		if (v[2] == 'c') {// XX
			if (v[1] == 'l') {
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")+1]);
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")+1]);
			}
			if (v[1] == 'r') {
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")-1]);
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")-1]);
			}
			if (v[1] == 'u') {
				temp = sp;
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")+1]);
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")+1]);
				temp = reverse(temp);
			}
			if (v[1] == 'd') {
				temp = sp;
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")-1]);
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")-1]);
				temp = reverse(temp);
			}
		}
	}
	if (v[0] == '9') {
		if (v[2] == 'c') {// XX
			if (v[1] == 'l') {
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")+2]);
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")+2]);
			}
			if (v[1] == 'r') {
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")-2]);
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")-2]);
			}
			if (v[1] == 'u') {
				temp = sp;
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")+2]);
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")+2]);
				temp = reverse(temp);
			}
			if (v[1] == 'd') {
				temp = sp;
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")-2]);
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")-2]);
				temp = reverse(temp);
			}
		}
	}
	if (v[0] == '5' || v[0] == '6' || v[0] == '7' || v[0] == '8') {
		if (v[2] == 'b') {// last X
			if (v[1] == 'l') {
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")+1]);
			}
			if (v[1] == 'r') {
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")-1]);
			}
			if (v[1] == 'd') {
				temp = sp;
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")+1]);
				temp = reverse(temp);
			}
			if (v[1] == 'u') {
				temp = sp;
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")-1]);
				temp = reverse(temp);
			}
		}
		if (v[2] == 'a') {// first X
			if (v[1] == 'l') {
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")+1]);
			}
			if (v[1] == 'r') {
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")-1]);
			}
			if (v[1] == 'd') {
				temp = sp;
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")+1]);
				temp = reverse(temp);
			}
			if (v[1] == 'u') {
				temp = sp;
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")-1]);
				temp = reverse(temp);
			}
		}
	}
	if (v[0] == '0' || v[0] == '1' || v[0] == '2' || v[0] == '3' || v[0] == '4') {
		if (v[2] == 'b') {// last X
			if (v[1] == 'l') {
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")+2]);
			}
			if (v[1] == 'r') {
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")-2]);
			}
			if (v[1] == 'd') {
				temp = sp;
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")+2]);
				temp = reverse(temp);
			}
			if (v[1] == 'u') {
				temp = sp;
				swap(temp[temp.find_last_of("X")], temp[temp.find_last_of("X")-2]);
				temp = reverse(temp);
			}
		}
		if (v[2] == 'a') {// first X
			if (v[1] == 'l') {
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")+2]);
			}
			if (v[1] == 'r') {
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")-2]);
			}
			if (v[1] == 'd') {
				temp = sp;
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")+2]);
				temp = reverse(temp);
			}
			if (v[1] == 'u') {
				temp = sp;
				swap(temp[temp.find_first_of("X")], temp[temp.find_first_of("X")-2]);
				temp = reverse(temp);
			}
		}
	}
	vect.push_back(temp);
	v.erase(0,3);
	}
	return vect;
}

void printm(string a) { 
	/*	for testing purposes
		prints the board in the command prompt
	*/
	for (size_t i = 0; i < 20; i++) {
		cout << a[i];
		if (i%4 == 3)
			cout << endl;
	}
}

string skeleton(string s) {
	/*	this was my attempt at shorting the time and it does do its job
		but the solver in my opinion is still very slow.
		i realize that from part 1 of my bfs i was getting alot of clone
		strings for example,
		001122334456997899XX
		112233440087995699XX
		these 2 string are considered different but actually they are 
		the same if we represent them as blocks on the screen.
		so having this layer of check eliminates several nodes in the
		graph of the bfs shorting the search process.
	*/	
	string sp = s;

	if (sp.find("00") < 20) {sp.replace(sp.find("00"),2,"AA");}
	if (sp.find("11") < 20) {sp.replace(sp.find("11"),2,"AA");}
	if (sp.find("22") < 20) {sp.replace(sp.find("22"),2,"AA");}
	if (sp.find("33") < 20) {sp.replace(sp.find("33"),2,"AA");}
	if (sp.find("44") < 20) {sp.replace(sp.find("44"),2,"AA");}

	if (sp.find("0") < 20) {sp.replace(sp.find_first_of("0"),1,"B");}
	if (sp.find("0") < 20) {sp.replace(sp.find_first_of("0"),1,"B");}
	if (sp.find("1") < 20) {sp.replace(sp.find_first_of("1"),1,"B");}
	if (sp.find("1") < 20) {sp.replace(sp.find_first_of("1"),1,"B");}
	if (sp.find("2") < 20) {sp.replace(sp.find_first_of("2"),1,"B");}
	if (sp.find("2") < 20) {sp.replace(sp.find_first_of("2"),1,"B");}
	if (sp.find("3") < 20) {sp.replace(sp.find_first_of("3"),1,"B");}
	if (sp.find("3") < 20) {sp.replace(sp.find_first_of("3"),1,"B");}
	if (sp.find("4") < 20) {sp.replace(sp.find_first_of("4"),1,"B");}
	if (sp.find("4") < 20) {sp.replace(sp.find_first_of("4"),1,"B");}

	sp.replace(sp.find_first_of("5"),1,"C");
	sp.replace(sp.find_first_of("6"),1,"C");
	sp.replace(sp.find_first_of("7"),1,"C");
	sp.replace(sp.find_first_of("8"),1,"C");

	sp.replace(sp.find_first_of("9"),1,"D");
	sp.replace(sp.find_first_of("9"),1,"D");
	sp.replace(sp.find_first_of("9"),1,"D");
	sp.replace(sp.find_first_of("9"),1,"D");

	return sp;
}

vector<string> myBFSx2(string first_string) {
	/*	there are 2 parts to this search both has elements of bfs 
		arguably not standard but my bootleg version.
		first of all i need to impliment a recursion which creates my graph
		with vertices and edges.
	*/
	
	//x1
	string str, str_skel;
	vector<string> finalpath, vec, vect, skel;
	list<string> que;
	vector<vector<int> > edges;

	vec.push_back(first_string);
	que.push_back(first_string);
	skel.push_back(skeleton(first_string));
	edges.push_back(vector<int>());

	bool solutionfound = false;
	bool match = false;
	int tempint = 1;

	if (first_string.find_first_of("9") == 13) {
		solutionfound = true;
		finalpath.push_back(first_string);
		return finalpath;
	}

	while (!que.empty() && !solutionfound) {
		vect = possiblemove(que.front()); // vect holds all of the vertices
        que.pop_front(); //  the queue goes trough all the vect

		for (size_t i = 0; i < vect.size(); i++) {
			str = vect[i];
			str_skel = skeleton(str);

			if (str.find_first_of("9") == 13) { // if answer is found 
				solutionfound = true;
				vec.push_back(str);
				edges[edges.size()-1].push_back(vec.size()-1);		//	edges are stored by index (int) rather then string 
				break;												//	i thought this way i can reduce storage use.
			}

			tempint = vec.size();
			match = false;
			for (size_t j = 0; j < tempint; j++) {	//	this is where all the comparasion tests happen
				if (vec[j] == str) {				//	and the reason my code is so slow because as
					match = true;					//	the size of vect increase the longet this step takes 
				}										
				if (skel[j] == str_skel) {			// skeleton comparasion
					match = true;
				}
			}
			if (!match) {
				edges[edges.size()-1].push_back(vec.size());
				vec.push_back(str);
				que.push_back(str);
				skel.push_back(str_skel);
			}
		}
		edges.push_back(vector<int>());
		if (que.empty()) { // if queue is empty that means theres no solution
			finalpath.clear();
			return finalpath;
		}
	}
	
	// once we have the graph we can do the search for shortest path
	
	//x2
	int start = 0;
	int finish = vec.size()-1;
	int size = vec.size();
	int last[size];
	int totalsteps[size];
	list<int> queue;
	bool visited[size];
	vector<int> path;
    int move = finish;

    path.push_back(move);
	for (int i = 0; i < size; i++) {
        visited[i] = false;
        totalsteps[i] = INT_MAX; // we start from the answer and back track
        last[i] = -1;
    }

	visited[start] = true;
    totalsteps[start] = 0;
    queue.push_back(start);

    bool loop = true;
	while (loop) {
        int u = queue.front();
        queue.pop_front(); 								// again we have a queue that goes through the completed
		for (int i = 0; i < edges[u].size(); i++) {		// graph this time. step 1 ensured that the path
            if (visited[edges[u][i]] == false) {		// exists we just have to back track to find it
                visited[edges[u][i]] = true;
                totalsteps[edges[u][i]] = totalsteps[u] + 1;
                last[edges[u][i]] = u;							// records all the movement/path
                queue.push_back(edges[u][i]);
                if (edges[u][i] == finish) {
					loop = false;
				}
			}
        }
    }
    while (last[move] != -1) {
        path.push_back(last[move]);
        move = last[move];
    }

	for(int i = path.size()-1; i >= 0; i--) { 	// since all this time i was working with index so i need 
		int temp = path[i];						// to turn it back into string inorder to be able to display it
		finalpath.push_back(vec[temp]);			// i did this backward (i--) to get result vector in right order
	}

	return finalpath;
}

bool checkstring(string s) { 
	/* 	this is not complete, i did not want to focus all my time on this.
		but the idea is that i can write 10-20 more lines worth of conditions 
		to check if the input (from cmd) is valid. for our purpose i felt this was
		enough. but yes, code will break if input is invalid beyond these conditions 
	*/
	if (s.size() == 20 && s.find_first_of("X") != s.find_last_of("X") && s.find_first_of("99") != s.find_last_of("99")) {
		if (s.find_first_of("5") == s.find_last_of("5") && s.find_first_of("8") == s.find_last_of("8")) {
			return true;
		}
	}
	return false;
}

int main(int argc, char *argv[]){
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	printf("Welcome to Klotski Solver\n");
	printf("By: Md Sakil Khan\n");
	printf("--------------------------------------------------------------\n");
	printf("Controls:\n");
	printf("Press l - to load uploaded board.\n");
	printf("Press p - to load preset boards.\n");
	printf("Press s - to solve current board.\n");
	printf("Press left - to go back through the solution.\n");
	printf("Press right - to go forward through the solution.\n");
	printf("Press g - to enter or exit game mode.\n");
	printf("Press q - at anytime to exit program.\n");
	printf("--------------------------------------------------------------\n");
	printf("While in game mode:\n");
	printf("Press left or right - to go through possible move options.\n");
	printf("Press x - to confirm option.\n");
	printf("Press g - to end game mode.\n");
	printf("--------------------------------------------------------------\n");
	printf("If you want to upload a specific block configuration, enter a\n");
	printf("4x5 materix using numbers 0-9 for blocks and X for empty.\n");
	printf("For example:\n");
	printf(">0056\n");
	printf(">1178\n");
	printf(">2299\n");
	printf(">3499\n");
	printf(">34XX\n");
	printf("Or:\n");
	printf(">005611782299349934XX\n");
	printf("--------------------------------------------------------------\n");
	printf("Otherwise press Enter after '>' to start game.\n");

	string inputText, savedText, masterString, tempstring;
	vector<string> path, game;
	vector<string> presetString = {"012X012X349934995678", "005611782299349934XX", "0011223349954996X78X", "567099109912X442X833", "09910991233425647XX8"};
	bool gamemode = false;
	int index = 0;
	int presetInt = 0;
	size_t gameindex = 0;
	int time_f, time_i;
	while (savedText.size() < 20) { // a getline method for getting string info from cmd
        cout << ">";
        getline (cin, inputText);
        savedText.insert(savedText.size(), inputText);
		if (inputText == "") {break;}
	}

	if(!init()) {
		printf( "Failed to initialize from main().\n" );
		return 1;
	}

	atexit(close);
	bool quit = false;
	SDL_Event e;

	while(!quit) {
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_MOUSEMOTION && !gamemode) {
				if (mousestate == 1 && dragged) {
					int dx = e.button.x - lastm.x;
					int dy = e.button.y - lastm.y;
					lastm.x = e.button.x;
					lastm.y = e.button.y;
					dragged->R.x += dx;
					dragged->R.y += dy;
				}
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN && !gamemode) {
				if (e.button.button == SDL_BUTTON_RIGHT) {
					block* b = findBlock(e.button.x,e.button.y);
					if (b) {
						b->rotate();
					}
				}
				else {
					mousestate = 1;
					lastm.x = e.button.x;
					lastm.y = e.button.y;
					dragged = findBlock(e.button.x,e.button.y);
				}
			}
			else if (e.type == SDL_MOUSEBUTTONUP && !gamemode) {
				if (e.button.button == SDL_BUTTON_LEFT) {
					mousestate = 0;
					lastm.x = e.button.x;
					lastm.y = e.button.y;
					if (dragged) {
						snap(dragged);
					}
					dragged = NULL;
				}
			}
			else if (e.type == SDL_QUIT) {
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
					case SDLK_ESCAPE:
					case SDLK_q:
						quit = true;
						break;
					case SDLK_LEFT: // go back
						if (gamemode && !game.empty()) {				// left/right work the same i take a vector
							gameindex++;								// of strings and iterate trough the vector
							setboard(game[gameindex % game.size()]);	// rendering on screen and remembering the index.
							render();
						}
						else if (index > 0 && !path.empty()) {
							index--;
							setboard(path[index]);
							render();
						}
						break;
					case SDLK_RIGHT: // go forward
						if (gamemode && !game.empty()) {
							gameindex++;
							setboard(game[gameindex % game.size()]);
							render();
						}
						else if (index < path.size()-1 && !path.empty()) {
							index++;
							setboard(path[index]);
							render();
						}
						break;
					case SDLK_l: // load board from cmd input
						if (gamemode) {break;}
						if (checkstring(savedText)) {
							masterString = savedText;
							setboard(masterString);
							render();
						}
						else {
							printf( "Saved string is corrupt. Failed to initialize.\n" );
						}
						break;
					case SDLK_p: // load preset board
						if (gamemode) {break;}
						masterString = presetString[presetInt % presetString.size()];
						setboard(masterString);
						render();
						presetInt++;
						break;
					case SDLK_s: // solve current board
						if (gamemode) {break;}
						masterString = setstring();
						if (checkstring(masterString)) {
							time_i = time(0);
							cout << "Loading... Please wait...\n";
							path = myBFSx2(masterString);
							if (path.size() == 0) {
								cout << "No solutions found" << endl;
							}
							else if (path.size() == 1) {
								cout << "Already solved" << endl;
							}
							else {
								for (size_t i = 0; i < path.size(); i++) {
									string temp = path[i];
									/* testing in cmd
									printm(temp);
									cout << endl;
									*/
									setboard(temp);
									render();
								}
								index = path.size()-1;
								cout << "Solved!!!" << endl;
								cout << "Amount of Steps: " << path.size()-1 << endl;
							}
							time_f = time(0);
							cout << "Amount of time: < " << time_f - time_i << "s" << endl;
						}
						else {
							printf( "Invalid configuration\n" );
						}
						break;
					case SDLK_g: // starts/ends gamemode
						/*	how gamemode work:
							its simple didnt take much extra coding. i use setstring() to read the current board
							on screen and get the results from possiblemove() of that state. using the left/right 
							now i can iterate trough the possible move vector. pressing x just repeats this process.
						*/
						if (!gamemode) {
							gamemode = true;
							game = possiblemove(setstring());
							game.push_back(setstring());
							gameindex = 0;
							cout << "Game mode entered" << endl;
						}
						else {
							gamemode = false;
							cout << "Game mode exit" << endl;
						}
						break;
					case SDLK_x: // a trigger for gamemode
						tempstring = setstring();
						if (gamemode) {
							if (13 == tempstring.find_first_of("9")) {
								cout << "You win!!!" << endl;
								gamemode = false;
								break;
							}
							game = possiblemove(setstring());
							game.push_back(setstring());
							gameindex = 0;
						}
						break;
					default:
						break;
				}
			}
		}
		fcount++;
		render();
	}
	printf("total frames rendered: %i\n",fcount);
	return 0;
}
