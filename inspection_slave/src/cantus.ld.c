OUTPUT_FORMAT("elf32-ae32000-little", "elf32-ae32000-big",
			"elf32-ae32000-little")
OUTPUT_ARCH(ae32000)
ENTRY(_Reset_Start)
/*GROUP(-lc -lgcc -lm)*/
/*SEARCH_DIR(.);*/

MEMORY
{
    /* 512KB Flash */
	rom (rx) : ORIGIN = 0x00010000, LENGTH = 512K-0x10000
	ram (!rx): ORIGIN = 0x20000000, LENGTH = 80K
}
	
SECTIONS
{
  .text : {
    *(.vects)
    *(.text)
    . = ALIGN(4);
    *(.stub)
    *(.rodata)
    *(.rodata.str1.*)
    *(.rodata1)
    ___ctors = . ;
    *(.ctors)
    ___ctors_end = . ;
    ___dtors = . ;
    *(.dtors)
    ___dtors_end = . ;
  } > rom
  _etext = . ;
  PROVIDE (etext = .);

  .data : AT ( (LOADADDR (.text) + SIZEOF(.text) + 3) & ~ (4 - 1)) {
    __ram_data_start = . ;
	*(.flash_code)
    . = ALIGN(4);
    /*PROVIDE (___ram_data_start = .);*/
    *(.data)
    *(.sdata)
    CONSTRUCTORS
    . = ALIGN(4);
    __ram_data_end = .;
    /*PROVIDE (___ram_data_end = .);*/
  } > ram
  __rom_data_start  =  LOADADDR (.data);
  /*PROVIDE (___rom_data_start = .);*/

  .bss ALIGN(0x4) : AT ((LOADADDR (.data) + SIZEOF(.data) + 3) & ~ (4 - 1) ) {
    __bss_start = .;
    /*PROVIDE (__bss_start = .);*/
    *(.dynbss)
    *(.bss)
    *(COMMON)
    . = ALIGN(4);
    __bss_end = .;
    /*PROVIDE (___bss_end = .);*/
  } > ram

  _end = . ;
  PROVIDE (end = .);


  ._stack 0x20013FFC : {
    _stack = .;
    /*PROVIDE (__stack = .);*/
    *(._stack)
  } 
}
PROVIDE(___data_size = SIZEOF(.data));
PROVIDE(___bss_size = SIZEOF(.bss));


