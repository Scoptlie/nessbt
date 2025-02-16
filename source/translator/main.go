package main

import (
	"bytes"
	"encoding/binary"
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

func findBBlocks(prgRom []uint8) map[uint16][]opInstT {
	bBlockAddrs := make(map[uint16]bool)
	for i := range len(prgRom) - 1 {
		v := prgRom[i : i+2]
		/*if v[0] == 0x10 ||
			v[0] == 0x30 ||
			v[0] == 0x50 ||
			v[0] == 0x70 ||
			v[0] == 0x90 ||
			v[0] == 0xb0 ||
			v[0] == 0xd0 ||
			v[0] == 0xf0 {

			offs := uint16(int16(int8(v[1])))

			addr := 0x8000 + uint16(i) + 2 + offs
			if addr >= 0x8000 {
				bBlockAddrs[addr] = true
			}
		}*/

		addr := binary.LittleEndian.Uint16(v)
		if addr >= 0x8000 {
			bBlockAddrs[addr] = true
		}
	}

	bBlocks := make(map[uint16][]opInstT)
	for len(bBlockAddrs) > 0 {
		newAddrs := make(map[uint16]bool)
		for addr := range bBlockAddrs {
			_, exists := bBlocks[addr]
			if exists {
				continue
			}

			buf := bytes.NewReader(prgRom[addr&uint16(len(prgRom)-1):])
			bBlock := readBBlock(addr, buf)
			if bBlock != nil {
				bBlocks[addr] = bBlock

				lastInst := bBlock[len(bBlock)-1]
				if lastInst.op.addrMode == addrModeRel {
					addr1 := lastInst.addr + 2
					if addr1 >= 0x8000 {
						newAddrs[addr1] = true
					}

					addr2 := addr1 + lastInst.operand
					if addr2 >= 0x8000 {
						newAddrs[addr2] = true
					}
				} else if lastInst.op.name == opNameBrk {
					addr := lastInst.addr + 2
					if addr >= 0x8000 {
						newAddrs[addr] = true
					}
				} else if lastInst.op.name == opNameJsr {
					addr := lastInst.addr + 3
					if addr >= 0x8000 {
						newAddrs[addr] = true
					}
				}
			}
		}
		bBlockAddrs = newAddrs
	}

	return bBlocks
}

func main() {
	prgRomCppFile := flag.String("prgRomCpp", "", "PRG ROM .cpp file to generate")
	chrRomCppFile := flag.String("chrRomCpp", "", "CHR ROM .cpp file to generate")
	romFile := flag.String("rom", "", "ROM file to translate")

	flag.Parse()

	mirrorV, prgRom, chrRom := parseRom(*romFile)

	bBlocks := findBBlocks(prgRom)

	prgRomCpp := "#include \"runtime/cpu.h\"\n" +
		"\n" +
		"#include \"runtime/env.h\"\n" +
		"#include \"runtime/ppu.h\"\n" +
		"\n" +
		"namespace Cpu {\n" +
		"\tU8 prgRom[0x8000] = {"
	for _, b := range prgRom {
		prgRomCpp += fmt.Sprintf("%d,", int(b))
	}
	prgRomCpp += "};\n" +
		fmt.Sprintf("\tUSize prgRomSize = %d;\n", len(prgRom)) +
		"\t\n"

	for addr, bBlock := range bBlocks {
		funcName := fmt.Sprintf("stBBlock%.4X", addr)

		prgRomCpp += "\tvoid " + funcName + "() {\n" +
			"\t\tEnv::update(nCycles);\n" +
			"\t\tnCycles = 0;\n" +
			"\t\t\n" +
			"\t\tif (Ppu::nmi) {\n" +
			"\t\t\tjumpInt(0);\n" +
			"\t\t\tnCycles += 7;\n" +
			"\t\t\ttailCall(runBBlockDyn());\n" +
			"\t\t}\n" +
			"\t\t\n"

		nCycles := 0
		for _, inst := range bBlock {
			prgRomCpp += "\t\t" + inst.asCpp() + "\n"
			nCycles += inst.op.nCycles
		}

		prgRomCpp += "\t\t\n" +
			fmt.Sprintf("\t\tnCycles += %d;\n", nCycles) +
			"\t\ttailCall(runBBlockDyn());\n" +
			"\t}\n" +
			"\t\n"
	}

	bBlockFuncCpps := make([]string, 0x8000)
	for i := range bBlockFuncCpps {
		bBlockFuncCpps[i] = "nullptr"
	}

	for addr := range bBlocks {
		bBlockFuncCpps[addr-0x8000] = fmt.Sprintf("stBBlock%.4X", addr)
	}

	prgRomCpp += "\tvoid (*stBBlockFuncs[0x8000])() = {"
	for _, funcCpp := range bBlockFuncCpps {
		prgRomCpp += funcCpp + ","
	}
	prgRomCpp += "};\n" +
		"\t\n"

	prgRomCpp += "}\n"

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
