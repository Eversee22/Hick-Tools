#pragma once
#include"hfc.h"
#include<stdio.h>
#include<string>
//#include<fstream>

#define DEBUG 0
#define MASK 0xF0F0

long read_fsize;
std::string org_fname;

char* readf(const char* filename, unsigned int weights[]) {
	FILE* pFile;
	//unsigned fsize;
	char* buffer;
	size_t n;
	errno_t err;

	for (int i = 0; i < 256; ++i)
		weights[i] = 0;

	err = fopen_s(&pFile, filename, "rb");
	if (err) {
		printf("open file %s failed\n", filename);
		exit(0);
	}
	fseek(pFile, 0, SEEK_END);
	read_fsize = ftell(pFile);
	rewind(pFile);

	if(DEBUG)
		printf("file size:%d bytes\n", read_fsize);

	buffer = (char*)malloc(sizeof(char)*read_fsize);
	if (buffer == NULL) {
		printf("memory error\n");
		exit(1);
	}

	n = fread(buffer, 1, read_fsize, pFile);
	if (n != read_fsize) {
		printf("reading error\n");
		exit(2);
	}
	
	for (unsigned i = 0; i < read_fsize; ++i) {
		unsigned char c = buffer[i];
		weights[c] += 1;
	}
	fclose(pFile);

	return buffer;
}

//int compare(const void * a, const void * b) {
//	MWP w1 = (MWP)a;
//	MWP w2 = (MWP)b;
//
//	return (w1->freq - w2->freq);
//}

void coding(const char* filename, char* buffer, HuffmanTree ht, HuffmanCode hc,short int n) {
	FILE* pF;
	errno_t err;

	err = fopen_s(&pF, filename, "wb");
	if (err) {
		printf("open file %s error\n", filename);
		exit(0);
	}

	int m = 2 * n - 1;
	unsigned char byc = 0;
	unsigned char zero = 0;
	unsigned char r = 0;
	unsigned int countbytes = 0;
	unsigned int shidsize = 0;

	//size_t found = org_fname.find_last_of(".");
	//std::string fmt = org_fname.substr(found + 1);
	char FLAG[4] = { 'G','H','L','L' };
	for (int i = 0; i < 4; ++i) {
		FLAG[i] = FLAG[i] ^ MASK;
	}
	fwrite(FLAG, sizeof(char), 4, pF);
	unsigned char fmtlen = org_fname.size();
	fwrite(&fmtlen, sizeof(char), 1, pF);
	char* fname = (char*)malloc(sizeof(char)*(fmtlen + 1));
	const char* orgfname = org_fname.c_str();
	for (int i = 0; i < fmtlen; ++i) {
		fname[i] = orgfname[i] ^ MASK;
	}
	fname[fmtlen] = '\0';
	fwrite(fname, sizeof(char),fmtlen,pF);
	fwrite(&n, sizeof(short int), 1, pF);
	fwrite(ht + 1, sizeof(HTNODE), m, pF);
	free(fname);

	shidsize += 4 + sizeof(fmtlen)+fmtlen+sizeof(short int) + m * sizeof(HTNODE);
	
	char coding[256];
	for (int i = 0; i < read_fsize; ++i) {
		unsigned char c = buffer[i];

		int j;
		for (j = 1; j <= n; ++j) {
			if (ht[j].weight.val == c) {
				break;
			}
				
		}

		size_t len = strlen(hc[j]);
		strcpy_s(coding, hc[j]);

		if (zero) {
			if (len >= zero) {
				
				for (int z = 0; z < zero; ++z) {
					byc = byc * 2 + (coding[z] - '0');
				}
				fwrite(&byc, sizeof(char), 1, pF);
				if (DEBUG)
					printf("%u ", byc);
				byc = 0;

				for (int k = zero; k < len; ++k)
					coding[k - zero] = coding[k];
				len -= zero;
				coding[len] = '\0';

				zero = 0;
				countbytes += 1;
			}
			else {
				zero -= len;
				for (int z = 0; z < len; ++z) {
					byc = byc * 2 + (coding[z] - '0');
				}
				continue;
			}
		}
		/*if (DEBUG)
			printf("coding:%s\n", coding);*/
		r = len % 8;
		//char* bycs = (char*)malloc(sizeof(char)*(len / 8 + (r == 0 ? 0 : 1)));
		int k;
		for (k = 0; k < len / 8; ++k) {
			for (int z = 0; z < 8; ++z) {
				byc = byc * 2 + (coding[k * 8 + z] - '0');
			}
			fwrite(&byc, sizeof(char), 1, pF);
			countbytes += 1;
			if (DEBUG)
				printf("%u ", byc);

			byc = 0;
		}
		for (int z = 0; z < r; ++z) {
			byc = byc * 2 + (coding[k * 8 + z] - '0');
			zero = 8 - r;
		}
	}

	if (zero||r) {
		fwrite(&byc, sizeof(char), 1, pF);
		countbytes += 1;
		if (DEBUG)
			printf("%u\n", byc);
	}
	if (DEBUG)
		printf("countbytes:%d,zero:%d,r:%d\n", countbytes, zero, r);

	fwrite(&zero, sizeof(char), 1, pF);
	fwrite(&r, sizeof(char), 1, pF);
	fwrite(&countbytes, sizeof(unsigned int), 1, pF);

	shidsize += countbytes + sizeof(char) * 2 + sizeof(unsigned int);
	if (DEBUG)
		printf("shide size:%u\n", shidsize);

	fclose(pF);
}

