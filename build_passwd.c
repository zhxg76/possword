/*
 * 郑翔 202010 将诗词转成密码
 * 
 * 转换步骤：
 * 1、诗句转拼音
 * 2、处理特殊拼音，转成特殊字符
 * 3、用拼音首字母和特殊字符组成密码
 * 4、密码长度不够8字节，部分拼音用全拼
 * 5、没有数字，尾部添0
 * 6、没有特殊字符，尾部添#
 */

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "unicode_pinyin.h"
#include "special_pinyin.h"

#define HANZINUM 64
int pinyincount = 0;
int size = HANZINUM * 8;
char special_hanzi[HANZINUM][8] = {{0}};
char pinyin[HANZINUM][8] = {{0}};
char pinyin_str[HANZINUM][8] = {{0}};
char passwd_str[HANZINUM][8] = {{0}};
char passwd[1024] = {0};
char remark[2048] = {0};
char info[4096] = {0};

void poem2pinyin(char *poem)
{
	int i = 0, unicode_value = 0, pinyin_idx = 0;
	unsigned char c = 0, c2 = 0, c3 = 0;
	char *ptr = NULL;

	/* utf8转unicode */
	ptr = poem;
	while (*ptr != 0 && i < HANZINUM) {
		c = *ptr;
		if ((c & 0xFB) == 0xFB) { /* 11111100，6字节字符 */
			ptr += 6;
			continue;
		}
		if ((c & 0xF8) == 0xF8) { /* 11111000，5字节字符 */
			ptr += 5;
			continue;
		}
		if ((c & 0xF0) == 0xF0) { /* 11110000，4字节字符 */
			ptr += 4;
			continue;
		}

		if ((c & 0xE0) == 0xE0) { /* 11100000，3字节字符，这是汉字的unicode编码 */
			c2 = *(ptr+1);
			c3 = *(ptr+2);
			unicode_value = (((int)(c & 0x0F)) << 12) | (((int)(c2 & 0x3F)) << 6) | (c3 & 0x3F);

			if (unicode_value == 12288) { /* 空格 */
				pinyin[i][0] = ' ';
				pinyin[i][1] = ' ';
			} else if (unicode_value >= 65281 && unicode_value <= 65374) { /* 全角符号 */
				pinyin[i][0] = unicode_value - 65281 + 33;
				pinyin[i][1] = ' ';
			} else if (unicode_value >= 19968 && unicode_value <= 40869) { /* 20902个汉字 */
				pinyin_idx = unicode_value - 19968;
				strncpy(pinyin[i], PINYIN[pinyin_idx], 7);

				/* 使得能对齐显示转换规则：wan->万->10000 */
				if (strcmp(pinyin[i], "wan") == 0) {
					/* 十万、百万、千万，万不转，0太多不好记 */
					if (i == 0) {
						pinyin[i][3] = ' ';
						pinyin[i][4] = ' ';
					} else if (strcmp(pinyin[i-1], "shi") != 0 &&
						   strcmp(pinyin[i-1], "bai") != 0 &&
						   strcmp(pinyin[i-1], "qian") != 0) {
						pinyin[i][3] = ' ';
						pinyin[i][4] = ' ';
					}
				}
			} else {
				pinyin[i][0] = '?'; /* 不认识的字 */
			}

			ptr += 3;
			i++;
			continue;
		}

		if ((c & 0xB0) == 0xB0) { /* 11000000，2字节字符 */
			ptr += 2;
			continue;
		}

		if ((c & 0x80) == 0) { /* 00000000，单字节字符，即半角符号 */
			pinyin[i][0] = c;
			ptr += 1;
			i++;
			continue;
		}

		ptr++; /* 不能走到这里。10000000，不可能是字符的首字节，只可能是后续字节 */
	}
	pinyincount = i;

	for (i = 0; i < pinyincount-1; i++) {
		strncat(info, pinyin[i], 8);
		strcat(info, " ");
	}
	strncat(info, pinyin[i], 8);
	strcat(info, "\n");
}

