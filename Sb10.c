/*Sb10.c*/
/* s17011 2J 板橋竜太 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_FNAME 256
#define MAX_PIXEL 256

#define ONDEBUG

/*
　ヘッダは必ず存在するものとする。
　ファイル中のコメントは独立したコメント行になっている。
　コメント行は，あるとしたらヘッダ内部にあり，複数行あってもよい
　マジックナンバーは独立した行に行頭から記述されている。
　画像サイズの高さと幅の値は独立した行に記述されている。
　P2形式の最大輝度値は独立した行に記述されている。
　画像データ中には区切り文字があるものとする。
*/

typedef struct {
	int Magic; /*マジックナンバーの整数部のみ切り出した値　1..6*/
	int HSize; /*横画素数*/
	int VSize; /*縦画素数*/
	int MaxY;  /*最大輝度*/
} PNMHeader;

/*PBM/PGMファイル出力関数*/
int writePNMMonoFile(char fname[],PNMHeader head, unsigned char y[][MAX_PIXEL])
{
	FILE *fp;
	int i,j;
	int cnt;
	int ndpl; //number of datas per line
	if (head.Magic!=1 && head.Magic!=2) return -1;
	if (head.HSize<=0 || MAX_PIXEL<head.HSize) return -1;
	if (head.VSize<=0 || MAX_PIXEL<head.VSize) return -1;
	fp=fopen(fname,"wb");
	if (fp==NULL) return -1;
	fprintf(fp,"P%d\n",head.Magic);
	fprintf(fp,"%d %d\n",head.HSize, head.VSize);
	if (head.Magic!=1) {
		fprintf(fp,"%d\n", head.MaxY);
	}
	if (head.Magic==1) ndpl=32;
	else ndpl=16;
	cnt=0;
	for (i=0; i<head.VSize; i++) {
		for (j=0; j<head.HSize; j++) {
			fprintf(fp,"%d ", y[i][j]);
			cnt++;
			if (cnt==ndpl) {
				fprintf(fp,"\n");
				cnt=0;
			}
		}
		fprintf(fp,"\n");
		cnt=0;
	}
	fclose(fp);
	return 0;
}

PNMHeader readPNMHeader(FILE *fp){
	PNMHeader head ={-1,-1,-1,-1};
	PNMHeader errhead ={-1,-1,-1,-1};
	char buff[1024] = {'\0'};
	
	while(buff[0]=='#' || buff[0]=='\0'){
		fgets(buff, 1024, fp);
	}
	
	if(!(buff[1]-'0'==1 || buff[1]-'0'==2)){
		return errhead;
	} else {
		head.Magic=buff[1]-'0';
		buff[0] = '\0';
	}
	
	while(buff[0]=='#' || buff[0]=='\0'){
		fgets(buff, 1024, fp);
	}
	
	sscanf(buff, "%d %d", &head.HSize, &head.VSize);
	buff[0] = '\0';
	
	if(head.Magic == 2){
		while(buff[0]=='#' || buff[0]=='\0'){
			fgets(buff, 1024, fp);
		}
		sscanf(buff, "%d", &head.MaxY);
	}
	
	return head;
}



/* PBM/PGMファイル読み込み関数 */
/*   char fname[]:読み込みファイル名                       */
/*   unsigned char y[][MAX_PIXEL]:読み込まれた画像（更新） */
PNMHeader readPNMMonoFile(char fname[],unsigned char y[][MAX_PIXEL])
{
	PNMHeader head ={-1,-1,-1,-1};
	PNMHeader errhead ={-1,-1,-1,-1};
	FILE *fp;
	int buff[1024];
	int i, j, cnt, sret=0, pixel=0;
	
	fp=fopen(fname,"rb");
	if (fp==NULL) {
		return errhead;
	}
	
	head=readPNMHeader(fp);
	
	if(head.HSize<=0||head.HSize>MAX_PIXEL||head.VSize<=0||head.VSize>MAX_PIXEL) {
		return errhead;
	} else if(head.Magic==2){
		if(head.MaxY<=0||head.MaxY>255){
			return errhead;
		}
	}
	
	for(i=0;i<head.VSize&&sret!=-1;i++){
		for(j=0;j<head.HSize&&sret!=-1;j++){
			sret = fscanf(fp, "%d ", &buff[j]);
			if(sret!=-1) pixel++;
		}
		for(cnt=0;cnt<=j;cnt++){
			y[i][cnt]=buff[cnt];
		}
	}
	
	if(pixel<head.HSize*head.VSize){
		return errhead;
	}
	
	fclose(fp);
	
	return head;
}

void modifyfname(char out[],char in[])
{
	int i=0;
	int at=-1;
	strcpy(out,in);
	while(out[i]) {
		if (out[i]=='.') at=i;
		i++;
	}
	if (at!=-1) out[at]='\0';
	strcat(out,"_copy");
	if (at!=-1) strcat(out,&in[at]);
}

