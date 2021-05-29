#include <stdio.h>
#include "lib.h"

#include "obj.h"

char mode=0; // combat entity tiles textinput_names retile_menu
char s[256],sp; // for text input
char current=0,cx,cy;
unsigned char newtile;short newcol; // for retiling
field rin;

//img doom=loadimg("doom.ppm");
//img hc=loadimg("beryldintro.ppm");

int main(){
	Framework window=Framework(640,480,2);

	rin.load("save.dnd");

	window.userfunc=[](Framework*window){
		//window->pi(doom,0,rin.size*8+24);
		//window->pi(hc,123,rin.size*8+24);

		// draw field border
		window->pc('+',0,1,0xfff,0);
		window->pc('+',rin.size+1,1,0xfff,0);
		window->pc('+',0,rin.size+2,0xfff,0);
		window->pc('+',rin.size+1,rin.size+2,0xfff,0);
		for(int i=0;i<rin.size;i++){
			window->pc('|',0,i+2,0xfff,0);
			window->pc('|',rin.size+1,i+2,0xfff,0);

			window->pc('-',i+1,1,0xfff,0);
			window->pc('-',i+1,rin.size+2,0xfff,0);
		}

		// draw field
		for(int x=0;x<rin.size;x++)
		for(int y=0;y<rin.size;y++)
		window->pc(
			rin.t[y*rin.size+x].c,
			x+1,
			y+2,
			rin.t[y*rin.size+x].fgc,
			rin.t[y*rin.size+x].bgc
		);

		// draw entities
		for(int i=0;i<rin.ent;i++){
			entity e=*rin.e[i];
			short c=
				e.effect==1?(e.color>>2)&0x333:
				e.effect==9?(e.color>>1)&0x777:
				e.effect==11?0xf22:
				e.color;

			window->pc(e.c,e.x+1,e.y+2,
				i==current?0:c,
				i==current?c:0
			);

			window->ps(e.name,rin.size+2,i+1,
				i==current?0:e.color,
				i==current?e.color:0
			);

			// effects
			window->ps(
				stat_names[e.effect],
				rin.size+strlen(e.name)+3,i+1,
				0x777,0
			);
		}

		// draw header and tooltips
		switch(mode){
			case 0: // combat
				window->ps("Combat",0,0,0xf00,0);
				break;
			case 1: // entity edit
				window->ps("Entities",0,0,0x48f,0);
				window->ps("A - new entity\nEnter - rename current",1,rin.size+3,0xfff,0);
				// true to its name, a only creates a new entity w/ default parameters, and we gotta rename it afterwards
				break;
			case 2: // tiles
				// draw currently selected
				window->pc(
					rin.t[cy*rin.size+cx].c,
					cx+1,cy+2,
					//rin.t[cx*rin.size+cy].bgc,
					0x333,
					rin.t[cy*rin.size+cx].fgc
				);
				window->ps("Map edit",0,0,0x4f4,0);
				window->ps("Arrow keys - select\nEnter - retile",1,rin.size+3,0xfff,0);
				break;
			case 3: // textinput
				// XXX tiles n shit
				window->ps("Rename",0,0,0xf84,0);
				window->ps("Press 'Enter' to confirm name",1,rin.size+3,0xfff,0);
				// XXX print current buffer
				window->ps(s,1,1,0xfff,0);
				break;
			case 4: // retile
				// character picker
				for(int x=0;x<16;x++)
				for(int y=0;y<16;y++){
					bool c=((y<<4)|x)==newtile;
					window->pc(
						(y<<4)|x,
						x+1,y+2,
						c?0xfff:0x888,
						c?0x444:0x000
					);
				}

				// and color pickers
				for(int i=0;i<16;i++){
					window->pc((newcol&0xf00)==(i<<8)?'#':'=',i+1,18,i<<8 | newcol&0x0ff);
					window->pc((newcol&0x0f0)==(i<<4)?'#':'=',i+1,19,i<<4 | newcol&0xf0f);
					window->pc((newcol&0x00f)==(i<<0)?'#':'=',i+1,20,i<<0 | newcol&0xff0);
				}

				window->ps("Tile selection",0,0,0xfff,0);
				window->ps("Z\nX\nC\nArrow keys - select",1,18,0xfff,0);
				window->ps("A\nS\nD",16,18,0xfff,0);
				break;
		}



		// debugging mouse
		window->p(0xfff,window->mouse.x,window->mouse.y);
		if(window->mouse.left)window->pc('L',0,50,0xfff,0);
		if(window->mouse.right)window->pc('R',2,50,0xfff,0);
		if(window->mouse.left || window->mouse.right)
			printf("%c%c\n",
				window->mouse.left?'l':'.',
				window->mouse.right?'r':'.'
			);


		char inc=1;
		if(window->key==0xe1)inc=6;
		if(SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LSHIFT])inc=6;


		// process input
		if(window->keypress)
		if(window->event.type==SDL_KEYDOWN){
			if(window->key==0x1b){rin.save("backup.dnd");return false;} // ESC
			if(window->key==0x9&&mode<3)mode=(mode+1)%3; // XXX maybe enter this individually?

			// XXX these should not work in text entry mode
			if(mode!=3){
				if(window->key==0x6d)rin.save("save.dnd"); // m
				if(window->key==0x6f)rin.load("save.dnd"); // o
				if(window->key==0x6c)rin.import("chart"); // l
			}

			switch(mode){
				case 0: // combat

					if(window->key==0x4f)rin.e[current]->x+=inc;
					if(window->key==0x50)rin.e[current]->x-=inc;
					if(window->key==0x51)rin.e[current]->y+=inc;
					if(window->key==0x52)rin.e[current]->y-=inc;
					if(window->key==0x73){rin.e[current]->effect++;rin.e[current]->effect%=stat_num;}
					if(window->key==0x61){rin.e[current]->effect+=stat_num-1;rin.e[current]->effect%=stat_num;} // dont ask
					if(window->key==0x20){current++;current%=rin.ent;}
					if(window->key==0xd){
						entity*t=rin.e[current];
						if(rin.e[current+1]){
							rin.e[current]=rin.e[current+1];
							rin.e[current+1]=t;
							current++;
						}else{
							for(int i=current;i>0;i--)rin.e[i]=rin.e[i-1];
								rin.e[0]=t;current=0;
						}
					}
					break;

				case 1: // entity
					if(window->key==0x20){current++;current%=rin.ent;}
					if(window->key==0xd){
						printf("rename\n");
						for(int i=0;i<64;i++)s[i]=0;
						sp=0;
						mode=3;
					}
					if(window->key==0x61)
						// XXX do we append or insert?
						rin.e[rin.ent++]=new entity('@',0xfff,"Urist");
					if(window->key==0x8){
						delete rin.e[current];
						// XXX shift the rest
					}
					// XXX also recolor - we can probably copy most of that from tile edit
					break;

				case 2: // tile
					if(window->key==0x4f)cx+=inc;
					if(window->key==0x50)cx-=inc;
					if(window->key==0x51)cy+=inc;
					if(window->key==0x52)cy-=inc;
					if(window->key==0xd){
						newtile=rin.t[cy*rin.size+cx].c;
						newcol=rin.t[cy*rin.size+cx].fgc;
						mode=4;
					}
					cx%=rin.size;
					cy%=rin.size;
					break;

				case 3: // rename
					// XXX this gon need a lot more
					//61-7a, d confirms
					if(0x60<window->key && window->key<0x7b){
						s[sp++]=window->key-0x61+(inc-1?'A':'a');
						printf("> %c\n",window->key-0x61+'A');
					}

					if(window->key==0x20){
						printf("> ' '\n");
						s[sp++]=' ';
					}

					if(window->key==0x8){
						s[--sp]=0;
					}


					if(window->key==0xd){
						//free(rin.e[current]->name); // is this needed?
						rin.e[current]->name=(char*)malloc(sp);
						for(int i=0;i<sp;i++)
							rin.e[current]->name[i]=s[i];
						rin.e[current]->name[sp]=0;

						mode=1;
					}
					break;

				case 4: // retile
					if(window->key==0x4f)newtile++;
					if(window->key==0x50)newtile--;
					if(window->key==0x51)newtile+=0x10;
					if(window->key==0x52)newtile-=0x10;

					// color picks
					if(window->key==0x61)newcol=newcol&0x0ff|(newcol+0x100)&0xf00;
					if(window->key==0x73)newcol=newcol&0xf0f|(newcol+0x010)&0x0f0;
					if(window->key==0x64)newcol=newcol&0xff0|(newcol+0x001)&0x00f;

					if(window->key==0x7a)newcol=newcol&0x0ff|(newcol-0x100)&0xf00;
					if(window->key==0x78)newcol=newcol&0xf0f|(newcol-0x010)&0x0f0;
					if(window->key==0x63)newcol=newcol&0xff0|(newcol-0x001)&0x00f;

					newcol%=0x1000;
					if(window->key==0xd){
						// set tile
						rin.t[cy*rin.size+cx]=(tile){newtile,newcol,0,0};
						mode=2;
					}
					break;

			} // end switch
		}

		// you can select tiles with the mouse
		// had to write it separately because the above only handles keypresses
		if(mode==4 && window->mouse.left){
			int x=window->mouse.x/8-1,y=window->mouse.y/8-2;
			if(x<16&&y<16)
			newtile=(y<<4)|x;
		}

		return true;
	};

	window.run();

	return 0;
}