void handle_special_pinyin(void)
{
	int i = 0, j = 0, len = 0;
	int hanzilen = strlen("十");

	for (i = 0; i < pinyincount; i++) {
		len = strlen(pinyin[i]);
		memset(special_hanzi[i], ' ', len);
		memset(pinyin_str[i], ' ', len);

		if (strcmp(pinyin[i], "shi") == 0) {
			memcpy(special_hanzi[i], "十", hanzilen);
			special_hanzi[i][len-3+hanzilen] = ' ';
			if (i && pinyin_str[i-1][0] >= '1' && pinyin_str[i-1][0] <= '9' && pinyin_str[i-1][1] != '0') {
				memcpy(pinyin_str[i], "0", 1);
			} else {
				memcpy(pinyin_str[i], "10", 2);
			}
			continue;
		}
		if (strcmp(pinyin[i], "bai") == 0) {
			memcpy(special_hanzi[i], "百", hanzilen);
			special_hanzi[i][len-3+hanzilen] = ' ';
			if (i && pinyin_str[i-1][0] >= '1' && pinyin_str[i-1][0] <= '9' && pinyin_str[i-1][1] != '0') {
				memcpy(pinyin_str[i], "00", 2);
			} else {
				memcpy(pinyin_str[i], "100", 3);
			}
			continue;
		}
		if (strcmp(pinyin[i], "qian") == 0) {
			memcpy(special_hanzi[i], "千", hanzilen);
			special_hanzi[i][len-3+hanzilen] = ' ';
			if (i && pinyin_str[i-1][0] >= '1' && pinyin_str[i-1][0] <= '9' && pinyin_str[i-1][1] != '0') {
				memcpy(pinyin_str[i], "000", 3);
			} else {
				memcpy(pinyin_str[i], "1000", 4);
			}
			continue;
		}
		if (strcmp(pinyin[i], "wan  ") == 0) {
			memcpy(special_hanzi[i], "万", hanzilen);
			special_hanzi[i][len-3+hanzilen] = ' ';
			if (i && pinyin_str[i-1][0] >= '1' && pinyin_str[i-1][0] <= '9' && pinyin_str[i-1][1] != '0') {
				memcpy(pinyin_str[i], "0000", 4);
			} else {
				memcpy(pinyin_str[i], "10000", 5);
			}
			continue;
		}

		j = 0;
		while (special_pinyin[j].pinyin[0]) {
			if (strcmp(special_pinyin[j].pinyin, pinyin[i]) == 0) {
				int speclen = strlen(special_pinyin[j].hanzi);

				memcpy(special_hanzi[i], special_pinyin[j].hanzi, speclen);
				if (speclen == hanzilen) {
					if (len > 2) {
						special_hanzi[i][len-3+hanzilen] = ' ';
					}
				} else {
				}
				pinyin_str[i][0] = special_pinyin[j].c;
				break;
			}
			j++;
		}

		if (special_pinyin[j].pinyin[0] == 0) {
			memcpy(special_hanzi[i], pinyin[i], strlen(pinyin[i]));
			pinyin_str[i][0] = pinyin[i][0];
		}
	}

	for (i = 0; i < pinyincount-1; i++) {
		strncat(info, special_hanzi[i], 8);
		strcat(info, " ");
	}
	strncat(info, special_hanzi[i], 8);
	strcat(info, "\n");

	for (i = 0; i < pinyincount-1; i++) {
		strncat(info, pinyin_str[i], 8);
		strcat(info, " ");
	}
	strncat(info, pinyin_str[i], 8);
	strcat(info, "\n");
}

