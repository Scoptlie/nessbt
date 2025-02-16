package main

import (
	"bytes"
	"flag"
	"fmt"
	"os"
)

func parseRom(romFile string) (mirrorV bool, prgRom []byte, chrRom []byte) {
	buf, err := os.ReadFile(romFile)
	if err != nil {
		panic(err)
	}

	if !bytes.Equal(buf[:4], []byte{'N', 'E', 'S', 0x1a}) {
		panic("missing nes rom signature")
	}

	if buf[4] != 1 && buf[4] != 2 {
		panic("unsupported prg rom size")
	}

	if buf[5] != 1 && buf[5] != 2 {
		panic("unsupported chr rom size")
	}

	if (buf[6] & ^byte(1)) != 0 {
		panic("unsupported flags 6")
	}

	if buf[7] != 0 && buf[7] != 0b00001000 {
		panic("unsupported flags 7")
	}

	mirrorV = buf[6]&1 != 0

	prgRomSize := uint(buf[4]) * 0x4000
	prgRom = buf[16:(16 + prgRomSize)]
	chrRom = buf[(16 + prgRomSize):]

	return
}

func main() {
	prgRomCppFile := flag.String("prgRomCpp", "", "PRG ROM .cpp file to generate")
	chrRomCppFile := flag.String("chrRomCpp", "", "CHR ROM .cpp file to generate")
	romFile := flag.String("rom", "", "ROM file to translate")

	flag.Parse()

	mirrorV, prgRom, chrRom := parseRom(*romFile)

	prgRomCpp := "#include \"runtime/cpu.h\"\n" +
		"\n" +
		"namespace Cpu {\n" +
		"\tU8 prgRom[0x8000] = {"
	for _, b := range prgRom {
		prgRomCpp += fmt.Sprintf("%d,", int(b))
	}
	prgRomCpp += "};\n" +
		fmt.Sprintf("\tUSize prgRomSize = %d;\n", len(prgRom)) +
		"}\n"

	chrRomCpp := "#include \"runtime/ppu.h\"\n" +
		"\n" +
		"namespace Ppu {\n" +
		"\tU8 chrRom[0x4000] = {"
	for _, b := range chrRom {
		chrRomCpp += fmt.Sprintf("%d,", int(b))
	}
	chrRomCpp += "};\n" +
		fmt.Sprintf("\tUSize chrRomSize = %d;\n", len(chrRom)) +
		"\t\n" +
		fmt.Sprintf("\tbool mirrorV = %t;\n", mirrorV) +
		"}\n"

	_, _ = mirrorV, chrRom

	os.WriteFile(*prgRomCppFile, []byte(prgRomCpp), 0644)
	os.WriteFile(*chrRomCppFile, []byte(chrRomCpp), 0644)

	flag.Parse()
}
