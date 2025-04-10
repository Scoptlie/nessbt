package main

import (
	"bytes"
	"encoding/binary"
	"flag"
	"fmt"
	"os"
	"sort"
)

func parseRomFile(romFile string) (mirrorV bool, prgRom []byte, chrRom []byte) {
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

func parseAddrsFile(file string) []uint16 {
	s, err := os.Open(file)
	if err != nil {
		panic(err)
	}
	defer s.Close()

	r := make([]uint16, 0)
	buf := [2]byte{}

	for {
		_, err := s.Read(buf[:])
		if err != nil {
			break
		}

		v := binary.LittleEndian.Uint16(buf[:])
		r = append(r, v)
	}

	return r
}

func findBBlockAddrs(prgRom []uint8) []uint16 {
	set := make(map[uint16]bool)
	for i := range len(prgRom) - 1 {
		v := prgRom[i : i+2]
		addr := binary.LittleEndian.Uint16(v)
		if addr >= 0x8000 {
			set[addr] = true
		}
	}
	r := make([]uint16, 0)
	for addr := range set {
		if addr == 0 {
			addr = 0
		}
		r = append(r, addr)
	}
	return r
}

func findBBlocks(addrs []uint16, prgRom []uint8) map[uint16][]opInstT {
	if addrs == nil {
		addrs = findBBlockAddrs(prgRom)
	}

	bBlocks := make(map[uint16][]opInstT)
	for len(addrs) > 0 {
		newAddrs := make([]uint16, 0)
		for _, addr := range addrs {
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
						newAddrs = append(newAddrs, addr1)
					}

					addr2 := addr1 + lastInst.operand
					if addr2 >= 0x8000 {
						newAddrs = append(newAddrs, addr2)
					}
				} else if lastInst.op.name == opNameBrk {
					addr := lastInst.addr + 2
					if addr >= 0x8000 {
						newAddrs = append(newAddrs, addr)
					}
				} else if lastInst.op.name == opNameJsr {
					addr1 := lastInst.operand
					if addr1 >= 0x8000 {
						newAddrs = append(newAddrs, addr1)
					}

					addr2 := lastInst.addr + 3
					if addr2 >= 0x8000 {
						newAddrs = append(newAddrs, addr2)
					}
				} else if lastInst.op.name == opNameJmp && lastInst.op.addrMode == addrModeAbs {
					addr := lastInst.operand
					if addr >= 0x8000 {
						newAddrs = append(newAddrs, addr)
					}
				}
			}
		}
		addrs = newAddrs
	}

	return bBlocks
}

