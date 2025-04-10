package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"strconv"
)

const (
	_ = iota
	opNameAdc
	opNameAnd
	opNameAsl
	opNameBcc
	opNameBcs
	opNameBeq
	opNameBit
	opNameBmi
	opNameBne
	opNameBpl
	opNameBrk
	opNameBvc
	opNameBvs
	opNameClc
	opNameCld
	opNameCli
	opNameClv
	opNameCmp
	opNameCpx
	opNameCpy
	opNameDec
	opNameDex
	opNameDey
	opNameEor
	opNameInc
	opNameInx
	opNameIny
	opNameJmp
	opNameJsr
	opNameLda
	opNameLdx
	opNameLdy
	opNameLsr
	opNameNop
	opNameOra
	opNamePha
	opNamePhp
	opNamePla
	opNamePlp
	opNameRol
	opNameRor
	opNameRti
	opNameRts
	opNameSbc
	opNameSec
	opNameSed
	opNameSei
	opNameSta
	opNameStx
	opNameSty
	opNameTax
	opNameTay
	opNameTsx
	opNameTxa
	opNameTxs
	opNameTya
)

const (
	_ = iota
	addrModeImpl
	addrModeImm
	addrModeZpg
	addrModeZpgX
	addrModeZpgY
	addrModeAbs
	addrModeAbsX
	addrModeAbsY
	addrModeInd
	addrModeIndX
	addrModeIndY
	addrModeRel
)

type opT struct {
	name     int
	addrMode int
	nCycles  int
}

