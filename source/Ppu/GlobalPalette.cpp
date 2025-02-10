#include "GlobalPalette.h"

namespace Ppu::GlobalPalette {
	Rgb colours[512] = {
		Rgb{82, 82, 82},
		Rgb{1, 26, 81},
		Rgb{15, 15, 101},
		Rgb{35, 6, 99},
		Rgb{54, 3, 75},
		Rgb{64, 4, 38},
		Rgb{63, 9, 4},
		Rgb{50, 19, 0},
		Rgb{31, 32, 0},
		Rgb{11, 42, 0},
		Rgb{0, 47, 0},
		Rgb{0, 46, 10},
		Rgb{0, 38, 45},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{160, 160, 160},
		Rgb{30, 74, 157},
		Rgb{56, 55, 188},
		Rgb{88, 40, 184},
		Rgb{117, 33, 148},
		Rgb{132, 35, 92},
		Rgb{130, 46, 36},
		Rgb{111, 63, 0},
		Rgb{81, 82, 0},
		Rgb{49, 99, 0},
		Rgb{26, 107, 5},
		Rgb{14, 105, 46},
		Rgb{16, 92, 104},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{254, 255, 255},
		Rgb{105, 158, 252},
		Rgb{137, 135, 255},
		Rgb{174, 118, 255},
		Rgb{206, 109, 241},
		Rgb{224, 112, 178},
		Rgb{222, 124, 112},
		Rgb{200, 145, 62},
		Rgb{166, 167, 37},
		Rgb{129, 186, 40},
		Rgb{99, 196, 70},
		Rgb{84, 193, 125},
		Rgb{86, 179, 192},
		Rgb{60, 60, 60},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{254, 255, 255},
		Rgb{190, 214, 253},
		Rgb{204, 204, 255},
		Rgb{221, 196, 255},
		Rgb{234, 192, 249},
		Rgb{242, 193, 223},
		Rgb{241, 199, 194},
		Rgb{232, 208, 170},
		Rgb{217, 218, 157},
		Rgb{201, 226, 158},
		Rgb{188, 230, 174},
		Rgb{180, 229, 199},
		Rgb{181, 223, 228},
		Rgb{169, 169, 169},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{75, 52, 50},
		Rgb{0, 7, 46},
		Rgb{11, 1, 65},
		Rgb{29, 0, 66},
		Rgb{48, 0, 49},
		Rgb{59, 0, 22},
		Rgb{61, 3, 0},
		Rgb{47, 9, 0},
		Rgb{28, 16, 0},
		Rgb{9, 22, 0},
		Rgb{0, 24, 0},
		Rgb{0, 21, 0},
		Rgb{0, 13, 17},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{148, 113, 108},
		Rgb{23, 39, 103},
		Rgb{48, 28, 132},
		Rgb{79, 19, 133},
		Rgb{107, 17, 107},
		Rgb{124, 21, 65},
		Rgb{126, 33, 22},
		Rgb{106, 44, 0},
		Rgb{76, 56, 0},
		Rgb{45, 66, 0},
		Rgb{22, 69, 0},
		Rgb{10, 64, 16},
		Rgb{8, 50, 57},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{238, 188, 181},
		Rgb{93, 103, 176},
		Rgb{124, 89, 207},
		Rgb{160, 79, 209},
		Rgb{192, 75, 180},
		Rgb{211, 81, 132},
		Rgb{214, 96, 81},
		Rgb{191, 109, 37},
		Rgb{157, 123, 15},
		Rgb{121, 135, 14},
		Rgb{92, 138, 34},
		Rgb{75, 132, 73},
		Rgb{73, 117, 123},
		Rgb{54, 35, 33},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{238, 188, 181},
		Rgb{175, 152, 179},
		Rgb{189, 145, 192},
		Rgb{205, 141, 192},
		Rgb{219, 139, 181},
		Rgb{227, 142, 161},
		Rgb{228, 148, 138},
		Rgb{218, 154, 117},
		Rgb{204, 161, 105},
		Rgb{188, 165, 105},
		Rgb{175, 167, 116},
		Rgb{167, 164, 134},
		Rgb{166, 158, 157},
		Rgb{157, 120, 115},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{46, 69, 39},
		Rgb{0, 19, 44},
		Rgb{0, 7, 58},
		Rgb{10, 0, 53},
		Rgb{21, 0, 32},
		Rgb{31, 0, 8},
		Rgb{33, 3, 0},
		Rgb{25, 13, 0},
		Rgb{12, 25, 0},
		Rgb{1, 37, 0},
		Rgb{0, 44, 0},
		Rgb{0, 40, 0},
		Rgb{0, 31, 19},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{103, 139, 92},
		Rgb{8, 61, 100},
		Rgb{24, 42, 121},
		Rgb{45, 26, 114},
		Rgb{64, 18, 81},
		Rgb{80, 22, 41},
		Rgb{82, 34, 4},
		Rgb{70, 52, 0},
		Rgb{49, 72, 0},
		Rgb{28, 89, 0},
		Rgb{12, 100, 0},
		Rgb{2, 94, 19},
		Rgb{1, 80, 61},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{174, 225, 157},
		Rgb{60, 137, 166},
		Rgb{82, 114, 189},
		Rgb{108, 95, 182},
		Rgb{129, 84, 146},
		Rgb{148, 90, 100},
		Rgb{150, 104, 52},
		Rgb{137, 126, 18},
		Rgb{113, 149, 4},
		Rgb{86, 169, 8},
		Rgb{66, 182, 33},
		Rgb{51, 175, 72},
		Rgb{49, 159, 122},
		Rgb{30, 49, 24},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{174, 225, 157},
		Rgb{125, 187, 161},
		Rgb{134, 177, 170},
		Rgb{146, 169, 167},
		Rgb{155, 164, 153},
		Rgb{163, 167, 133},
		Rgb{164, 173, 112},
		Rgb{158, 183, 95},
		Rgb{148, 193, 86},
		Rgb{137, 202, 89},
		Rgb{127, 207, 102},
		Rgb{120, 204, 121},
		Rgb{119, 197, 143},
		Rgb{110, 147, 98},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{45, 46, 30},
		Rgb{0, 4, 33},
		Rgb{0, 0, 47},
		Rgb{10, 0, 45},
		Rgb{21, 0, 29},
		Rgb{31, 0, 6},
		Rgb{32, 0, 0},
		Rgb{24, 5, 0},
		Rgb{11, 12, 0},
		Rgb{0, 19, 0},
		Rgb{0, 22, 0},
		Rgb{0, 19, 0},
		Rgb{0, 11, 12},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{102, 102, 77},
		Rgb{7, 35, 82},
		Rgb{24, 23, 103},
		Rgb{44, 14, 100},
		Rgb{63, 9, 76},
		Rgb{79, 13, 37},
		Rgb{81, 24, 2},
		Rgb{68, 36, 0},
		Rgb{48, 49, 0},
		Rgb{27, 60, 0},
		Rgb{11, 66, 0},
		Rgb{1, 60, 9},
		Rgb{0, 47, 47},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{172, 172, 137},
		Rgb{59, 95, 142},
		Rgb{81, 80, 165},
		Rgb{106, 68, 162},
		Rgb{128, 62, 135},
		Rgb{147, 68, 90},
		Rgb{149, 82, 44},
		Rgb{134, 96, 13},
		Rgb{110, 112, 1},
		Rgb{85, 125, 2},
		Rgb{64, 131, 18},
		Rgb{49, 125, 54},
		Rgb{47, 110, 102},
		Rgb{29, 29, 17},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{172, 172, 137},
		Rgb{123, 139, 139},
		Rgb{133, 133, 148},
		Rgb{144, 127, 147},
		Rgb{154, 125, 136},
		Rgb{161, 127, 117},
		Rgb{162, 133, 96},
		Rgb{156, 140, 81},
		Rgb{146, 147, 72},
		Rgb{134, 152, 73},
		Rgb{125, 155, 83},
		Rgb{118, 152, 101},
		Rgb{117, 146, 122},
		Rgb{108, 108, 83},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{55, 55, 89},
		Rgb{0, 15, 83},
		Rgb{9, 8, 104},
		Rgb{23, 0, 99},
		Rgb{36, 0, 75},
		Rgb{42, 0, 40},
		Rgb{38, 0, 7},
		Rgb{26, 3, 0},
		Rgb{9, 10, 0},
		Rgb{0, 19, 0},
		Rgb{0, 26, 0},
		Rgb{0, 27, 14},
		Rgb{0, 23, 49},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{117, 117, 171},
		Rgb{19, 55, 161},
		Rgb{44, 42, 192},
		Rgb{67, 27, 184},
		Rgb{87, 19, 148},
		Rgb{97, 18, 95},
		Rgb{91, 23, 42},
		Rgb{72, 33, 6},
		Rgb{44, 45, 0},
		Rgb{23, 61, 0},
		Rgb{8, 72, 13},
		Rgb{3, 73, 55},
		Rgb{6, 67, 110},
		Rgb{0, 0, 1},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{194, 193, 255},
		Rgb{80, 123, 255},
		Rgb{110, 108, 255},
		Rgb{137, 90, 255},
		Rgb{160, 79, 245},
		Rgb{171, 77, 186},
		Rgb{165, 84, 123},
		Rgb{143, 97, 74},
		Rgb{111, 111, 48},
		Rgb{85, 131, 54},
		Rgb{64, 142, 85},
		Rgb{56, 144, 138},
		Rgb{61, 137, 202},
		Rgb{37, 37, 66},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{194, 193, 255},
		Rgb{145, 164, 255},
		Rgb{159, 157, 255},
		Rgb{170, 149, 255},
		Rgb{180, 144, 255},
		Rgb{185, 143, 234},
		Rgb{182, 146, 207},
		Rgb{173, 152, 184},
		Rgb{159, 159, 171},
		Rgb{147, 167, 174},
		Rgb{138, 172, 189},
		Rgb{134, 173, 214},
		Rgb{136, 170, 241},
		Rgb{125, 124, 180},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{50, 39, 53},
		Rgb{0, 4, 47},
		Rgb{5, 0, 67},
		Rgb{18, 0, 65},
		Rgb{30, 0, 48},
		Rgb{37, 0, 23},
		Rgb{36, 0, 1},
		Rgb{24, 0, 0},
		Rgb{8, 6, 0},
		Rgb{0, 12, 0},
		Rgb{0, 15, 0},
		Rgb{0, 14, 0},
		Rgb{0, 10, 18},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{109, 91, 115},
		Rgb{13, 33, 105},
		Rgb{37, 22, 134},
		Rgb{59, 13, 132},
		Rgb{79, 9, 106},
		Rgb{90, 10, 67},
		Rgb{88, 16, 27},
		Rgb{69, 26, 0},
		Rgb{42, 38, 0},
		Rgb{21, 48, 0},
		Rgb{6, 54, 0},
		Rgb{0, 52, 20},
		Rgb{1, 44, 59},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{182, 157, 190},
		Rgb{69, 90, 180},
		Rgb{99, 76, 212},
		Rgb{125, 64, 209},
		Rgb{148, 59, 181},
		Rgb{160, 60, 136},
		Rgb{159, 69, 89},
		Rgb{137, 81, 44},
		Rgb{105, 95, 21},
		Rgb{80, 108, 22},
		Rgb{60, 114, 43},
		Rgb{49, 112, 81},
		Rgb{51, 103, 127},
		Rgb{33, 24, 36},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{182, 157, 190},
		Rgb{133, 129, 186},
		Rgb{147, 122, 199},
		Rgb{158, 117, 198},
		Rgb{168, 114, 186},
		Rgb{173, 115, 168},
		Rgb{172, 119, 147},
		Rgb{163, 125, 126},
		Rgb{149, 131, 113},
		Rgb{138, 136, 114},
		Rgb{129, 139, 125},
		Rgb{124, 138, 143},
		Rgb{124, 134, 163},
		Rgb{115, 98, 122},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{36, 47, 48},
		Rgb{0, 10, 47},
		Rgb{0, 3, 61},
		Rgb{7, 0, 57},
		Rgb{18, 0, 35},
		Rgb{25, 0, 12},
		Rgb{24, 0, 0},
		Rgb{16, 1, 0},
		Rgb{5, 8, 0},
		Rgb{0, 17, 0},
		Rgb{0, 23, 0},
		Rgb{0, 22, 1},
		Rgb{0, 17, 23},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{87, 104, 106},
		Rgb{4, 45, 104},
		Rgb{20, 32, 126},
		Rgb{40, 18, 119},
		Rgb{59, 10, 86},
		Rgb{69, 11, 48},
		Rgb{68, 18, 12},
		Rgb{55, 29, 0},
		Rgb{35, 41, 0},
		Rgb{15, 57, 0},
		Rgb{2, 68, 1},
		Rgb{0, 66, 28},
		Rgb{0, 57, 67},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{151, 175, 178},
		Rgb{51, 107, 176},
		Rgb{72, 92, 200},
		Rgb{97, 74, 192},
		Rgb{119, 63, 155},
		Rgb{131, 65, 112},
		Rgb{129, 74, 67},
		Rgb{114, 88, 33},
		Rgb{91, 103, 17},
		Rgb{66, 122, 22},
		Rgb{47, 134, 49},
		Rgb{37, 132, 87},
		Rgb{39, 122, 134},
		Rgb{22, 30, 32},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{151, 175, 178},
		Rgb{107, 146, 177},
		Rgb{117, 140, 187},
		Rgb{128, 131, 184},
		Rgb{137, 127, 169},
		Rgb{142, 127, 150},
		Rgb{142, 131, 130},
		Rgb{136, 138, 114},
		Rgb{126, 145, 105},
		Rgb{115, 153, 108},
		Rgb{106, 158, 122},
		Rgb{101, 157, 139},
		Rgb{101, 153, 160},
		Rgb{93, 110, 113},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{37, 37, 37},
		Rgb{0, 3, 35},
		Rgb{0, 0, 49},
		Rgb{7, 0, 47},
		Rgb{18, 0, 32},
		Rgb{25, 0, 9},
		Rgb{24, 0, 0},
		Rgb{16, 0, 0},
		Rgb{5, 5, 0},
		Rgb{0, 11, 0},
		Rgb{0, 15, 0},
		Rgb{0, 14, 0},
		Rgb{0, 9, 13},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{88, 88, 88},
		Rgb{5, 31, 86},
		Rgb{20, 19, 107},
		Rgb{40, 10, 104},
		Rgb{59, 6, 80},
		Rgb{69, 7, 43},
		Rgb{68, 14, 8},
		Rgb{55, 24, 0},
		Rgb{36, 36, 0},
		Rgb{16, 47, 0},
		Rgb{2, 53, 0},
		Rgb{0, 51, 14},
		Rgb{0, 43, 50},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{152, 152, 152},
		Rgb{51, 86, 150},
		Rgb{72, 71, 172},
		Rgb{97, 60, 170},
		Rgb{119, 54, 143},
		Rgb{131, 56, 100},
		Rgb{129, 64, 56},
		Rgb{115, 78, 24},
		Rgb{92, 93, 9},
		Rgb{67, 105, 10},
		Rgb{48, 112, 29},
		Rgb{38, 110, 64},
		Rgb{39, 100, 109},
		Rgb{22, 22, 22},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0},
		Rgb{152, 152, 152},
		Rgb{108, 124, 151},
		Rgb{118, 117, 160},
		Rgb{129, 112, 159},
		Rgb{138, 109, 148},
		Rgb{143, 110, 130},
		Rgb{142, 114, 110},
		Rgb{136, 120, 94},
		Rgb{126, 127, 86},
		Rgb{115, 132, 87},
		Rgb{106, 135, 97},
		Rgb{101, 134, 114},
		Rgb{102, 130, 134},
		Rgb{94, 94, 94},
		Rgb{0, 0, 0},
		Rgb{0, 0, 0}
	};
	
	Rgb colour(U8 idx, U8 emphasis) {
		return colours[(64 * emphasis) + idx];
	}
}
