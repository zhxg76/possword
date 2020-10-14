#ifndef __SPECIAL_PINYIN_H
#define __SPECIAL_PINYIN_H

struct special_pinyin {
	char pinyin[8];
	char c;
	char hanzi[8];
} special_pinyin[] = {
	{"yi",    '1', "一"},
	{"er",    '2', "二"},
	{"liang", '2', "两"},
	{"san",   '3', "三"},
	{"si",    '4', "四"},
	{"wu",    '5', "五"},
	{"liu",   '6', "六"},
	{"qi",    '7', "七"},
	{"ba",    '8', "八"},
	{"jiu",   '9', "九"},
	{"ling",  '0', "零"},
	{"xing",  '*', "星"},
	{"jia",   '+', "加"},
	{"jian",  '-', "减"},
	{"cheng", '*', "乘"},
	{"chu",   '/', "除"},
	{"deng",  '=', "等"},
	{"dian",  '.', "点"},
	{"wen",   '?', "问"},
	{"heng",  '-', "横"},
	{"shu",   '|', "竖"},
	{"zhi",   '|', "直"},
	{"xie",   '/', "斜"},
	{"bo",    '~', "波"},
	{"jing",  '#', "井"},
	{"jiao",  '<', "角"},
	{"kuang", '[', "框"},
	{"he",    '&', "和"},
	{"yu",    '&', "与"},
	{"qie",   '&', "且"},
	{"zai",   '@', "在"},
	{"quan",  'O', "圈"},
	{"huan",  'O', "环"},
	{"yuan",  'O', "圆"},
	{"kong",  ' ', "空"},
	{"you",   'U', "u"},
	{"wai",   'Y', "y"},
	{"pi",    'P', "p"},
	{"ge",    'G', "g"},
	{"te",    'T', "t"},
	{"",      ' ', ""}
};

#endif
