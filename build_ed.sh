./xa -w -l ed.lst -o ed.bin ed.asm && od -A none -t x1 ed.bin | tr -d '\n' > ed.hex
