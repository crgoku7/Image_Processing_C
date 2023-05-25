//IMPORTING ESSESNTIAL LIBRARIES
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>


//GOAL - TO APPLY SOBBLE FILTER TO THE IMAGE
//PROGRESS - 
//SHORTCOMINGS - NONE

#define EDGE_THRES 100

int Image_Process(char src[50]);
void Greyscale(unsigned char *img_matrix, unsigned char * grey_matrix, int img_size);
int * G_blur(unsigned char *grey_matrix, int img_h, int img_w);
int* Sobble(int * blur_matrix, int img_h, int img_w);
//BETA IMPLIMENTATION OF PATCHES
int* Patch(int* sobble_matrix, int img_h, int img_w, int patch_s );


int main(){
	char src[50];

	//READING IMG PATH FROM TERMINAL
	printf("ENTER PATH TO IMAGE FILE:");
	fgets(src, 50, stdin);

	//CALLING THE IMG PROCESSING FUNCTION
	Image_Process(src);


}

//CHANGES COLOR IMAGE DATA INTO GREYSCALE FOR FURTHER PROCESSING
void Greyscale(unsigned char * img_data, unsigned char * grey_matrix, int img_size){
	for(int i  = 0; i <img_size;i++){
		unsigned char I = __max(img_data[3*i],img_data[3*i+1]);
		I = __max(I, img_data[3*i+2]);
		grey_matrix[i] = I;
	}

}

//APPLIES GAUSIAN BLUR TO GREYSCALE DATA FOR FURTHER PROCESSING
int * G_blur(unsigned char * grey_matrix, int img_h, int img_w){
	int img_size = img_h*img_w;
	int * blur_matrix = (void*)malloc(sizeof(int)*img_size);

	float v = 1.0;
	float kernel[3][3] = {{v,2*v,v},
						  {2*v,4*v,2*v},
				   		  {v,2*v,v}};
	for(int i = 0;i<img_size;i++){
		float s = 0.0;
		//DETECTS THAT PIXEL IS NOT ON BORDERS
		if(i>img_w && i < (img_size-img_w) && (i%img_w)%(img_w-1)){
			for(int j = 0; j<3; j++){
				for(int k = 0; k<3;k++){
					int idx = i + img_w*(j-1) + (k-1);
					s+= (float)grey_matrix[idx]*kernel[j][k];
				}
			}
			s = s/16;

		}
		else{
			s = grey_matrix[i];
		}
		blur_matrix[i] = s;

	}
	return blur_matrix;
	
}
//APPLIES SOBBLE FILTER TO MATRIX
int * Sobble(int * blur_matrix, int img_h, int img_w){
	int img_size = img_h*img_w;
	int * sobble_matrix = (void*)malloc(sizeof(int)*img_size);
	int * gy = (void*)malloc(sizeof(int)*img_size);
	int * gx = (void*)malloc(sizeof(int)*img_size);

	float v = 1.0;
	float ker_x[3][3] = {{-1,0,1},
					  {-2,0,2},
				   	  {-1,0,1}};
	float ker_y[3][3] = {{-1,-2,-1},
					  {0,0,0},
				   	  {1,2,1}};

	for(int i = 0;i<img_size;i++){
		float sx = 0.0, sy = 0.0;
		//DETECTS THAT PIXEL IS NOT ON BORDERS
		if(i>img_w && i < (img_size-img_w) && (i%img_w)%(img_w-1)){
			for(int j = 0; j<3; j++){
				for(int k = 0; k<3;k++){
					int idx = i + img_w*(j-1) + (k-1);
					sx+= (float)blur_matrix[idx]*ker_x[j][k];
					sy+= (float)blur_matrix[idx]*ker_y[j][k];
				}
			}

		}
		
		gx[i] = sx;
		gy[i] = sy;
		sobble_matrix[i] = pow((sx*sx+sy*sy), 0.5);

	}
	return sobble_matrix;
}

//STILL TESTING THIS - DIVIDES IMAGE IN PATCHES AND HIGHLIGHTS PATCHES WITH EDGES
int* Patch(int* sobble_matrix, int img_h, int img_w, int patch_s ){
	int n = (img_h*img_w)/(patch_s*patch_s);
	int *patch_matrix = (int*)malloc(sizeof(int)*n);
	int img_size = img_h*img_w;
	for(int i = 0; i< img_size;i++){
			int x = (i%img_w)/patch_s;
			int y = (i/img_h)/patch_s;
			int pn = y*(img_w/patch_s)+x;
			//printf("%d\n", pn);
			if(patch_matrix[pn]<0)patch_matrix[pn]=0;
			if(sobble_matrix[i]> EDGE_THRES) patch_matrix[pn] += 1;
	
	}
	return patch_matrix;
}