func main() {
	prgRomCppFile := flag.String("prgRomCpp", "", "PRG ROM .cpp file to generate")
	chrRomCppFile := flag.String("chrRomCpp", "", "CHR ROM .cpp file to generate")
	romFile := flag.String("rom", "", "ROM file to translate")
	nmiLocationsFile := flag.String("nmiLocations", "", "Input NMI locations file")
	jumpTargetsFile := flag.String("jumpTargets", "", "Input jump targets file")

	flag.Parse()

	mirrorV, prgRom, chrRom := parseRomFile(*romFile)

	var nmiLocations []uint16
	if len(*nmiLocationsFile) > 0 {
		nmiLocations = parseAddrsFile(*nmiLocationsFile)
	}

	var jumpTargets []uint16
	if len(*jumpTargetsFile) > 0 {
		jumpTargets = parseAddrsFile(*jumpTargetsFile)
	}

	bBlocks := findBBlocks(jumpTargets, prgRom)

	prgRomCpp := "#include \"runtime/cpu.h\"\n" +
		"\n" +
		"#include \"runtime/env.h\"\n" +
		"#include \"runtime/ppu.h\"\n" +
		"#include \"runtime/profiler.h\"\n" +
		"\n" +
		"namespace Cpu {\n" +
		"\tU8 prgRom[0x8000] = {"
	for _, b := range prgRom {
		prgRomCpp += fmt.Sprintf("%d,", int(b))
	}
	prgRomCpp += "};\n" +
		fmt.Sprintf("\tUSize prgRomSize = %d;\n", len(prgRom)) +
		"\t\n"

	for addr := range bBlocks {
		prgRomCpp += fmt.Sprintf("\tvoid stBBlock%.4X();\n", addr)
	}
	prgRomCpp += "\t\n"

	bBlockAddrs := make([]int, 0, len(bBlocks))
	for addr := range bBlocks {
		bBlockAddrs = append(bBlockAddrs, int(addr))
	}

	sort.Ints(bBlockAddrs)

	for _, addr := range bBlockAddrs {
		bBlock := bBlocks[uint16(addr)]

		funcName := fmt.Sprintf("stBBlock%.4X", addr)

		checkInterrupts := len(nmiLocations) == 0
		for _, nmiAddr := range nmiLocations {
			if addr == int(nmiAddr) {
				checkInterrupts = true
				break
			}
		}

		prgRomCpp += "\tvoid " + funcName + "() {\n" +
			fmt.Sprintf("\t\tProfiler::addJumpTarget(0x%x);\n", addr) +
			"\t\t\n"

		if checkInterrupts {
			prgRomCpp += "\t\tEnv::update(nCycles);\n" +
				"\t\tnCycles = 0;\n" +
				"\t\t\n" +
				"\t\tif (Ppu::nmi) {\n" +
				fmt.Sprintf("\t\t\tProfiler::addNmiLocation(0x%x);\n", addr) +
				"\t\t\t\n" +
				"\t\t\tjumpInt(0);\n" +
				"\t\t\tnCycles += 7;\n" +
				"\t\t\ttailCall(runBBlockDyn());\n" +
				"\t\t}\n" +
				"\t\t\n"
		}

		nCycles := 0
		for _, inst := range bBlock[:len(bBlock)-1] {
			prgRomCpp += "\t\t" + inst.asCpp() + "\n"

			nCycles += inst.op.nCycles
		}
		lastInst := bBlock[len(bBlock)-1]
		nCycles += lastInst.op.nCycles

		prgRomCpp += "\t\t\n" +
			fmt.Sprintf("\t\tnCycles += %d;\n", nCycles)

		if lastInst.op.name == opNameJmp {
			if lastInst.op.addrMode == addrModeAbs {
				_, exists := bBlocks[lastInst.operand]

				prgRomCpp += fmt.Sprintf("\t\tpc = 0x%x;\n", int(lastInst.operand))
				if exists {
					prgRomCpp += fmt.Sprintf("\t\tstBBlock%.4X();\n", int(lastInst.operand))
				} else {
					prgRomCpp += "\t\ttailCall(runBBlockDyn());\n"
				}
			} else {
				prgRomCpp += fmt.Sprintf("\t\tjumpInd(0x%x);\n", int(lastInst.operand)) +
					"\t\ttailCall(runBBlockDyn());\n"
			}
		} else if lastInst.op.name == opNameJsr {
			_, exists := bBlocks[lastInst.operand]

			prgRomCpp += fmt.Sprintf("\t\tpc = 0x%x;\n", int(lastInst.addr+3)) +
				fmt.Sprintf("\t\tjumpSub(0x%x);\n", int(lastInst.operand))
			if exists {
				prgRomCpp += fmt.Sprintf("\t\tstBBlock%.4X();\n", int(lastInst.operand))
			} else {
				prgRomCpp += "\t\ttailCall(runBBlockDyn());\n"
			}
		} else if lastInst.op.name == opNameBrk {
			prgRomCpp += fmt.Sprintf("\t\tpc = 0x%x;\n", int(lastInst.addr+2)) +
				"\t\tjumpInt(1);\n" +
				"\t\ttailCall(runBBlockDyn());\n"
		} else if lastInst.op.name == opNameRts {
			prgRomCpp += "\t\tretSub();\n" +
				"\t\ttailCall(runBBlockDyn());\n"
		} else if lastInst.op.name == opNameRti {
			prgRomCpp += "\t\tretInt();\n" +
				"\t\ttailCall(runBBlockDyn());\n"
		} else if lastInst.op.addrMode == addrModeRel {
			condCpp := "bad"
			switch lastInst.op.name {
			case opNameBcc:
				condCpp = "!p.c"
			case opNameBcs:
				condCpp = "p.c"
			case opNameBeq:
				condCpp = "p.z"
			case opNameBmi:
				condCpp = "p.n"
			case opNameBne:
				condCpp = "!p.z"
			case opNameBpl:
				condCpp = "!p.n"
			case opNameBvc:
				condCpp = "!p.v"
			case opNameBvs:
				condCpp = "p.v"
			}

			addr1 := lastInst.addr + 2
			addr2 := addr1 + lastInst.operand

			_, exists1 := bBlocks[addr1]
			_, exists2 := bBlocks[addr2]

			prgRomCpp += "\t\tif (" + condCpp + ") {\n" +
				"\t\t\tnCycles++;\n" +
				fmt.Sprintf("\t\t\tpc = 0x%x;\n", int(addr2))
			if exists2 {
				prgRomCpp += fmt.Sprintf("\t\t\tstBBlock%.4X();\n", int(addr2))
			} else {
				prgRomCpp += "\t\ttailCall(runBBlockDyn());\n"
			}
			prgRomCpp += "\t\t} else {\n" +
				fmt.Sprintf("\t\t\tpc = 0x%x;\n", int(addr1))
			if exists1 {
				prgRomCpp += fmt.Sprintf("\t\t\tstBBlock%.4X();\n", int(addr1))
			} else {
				prgRomCpp += "\t\ttailCall(runBBlockDyn());\n"
			}
			prgRomCpp += "\t\t}\n"

		} else {
			prgRomCpp += "bad"
		}

		prgRomCpp += "\t}\n" +
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
