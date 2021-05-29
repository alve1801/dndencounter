// the actual tiles of the field
struct tile{
	char c;
	short fgc,bgc;
	char stat; // XXX enum
};

const char*stat_names[]={"","invisible","grappled","paralyzed","prone","restrained","poisoned","stunned","unconscious","sneaky","blessed","dead"},
stat_num=12;
// grappling?
// blessed

// entities on the field
class entity{public:
	char c;
	short color;
	char*name;
	int x,y;
	char effect;

	entity(char c='c',short col=0xf1,char*name="Urist",int x=0,int y=0,char effect=0):c(c),color(col),name(name),x(x),y(y),effect(effect){}

	entity(FILE*f){
		c=getc(f);
		color=getc(f)<<4|getc(f); // this better work
		int namelen=getc(f);
		name=(char*)malloc(namelen+1);
		int i;
		for(i=0;i<namelen;i++)
			name[i]=getc(f);
		name[i]=0;

		printf("dorf:%i %s\n",namelen,name);

		x=getc(f);
		y=getc(f);
		effect=getc(f);
	}

	void save(FILE*f){
		putc(c,f);
		putc(color>>4,f);
		putc(color,f); // ?
		putc(strlen(name),f);
		fputs(name,f);
		putc(x,f);
		putc(y,f);
		putc(effect,f);
	}
};

// idk whether we need this
struct item{};

struct field{
	char size,ent;
	tile*t;
	entity*e[256];
	//item i[256];

	field():size(20),ent(4){
		t=(tile*)malloc(size*size*sizeof(tile));
		for(int i=0;i<size*size;t[i++]=(tile){'~',0x666,0,0});
		e[0]=new entity('1',0xf1,"Urist");
		e[1]=new entity('2',0xf1,"Entust");
		e[2]=new entity('3',0xf1,"Snodub");
		e[3]=new entity('4',0xff8,"Limul");
	}

	field(char*filename){load(filename);}

	~field(){free(t);}

	void append(entity a){}

	void load(const char*filename){
		// delete all previous data
		for(int i=0;i<256;i++)delete e[i];
		if(t)free(t); // not sure if needed

		FILE*f=fopen(filename,"r");
		size=getc(f);
		printf("size %p %i\n",f,size);
		t=(tile*)malloc(size*size*sizeof(tile));
		for(int i=0;i<size*size;t[i++]=(tile){(char)getc(f),getc(f)<<8|getc(f),0,0});
		ent=getc(f);
		for(int i=0;i<ent;i++)
			e[i]=new entity(f);
		printf("%i entities\n",ent);
		fclose(f);
		printf("World loaded\n");
	}

	void save(const char*filename){
		FILE*f=fopen(filename,"w");
		putc(size,f);
		for(int i=0;i<size*size;i++){
			putc(t[i].c,f);
			putc(t[i].fgc>>8,f);
			putc(t[i].fgc,f);
		}
		putc(ent,f);
		for(int i=0;e[i];i++)e[i]->save(f);
		fclose(f);
		printf("World saved\n");
	}

	void import(const char*filename){
		FILE*f=fopen(filename,"r");
		if(!f){printf("<%s> not found\n",filename);return;}
		int i=size=0;
		while((i=getc(f))!=10)size=size*10+i-'0';i=0;
		if(t)free(t);
		t=(tile*)malloc(size*size*sizeof(tile));
		for(int x=0;x<size;x++){
		for(int y=0;y<size;y++)
		t[i++]=(tile){(char)getc(f),0x666,0,0};
		getc(f); // newline
		}
		fclose(f);
		printf("New map size %i\nMap loaded\n",size);
	}
};
