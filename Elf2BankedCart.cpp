// Elf2BankedCart.cpp - builds clean in Linux as well.
// 
// This takes a program compiled for the TI-99/4A using Gcc-9900 and creates a bank-switched cartridge
// image. It has several requirements. The first is the standard crt0.s which needs to be provided,
// this is mostly so that it can properly load the program and the initialized data
// 
// Next is a particular naming scheme for sections, which you'll create in your linker file (you must
// create a custom one). The sections are:
// loader - this must have the crt0.s first, and will be at the start address in the cart
// .data - probably the only original section. Initialized data goes here and this will be parsed and included
// fixed - the rest of your fixed data. Note that only 16k is reserved here, and it must be linked to run at >A000
// After that come your banks, starting with bank 1. Because of my usage, you can either name banks like:
// bank1
// bank2
// bank3
// ...
// 
// or
// 
// bank1a (this is used by loader, so do not define it)
// bank1b
// bank2a
// bank2b
// bank3a
// bank3b
// ...
// 
// Either way, the name is searched for and specifies only the order of the output ROM. The actual banking
// is handled in your code however you need it to be handled.
// 
// The first 44 bytes of every page are reserved for the standard header. This gives you room for your cartridge
// header, some generic padding, and the first instruction to reset the page (such as CLR @>6000). This ensures
// that no matter what page your cart boots up on, it can start successfully. Your program must take this space
// into account and store nothing there!
// 
// 24k startup it shall be!
//

#include <stdio.h>
#include <string.h>

#define CART_HEADER_SIZE 46

// well, hell. A stock TI cart can do 32MB. A gigacart can do 8GB. We'll stick to 32MB for now.
unsigned char buf[32*1024*1024];
unsigned char elf[33*1024*1024];    // allow an extra meg of overhead ;)
int used[4096];                     // 4096 pages in 32MB
int load[4096];                     // linked address for each page (just to double check)
char name[4096][256];               // list of names
unsigned char carthdr[CART_HEADER_SIZE];

FILE *fp;
int outpos;
int sectoff;
int sectsiz;
int sectcnt;
int sectnam;
int sectnamoff;

// initialized in copysection, using globals as a lazy return
int virt;
int data;
int datsiz;

bool copysection(int bankidx, const char *str, int maxsiz = 8192, bool mute = false) {
    // find section 'str' in the ELF file, and copy its data to the buffer at outpos
    //printf("Searching for '%s'\n", str);

    for (int idx=0; idx<sectcnt; ++idx) {
        int off = sectoff + idx*sectsiz;
        int nam = (elf[off+0]<<24)|(elf[off+1]<<16)|(elf[off+2]<<8)|(elf[off+3]);
        nam += sectnamoff;
        // this does assume a properly formatted elf...
        //printf(".. try '%s' (0x%X)\n", &elf[nam], nam);
        if (0 == strcmp((const char*)(&elf[nam]), str)) {
            // found it!
            virt = (elf[off+0xc]<<24)|(elf[off+0xd]<<16)|(elf[off+0xe]<<8)|(elf[off+0xf]);      // load address, 0xa000 or 0x6000 expected
            data = (elf[off+0x10]<<24)|(elf[off+0x11]<<16)|(elf[off+0x12]<<8)|(elf[off+0x13]);  // where is the data
            datsiz = (elf[off+0x14]<<24)|(elf[off+0x15]<<16)|(elf[off+0x16]<<8)|(elf[off+0x17]);    // how big

            printf("Loading section '%s' address 0x%04X size %d\n", str, virt, datsiz);
            used[bankidx] += datsiz;
            // don't copy data if first char is '.' (so .data doesn't overwrite loader)
            if (*str != '.') {
                strcpy(name[bankidx], str);
                load[bankidx] = virt;
            }

            if (datsiz > maxsiz) {
                printf("Data larger than %d bytes - image is invalid\n", maxsiz);
                datsiz = maxsiz;  // to make it valid to copy
            }
            // todo: again, should check buffer sizes (both) here
            memcpy(&buf[outpos], &elf[data], datsiz);
            outpos += datsiz;

            return true;
        }
    }
    if (!mute) printf("Could not find section '%s'\n", str);
    return false;
}

