/* 
 256k banked cartridge with 32k RAM support
 Fixed bank is copied into 32k by the CRT0
 I think my cartridge builder will have to
 deal with how that works, but we will assume
 a maximum size of 16k since that's what the
 Coleco source used. All other pages here
 become 8k, so for now I'll use a PAGEA/PAGEB
 concept to split up the Coleco pages.
*/

OUTPUT_FORMAT("elf32-tms9900", "elf32-tms9900",
              "elf32-tms9900")
OUTPUT_ARCH(tms9900)
ENTRY(_start)
SEARCH_DIR("/home/tursi/gcc9900/tms9900/lib");

MEMORY
{
    INIT(xr) : ORIGIN = 0x6000, LENGTH = 128

    FIXED(xrw) : ORIGIN = 0xA000, LENGTH = 24K
    STACK(xrw) : ORIGIN = 0x2000, LENGTH = 8K
    SCRATCH(rw) : ORIGIN = 0x8320, LENGTH = 224
    CART1A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART1B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART2A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART2B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART3A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART3B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART4A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART4B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART5A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART5B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART6A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART6B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART7A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART7B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART8A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART8B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART9A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART9B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART10A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART10B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART11A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART11B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART12A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART12B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART13A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART13B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART14A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART14B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART15A(XR) : ORIGIN = 0x6000, LENGTH = 8K
    CART15B(XR) : ORIGIN = 0x6000, LENGTH = 8K
    
    FLASH(XR) : ORIGIN = 0, LENGTH = 256K
}