void decoding(const char* filename) {
	FILE* pF;
	errno_t err;

	err = fopen_s(&pF, filename, "rb");
	if (err) {
		printf("open file %s error\n", filename);
		exit(0);
	}

	
	short int n;
	unsigned char fmtlen;
	char* fmt;
	char FLAG[5];
	fread(FLAG, sizeof(char), 4, pF);
	for (int i = 0; i < 4; ++i) {
		FLAG[i] = FLAG[i] ^ MASK;
	}
	FLAG[4] = '\0';
	if (strcmp(FLAG, "GHLL") != 0) {
		printf("not support such format\n");
		exit(3);
	}
	fread(&fmtlen, sizeof(char), 1, pF);
	fmt = (char*)malloc(sizeof(char)*(fmtlen + 1));
	fread(fmt, sizeof(char), fmtlen, pF);
	for (int i = 0; i < fmtlen; ++i) {
		fmt[i] = fmt[i] ^ MASK;
	}
	fmt[fmtlen] = '\0';
	fread(&n, sizeof(short int), 1, pF);
	int m = 2 * n - 1;
	HuffmanTree ht = (HuffmanTree)malloc((m+1) * sizeof(HTNODE));
	fread(ht + 1, sizeof(HTNODE), m, pF);
	if (DEBUG)
		printf("var:%d\n", n);

	long headsize = 4 + sizeof(fmtlen) + fmtlen + sizeof(short int) + m * sizeof(HTNODE);

	int rearn = sizeof(char) * 2 + sizeof(unsigned int);
	fseek(pF, -rearn, SEEK_END);
	unsigned char r;
	unsigned char zero;
	unsigned int bytesn;
	fread(&zero, sizeof(char), 1, pF);
	fread(&r, sizeof(char), 1, pF);
	fread(&bytesn, sizeof(unsigned int), 1, pF);

	if (DEBUG) {
		printf("bytesn:%d,zero:%d,r:%d\n", bytesn, zero, r);
	}

	//long rsize = fsize - sizeof(short int) - htsize;
	char* buffer = (char*)malloc(bytesn*sizeof(char));
	if (buffer == NULL) {
		printf("memory error\n");
		exit(1);
	}

	fseek(pF, headsize, SEEK_SET);
	size_t results = fread(buffer, sizeof(char), bytesn, pF);

	if (results != bytesn) {
		printf("reading error\n");
		exit(2);
	}

	if (DEBUG) {
		for (int i = 0; i < bytesn; ++i) {
			printf("%u ", (unsigned char)buffer[i]);
		}
		printf("\n");
	}
	FILE* depF;
	std::string defn = std::string("outshid");
	err = fopen_s(&depF,(defn+"."+fmt).c_str(),"wb");
	free(fmt);
	if (err) {
		printf("open file %s error\n", filename);
		exit(0);
	}
	
	int p;
	unsigned char c;
	unsigned char result;
	unsigned char val;
	p = m;
	for (int i = 0; i < bytesn - 1; ++i) {
		c = buffer[i];

		for (int z = 0; z < 8; ++z) {
			result = c & 0x80;
			if (DEBUG)
				printf("result:%u ",result);

			if (result == 0)
				p = ht[p].lchild;
			else
				p = ht[p].rchild;

			if (ht[p].lchild == 0) {
				val = ht[p].weight.val;
				fwrite(&val, sizeof(char), 1, depF);
				p = m;
			}

			c <<= 1;
		}
		if (DEBUG)
			printf("\n");
	}
	c = buffer[bytesn - 1];
	int bitsn = 8;
	if (zero) {
		c <<= zero;
		bitsn = 8 - zero;
	}
	else if (r) {
		c <<= (8 - r);
		bitsn = r;
	}
	for (int z = 0; z < bitsn; ++z) {
		result = c & 0x80;
		if (DEBUG)
			printf("result:%u ", result);

		if (result == 0)
			p = ht[p].lchild;
		else
			p = ht[p].rchild;

		if (ht[p].lchild == 0) {
			val = ht[p].weight.val;
			fwrite(&val, sizeof(char), 1, depF);
			p = m;
		}

		c <<= 1;
	}
	if (DEBUG)
		printf("\n");

	//if (p != 0 && ht[p].lchild == 0) {
	//	val = ht[p].weight.val;
	//	fwrite(&val, sizeof(char), 1, depF);
	//	p = m;
	//}

	free(buffer);
	free(ht);
	fclose(pF);
	fclose(depF);

}