var ops = []opT{
	{opNameBrk, addrModeImm, 7},
	{opNameOra, addrModeIndX, 6},
	{},
	{},
	{},
	{opNameOra, addrModeZpg, 3},
	{opNameAsl, addrModeZpg, 5},
	{},
	{opNamePhp, addrModeImpl, 3},
	{opNameOra, addrModeImm, 2},
	{opNameAsl, addrModeImpl, 2},
	{},
	{},
	{opNameOra, addrModeAbs, 4},
	{opNameAsl, addrModeAbs, 6},
	{},

	{opNameBpl, addrModeRel, 2},
	{opNameOra, addrModeIndY, 5},
	{},
	{},
	{},
	{opNameOra, addrModeZpgX, 4},
	{opNameAsl, addrModeZpgX, 6},
	{},
	{opNameClc, addrModeImpl, 2},
	{opNameOra, addrModeAbsY, 4},
	{},
	{},
	{},
	{opNameOra, addrModeAbsX, 4},
	{opNameAsl, addrModeAbsX, 7},
	{},

	{opNameJsr, addrModeAbs, 6},
	{opNameAnd, addrModeIndX, 6},
	{},
	{},
	{opNameBit, addrModeZpg, 3},
	{opNameAnd, addrModeZpg, 3},
	{opNameRol, addrModeZpg, 5},
	{},
	{opNamePlp, addrModeImpl, 4},
	{opNameAnd, addrModeImm, 2},
	{opNameRol, addrModeImpl, 2},
	{},
	{opNameBit, addrModeAbs, 4},
	{opNameAnd, addrModeAbs, 4},
	{opNameRol, addrModeAbs, 6},
	{},

	{opNameBmi, addrModeRel, 2},
	{opNameAnd, addrModeIndY, 5},
	{},
	{},
	{},
	{opNameAnd, addrModeZpgX, 4},
	{opNameRol, addrModeZpgX, 6},
	{},
	{opNameSec, addrModeImpl, 2},
	{opNameAnd, addrModeAbsY, 4},
	{},
	{},
	{},
	{opNameAnd, addrModeAbsX, 4},
	{opNameRol, addrModeAbsX, 7},
	{},

	{opNameRti, addrModeImpl, 6},
	{opNameEor, addrModeIndX, 6},
	{},
	{},
	{},
	{opNameEor, addrModeZpg, 3},
	{opNameLsr, addrModeZpg, 5},
	{},
	{opNamePha, addrModeImpl, 3},
	{opNameEor, addrModeImm, 2},
	{opNameLsr, addrModeImpl, 2},
	{},
	{opNameJmp, addrModeAbs, 3},
	{opNameEor, addrModeAbs, 4},
	{opNameLsr, addrModeAbs, 6},
	{},

	{opNameBvc, addrModeRel, 2},
	{opNameEor, addrModeIndY, 5},
	{},
	{},
	{},
	{opNameEor, addrModeZpgX, 4},
	{opNameLsr, addrModeZpgX, 6},
	{},
	{opNameCli, addrModeImpl, 2},
	{opNameEor, addrModeAbsY, 4},
	{},
	{},
	{},
	{opNameEor, addrModeAbsX, 4},
	{opNameLsr, addrModeAbsX, 7},
	{},

	{opNameRts, addrModeImpl, 6},
	{opNameAdc, addrModeIndX, 6},
	{},
	{},
	{},
	{opNameAdc, addrModeZpg, 3},
	{opNameRor, addrModeZpg, 5},
	{},
	{opNamePla, addrModeImpl, 4},
	{opNameAdc, addrModeImm, 2},
	{opNameRor, addrModeImpl, 2},
	{},
	{opNameJmp, addrModeInd, 5},
	{opNameAdc, addrModeAbs, 4},
	{opNameRor, addrModeAbs, 6},
	{},

	{opNameBvs, addrModeRel, 2},
	{opNameAdc, addrModeIndY, 5},
	{},
	{},
	{},
	{opNameAdc, addrModeZpgX, 4},
	{opNameRor, addrModeZpgX, 6},
	{},
	{opNameSei, addrModeImpl, 2},
	{opNameAdc, addrModeAbsY, 4},
	{},
	{},
	{},
	{opNameAdc, addrModeAbsX, 4},
	{opNameRor, addrModeAbsX, 7},
	{},

	{},
	{opNameSta, addrModeIndX, 6},
	{},
	{},
	{opNameSty, addrModeZpg, 3},
	{opNameSta, addrModeZpg, 3},
	{opNameStx, addrModeZpg, 3},
	{},
	{opNameDey, addrModeImpl, 2},
	{},
	{opNameTxa, addrModeImpl, 2},
	{},
	{opNameSty, addrModeAbs, 4},
	{opNameSta, addrModeAbs, 4},
	{opNameStx, addrModeAbs, 4},
	{},

	{opNameBcc, addrModeRel, 2},
	{opNameSta, addrModeIndY, 6},
	{},
	{},
	{opNameSty, addrModeZpgX, 4},
	{opNameSta, addrModeZpgX, 4},
	{opNameStx, addrModeZpgY, 4},
	{},
	{opNameTya, addrModeImpl, 2},
	{opNameSta, addrModeAbsY, 5},
	{opNameTxs, addrModeImpl, 2},
	{},
	{},
	{opNameSta, addrModeAbsX, 5},
	{},
	{},

	{opNameLdy, addrModeImm, 2},
	{opNameLda, addrModeIndX, 6},
	{opNameLdx, addrModeImm, 2},
	{},
	{opNameLdy, addrModeZpg, 3},
	{opNameLda, addrModeZpg, 3},
	{opNameLdx, addrModeZpg, 3},
	{},
	{opNameTay, addrModeImpl, 2},
	{opNameLda, addrModeImm, 2},
	{opNameTax, addrModeImpl, 2},
	{},
	{opNameLdy, addrModeAbs, 4},
	{opNameLda, addrModeAbs, 4},
	{opNameLdx, addrModeAbs, 4},
	{},

	{opNameBcs, addrModeRel, 2},
	{opNameLda, addrModeIndY, 5},
	{},
	{},
	{opNameLdy, addrModeZpgX, 4},
	{opNameLda, addrModeZpgX, 4},
	{opNameLdx, addrModeZpgY, 4},
	{},
	{opNameClv, addrModeImpl, 2},
	{opNameLda, addrModeAbsY, 4},
	{opNameTsx, addrModeImpl, 2},
	{},
	{opNameLdy, addrModeAbsX, 4},
	{opNameLda, addrModeAbsX, 4},
	{opNameLdx, addrModeAbsY, 4},
	{},

	{opNameCpy, addrModeImm, 2},
	{opNameCmp, addrModeIndX, 6},
	{},
	{},
	{opNameCpy, addrModeZpg, 3},
	{opNameCmp, addrModeZpg, 3},
	{opNameDec, addrModeZpg, 5},
	{},
	{opNameIny, addrModeImpl, 2},
	{opNameCmp, addrModeImm, 2},
	{opNameDex, addrModeImpl, 2},
	{},
	{opNameCpy, addrModeAbs, 4},
	{opNameCmp, addrModeAbs, 4},
	{opNameDec, addrModeAbs, 6},
	{},

	{opNameBne, addrModeRel, 2},
	{opNameCmp, addrModeIndY, 5},
	{},
	{},
	{},
	{opNameCmp, addrModeZpgX, 4},
	{opNameDec, addrModeZpgX, 6},
	{},
	{opNameCld, addrModeImpl, 2},
	{opNameCmp, addrModeAbsY, 4},
	{},
	{},
	{},
	{opNameCmp, addrModeAbsX, 4},
	{opNameDec, addrModeAbsX, 7},
	{},

	{opNameCpx, addrModeImm, 2},
	{opNameSbc, addrModeIndX, 6},
	{},
	{},
	{opNameCpx, addrModeZpg, 3},
	{opNameSbc, addrModeZpg, 3},
	{opNameInc, addrModeZpg, 5},
	{},
	{opNameInx, addrModeImpl, 2},
	{opNameSbc, addrModeImm, 2},
	{opNameNop, addrModeImpl, 2},
	{},
	{opNameCpx, addrModeAbs, 4},
	{opNameSbc, addrModeAbs, 4},
	{opNameInc, addrModeAbs, 6},
	{},

	{opNameBeq, addrModeRel, 2},
	{opNameSbc, addrModeIndY, 5},
	{},
	{},
	{},
	{opNameSbc, addrModeZpgX, 4},
	{opNameInc, addrModeZpgX, 6},
	{},
	{opNameSed, addrModeImpl, 2},
	{opNameSbc, addrModeAbsY, 4},
	{},
	{},
	{},
	{opNameSbc, addrModeAbsX, 4},
	{opNameInc, addrModeAbsX, 7},
	{},
}