//MAIN IMAGE PROCESSING FUNCTION
int Image_Process(char src[50]){
	FILE *fin, *fout, *fsob;
	fin = fopen(src, "rb");
	fout = fopen("results\\image_o.bmp", "wb");
	fsob = fopen("results\\image_b.bmp", "wb");

	//READ IMAGE HEADER AND ASSIGN H,W,BITDEPTH
	unsigned char header[54];				
	fread(header, sizeof(unsigned char), 54, fin);
	fwrite(header, sizeof(unsigned char), 54, fout);
	fwrite(header, sizeof(unsigned char), 54, fsob);

	int img_h = *(int *)&header[22];
	int img_w = *(int *)&header[18];
	int img_size = img_h*img_w;
	int bitDepth = *(int *)&header[28];

	//FINDING IF IT HAS A COLOR TABLE
	//IF BITDEPTH <= 8 COLOR TABLE EXISTS
	printf("\n%d %d %d %d\n",img_h, img_w, img_size, bitDepth);
	if(bitDepth <= 8){
		unsigned char colorTable[1024];
		fread(colorTable, sizeof(unsigned char), 1024, fin);
		fwrite(colorTable, sizeof(unsigned char), 1024, fout);
		fwrite(colorTable, sizeof(unsigned char), 1024, fsob);
	}

	//BITDEPTH = 8 => GREYSCALE
	if(bitDepth <= 8){
		unsigned char *img_data ;
		img_data = (void*)malloc(img_size*sizeof(unsigned char));
		int *img_matrix = (void*)malloc(img_size*sizeof(int));
		for(int i = 0; i <img_size; i++){
			img_data[i] = fgetc(fin);
			int x = *(int*)&img_data[i];
			img_matrix[i] = x;
		}
		int *blur_matrix = G_blur(img_data, img_h, img_w);
		int *sobble_matrix = Sobble(blur_matrix, img_h, img_w);

		for(int i = 0; i< img_size; i++){
			fputc(sobble_matrix[i], fout);
			fputc(blur_matrix[i], fsob);
		}


	}

	//BITDEPTH = 24 => COLOR
	else if(bitDepth == 24){
		unsigned char *img_data = (void *)malloc(sizeof(int*)*3*img_size);
		unsigned char *grey_data = (void *)malloc(sizeof(int*)*img_size);
		int *img_matrix, *grey_matrix;
		img_matrix = (void *)malloc(sizeof(int*)*3*img_size);
		grey_matrix = (void *)malloc(sizeof(int*)*img_size);

		//READING FROM INPUT IMAGE TO IMG_DAT AND IMG_MATRIX
		for(int i  = 0; i <img_size;i++){
			img_data[3*i+2] = fgetc(fin);
			img_data[3*i+1] = fgetc(fin);
			img_data[3*i] = fgetc(fin);

			img_matrix[3*i] = *(int*)&img_data[3*i];
			img_matrix[3*i+1] = *(int*)&img_data[3*i+1];
			img_matrix[3*i+2] = *(int*)&img_data[3*i+2];

		}

		//MAKING A GREYSCALE IMAGE MATRIX
		Greyscale(img_data, grey_data, img_size);

		//APPLYING GAUSSIAN BLUR TO GREYSCALE IMG
		int *blur_matrix = G_blur(grey_data, img_h, img_w);

		//APPLYING SOBBLE OPERATOR TO BLURRED IMG
		int *sobble_matrix = Sobble(blur_matrix, img_h, img_w);

		//PATCHS?
		int patch_s = 32;
		int *patch_matrix = Patch(sobble_matrix, img_h, img_w, patch_s);
		//WRITING INTO OUTPUT IMAGE


		for(int i  = 0; i <img_size;i++){
			//CALCULATING PATCH
			int x = (i%img_w)/patch_s;
			int y = (i/img_h)/patch_s;
			
			if((i/img_w)%patch_s!=0 && i%patch_s!=0){

				fputc(img_matrix[3*i+2],fout);
				fputc(img_matrix[3*i+1], fout);
				fputc(img_matrix[3*i],fout);

			}
			else{
				int pn = y*(img_w/patch_s)+x;
				//printf("%d\n", patch_matrix[pn]);
				if(patch_matrix[pn]>8){
					fputc(0,fout);
					fputc(255, fout);
					fputc(0,fout);
			}
			fputc(sobble_matrix[i],fsob);
			fputc(sobble_matrix[i], fsob);
			fputc(sobble_matrix[i],fsob);
		}
	
	fclose(fin);
	fclose(fout);
	fclose(fsob);

}
}
