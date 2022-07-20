#include <io.h>
#include <gdt/gdt.h>
#include <drivers/PIT.h>
#include <render/frender.h>
#include <interrupts/IDT.h>
#include <memory/Heap.h>
#include <memory/memory.h>
//#include <drivers/ata.h>
#include <drivers/pc-speaker.h>
#include <drivers/VGA.h>
#include <render/renderer.h>
#include "colors.h"
extern "C" int kernel_main(){
     pcspeaker sp;
    //GDT gdt;
//     InitHeap(0x100000,0x100000);
    // sp.play_sound(1000);
     // sp.beep();

//     inb8(0x3DA); // to index state
//     outb8(0x3C0, 0x10); // register index
//     uint_8 value = inb8(0x3C1);
//     print(hex2str((uint_64)value));
//     return;
//     inb8(0x3DA);
//     outb8(0x3C0, 0x10);
//     outb8(0x3C0, value & 0b11110111 || 0b000000100); // disable 4th bit (numbering from 1) to go to 16 background colors


//     Clearscr(0x1f);
//     setCursorpos(27);
//     print("The blue screen of death!",0x1f);
//     setCursorpos(320);
//     print("something went wrong with your device",0x1f);
//     setCursorpos(80 * 24);
//     print("                                                                                ",0xf1);
    // Clearscr(0x0F);
    //print("Hello, World!\n", 0xAF);
    //PIT::SetFrequency(1000);
    InitIDT();

    VGA vga;

    char str[] {
        55,//W
        69,//e
        76,//l
        67,//c
        79,//o
        77,//m
        69,//e
        96,// 
        69,//e
        0
    };
    vga.SetMode(320, 200, 8);
    for(int y = 0; y < 200; y++) {
        for(int x = 0; x < 320; x++) {
            vga.PutPixel(x, y, BLUE);
        }
    }
    window(0,0,150,70,str,0x0,RED);
    //RenderCircle(30,30,10,BLUE);
    


    while(1){
        //print("A");
        //PIT::Sleep(1000);
        //print(hex2str(PIT::tickssincestart));
        //setCursorpos(0);
        //sp.beep();
    }
}