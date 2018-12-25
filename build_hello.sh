./xa -w -l hello.lst -o hello.bin hello.asm && od -A none -t x1 hello.bin | tr -d '\n' > hello.hex
