
import json
import os
import subprocess

palette_file = os.environ['PALETTE']
rom_file = os.environ['ROM']

input_nmi_locations = os.getenv('INPUT_NMI_LOCATIONS', '0') == '1'
input_jump_targets = os.getenv('INPUT_JUMP_TARGETS', '0') == '1'
output_nmi_locations = os.getenv('OUTPUT_NMI_LOCATIONS', '0') == '1'
output_jump_targets = os.getenv('OUTPUT_JUMP_TARGETS', '0') == '1'
output_frame_profiles = os.getenv('OUTPUT_FRAME_PROFILES', '0') == '1'
only_use_emulation = os.getenv('ONLY_USE_EMULATION', '0') == '1'
only_use_translations = os.getenv('ONLY_USE_TRANSLATIONS', '0') == '1'

use_optimisations = os.getenv('USE_OPTIMISATIONS', '0') == '1'

c_compiler = os.environ['C_COMPILER']
cpp_compiler = os.environ['CPP_COMPILER']
linker = os.environ['LINKER']

def is_up_to_date(
	file: str,
	info_file: str,
	params: object
) -> bool:
	if not os.path.exists(file) or not os.path.exists(info_file):
		return False
	
	with open(info_file, 'r') as s:
		info = json.load(s)
	
	if info['params'] != params:
		return False
	
	file_mtime = os.stat(file).st_mtime
	for dep in info['deps']:
		if not os.path.exists(dep) or\
			os.stat(dep).st_mtime > file_mtime:
			
			return False
	
	return True

def gen_info_file(
	info_file: str,
	params: object,
	deps: list[str]
):
	os.makedirs(os.path.dirname(info_file), exist_ok=True)
	
	with open(info_file, 'w') as s:
		json.dump({
			'params': params,
			'deps': deps
		}, s)

def gen_palette_cpp_file(
	cpp_file: str,
	palette_file: str
):
	info_file = 'gen/info/'+cpp_file+'.info'
	if is_up_to_date(cpp_file, info_file, palette_file):
		return
	
	palette = []
	with open(palette_file, 'rb') as s:
		for i in range(0, 64):
			rgb = s.read(3)
			palette.append(rgb)
	
	os.makedirs(os.path.dirname(cpp_file), exist_ok=True)
	
	print('\x1b[96m' + cpp_file + '\x1b[0m')
	
	with open(cpp_file, 'w') as s:
		s.write(
			'#include "runtime/ppu.h"\n'
			'\n'
			'namespace Ppu {\n'
			'\tRgb globalPalette[0x40] = {\n'
		)
		
		for rgb in palette:
			s.write(f'\t\tRgb{{{rgb[0]}, {rgb[1]}, {rgb[2]}}},\n')
		
		s.write(
			'\t};\n'
			'}\n'
		)
	
	gen_info_file(info_file, palette_file, [palette_file])

def gen_rom_cpp_files(
	prg_rom_cpp_file: str,
	chr_rom_cpp_file: str,
	rom_file: str,
	nmi_locations_file: str | None,
	jump_targets_file: str | None
):
	params = {
		"rom_file": rom_file,
		"jump_targets_file": jump_targets_file,
		"nmi_locations_file": nmi_locations_file
	}
	
	prg_rom_info_file = 'gen/info/'+prg_rom_cpp_file+'.info'
	chr_rom_info_file = 'gen/info/'+chr_rom_cpp_file+'.info'
	if (
		is_up_to_date(prg_rom_cpp_file, prg_rom_info_file, params) and
		is_up_to_date(chr_rom_cpp_file, chr_rom_info_file, params)
	):
		return
	
	os.makedirs(os.path.dirname(prg_rom_cpp_file), exist_ok=True)
	os.makedirs(os.path.dirname(chr_rom_cpp_file), exist_ok=True)
	
	print('\x1b[96m' + prg_rom_cpp_file + '\x1b[0m')
	print('\x1b[96m' + chr_rom_cpp_file + '\x1b[0m')
	
	cmd = [
		'go', 'run', './source/translator',
		'-prgRomCpp', prg_rom_cpp_file,
		'-chrRomCpp', chr_rom_cpp_file,
		'-rom', rom_file
	]
	if nmi_locations_file != None:
		cmd += ['-nmiLocations', nmi_locations_file]
	if jump_targets_file != None:
		cmd += ['-jumpTargets', jump_targets_file]
	
	r = subprocess.run(cmd).returncode
	if r != 0:
		exit(1)
	
	deps = [rom_file]
	if nmi_locations_file != None:
		deps += [nmi_locations_file]
	if jump_targets_file != None:
		deps += [jump_targets_file]
	
	gen_info_file(prg_rom_info_file, params, deps)
	gen_info_file(chr_rom_info_file, params, deps)
	
	pass

