// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
static void consputc(int);

static int panicked = 0;

static struct {
	struct spinlock lock;
	int locking;
} cons;

static void
printint(int xx, int base, int sign)
{
	static char digits[] = "0123456789abcdef";
	char buf[16];
	int i;
	uint x;

	if(sign && (sign = xx < 0))
		x = -xx;
	else
		x = xx;

	i = 0;
	do{
		buf[i++] = digits[x % base];
	}while((x /= base) != 0);

	if(sign)
		buf[i++] = '-';

	while(--i >= 0)
		consputc(buf[i]);
}

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
	int i, c, locking;
	uint *argp;
	char *s;

	locking = cons.locking;
	if(locking)
		acquire(&cons.lock);

	if (fmt == 0)
		panic("null fmt");

	argp = (uint*)(void*)(&fmt + 1);
	for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
		if(c != '%'){
			consputc(c);
			continue;
		}
		c = fmt[++i] & 0xff;
		if(c == 0)
			break;
		switch(c){
		case 'd':
			printint(*argp++, 10, 1);
			break;
		case 'x':
		case 'p':
			printint(*argp++, 16, 0);
			break;
		case 's':
			if((s = (char*)*argp++) == 0)
				s = "(null)";
			for(; *s; s++)
				consputc(*s);
			break;
		case '%':
			consputc('%');
			break;
		default:
			// Print unknown % sequence to draw attention.
			consputc('%');
			consputc(c);
			break;
		}
	}

	if(locking)
		release(&cons.lock);
}

void
panic(char *s)
{
	int i;
	uint pcs[10];

	cli();
	cons.locking = 0;
	// use lapiccpunum so that we can call panic from mycpu()
	cprintf("lapicid %d: panic: ", lapicid());
	cprintf(s);
	cprintf("\n");
	getcallerpcs(&s, pcs);
	for(i=0; i<10; i++)
		cprintf(" %p", pcs[i]);
	panicked = 1; // freeze other CPU
	for(;;)
		;
}

#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

//DOdati
static int boji=0; 				//dA LI BOJI
static int pozicijaPocetkaSelektovanja=-1;	//Kada selektuje prvi element
static char kopiraniTekst[128];     		//Tekst koji je kopiran
static int prva_pozicija=0;			//Pozicija sa koje je mod kpiranja zapoceo rad
static int prom_prva_poz=0;
static int min=1921;
static int max=0;
static int nekapoljasubela=0;
static int duzinaKopiranogteksta=0;			
//kraj dodato
static void
cgaputc(int c)
{
	int pos;
	//Dodato
	static int tren;
	// Cursor position: col + 80*row.
	outb(CRTPORT, 14);
	pos = inb(CRTPORT+1) << 8;
	outb(CRTPORT, 15);
	pos |= inb(CRTPORT+1);

	if(c<0 && prom_prva_poz==0)
	{
		prva_pozicija=pos;
		prom_prva_poz=1;	
	}
	
	if(c==-1)
	{
		if(pos+1<24*80)
		{
		tren=crt[pos+1];
		pos++;
		}
		
	}	
	else if(c==-2)
	{
		if(pos-1>=0)
		{
		tren=crt[pos-1];
		pos--;
		}
		
	}
	else if(c==-3)
	{
		if(pos+80<24*80)
		{
		tren=crt[pos+80];
		pos=pos+80;
		}
	}
	else if(c==-4)
	{
		if(pos-80>=0)
		{
			tren=crt[pos-80];
			pos=pos-80;		
		}
	}
	else if(c==-5)
	{
		//Zapocni
		if(pozicijaPocetkaSelektovanja==-1)
			pozicijaPocetkaSelektovanja=pos;
		boji=1;
		nekapoljasubela=1;
	}
	else if(c==-6)
	{	
		int brojac=0;
		//Prekini
		if(max>min)
		{
			if(min==pozicijaPocetkaSelektovanja)
			{
				for(int i=min;i<=max;i++)//24*80
				{	
					if(brojac<128)
					{
						kopiraniTekst[brojac]=crt[i];
						brojac++; 
					}
					else
					{
						break;
					}
				}						
			}
			else
			{
				for(int i=min;i<max;i++)//24*80
				{
					if(brojac<128)
					{
						kopiraniTekst[brojac]=crt[i];
						brojac++;
					}
					else
					{
						break;
					} 
				}
			}
		}
		duzinaKopiranogteksta=brojac;
		nekapoljasubela=0;
		pozicijaPocetkaSelektovanja=-1;
		boji=0;
		//obrisi selekciju
		for(int i=0;i<1920;i++)//24*80
		{	
			crt[i] = crt[i]&0xff | 0x0700;
		}
	}
	else
	{

	if(nekapoljasubela==1)
	{
		for(int i=0;i<1920;i++)//24*80
		{	
			crt[i] = crt[i]&0xff | 0x0700;
		}
		boji=0;
		nekapoljasubela=0;
	}
	if(prom_prva_poz==1)
	{
		prom_prva_poz=0;
		pos=prva_pozicija;
		tren=0;
	}
	if(c == '\n')
		pos += 80 - pos%80;
	else if(c == BACKSPACE){
		if(pos > 0) --pos;
	} else
		crt[pos++] = (c&0xff) | 0x0700;  // black on white	
	
	
	}

	if(pos < 0 || pos > 25*80)
		panic("pos under/overflow");

	if((pos/80) >= 24){  // Scroll up.
		memmove(crt, crt+80, sizeof(crt[0])*23*80);
		pos -= 80;
		memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
	}

	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos>>8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);


	//Bojenje polja
	if(pozicijaPocetkaSelektovanja!=-1 && ker!=0)
	{
		if(pozicijaPocetkaSelektovanja>pos)
		{
			min=pos;
			//proveralevodesno=0;	
		}
		else
		{
			min=pozicijaPocetkaSelektovanja;
			//proveralevodesno=1;	
		}
		max=pozicijaPocetkaSelektovanja+pos-min;	
		for(int i=0;i<1920;i++)//24*80
		{
			if(pozicijaPocetkaSelektovanja<pos)
			{
				if(i>=min && i<=max)
				{
					crt[i] = crt[i]&0xff | 0x7000;
				}
				else
				{
					crt[i] = crt[i]&0xff | 0x0700;
				}				
			}
			else
			{
				if(i>=min && i<max)
				{
					crt[i] = crt[i]&0xff | 0x7000;
				}
				else
				{
					crt[i] = crt[i]&0xff | 0x0700;
				}
			}
				
			
		}
		//Kraj bojenja polja
	}


	if(boji==0)
		crt[pos] = tren&0xff | 0x0700;
	else
		crt[pos] = tren&0xff | 0x7000;
}

