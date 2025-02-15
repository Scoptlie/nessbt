
import json
import os
import subprocess

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
	
	info_file = 'gen/info/'+source_file+'.info'
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
		exit(-1)
	
	gen_info_file(info_file, cmd, deps)

def gen_pch_file(
	pch_file: str,
	header_file: str
):
	cmd = [cpp_compiler, '-o', pch_file, '-x', 'c++-header', header_file] + cpp_compiler_args + c_cpp_compiler_args
	
	info_file = 'gen/info/'+pch_file+'.info'
	if is_up_to_date(pch_file, info_file, cmd):
		return
	
	os.makedirs(os.path.dirname(pch_file), exist_ok=True)
	
	print('\x1b[93m' + pch_file + '\x1b[0m')
	r = subprocess.run(cmd).returncode
	if r != 0:
		exit(-1)
	
	gen_info_file(info_file, cmd, [header_file])

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
		exit(-1)
	
	gen_info_file(info_file, cmd, obj_files)

c_compiler = os.environ['C_COMPILER']
cpp_compiler = os.environ['CPP_COMPILER']
linker = os.environ['LINKER']

c_compiler_args = []
cpp_compiler_args = [
	'-std=c++23'
]
c_cpp_compiler_args = [
	'-g',
	#'-O3',
	'-Isource',
	'-Ithirdparty/source',
]
linker_args = [
	'-std=c++2a',
	'-Lthirdparty/lib',
	'-lSDL2main',
	'-lSDL2',
	'-lopengl32',
	'-lopenal'
]

compile_commands = []

header_files_to_precompile = [
	'thirdparty/source/GL/gl3w.h',
	'thirdparty/source/glm/glm.hpp'
]

pch_files = []
for file in header_files_to_precompile:
	pch_file = 'gen/pch/'+file+'.pch'
	gen_pch_file(pch_file, file)
	pch_files.append(pch_file)

source_dirs = ['source', 'thirdparty/source']

source_files = [os.path.join(root, file)
	for dir in source_dirs
	for root, _, files in os.walk(dir)
	for file in files if file.endswith(('.c', '.cpp'))
]

obj_files = []
for file in source_files:
	obj_file = 'gen/obj/'+file+'.o'
	gen_obj_file(obj_file, file, pch_files)
	obj_files.append(obj_file)

gen_bin_file('gen/a.exe', obj_files)

with open('gen/compile_commands.json', 'w') as s:
	json.dump(compile_commands, s)
