#include <SDL2/SDL.h>
#include <time.h>

struct img{
	int sx,sy;
	short*data;
};

img loadimg(const char*filename){
	img rin=(img){0,0,0};
  char a;
  FILE*f=fopen(filename,"r");
  for(int i=0;i<3;i++)getc(f); // P6\n
  while((a=getc(f))!=' ')rin.sx=rin.sx*10+a-48; // <X> 
  while((a=getc(f))!=10)rin.sy=rin.sy*10+a-48; // <Y>\n
  rin.data=(short*)malloc(rin.sx*rin.sy*sizeof(short));
  while((a=getc(f))!=10); // 255\n

	for(int i=0;i<rin.sx*rin.sy;i++)
    rin.data[i]=
      (getc(f)>>4)<<8|
      (getc(f)>>4)<<4|
      (getc(f)>>4)<<0; // fancy!
  fclose(f);
	return rin;
}

class Framework{public:
	int _sx,_sy,_ss; // width, height - in that order!
	short*newmap,*oldmap;
	char font[1<<14];
	clock_t timeframe,currentframe;
	bool(*userfunc)(Framework*);
	bool keypress; // XXX tells us whether a new key was pressed last frame - please figure out a better way to handle this
	char key; // what key was pressed
	struct{
		bool left,right;
		int x=0,y=0;
	}mouse;

	SDL_Window*window;
	SDL_Renderer*renderer;
	SDL_Event event;

	enum{ // XXX
		NONE
	}keymap;

	Framework(int x,int y,int s=1):_sx(x),_sy(y),_ss(s?s:1),userfunc(0){
		newmap=(short*)malloc(x*y*sizeof(short));
		oldmap=(short*)malloc(x*y*sizeof(short));
		timeframe=clock();
		// ah, me old trusty font loader... where would i be without you?
		FILE*f=fopen("font","r");for(int i=0;i<3200;i++)*(font+i)=getc(f);fclose(f);
		SDL_CreateWindowAndRenderer(_sx*_ss,_sy*_ss,0,&window,&renderer);
	}

	~Framework(){
		free(newmap);
		free(oldmap);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void p(short c,int x,int y){newmap[y*_sx+x]=c;}

	void pc(char c,int cx,int cy,short fgc=0xfff,short bgc=0){
		if(c)for(int x=0;x<8;x++)for(int y=0;y<8;y++)
		p(((char)font[(unsigned char)c*8+y]&(char)(128>>x))?fgc:bgc,cx*8+x,cy*8+y);
	}

	void ps(const char*c,int sx,int sy,short fgc,short bgc){
		int x=0,y=0;
		for(int i=0;c[i];i++){
			pc(c[i]==10?0:c[i],sx+x++,sy+y,fgc,bgc);
			if(sx+x==_sx||c[i]==10)
				x=0,y++;
		}
	}

	void pi(short*img,int cx,int cy,int sx,int sy){ // blip a whole image
		for(int x=0;x<sx;x++)
		for(int y=0;y<sy;y++)
		if(cx+x<_sx && cy+y<_sy)
		p(img[y*sx+x],cx+x,cy+y);
	}

	void pi(img a,int cx,int cy){pi(a.data,cx,cy,a.sx,a.sy);}

	void run(){
		for(;;){
			// clear newmap
			for(int i=0;i<_sx*_sy;i++)newmap[i]=0;

			// update
			if(userfunc)
			if(!userfunc(this))
			return;

			// blit screenbuf to screen
			for(int x=0;x<_sx;x++)
			for(int y=0;y<_sy;y++)
			if(newmap[y*_sx+x]!=oldmap[y*_sx+x]){
				// calculate pixel color
				short color=oldmap[y*_sx+x]=newmap[y*_sx+x];
				int r = (color&0x0f00)>>4;
				int g = (color&0x00f0);
				int b = (color&0x000f)<<4;
				r=r|r>>4;g=g|g>>4;b=b|b>>4;

				// also please tell me this isnt really the easiest way...
				SDL_SetRenderDrawColor(renderer,r,g,b,255);
				if(_ss==1) // XXX we could remove this
					SDL_RenderDrawPoint(renderer,x,y); // this is correct

				else
					for(int scx=0;scx<_ss;scx++)
					for(int scy=0;scy<_ss;scy++)
					SDL_RenderDrawPoint(renderer,x*_ss+scx,y*_ss+scy);

			}

			// delay XXX this is gonna need more conversion
			currentframe=clock();
			//printf("%i %i\n",(currentframe-timeframe),CLOCKS_PER_SEC);
			//SDL_Delay((currentframe-timeframe)/CLOCKS_PER_SEC);

			SDL_Delay(20);
			//usleep(20);
			timeframe=currentframe;
			SDL_RenderPresent(renderer);

			mouse.left=mouse.right=key=0;

			// also, event.key.repeat tells us how many times it was repeated (we only want it if its once)
			if(keypress=SDL_PollEvent(&event)){ // returns 1 if it changed
				key=event.key.keysym.sym;
				if(event.type==SDL_KEYDOWN)
					printf("k:%x\n",event.key.keysym.sym&0xff);
				if(event.type==SDL_MOUSEBUTTONDOWN){
					if(event.button.button==SDL_BUTTON_LEFT)
						mouse.left=1;
					if(event.button.button==SDL_BUTTON_RIGHT)
						mouse.right=1;
				}
				if(event.type==SDL_MOUSEMOTION){
					mouse.x=event.motion.x/_ss;
					mouse.y=event.motion.y/_ss;
				}
				if(event.type==SDL_QUIT)return;
			}

		}
	}

};

/*
keys
	key update
	keysym
	plz make an enum for checking, this is currently a nightmare
	modifiers (shift ctrl alt etc)
fix delay
optional sound
*/