void padto(int x) {
    // pad file to a multiple of x, using 0xff bytes
    int slack = outpos%x;
    if (slack) {
        int cnt = x-slack;
        //printf("Adding %d bytes of padding...\n", cnt);
        memset(&buf[outpos], 0xff, cnt);
        outpos += cnt;
    }
    
    // now that we are padded, leave room for the cart header
    // this means the one bank of fixed loader won't have a cart header, but the CRT0 will ensure
    // we don't spend much time there, and it's very unlikely to come up at random.
    outpos += CART_HEADER_SIZE;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Pass the name of a TI-99/4A ELF file to parse, and an output binary filename\n");
        return 0;
    }

    fp = fopen(argv[1], "rb");
    if (NULL == fp) {
        printf("Can't open '%s'\n", argv[1]);
        return 1;
    }

    printf("Opening elf '%s'...\n", argv[1]);
    int sz = fread(elf, 1, sizeof(elf), fp);
    fclose(fp);

    // sanity check header
    if (sz < 52) {
        printf("too short to be ELF\n");
        return 1;
    }
    if (0 != memcmp(elf, "\177ELF", 4)) {      // \177 is octal for \x7f
        printf("No magic signature for ELF\n");
        return 1;
    }
    if (elf[4]!=1) {
        printf("Not 32-bit ELF\n");
        return 1;
    }
    if (elf[5]!=2) {
        printf("Not big endian ELF\n");
        return 1;
    }
    if (elf[6]!=1) {
        printf("Not original version ELF\n");
        return 1;
    }
    if ((elf[0x10]!=0)||(elf[0x11]!=2)) {
        printf("Not executable ELF\n");
        return 1;
    }
    if ((elf[0x12]!=0xab)||(elf[0x13]!=0xba)) {
        printf("Not (probably) TI-99 ELF\n");
        return 1;
    }
    if ((elf[0x14]!=0)||(elf[0x15]!=0)||(elf[0x16]!=0)||(elf[0x17]!=1)) {
        printf("Not version 1 ELF\n");
        return 1;
    }

    // good enough, we can start loading the sections. We assume everything else is fair,
    // like there's nothing relocatable and no weird sections
    memset(used, 0, sizeof(used));
    memset(load, 0, sizeof(load));
    memset(name, 0, sizeof(name));

    sectoff = (elf[0x20]<<24)|(elf[0x21]<<16)|(elf[0x22]<<8)|(elf[0x23]);    // offset of section headers
    sectsiz = (elf[0x2e]<<8)|(elf[0x2f]);                                   // size of one section header
    sectcnt = (elf[0x30]<<8)|(elf[0x31]);                                   // number of section headers

    sectnam = (elf[0x32]<<8)|(elf[0x33]);                                   // index of section header that contains section names
    sectnamoff = sectoff + sectsiz*sectnam;                                 // offset of the section names header
    sectnamoff = (elf[sectnamoff+0x10]<<24)|(elf[sectnamoff+0x11]<<16)|(elf[sectnamoff+0x12]<<8)|(elf[sectnamoff+0x13]);
    printf("Sector name offset at 0x%x\n", sectnamoff);

    // before we do anything too fancy, we locate the .bss section to copy it's data for the crt0 later
    outpos = 0;
    copysection(0, ".bss", 32*1024, true);
    int bsspos = virt;
    int bsssiz = datsiz;

    // then we reset output
    outpos = 0;
    used[0]=0;
    if (!copysection(0, "loader")) {
        return 1;
    }

    int datapos = outpos;
    if (!copysection(0, ".data")) {     // add to 0 size
        return 1;
    }
    int dataram = virt;
    // the data section MUST be an even size
    if (outpos&1) {
        buf[outpos++]=0xff;
    }

    int fixedpos = outpos;
    if (!copysection(1, "fixed", 8192*3)) {     // up to 24k - though not all usable because of the above sections
        return 1;
    }

    // fixed area free space
    int totalsize = used[0]+used[1];
    used[0] = totalsize;
    if (used[0] > 8192) used[0]=8192;
    used[1] = totalsize-used[0];
    if (used[1] > 8192) used[1]=8192;
    used[2] = totalsize-used[0]-used[1];
    // now fudge the sizes cause these pages don't have the header reserve
    used[0]-=CART_HEADER_SIZE;
    used[1]-=CART_HEADER_SIZE;
    used[2]-=CART_HEADER_SIZE;

    if (outpos > 3*8192) {
        printf("Loader/data/fixed is greater than 24k! Cart is not valid.\n");
        // but keep going so we can see everything
        // truncate before we pad so that the rest builds correctly
        outpos = 3*8192;
    }

    padto(3*8192);               // pad the initial bit to 24k

    // now parse and load any banks we may have
    int bank = 1;
    int usedbank = 3;
    bool first = true;
    for (;;) {
        char buf[128];
        bool found = false;
        if (first) {
            first = false;
            // we need to only check b, not a, cause a got used by the loader
            sprintf(buf, "bank%db", bank);
            if (copysection(usedbank, buf)) {
                found = true;
                padto(8192);
                usedbank++;
            }
        } else {
            sprintf(buf, "bank%da", bank);
            if (copysection(usedbank, buf, 8192, true)) {
                found = true;
                padto(8192);
                usedbank++;
                sprintf(buf, "bank%db", bank);
                if (!copysection(usedbank, buf)) {
                    printf("Warning: bank A without matching bank B may throw off indexing\n");
                } else {
                    padto(8192);
                    usedbank++;
                }
            }
        }

        // if not bankNa/b, try just bankN
        if (!found) {
            sprintf(buf, "bank%d", bank);
            if (!copysection(usedbank, buf)) {
                // must be done
                break;
            }
            padto(8192);
            usedbank++;
        }
        ++bank;
    }

    // ignore the last cart header padding - assumes at least one padto!
    outpos -= CART_HEADER_SIZE;

    printf("Got cart size of %dk\n", outpos/1024);

    // fill in the C startup data in the cart header
    // this'll confuse people ;)
    // Actual header as we use it:

    // 0xAA01 - header byte and version
    // Address of initialized data in RAM
    // Address of initialized data in ROM
    // Program List (as normal)
    // Size of initialized data
    // BSS start in RAM
    // Size of BSS (if odd, be aware an extra byte is cleared)
    // Start address of fixed program in ROM bank 0
    
    //f[0]=0xaa
    //f[1]=0x01
    buf[2]=(dataram>>8)&0xff;
    buf[3]=(dataram)&0xff;
    buf[4]=(datapos>>8)&0xff;
    buf[5]=(datapos)&0xff;
    //f[6]=program list msb
    //f[7]=program list lsb
    buf[8]=((fixedpos-datapos)>>8)&0xff;
    buf[9]=((fixedpos-datapos))&0xff;
    buf[10]=(bsspos>>8)&0xff;
    buf[11]=(bsspos)&0xff;
    buf[12]=(bsssiz>>8)&0xff;
    buf[13]=(bsssiz)&0xff;
    fixedpos += 0x6000;     // make a cartridge address
    buf[14]=(fixedpos>>8)&0xff;
    buf[15]=(fixedpos)&0xff;

    // now see where we ended up, and pad to a power of 2
    // TODO: cart headers would go here too
    int validsize = 8192;   // smallest cart
    for (;;) {
        if (outpos == validsize) {
            break;
        }
        if (outpos < validsize) {
            printf("Padding to %dk\n", validsize/1024);
            padto(validsize);
            // ignore the last cart header padding (again)
            outpos -= CART_HEADER_SIZE;
            break;
        }
        validsize*=2;
        if (validsize > sizeof(buf)) {
            printf("Next valid cartridge size exceeds internal buffer.\n");
            return 1;
        }
    }

    // dump the banking map - we'll just use the size again
    // while we're in there, we'll copy the cart header everywhere but the loader/fixed pages
    validsize = 8192;
    bank = 0;
    printf("##  BANK  ROM     NAME         LOAD  FREE\n");
    printf("--  ----  ------  -----------  ----  -----\n");
    while (outpos >= validsize) {
        printf("%02d  %04X  %06X  %-11s  %04X  %5d\n", bank, 0x6000+bank*2, 8192*bank, name[bank], load[bank], 8192-used[bank]-CART_HEADER_SIZE);
        if (bank > 2) {
            // bank >2 for 24k fixed
            memcpy(&buf[bank*8192], buf, CART_HEADER_SIZE);
        }
        ++bank;
        validsize+=8192;
    }
    printf("\n");

    fp = fopen(argv[2], "wb");
    if (NULL == fp) {
        printf("Can't open output '%s'\n", argv[2]);
        return 1;
    }
    fwrite(buf, 1, outpos, fp);
    fclose(fp);

    printf("Wrote '%s'\n", argv[2]);

    return 0;
}