static int expand_pwdlen(int pwdlen, int len)
{
	int i = 0, j = 0, pinyin_len = 0;

	for (i = 0; i < pinyincount; i++) {
		pinyin_len = strlen(pinyin[i]);
		if (islower(passwd_str[i][0]) && pinyin_len == len) {
			strncpy(pinyin_str[i], pinyin[i], 7);
			strncpy(passwd_str[i], pinyin[i], 7);

			for (j = 0; j < i; j++) {
				strncat(info, pinyin_str[j], 8);
				strcat(info, " ");
			}
			strncat(info, pinyin_str[j], 8);
			strcat(info, "用全拼，以补足8字节\n");

			pwdlen += len - 1;
			if (pwdlen >= 8) {
				return pwdlen;
			}
		}
	}
	return pwdlen;
}

void build_passwd(void)
{
	char *ptr = NULL;
	int i = 0, len = 0, pwdlen = 0, min_pinyin_len = 0;
	int digit = 0, lower = 0, upper = 0, special = 0;

	for (i = 0; i < pinyincount; i++) {
		strncpy(passwd_str[i], pinyin_str[i], 7);
		ptr = strchr(&passwd_str[i][1], ' ');
		if (ptr) {
			*ptr = 0;
		}
		pwdlen += strlen(passwd_str[i]);

		if (isdigit(passwd_str[i][0])) {
			digit++;
		} else if (islower(passwd_str[i][0])) {
			lower++;
		} else if (isupper(passwd_str[i][0])) {
			upper++;
		} else {
			special++;
		}
	}

	/* 如果不足8位，将字母扩展成全拼，扩展到不少于8位 */
	min_pinyin_len = 9 - pwdlen;
	if (pwdlen < 8) {
		for (len = min_pinyin_len; len <= 6; len++) {
			pwdlen = expand_pwdlen(pwdlen, len);
			if (pwdlen >= 8) {
				break;
			}
		}
	}

	/* 不能用一个全拼补足8位，用多个全拼补 */
	if (pwdlen < 8) {
		for (len = 2; len < min_pinyin_len; len++) {
			pwdlen = expand_pwdlen(pwdlen, len);
			if (pwdlen >= 8) {
				break;
			}
		}
	}


	/* 如果没有大写字符，将最后一个首字母大写 */
	if (upper == 0) {
		for (i = pinyincount-1; i >=0 ; i--) {
			if (islower(passwd_str[i][0])) {
				passwd_str[i][0] += 'A' - 'a';
				pinyin_str[i][0] += 'A' - 'a';
				break;
			}
		}
		for (i = 0; i < pinyincount-1; i++) {
			strncat(info, pinyin_str[i], 8);
			strcat(info, " ");
		}
		strncat(info, pinyin_str[i], 8);
		strcat(info, "\n");
	}

	for (i = 0; i < pinyincount; i++) {
		strcat(passwd, passwd_str[i]);
	}
	if (digit == 0) {
		for (i = 0; i < pinyincount; i++) {
			strncat(info, pinyin_str[i], 8);
			strcat(info, " ");
		}
		strncat(info, pinyin_str[i], 8);
		strcat(info, " 没有数字，结尾补0\n");
		strcat(passwd, "0");
	}
	if (special == 0) {
		for (i = 0; i < pinyincount; i++) {
			strncat(info, pinyin_str[i], 8);
			strcat(info, " ");
		}
		strncat(info, pinyin_str[i], 8);
		strcat(info, " 没有特殊字符，结尾补#\n");
		strcat(passwd, "#");
	}

	snprintf(remark, 2048, "可以修改密码的字符或添加个人属性，如%s@zx76", passwd);
}

void poem2passwd(char *poem)
{
	memset(special_hanzi, 0, size);
	memset(pinyin, 0, size);
	memset(pinyin_str, 0, size);
	memset(passwd_str, 0, size);
	memset(passwd, 0, 1024);
	memset(remark, 0, 2048);
	memset(info, 0, 4096);

	poem2pinyin(poem);
	handle_special_pinyin();
	build_passwd();
}
