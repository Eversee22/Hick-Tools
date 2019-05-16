#include"cf.h"
#include<iostream>

#define MAXPL 256
#define PARAMNUM 3

typedef union {
	char name[MAXPL];
	int num;
}Param;

std::string options[PARAMNUM][2] = {
	"-c","encode",
	"-d","decode"
	"  ","filename"
};

enum OPTIONS{
	ENCODE,
	DECODE,
	FNAME
};

//void printsizeoftypes() {
//	printf("size of short int:%u\n", sizeof(short int));
//	printf("size of unsigned int:%u\n", sizeof(unsigned int));
//	printf("size of unsigned char:%u\n", sizeof(unsigned char));
//	printf("size of HTNode:%u\n", sizeof(HTNODE));
//	printf("size of MWeights:%u\n", sizeof(MWeights));
//}

void printusage() {
	std::cout << "usage: .. [options] filename" << std::endl;
	std::cout << "\n[Options]" << std::endl;
	for (int i = 0; i < PARAMNUM; ++i) {
		std::cout << options[i][0] << "  " << options[i][1] << std::endl;
	}
}

void argparser(int argc, char** argv, Param* params) {
	//char name[MAXPL];
	const char* arg;
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			//strcpy_s(name, &argv[i][1]);
			arg = &argv[i][1];
			if (strcmp(arg, "c") == 0) {
				params[ENCODE].num = 1;
			}
			else if (strcmp(arg, "d") == 0) {
				params[DECODE].num = 1;
			}
		}
		else {
			strcpy_s(params[FNAME].name, argv[i]);
		}
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		printusage();
		exit(0);
	}
	Param params[PARAMNUM];
	memset(params, 0, sizeof(Param)*PARAMNUM);

	argparser(argc, argv,params);
	if (params[FNAME].name[0] == '\0') {
		printf("please specify filename\n");
		exit(1);
	}
	std::string fname = std::string(params[FNAME].name);
	if (params[ENCODE].num == 1) {
		bytesCoding(fname.c_str());
		printf("encodes over\n");
	}else if (params[DECODE].num == 1) {
		decoding(fname.c_str());
		printf("decodes over\n");
	}
	else {
		bytesCoding(fname.c_str());
		printf("encode over\n");
	}
	//printsizeoftypes();
}