void bytesCoding(const char* filename) {
	unsigned int w[256];
	MWeights weights[256];

	org_fname = std::string(filename);

	char* read_buffer = readf(filename, w);
	/*if (DEBUG) {
		for (int i = 0; i < read_fsize; ++i) {
			printf("%c", read_buffer[i]);
		}
		printf("\n");
		for (int i = 0; i < read_fsize; ++i) {
			printf("%u ", (unsigned char)read_buffer[i]);
		}
		printf("\n");
	}*/
	for (int i = 0; i < 256; ++i) {
		weights[i].val = i;
		weights[i].freq = w[i];
	}

	int count = 0;
	for (int i = 0; i < 256; ++i) {
		if (weights[i].freq == 0)
			count += 1;
		else {
			weights[i - count] = weights[i];
		}
	}

	if (DEBUG) {
		unsigned int t = 0;
		for (int i = 0; i < 256 - count; ++i) {
			printf("v%d:w%u ", weights[i].val, weights[i].freq);
			t += weights[i].freq;
		}
		printf("total:%u\n", t);
	}

	HuffmanTree hft = NULL;
	HuffmanCode hc = NULL;
	int n = 256 - count;

	/*qsort(weights, n, sizeof(MWeights), compare);
	for (int i = 0; i < n; ++i) {
		printf("%d:%u ", weights[i].val,weights[i].freq);
	}
	printf("\n");*/

	huffmanCoding(hft, hc, weights, n);
	if (DEBUG) {
		for (int i = 1; i <= n; ++i) {
			printf("v%u:w%u:c%s\n", hft[i].weight.val, hft[i].weight.freq, hc[i]);
		}
	}
	std::string fname(filename);
	fname += ".shid";
	coding(fname.c_str(), read_buffer, hft, hc, n);
	//decoding(fname.c_str());
	free(read_buffer);
	
	if (hft)
		free(hft);
	if (hc) {
		for (int i = 1; i < n + 1; ++i) {
			free(hc[i]);
		}
		free(hc);
	}
	

}