type opInstT struct {
	addr    uint16
	op      opT
	operand uint16
}

func readOpInst(addr uint16, buf *bytes.Reader) opInstT {
	opcode, err := buf.ReadByte()
	if err != nil {
		return opInstT{}
	}

	op := ops[opcode]

	operand := uint16(0)
	if op.addrMode == addrModeAbs ||
		op.addrMode == addrModeAbsX ||
		op.addrMode == addrModeAbsY ||
		op.addrMode == addrModeInd {

		v := make([]byte, 2)
		_, err := buf.Read(v)
		if err != nil {
			return opInstT{}
		}

		operand = binary.LittleEndian.Uint16(v)
	} else if op.addrMode != addrModeImpl {
		v, err := buf.ReadByte()
		if err != nil {
			return opInstT{}
		}

		if op.addrMode == addrModeRel {
			operand = uint16(int16(int8(v)))
		} else {
			operand = uint16(v)
		}
	}

	return opInstT{addr, op, operand}
}

func (inst opInstT) asCpp() string {
	if inst.op.name == opNameBrk {
		return fmt.Sprintf("pc = 0x%x; jumpInt(1);", int(inst.addr+2))
	} else if inst.op.name == opNameJmp {
		if inst.op.addrMode == addrModeAbs {
			return fmt.Sprintf("pc = 0x%x;", int(inst.operand))
		} else {
			return fmt.Sprintf("jumpInd(0x%x);", int(inst.operand))
		}
	} else if inst.op.name == opNameJsr {
		return fmt.Sprintf("pc = 0x%x; jumpSub(0x%x);", int(inst.addr+3), int(inst.operand))
	} else if inst.op.addrMode == addrModeImpl {
		switch inst.op.name {
		case opNameAsl:
			return "a = shL(a);"
		case opNameClc:
			return "p.c = 0;"
		case opNameCld:
			return "p.d = 0;"
		case opNameCli:
			return "p.i = 0;"
		case opNameClv:
			return "p.v = 0;"
		case opNameDex:
			return "setX(x - 1);"
		case opNameDey:
			return "setY(y - 1);"
		case opNameInx:
			return "setX(x + 1);"
		case opNameIny:
			return "setY(y + 1);"
		case opNameLsr:
			return "a = shR(a);"
		case opNameNop:
			return ";"
		case opNamePha:
			return "push(a);"
		case opNamePhp:
			return "pushP(1);"
		case opNamePla:
			return "setA(pull());"
		case opNamePlp:
			return "pullP();"
		case opNameRol:
			return "a = roL(a);"
		case opNameRor:
			return "a = roR(a);"
		case opNameRti:
			return "retInt();"
		case opNameRts:
			return "retSub();"
		case opNameSec:
			return "p.c = 1;"
		case opNameSed:
			return "p.d = 1;"
		case opNameSei:
			return "p.i = 1;"
		case opNameTax:
			return "setX(a);"
		case opNameTay:
			return "setY(a);"
		case opNameTsx:
			return "setX(spl);"
		case opNameTxa:
			return "setA(x);"
		case opNameTxs:
			return "spl = x;"
		case opNameTya:
			return "setA(y);"
		}

		return "bad"
	} else if inst.op.name == opNameSta ||
		inst.op.name == opNameStx ||
		inst.op.name == opNameSty {

		reg := "bad"
		switch inst.op.name {
		case opNameSta:
			reg = "a"
		case opNameStx:
			reg = "x"
		case opNameSty:
			reg = "y"
		}

		switch inst.op.addrMode {
		case addrModeZpg:
			return fmt.Sprintf("ram[0x%x] = %s;", int(inst.operand), reg)
		case addrModeZpgX:
			return fmt.Sprintf("ram[U8(0x%x + x)] = %s;", int(inst.operand), reg)
		case addrModeZpgY:
			return fmt.Sprintf("ram[U8(0x%x + y)] = %s;", int(inst.operand), reg)
		case addrModeAbs:
			return fmt.Sprintf("write(0x%x, %s);", int(inst.operand), reg)
		case addrModeAbsX:
			return fmt.Sprintf("write(0x%x + x, %s);", int(inst.operand), reg)
		case addrModeAbsY:
			return fmt.Sprintf("write(0x%x + y, %s);", int(inst.operand), reg)
		case addrModeIndX:
			return fmt.Sprintf("write(read16Zpg(0x%x + x), %s);", int(inst.operand), reg)
		case addrModeIndY:
			return fmt.Sprintf("write(read16Zpg(0x%x) + y, %s);", int(inst.operand), reg)
		}

		return "bad"
	} else if inst.op.name == opNameAsl ||
		inst.op.name == opNameLsr ||
		inst.op.name == opNameRol ||
		inst.op.name == opNameRor ||
		inst.op.name == opNameInc ||
		inst.op.name == opNameDec {

		funcCpp := "bad"
		switch inst.op.name {
		case opNameAsl:
			funcCpp = "shL"
		case opNameLsr:
			funcCpp = "shR"
		case opNameRol:
			funcCpp = "roL"
		case opNameRor:
			funcCpp = "roR"
		case opNameInc:
			funcCpp = "inc"
		case opNameDec:
			funcCpp = "dec"
		}

		switch inst.op.addrMode {
		case addrModeZpg:
			return fmt.Sprintf("rmw(0x%x, %s);", int(inst.operand), funcCpp)
		case addrModeZpgX:
			return fmt.Sprintf("rmw(U8(0x%x + x), %s);", int(inst.operand), funcCpp)
		case addrModeZpgY:
			return fmt.Sprintf("rmw(U8(0x%x + y), %s);", int(inst.operand), funcCpp)
		case addrModeAbs:
			return fmt.Sprintf("rmw(0x%x, %s);", int(inst.operand), funcCpp)
		case addrModeAbsX:
			return fmt.Sprintf("rmw(0x%x + x, %s);", int(inst.operand), funcCpp)
		case addrModeAbsY:
			return fmt.Sprintf("rmw(0x%x + y, %s);", int(inst.operand), funcCpp)
		case addrModeIndX:
			return fmt.Sprintf("rmw(read16Zpg(0x%x + x), %s);", int(inst.operand), funcCpp)
		case addrModeIndY:
			return fmt.Sprintf("rmw(read16Zpg(0x%x) + y, %s);", int(inst.operand), funcCpp)
		}

		return "bad"
	} else if inst.op.addrMode == addrModeRel {
		condCpp := "bad"
		switch inst.op.name {
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

		return fmt.Sprintf("if (%s) { pc = 0x%x; } else { pc = 0x%x; }",
			condCpp, int(inst.addr+2+inst.operand), int(inst.addr+2))
	} else {
		operandCpp := ""
		switch inst.op.addrMode {
		case addrModeImm:
			operandCpp = strconv.Itoa(int(inst.operand))
		case addrModeZpg:
			operandCpp = fmt.Sprintf("ram[0x%x]", int(inst.operand))
		case addrModeZpgX:
			operandCpp = fmt.Sprintf("ram[U8(0x%x + x)]", int(inst.operand))
		case addrModeZpgY:
			operandCpp = fmt.Sprintf("ram[U8(0x%x + y)]", int(inst.operand))
		case addrModeAbs:
			operandCpp = fmt.Sprintf("read(0x%x)", int(inst.operand))
		case addrModeAbsX:
			operandCpp = fmt.Sprintf("read(0x%x + x)", int(inst.operand))
		case addrModeAbsY:
			operandCpp = fmt.Sprintf("read(0x%x + y)", int(inst.operand))
		case addrModeInd:
			operandCpp = fmt.Sprintf("read16(0x%x)", int(inst.operand))
		case addrModeIndX:
			operandCpp = fmt.Sprintf("read(read16Zpg(0x%x + x))", int(inst.operand))
		case addrModeIndY:
			operandCpp = fmt.Sprintf("read(read16Zpg(0x%x) + y)", int(inst.operand))
		}

		switch inst.op.name {
		case opNameAdc:
			return "addA(" + operandCpp + ");"
		case opNameAnd:
			return "andA(" + operandCpp + ");"
		case opNameBit:
			return "bitA(" + operandCpp + ");"
		case opNameCmp:
			return "cmpA(" + operandCpp + ");"
		case opNameCpx:
			return "cmpX(" + operandCpp + ");"
		case opNameCpy:
			return "cmpY(" + operandCpp + ");"
		case opNameEor:
			return "eorA(" + operandCpp + ");"
		case opNameLda:
			return "setA(" + operandCpp + ");"
		case opNameLdx:
			return "setX(" + operandCpp + ");"
		case opNameLdy:
			return "setY(" + operandCpp + ");"
		case opNameOra:
			return "orA(" + operandCpp + ");"
		case opNameSbc:
			return "subA(" + operandCpp + ");"
		}

		return "bad"
	}
}

func readBBlock(addr uint16, buf *bytes.Reader) []opInstT {
	r := make([]opInstT, 0)
	for {
		pos, err := buf.Seek(0, 1)
		if err != nil {
			return nil
		}

		inst := readOpInst(addr+uint16(pos), buf)
		if inst.op.name == 0 {
			return nil
		}

		r = append(r, inst)

		if inst.op.addrMode == addrModeRel ||
			inst.op.name == opNameJmp ||
			inst.op.name == opNameJsr ||
			inst.op.name == opNameBrk ||
			inst.op.name == opNameRts ||
			inst.op.name == opNameRti {

			return r
		}
	}
}