void
consputc(int c)
{
	if(panicked){
		cli();
		for(;;)
			;
	}

	if(c == BACKSPACE){
		uartputc('\b'); uartputc(' '); uartputc('\b');
	} else
		uartputc(c);
	cgaputc(c);
}

#define INPUT_BUF 128
struct {
	char buf[INPUT_BUF];
	uint r;  // Read index
	uint w;  // Write index
	uint e;  // Edit index
} input;

#define C(x)  ((x)-'@')  // Control-x

int ker=0;
int postavi=0; 
int vratio_se=0;
void
consoleintr(int (*getc)(void))
{
	int c, doprocdump = 0;
	
	acquire(&cons.lock);
	
	while((c = getc()) >= 0){
		
		if(c==C('P') && ker==0)
		{
			// Process listing.
			// procdump() locks cons.lock indirectly; invoke later
			doprocdump = 1;
			break;
		} 
		if(c==C('U')&& ker==0)
		{  // Kill line.
			while(input.e != input.w &&
			      input.buf[(input.e-1) % INPUT_BUF] != '\n'){
				input.e--;
				consputc(BACKSPACE);
			}
			break;
		}
		if((c==C('H') || c==C('\x7f'))&& ker==0)
		{   // Backspace
			if(input.e != input.w){
				input.e--;
				consputc(BACKSPACE);
			}
			break;
		}
		else
		{
	
			if(c != 0 && input.e-input.r < INPUT_BUF){
				//Dodato
				if(postavi==1)
				{//Pejstovanje ako bog da
				postavi=0;
				int br=0;
				for(int i=0;i<duzinaKopiranogteksta;i++)
				{	
					
					if(c != 0 && input.e-input.r < INPUT_BUF)
					{
						int brojj = kopiraniTekst[br]&0xff | 0x0700;
						br++;
						brojj = (brojj == '\r') ? '\n' : brojj;
						input.buf[input.e++ % INPUT_BUF] = brojj;	
						consputc(brojj);	
						
					}
					
				}
				
				}
				else if(ker==5)
				{
				//Gore
				cgaputc(-4);
				vratio_se=1;
				}
				else if(ker==2)
				{
				//Dole
				cgaputc(-3);
				vratio_se=1;
				}
				else if(ker==3)
				{
				cgaputc(-2);
				//Levo
				vratio_se=1;
				}
				else if(ker==4)
				{
				cgaputc(-1);
				//Desno
				vratio_se=1;
				}
				else if(ker==6)
				{
				//Zapocni selekciju	
				cgaputc(-5);
				vratio_se=1;
				}
				else if(ker==7)
				{
				//Zavrsi selekciju
				cgaputc(-6);
				vratio_se=1;								
				}
				else if(ker==1)
				{
				vratio_se=1;							
				}
				else
				{
					if(vratio_se==1)
					{
						vratio_se=0;
						consputc(' ');
						consputc(BACKSPACE);
						
					}
					else
					{
						c = (c == '\r') ? '\n' : c;
						input.buf[input.e++ % INPUT_BUF] = c;
						consputc(c);
					}
					pozicijaPocetkaSelektovanja=-1;//Ako je nesto bilo selektovano vise nije
					
				}	

				if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
					input.w = input.e;
					wakeup(&input.r);
				}
			}
			break;
		}
		

		
	}
	release(&cons.lock);
	if(doprocdump) {
		procdump();  // now call procdump() wo. cons.lock held
	}
}

int
consoleread(struct inode *ip, char *dst, int n)
{
	uint target;
	int c;

	iunlock(ip);
	target = n;
	acquire(&cons.lock);
	while(n > 0){
		while(input.r == input.w){
			if(myproc()->killed){
				release(&cons.lock);
				ilock(ip);
				return -1;
			}
			sleep(&input.r, &cons.lock);
		}
		c = input.buf[input.r++ % INPUT_BUF];
		if(c == C('D')){  // EOF
			if(n < target){
				// Save ^D for next time, to make sure
				// caller gets a 0-byte result.
				input.r--;
			}
			break;
		}
		*dst++ = c;
		--n;
		if(c == '\n')
			break;
	}
	release(&cons.lock);
	ilock(ip);

	return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
	int i;

	iunlock(ip);
	acquire(&cons.lock);
	for(i = 0; i < n; i++)
		consputc(buf[i] & 0xff);
	release(&cons.lock);
	ilock(ip);

	return n;
}

void
consoleinit(void)
{
	initlock(&cons.lock, "console");

	devsw[CONSOLE].write = consolewrite;
	devsw[CONSOLE].read = consoleread;
	cons.locking = 1;

	ioapicenable(IRQ_KBD, 0);
}

