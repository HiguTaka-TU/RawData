#include <stdio.h>   
#include <string.h>
#include <math.h>
#include <stdlib.h>

void dicominput(char* infile, char* outfile, char* outbinary, int *header_size, int *raw_size, int *row_size, int *col_size);

int main(int argc, char *argv[]){
    char    infile[64], outfile[32], outbinary[32];
    int     header_size, raw_size, row_size, col_size;
    
    sprintf(infile, "../DICOM_image/D202012/B1521828");
    sprintf(outfile, "dicom_header_0.txt");
    sprintf(outbinary, "image_binary_0.raw");
    
    dicominput(infile, outfile, outbinary, &header_size, &raw_size, &row_size, &col_size);
    
    short *outputimg0;
    outputimg0= (short*)malloc(raw_size);
    
    return 0;
}

void
dicominput(char* infile, char* outfile, char* outbinary, int *header_size, int *raw_size, int *row_size, int *col_size)
{
    FILE    *fp, *fp1, *fp2, *fpp;
    unsigned char   buf[4],gTag1,gTag2,eTag1,eTag2;
    unsigned char   *buf3;
    char *buff3;
    unsigned char   allendflag=0;
    char Header[32]={0};
    short  *outputimg;
    int headsum = 0;
    float xx,yy,zz;
    int dpuli_patientName = 0;
    
    if((fp=fopen(infile,"rb")) == NULL) {
        printf("OPEN FAILED %s\n", infile);
        exit(0);
    }
    
    if((fp1=fopen(outfile,"w")) == NULL) {
        printf("OPEN FAILED %s\n", outfile);
        exit(0);
    }
    
    gTag1=0x00;gTag2=0x00;eTag1=0x00;eTag2=0x00;
    
    
    while(gTag1!=0x7f || gTag2!=0xe0 || eTag1!=0x00 || eTag2!=0x10){
        fread(buf,sizeof(unsigned char),1,fp);
        headsum++;
        //printf("%d\n",headsum);
        gTag2=gTag1;
        gTag1=eTag2;
        eTag2=eTag1;
        eTag1=*(unsigned char*)buf;
        
        //おそらくプライベートTag 説明書探す
        if(gTag1==0x00 && gTag2==0x19 && eTag1==0x11 && eTag2==0x01) strcpy(Header,     "Row_number");
        else if(gTag1==0x00 && gTag2==0x19 && eTag1==0x11 && eTag2==0x02) strcpy(Header,     "Col_number");
        else if(gTag1==0x00 && gTag2==0x08 && eTag1==0x00 && eTag2==0x22) strcpy(Header,            "Acquisition DATE");
        
        
        //行数と列数を取得
        if((gTag1==0x00 && gTag2==0x19 && eTag1==0x11 && eTag2==0x01) ||
           (gTag1==0x00 && gTag2==0x19 && eTag1==0x11 && eTag2==0x02))
        {
            //VRのデータ長の部分
            fread(buf,sizeof(unsigned char),4,fp);
            
            long dLen = *(long*)buf;
            headsum +=dLen +4;
            
            fprintf(fp1,"%s\n",Header);
            
            //今回は数値ではなく、文字としてデータサイズが格納されている
            fread(buf,sizeof(unsigned char),dLen,fp);
            fwrite(buf, sizeof(unsigned char), dLen, fp1);
            fprintf(fp1,"\n");
            
            
        }
        
        //撮影した日時を取得
        if (gTag1==0x00 && gTag2==0x08 && eTag1==0x00 && eTag2==0x22)
        {
            fread(buf,sizeof(unsigned char),2,fp);
            headsum+=2;
            
            //同じタグのものが2つあるのでDA(DATEの略)がついている方を読み取る
            if(buf[0]=='D' && buf[1]=='A'){
                fread(buf,sizeof(unsigned char),2,fp);
                
                long dLen = *(long*)buf;
                headsum+=dLen+2;
                
                fprintf(fp1,"%s\n",Header);
                
                for(int i=0;i<dLen;i++){
                    fread(buf,sizeof(unsigned char),1,fp);
                    fwrite(buf, sizeof(unsigned char), 1, fp1);
                }
                fprintf(fp1,"\n");
            }
        }
    }// end while
    
    fread(buf,sizeof(unsigned char),2,fp);
    if(buf[0]=='O' && buf[1]=='W')
    {
        headsum+=8; //'O'+'W'+空白+raw_size
        //2バイトは空白
        fread(buf,sizeof(unsigned char),2,fp);
        //4バイトにraw_sizeが格納
        fread(buf,sizeof(unsigned char),4,fp);
        
        long dLen = *(long*)buf;
        *raw_size = dLen;
    }
    else
    {
        headsum+=4; //'raw_size'
        //4バイトにraw_sizeが格納(空白はなし)
        fread(buf,sizeof(unsigned char),4,fp);
        long dLen = *(long*)buf;
        *raw_size = dLen;
    }
    
    fprintf(fp1,"Header size\n%d\n",headsum);
    *header_size = headsum;
    printf("%d\n",*header_size);
    
    outputimg=(short*)malloc(*raw_size);
    fread(outputimg, 1, *raw_size, fp);
    
    if((fp2=fopen(outbinary,"wb")) == NULL) {
        printf("OPEN FAILED %s\n", outbinary);
        exit(0);
    }
    //ヘッダー情報なしのrawデータのみを書き込み
    fwrite(outputimg, 1, *raw_size , fp2);
    free(outputimg);
    
    fclose(fp);fclose(fp1);fclose(fp2);
}
