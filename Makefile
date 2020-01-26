FLAGS = -mmcu=attiny13a -DF_CPU=9600000UL -Os -std=c99 -Wall -Werror

all: tree.hex

%.hex: %.elf
	avr-objcopy -O ihex $< $@

clean:
	rm -f *.elf *.hex

%.elf: %.c
	avr-gcc $(FLAGS) $^ -o $@
	avr-size -C --mcu=attiny13a $@
