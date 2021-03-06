/* Virtual keyboard layout */

#include "vkb_layout.h"

const tKeySection cpc_ks[CPC_KB_SECTIONS] = {
{263,  53, 312,  85,  6},
{263,   2, 312,  51,  9},
{  8,   2, 261,  17, 15},
{  8,  19, 235,  34, 13},
{  8,  36, 240,  51, 13},
{  8,  53, 261,  68, 13},
{  8,  70, 261,  85,  4},
{242,  19, 261,  51,  1},
};

const tKeySetting *CPC_DEFAULT_KEY = &cpc_kb[0];

const tKeySetting cpc_kb[CPC_KB_KEYS] = {
{263, 53,  16, 16,    0,    0, 1,  92,  48,  48, NULL, NULL, 68,  1, 14,  3,  41},
{280, 53,  16, 16,    0,    0, 1,  80, 240, 240, NULL, NULL,  0,  2, 13,  4,  41},
{297, 53,  16, 16,    0,    0, 1, 102,  46,  46, NULL, NULL,  1,  2, 12,  5,  41},
{263, 70,  16, 16,    0,    0, 1,  78, 242, 242, NULL, NULL, 72,  4,  0,  3,  49},
{280, 70,  16, 16,    0,    0, 1,  77, 241, 241, NULL, NULL,  3,  5,  1,  4,  49},
{297, 70,  16, 16,    0,    0, 1,  79, 243, 243, NULL, NULL,  4,  5,  2,  5,  49},
{297,  2,  16, 16,    0,    0, 1, 101,  57,  57, NULL, NULL,  7,  6,  6,  9,   0},
{280,  2,  16, 16,    0,    0, 1, 100,  56,  56, NULL, NULL,  8,  6,  7, 10,   0},
{263,  2,  16, 16,    0,    0, 1,  99,  55,  55, NULL, NULL, 29,  7,  8, 11,   0},
{297, 19,  16, 16,    0,    0, 1,  98,  54,  54, NULL, NULL, 10,  9,  6, 12,   7},
{280, 19,  16, 16,    0,    0, 1,  97,  53,  53, NULL, NULL, 11,  9,  7, 13,   7},
{263, 19,  16, 16,    0,    0, 1,  96,  52,  52, NULL, NULL, 29, 10,  8, 14,   7},
{297, 36,  16, 16,    0,    0, 1,  95,  51,  51, NULL, NULL, 13, 12,  9,  2,  24},
{280, 36,  16, 16,    0,    0, 1,  94,  50,  50, NULL, NULL, 14, 12, 10,  1,  24},
{263, 36,  16, 16,    0,    0, 1,  93,  49,  49, NULL, NULL, 73, 13, 11,  0,  24},
{  8,  2,  16, 16,    0,    0, 1,  90,   0,   0, NULL, NULL, 15, 16, 15, 30,   0},
{ 25,  2,  16, 16,    0,    0, 1,   1,  49,  33, NULL, NULL, 15, 17, 16, 31,   0},
{ 42,  2,  16, 16,    0,    0, 1,   2,  50,  34, NULL, NULL, 16, 18, 17, 32,   0},
{ 59,  2,  16, 16,    0,    0, 1,   3,  51,  35, NULL, NULL, 17, 19, 18, 33,   0},
{ 76,  2,  16, 16,    0,    0, 1,   4,  52,  36, NULL, NULL, 18, 20, 19, 34,   0},
{ 93,  2,  16, 16,    0,    0, 1,   5,  53,  37, NULL, NULL, 19, 21, 20, 35,   0},
{110,  2,  16, 16,    0,    0, 1,   6,  54,  38, NULL, NULL, 20, 22, 21, 36,   0},
{127,  2,  16, 16,    0,    0, 1,   7,  55,  39, NULL, NULL, 21, 23, 22, 37,   0},
{144,  2,  16, 16,    0,    0, 1,   8,  56,  40, NULL, NULL, 22, 24, 23, 38,   0},
{161,  2,  16, 16,    0,    0, 1,   9,  57,  41, NULL, NULL, 23, 25, 24, 39,   0},
{178,  2,  16, 16,    0,    0, 1,   0,  48,  95, NULL, NULL, 24, 26, 25, 40,   0},
{195,  2,  16, 16,    0,    0, 1, 110,  45,  61, NULL, NULL, 25, 42, 26, 41,   0},
{211,  2,  16, 16,    0,    0, 1, 116,  94, 163, NULL, NULL, 25, 28, 27, 41,   0},
{229,  2,  16, 16,    0,    0, 1,  68,   0,   0, NULL, NULL, 27, 29, 28, 42,   0},
{246,  2,  16, 16,    0,    0, 1,  86,   0,   0, NULL, NULL, 28,  8, 29, 73,   0},
{  8, 19,  24, 16,    0,    0, 1, 127,   0,   0, NULL, NULL, 30, 31, 15, 43,   7},
{ 33, 19,  16, 16,    0,    0, 1,  52, 113,  81, NULL, NULL, 30, 32, 16, 44,   7},
{ 50, 19,  16, 16,    0,    0, 1,  58, 119,  87, NULL, NULL, 31, 33, 17, 45,   7},
{ 67, 19,  16, 16,    0,    0, 1,  40, 101,  69, NULL, NULL, 32, 34, 18, 46,   7},
{ 84, 19,  16, 16,    0,    0, 1,  53, 114,  82, NULL, NULL, 33, 35, 19, 47,   7},
{101, 19,  16, 16,    0,    0, 1,  55, 116,  84, NULL, NULL, 34, 36, 20, 48,   7},
{118, 19,  16, 16,    0,    0, 1,  60, 121,  89, NULL, NULL, 35, 37, 21, 49,   7},
{135, 19,  16, 16,    0,    0, 1,  56, 117,  85, NULL, NULL, 36, 38, 22, 50,   7},
{152, 19,  16, 16,    0,    0, 1,  44, 105,  73, NULL, NULL, 37, 39, 23, 51,   7},
{169, 19,  16, 16,    0,    0, 1,  50, 111,  79, NULL, NULL, 38, 40, 24, 52,   7},
{186, 19,  16, 16,    0,    0, 1,  51, 112,  80, NULL, NULL, 39, 41, 25, 53,   7},
{203, 19,  16, 16,    0,    0, 1,  64,  64, 124, NULL, NULL, 40, 42, 26, 54,   7},
{220, 19,  16, 16,    0,    0, 1, 105,  91, 123, NULL, NULL, 41, 73, 27, 55,   7},
{  8, 36,  29, 16,    0,    0, 1,  67,   0,   0, NULL, NULL, 43, 44, 30, 56,  24},
{ 38, 36,  16, 16,    0,    0, 1,  36,  97,  65, NULL, NULL, 43, 45, 31, 57,  24},
{ 55, 36,  16, 16,    0,    0, 1,  54, 115,  83, NULL, NULL, 44, 46, 32, 58,  24},
{ 72, 36,  16, 16,    0,    0, 1,  39, 100,  68, NULL, NULL, 45, 47, 33, 59,  24},
{ 89, 36,  16, 16,    0,    0, 1,  41, 102,  70, NULL, NULL, 46, 48, 34, 60,  24},
{106, 36,  16, 16,    0,    0, 1,  42, 103,  71, NULL, NULL, 47, 49, 35, 61,  24},
{123, 36,  16, 16,    0,    0, 1,  43, 104,  72, NULL, NULL, 48, 50, 36, 62,  24},
{140, 36,  16, 16,    0,    0, 1,  45, 106,  74, NULL, NULL, 49, 51, 37, 63,  24},
{157, 36,  16, 16,    0,    0, 1,  46, 107,  75, NULL, NULL, 50, 52, 38, 64,  24},
{174, 36,  16, 16,    0,    0, 1,  47, 108,  76, NULL, NULL, 51, 53, 39, 65,  24},
{191, 36,  16, 16,    0,    0, 1,  69,  58,  42, NULL, NULL, 52, 54, 40, 66,  24},
{208, 36,  16, 16,    0,    0, 1, 124,  59,  43, NULL, NULL, 53, 55, 41, 67,  24},
{225, 36,  16, 16,    0,    0, 1, 119,  93, 125, NULL, NULL, 54, 73, 42, 67,  24},
{  8, 53,  37, 16,    0, 1200, 2, 109,   0,   0, NULL, NULL, 56, 57, 43, 69,  41},
{ 46, 53,  16, 16,    0,    0, 1,  61, 122,  90, NULL, NULL, 56, 58, 44, 70,  41},
{ 63, 53,  16, 16,    0,    0, 1,  59, 120,  88, NULL, NULL, 57, 59, 45, 70,  41},
{ 80, 53,  16, 16,    0,    0, 1,  38,  99,  67, NULL, NULL, 58, 60, 46, 70,  41},
{ 97, 53,  16, 16,    0,    0, 1,  57, 118,  86, NULL, NULL, 59, 61, 47, 71,  41},
{114, 53,  16, 16,    0,    0, 1,  37,  98,  66, NULL, NULL, 60, 62, 48, 71,  41},
{131, 53,  16, 16,    0,    0, 1,  49, 110,  78, NULL, NULL, 61, 63, 49, 71,  41},
{148, 53,  16, 16,    0,    0, 1,  48, 109,  77, NULL, NULL, 62, 64, 50, 71,  41},
{165, 53,  16, 16,    0,    0, 1,  70,  44,  60, NULL, NULL, 63, 65, 51, 71,  41},
{182, 53,  16, 16,    0,    0, 1, 112,  46,  62, NULL, NULL, 64, 66, 52, 72,  41},
{199, 53,  16, 16,    0,    0, 1, 125,  47,  63, NULL, NULL, 65, 67, 53, 72,  41},
{216, 53,  16, 16,    0,    0, 1,  66,  92,  96, NULL, NULL, 66, 68, 54, 72,  41},
{233, 53,  29, 16,    0, 1203, 2, 123,   0,   0, NULL, NULL, 67,  0, 55, 72,  41},
{  8, 70,  37, 16,    0, 1200, 2,  71,   0,   0, NULL, NULL, 69, 70, 56, 69,  49},
{ 46, 70,  26, 16,    0,    0, 1,  72,   0,   0, NULL, NULL, 69, 71, 57, 70,  49},
{ 73, 70, 137, 16,    0,    0, 1, 126,   0,   0, NULL, NULL, 70, 72, 62, 71,  49},
{211, 70,  51, 16,    0,    0, 1,  88,   0,   0, NULL, NULL, 71,  3, 68, 72,  49},
{242, 19,  20, 33,    0,    0, 1, 121,   0,   0, NULL, NULL, 42, 14, 29, 68,  15},
};