SECTIONS {
    PROVIDE (__executable_start = 0xA000); 

    PROVIDE (stext = .);
    
    . = 0x6000;
    
    loader :
    {
        crt0.o
    } > INIT AT> FLASH

    . = 0xA000;
    fixed :
    {
        crt0.o
        superspaceacer.o(.text .rodata)
        trampolines.o(.text .rodata)
        music.o(.text .rodata)
        sfxs.o(.text .rodata)
        *libti99.a:*(.text .rodata)
        *libtivgm2.a:*(.text .rodata)
    } > FIXED AT> FLASH
    
    PROVIDE (etext = .);
    
    bank1a :
    {
        enemy.o(.text .rodata)
        winhard2.o(.text .rodata)
        PROVIDE(ebank1a = .);
    } > CART1A AT> FLASH
    
    bank1b :
    {
        snowballbasep.o(.text .rodata)
        snowballbasec.o(.text .rodata)
        PROVIDE(ebank1b = .);
    } > CART1B AT> FLASH
        
    bank2a 0x6000 :
    {
        human.o(.text .rodata)
        PROVIDE(ebank2a = .);
    } > CART2A AT> FLASH

    bank2b 0x6000 :
    {
        human2.o(.text .rodata)
        cruiserendc.o(.text .rodata)
        PROVIDE(ebank2b = .);
    } > CART2B AT> FLASH

    bank3a 0x6000 :
    {
        outfile1.o(.text .rodata)
        PROVIDE(ebank3a = .);
    } > CART3A AT> FLASH

    bank3b 0x6000 :
    {
        selena_end_c.o(.text .rodata)
        selena_end_p.o(.text .rodata)
        PROVIDE(ebank3b = .);
    } > CART3B AT> FLASH

    bank4a 0x6000 :
    {
        difficulty.o(.text .rodata)
        PROVIDE(ebank4a = .);
    } > CART4A AT> FLASH

    bank4b 0x6000 :
    {
        outfile0.o(.text .rodata)
        PROVIDE(ebank4b = .);
    } > CART4B AT> FLASH

    bank5a 0x6000 :
    {
        boss1.o(.text .rodata)
        boss2.o(.text .rodata)
        boss3.o(.text .rodata)
        boss4.o(.text .rodata)
        boss5.o(.text .rodata)
        gnat5rlec.o(.text .rodata)
        gnat5rlep.o(.text .rodata)
        PROVIDE(ebank5a = .);
    } > CART5A AT> FLASH

    bank5b 0x6000 :
    {
        ssd_inc.o(.text .rodata)
        PROVIDE(ebank5b = .);
    } > CART5B AT> FLASH

    bank6a 0x6000 :
    {
        bg.o(.text .rodata)
        outfile2.o(.text .rodata)
        PROVIDE(ebank6a = .);
    } > CART6A AT> FLASH

    bank6b 0x6000 :
    {
        outfile3.o(.text .rodata)
        font.o(.text .rodata)
        PROVIDE(ebank6b = .);
    } > CART6B AT> FLASH

    bank7a 0x6000 :
    {
        boss.o(.text .rodata)
        PROVIDE(ebank7a = .);
    } > CART7A AT> FLASH

    bank7b 0x6000 :
    {
        bossdraw.o(.text .rodata)
        PROVIDE(ebank7b = .);
    } > CART7B AT> FLASH

    bank8a 0x6000 :
    {
        wineasy.o(.text .rodata)
        winmed.o(.text .rodata)
        ssd_inc2.o(.text .rodata)
        bossdat.o(.text .rodata)
        PROVIDE(ebank8a = .);
    } > CART8A AT> FLASH

    bank8b 0x6000 :
    {	
        winhard.o(.text .rodata)
        PROVIDE(ebank8b = .);
    } > CART8B AT> FLASH

    bank9a 0x6000 :
    {
        titlepage.o(.text .rodata)
        title_c.o(.text .rodata)
        title_p.o(.text .rodata)
        PROVIDE(ebank9a = .);
    } > CART9A AT> FLASH

    bank9b 0x6000 :
    {
        ship1_c.o(.text .rodata)
        ship1_p.o(.text .rodata)
        f18load3.o(.text .rodata)
        PROVIDE(ebank9b = .);
    } > CART9B AT> FLASH

    bank10a 0x6000 :
    {
        ship2_c.o(.text .rodata)
        ship2_p.o(.text .rodata)
        ship3_c.o(.text .rodata)
        PROVIDE(ebank10a = .);
    } > CART10A AT> FLASH

    bank10b 0x6000 :
    {
        ship3_p.o(.text .rodata)
        ship4_c.o(.text .rodata)
        ship4_p.o(.text .rodata)
        PROVIDE(ebank10b = .);
    } > CART10B AT> FLASH

    bank11a 0x6000 :
    {
        ladybugc.o(.text .rodata)
        PROVIDE(ebank11a = .);
    } > CART11A AT> FLASH

    bank11b 0x6000 :
    {
        ladybugp.o(.text .rodata)
        ladybugscreenc.o(.text .rodata)
        ladybugscreenp.o(.text .rodata)
        PROVIDE(ebank11b = .);
    } > CART11B AT> FLASH

    bank12a 0x6000 :
    {
        qwertianc.o(.text .rodata)
        qwertianp.o(.text .rodata)
        attract.o(.text .rodata)
        cruiserendp.o(.text .rodata)
        PROVIDE(ebank12a = .);
    } > CART12A AT> FLASH

    bank12b 0x6000 :
    {
        gnat1rlec.o(.text .rodata)
        gnat1rlep.o(.text .rodata)
        PROVIDE(ebank12b = .);
    } > CART12B AT> FLASH

    bank13a 0x6000 :
    {
        gnat3rlec.o(.text .rodata)
        gnat3rlep.o(.text .rodata)
        gnat4rlec.o(.text .rodata)
        gnat4rlep.o(.text .rodata)
        PROVIDE(ebank13a = .);
    } > CART13A AT> FLASH

    bank13b 0x6000 :
    {
        gnat2rlep.o(.text .rodata)
        gnat2rlec.o(.text .rodata)
        f18load4.o(.text .rodata)
        PROVIDE(ebank13b = .);
    } > CART13B AT> FLASH

    bank14a 0x6000 :
    {
        f18sprites.o(.text .rodata)
        PROVIDE(ebank14a = .);
    } > CART14A AT> FLASH

    bank14b 0x6000 :
    {
        f18load.o(.text .rodata)
        bossdraw2.o(.text .rodata)
        PROVIDE(ebank14b = .);
    } > CART14B AT> FLASH

    bank15a 0x6000 :
    {
        f18abosses.o(.text .rodata)
        PROVIDE(ebank15a = .);
    } > CART15A AT> FLASH

    bank15b 0x6000 :
    {
        f18load2.o(.text .rodata)
        PROVIDE(ebank15b = .);
    } > CART15B AT> FLASH

    . = 0x2080;
    PROVIDE(sdata = .);
    .data : { *(.data) } > STACK AT> FLASH
    PROVIDE(edata = .);
    
    PROVIDE(sbss = .);
    .bss :  { *(.bss)  } > STACK AT> FLASH
    PROVIDE(ebss = .);

    songDatVars : { *(.songDatVars) } > SCRATCH AT> FLASH
    
    _end = .; PROVIDE (end = .);
}
