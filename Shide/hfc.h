#ifndef HFC_H
#define HFC_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
	unsigned char val;
	unsigned int freq;
}MWeights,* MWP;

typedef struct {
	MWeights weight;
	short int parent;
	short int lchild;
	short int rchild;
}HTNODE,* HuffmanTree;

typedef char** HuffmanCode;

//select 2 nodes with smallest weight and 0 parent in HT[1..n]
void select(HuffmanTree &HT, int n, int &s1, int &s2) {
	s1 = 0;
	s2 = 0;
	for (int i = 1; i <= n; ++i) {
		if (HT[i].parent == 0) {
			if (s1 == 0)
				s1 = i;
			else if (s2 == 0)
				s2 = i;
			else if (HT[i].weight.freq < HT[s1].weight.freq)
				s1 = i;
			else if (HT[i].weight.freq < HT[s2].weight.freq)
				s2 = i;
		}
	}
}

void huffmanCoding(HuffmanTree &HT, HuffmanCode &HC, MWP w, int n) {
	//w is the pointer to weights of n characters, construct HuffmanTree HT, get code HC
	if (n <= 1)
		exit(1);

	int m = n * 2 - 1;

	HT = (HuffmanTree)malloc((m + 1) * sizeof(HTNODE)); //not using 0
	if (HT == NULL) {
		printf("HuffmanTree memory error\n");
		exit(-1);
	}

	int j;
	HuffmanTree p;
	for (p = HT + 1, j = 1; j <= n; ++j, ++p, ++w)
		*p = { *w,0,0,0 };
	for (; j <= m; ++j, ++p)
		*p = { 0,0,0,0 };

	for (int i = n + 1; i <= m; ++i) {
		int s1, s2;
		select(HT, i - 1, s1, s2);
		//printf("s1:%u,s2:%u\n", HT[s1].weight.freq, HT[s2].weight.freq);
		HT[s1].parent = i;
		HT[s2].parent = i;
		HT[i].lchild = s1;
		HT[i].rchild = s2;
		HT[i].weight.freq = HT[s1].weight.freq + HT[s2].weight.freq;
	}
	//printf("root:%u\n", HT[m].weight.freq);

	HC = (HuffmanCode)malloc((n + 1) * sizeof(char*));
	if (HC == NULL) {
		printf("HuffmanCode memory error\n");
		exit(-1);
	}

	char* cd = (char*)malloc(n * sizeof(char));
	if (cd == NULL) {
		printf("coding memory error\n");
		exit(-1);
	}

	cd[n - 1] = '\0';
	for (int i = 1; i <= n; ++i) {
		int start = n - 1;
		short int f = HT[i].parent;
		for (int c = i; f != 0; c = f, f = HT[f].parent) {
			if (HT[f].lchild == c)
				cd[--start] = '0';
			else
				cd[--start] = '1';
		}
		HC[i] = (char*)malloc((n - start) * sizeof(char));
		if (HC[i] == NULL) {
			printf("HC[%d] memory error\n", i);
			exit(-1);
		}
        STRCPY(HC[i], &cd[start]);
	}
	free(cd);
}

#endif