def gen_obj_file(
	obj_file: str,
	source_file: str,
	pch_files: list[str]
):
	if os.path.splitext(source_file)[1] == '.c':
		compiler = c_compiler
		compiler_args = c_compiler_args + c_cpp_compiler_args
	else:
		compiler = cpp_compiler
		compiler_args = cpp_compiler_args + c_cpp_compiler_args
		
		for file in pch_files:
			compiler_args += ['-include-pch', file]
	
	cmd = [compiler, '-o', obj_file, source_file, '-c'] + compiler_args
	
	compile_commands.append({
		'directory': os.getcwd(),
		'arguments': cmd,
		'file': source_file
	})
	
	info_file = 'gen/info/'+obj_file+'.info'
	if is_up_to_date(obj_file, info_file, cmd):
		return
	
	rule = subprocess.run(
		[compiler, '-M', source_file] + compiler_args,
		text=True, capture_output=True
	).stdout
	
	if len(rule) == 0 or rule[-1] != '\n':
		rule += '\n'
	
	deps = []
	dep = ''
	i = rule.find(':') + 1
	while i != len(rule):
		if rule[i : i+2] in ['\\\n', '\\\r']:
			i += 2
		elif rule[i : i+3] == '\\\r\n':
			i += 3
		elif rule[i] in [' ', '\t', '\n', '\r', '\f']:
			i += 1
			if len(dep) > 0:
				deps.append(dep)
				dep = ''
		else:
			if rule[i : i+2] in ['\\ ', '$$']:
				i += 1
			
			dep += rule[i]
			i += 1
	
	os.makedirs(os.path.dirname(obj_file), exist_ok=True)
	
	print('\x1b[95m' + obj_file + '\x1b[0m')
	r = subprocess.run(cmd).returncode
	if r != 0:
		exit(1)
	
	gen_info_file(info_file, cmd, deps)

def gen_bin_file(
	bin_file: str,
	obj_files: list[str]
):
	cmd = [linker, '-o', bin_file] + obj_files + linker_args
	
	info_file = 'gen/info/'+bin_file+'.info'
	if is_up_to_date(bin_file, info_file, cmd):
		return
	
	os.makedirs(os.path.dirname(bin_file), exist_ok=True)
	
	print('\x1b[95m' + bin_file + '\x1b[0m')
	r = subprocess.run(cmd).returncode
	if r != 0:
		exit(1)
	
	gen_info_file(info_file, cmd, obj_files)

palette_cpp_file = 'gen/source/runtime/ppu/globalPalette.cpp'

gen_palette_cpp_file(palette_cpp_file, palette_file)

prg_rom_cpp_file = 'gen/source/runtime/cpu/prgRom.cpp'
chr_rom_cpp_file = 'gen/source/runtime/ppu/chrRom.cpp'
jump_targets_file = 'gen/jumpTargets' if input_jump_targets else None
nmi_locations_file = 'gen/nmiLocations' if input_nmi_locations else None

gen_rom_cpp_files(
	prg_rom_cpp_file, chr_rom_cpp_file,
	rom_file,
	nmi_locations_file, jump_targets_file
)

c_compiler_args = []
cpp_compiler_args = [
	'-std=c++23'
]
c_cpp_compiler_args = [
	'-g',
	'-Isource',
	'-Ithirdparty/source',
]
linker_args = [
	'-std=c++2a',
	'-Lthirdparty/lib',
	'-lSDL2main',
	'-lSDL2',
	'-lopengl32'
]

if output_nmi_locations:
	c_cpp_compiler_args += ['-DoutputNmiLocations']
if output_jump_targets:
	c_cpp_compiler_args += ['-DoutputJumpTargets']
if output_frame_profiles:
	c_cpp_compiler_args += ['-DoutputFrameProfiles']
if only_use_emulation:
	c_cpp_compiler_args += ['-DonlyUseEmulation']
if only_use_translations:
	c_cpp_compiler_args += ['-DonlyUseTranslations']
if use_optimisations:
	c_cpp_compiler_args += ['-flto', '-O3']
	linker_args += ['-flto', '-O3']
else:
	c_cpp_compiler_args += ['-O1']

compile_commands = []

source_dirs = ['source', 'thirdparty/source']

source_files = [os.path.join(root, file)
	for dir in source_dirs
	for root, _, files in os.walk(dir)
	for file in files if file.endswith(('.c', '.cpp'))
] + [palette_cpp_file, prg_rom_cpp_file, chr_rom_cpp_file]

obj_files = []
for file in source_files:
	obj_file = 'gen/obj/'+file+'.o'
	gen_obj_file(obj_file, file, [])
	obj_files.append(obj_file)

gen_bin_file('gen/a.exe', obj_files)

with open('gen/compile_commands.json', 'w') as s:
	json.dump(compile_commands, s)
