/*
 * 郑翔 202010 将txt文本中的诗句转成密码
 * 编译：cc -o txt2pw txt2pw.c
 * 用法：./txt2pw 诗词.txt
 */

#include "build_passwd.c"

int main(int argc, char **argv)
{
	int linenum = 1;
	char line[1024] = {0};
	FILE *fp = NULL;

	if (argc != 2) {
		printf("Usage: %s filename\n", argv[0]);
		exit(1);
	}

	fp = fopen(argv[1], "r");
	if (!fp) {
		printf("open %s fail: %s\n", argv[1], strerror(errno));
		exit(1);
	}

	while (fgets(line, 1024, fp)) {
		int i = 0, j = 0;
		char c = 0, buf[64] = {0};

		for (i = 0; i < 63; i++) {
			c = line[i];
			if ((c & 0xFB) == 0xFB || (c & 0xF8) == 0xF8 || (c & 0xF0) == 0xF0) {
				goto out;
			}
			if ((c & 0xE0) == 0xE0) { /* 11100000，3字节字符，这是汉字的unicode编码 */
				if (i+2 >= 63) {
					break;
				}
				buf[j]   = line[i];
				buf[j+1] = line[i+1];
				buf[j+2] = line[i+2];
				j += 3;
				i += 2;
				continue;
			}
			if ((c & 0xB0) == 0xB0) {
				goto out;
			}
			if ((c & 0x80) == 0) { /* 00000000，单字节字符，即半角符号 */
				if (c == '\r' || c == '\n' || c == 0) {
					break;
				}
				if (isprint(c) && !isspace(c)) {
					buf[j] = c;
					j++;
				}
				continue;
			}
out:
			printf("line %d not UTF-8 Unicode text\n", linenum);
			fclose(fp);
			exit(1);
		}
		linenum++;

		if (j < 15) {
			continue;
		}

		poem2passwd(buf);
		printf("%s\n", buf);
		printf("%s", info);
		printf("%s\n", passwd);

		printf("\n");
	}
	fclose(fp);
}