/*配列yに関して左上から横幅width,高さheightの範囲を見える化する関数*/
/*配列内最大値が255の場合は16割った（整数割り算）値を16進表示する*/
void showArray(unsigned char y[][MAX_PIXEL], int width, int height)
{
	int i,j;
	int max=0;
	int shift=0;
	for (i=0; i<height; i++) {
		for (j=0; j<width; j++) {
			if (max<y[i][j]) max=y[i][j];
		}
	}
	if (1<max) shift=4;
	for (i=0; i<height; i++) {
		for (j=0; j<width; j++) {
			printf("%x",y[i][j]>>shift);
		}
		printf("\n");
	}
}

int main1(char *rfname)
{
	unsigned char y[MAX_PIXEL][MAX_PIXEL];
	//char rfname[MAX_FNAME]="test.pbm";
	char wfname[MAX_FNAME]="test_copy.pbm";
	PNMHeader head;
	int res;
	//printf("filename to read  : ");
	//gets(rfname);
	modifyfname(wfname,rfname);
	//printf("filename to write : ");
	//gets(wfname);
	head=readPNMMonoFile(rfname,y);
	if (head.Magic==-1) {
		printf("file [%s] : failed to read\n",rfname);
		printf("Magic = P%d\n", head.Magic);
		printf("HSize = %d,  ", head.HSize);
		printf("VSize = %d\n", head.VSize);
		if (head.Magic==2) {
			printf("MaxY = %d\n", head.MaxY);
		}
		return 1;
	}
	printf("file [%s] : completed to read\n",rfname);
	printf("Magic = P%d\n", head.Magic);
	printf("HSize = %d,  ", head.HSize);
	printf("VSize = %d\n", head.VSize);
	if (head.Magic==2) {
		printf("MaxY = %d\n", head.MaxY);
	}
	//showArray(y,head.HSize,head.VSize);
	res= writePNMMonoFile(wfname, head, y);
	if (res==0) printf("file [%s] : completed to write\n",wfname);
	else printf("file [%s] : failed to write\n",wfname);
	return 0;
}

int main()
{
	char *fnames[]={
		"lenna_bin.pbm",
		"auto_bin.pbm",
		"mandrill_gray.pgm",
		"home_gray.pgm",
		"lenna_bin2.pbm",
		"lenna_bin3.pbm",
		"auto_bin2.pbm",
		"mandrill_gray2.pgm",
		"mandrill_gray3.pgm",
		"home_gray2.pgm",
		"milkdrop_gray.pgm",
		"auto_color.ppm"
	};
	char *mark[]={"completed","failed"};
	int nFiles=sizeof(fnames)/sizeof(char *);
	int result[sizeof(fnames)/sizeof(char *)];
	int i;
	for (i=0; i<nFiles; i++){
		printf(">>> %s\n",fnames[i]);
		result[i]=main1(fnames[i]);
		printf("\n");
	}
	for (i=0; i<nFiles; i++){
		printf("%-20s  %s\n",fnames[i],mark[result[i]]);
	}
}

/***

>>> lenna_bin.pbm
file [lenna_bin.pbm] : completed to read
Magic = P1
HSize = 196,  VSize = 228
file [lenna_bin_copy.pbm] : completed to write

>>> auto_bin.pbm
file [auto_bin.pbm] : completed to read
Magic = P1
HSize = 200,  VSize = 101
file [auto_bin_copy.pbm] : completed to write

>>> mandrill_gray.pgm
file [mandrill_gray.pgm] : completed to read
Magic = P2
HSize = 245,  VSize = 238
MaxY = 255
file [mandrill_gray_copy.pgm] : completed to write

>>> home_gray.pgm
file [home_gray.pgm] : completed to read
Magic = P2
HSize = 174,  VSize = 114
MaxY = 255
file [home_gray_copy.pgm] : completed to write

>>> lenna_bin2.pbm
file [lenna_bin2.pbm] : failed to read
Magic = P-1
HSize = -1,  VSize = -1

>>> lenna_bin3.pbm
file [lenna_bin3.pbm] : failed to read
Magic = P-1
HSize = -1,  VSize = -1

>>> auto_bin2.pbm
file [auto_bin2.pbm] : failed to read
Magic = P-1
HSize = -1,  VSize = -1

>>> mandrill_gray2.pgm
file [mandrill_gray2.pgm] : failed to read
Magic = P-1
HSize = -1,  VSize = -1

>>> mandrill_gray3.pgm
file [mandrill_gray3.pgm] : failed to read
Magic = P-1
HSize = -1,  VSize = -1

>>> home_gray2.pgm
file [home_gray2.pgm] : failed to read
Magic = P-1
HSize = -1,  VSize = -1

>>> milkdrop_gray.pgm
file [milkdrop_gray.pgm] : failed to read
Magic = P-1
HSize = -1,  VSize = -1

>>> auto_color.ppm
file [auto_color.ppm] : failed to read
Magic = P-1
HSize = -1,  VSize = -1

lenna_bin.pbm         completed
auto_bin.pbm          completed
mandrill_gray.pgm     completed
home_gray.pgm         completed
lenna_bin2.pbm        failed
lenna_bin3.pbm        failed
auto_bin2.pbm         failed
mandrill_gray2.pgm    failed
mandrill_gray3.pgm    failed
home_gray2.pgm        failed
milkdrop_gray.pgm     failed
auto_color.ppm        failed

***/