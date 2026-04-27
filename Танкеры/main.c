#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include "lodepng.h"

// принимаем на вход: им€ файла, указатели на int дл€ хранени€ прочитанной ширины и высоты картинки
// возвращаем указатель на выделенную пам€ть дл€ хранени€ картинки
// ≈сли пам€ть выделить не смогли, отдаем нулевой указатель и пишем сообщение об ошибке
unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height)
{
  unsigned char* image = NULL;
  int error = lodepng_decode32_file(&image, width, height, filename);
  if(error != 0) {
    printf("error %u: %s\n", error, lodepng_error_text(error));
  }
  return (image);
}

// принимаем на вход: им€ файла дл€ записи, указатель на массив пикселей,  ширину и высоту картинки
// ≈сли преобразовать массив в картинку или сохранить не смогли,  пишем сообщение об ошибке
void write_png(const char* filename, const unsigned char* image, unsigned width, unsigned height)
{
  unsigned char* png;
  size_t pngsize;
  int error = lodepng_encode32(&png, &pngsize, image, width, height);
  if(error == 0) {
      lodepng_save_file(png, pngsize, filename);
  } else {
    printf("error %u: %s\n", error, lodepng_error_text(error));
  }
  free(png);
}

void contrast(unsigned char *col, int bw_size)
{
    int i;
    for(i=0; i < bw_size; i++)
    {
        if(col[i] < 80)
            col[i] = 0;
        if(col[i] > 187)
            col[i] = 255;
    }
    return;
}

// √ауссово размыттие
void Gauss_blur(unsigned char *col, unsigned char *blr_pic, int width, int height)
{
    int i, j;
    for(i=1; i < height-1; i++)
        for(j=1; j < width-1; j++)
        {
            blr_pic[width*i+j] = 0.084*col[width*i+j] + 0.084*col[width*(i+1)+j] + 0.084*col[width*(i-1)+j];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.084*col[width*i+(j+1)] + 0.084*col[width*i+(j-1)];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i+1)+(j+1)] + 0.063*col[width*(i+1)+(j-1)];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i-1)+(j+1)] + 0.063*col[width*(i-1)+(j-1)];
        }
   return;
}

int work_area[10][4]={
                        {313,93,374,20},
                        {543,274,595,163},
                        {631,310,708,55},
                        {507,314,566,301},
                        {550,333,566,314},
                        {574,468,846,328},
                        {535,551,610,499},
                        {566,600,605,574},
                        {568,560,913,469},
                        {703,639,1097,540},
                     };//вида {x0,y0,x1,y1}

//входит ли точка в любую из рабочих областей
int in_any_area(int x,int y)
{
    for (int i=0;i<10;i++)
    {
        if ((x>=work_area[i][0] && x<=work_area[i][2]) && (y>=work_area[i][3] && y<=work_area[i][1]))
            return 1;
    }
    return 0;
}

//dfs внутри областей
void dfs(int x,int y,int width,int height,unsigned char* image,int* visited)
{
    if (x<0 || x>width || y<0 || y>height)
        return;
    int index=y*width+x;
    if (visited[index])
        return;//уже посетили
    if (image[index*4]<190)
        return;//не корабль: черный пиксель или шум
    if (!in_any_area(x,y))
        return;//не в области
    visited[index]=1;
    //обход соседних
    dfs(x+1,y,width,height,image,visited);
    dfs(x-1,y,width,height,image,visited);
    dfs(x,y+1,width,height,image,visited);
    dfs(x,y-1,width,height,image,visited);
}

int main()
{
    const char* filename = "skull.png";
    unsigned int width, height;
    int size;
    int bw_size;

    // ѕрочитали картинку
    unsigned char* picture = load_png("skull.png", &width, &height);
    if (picture == NULL)
    {
        printf("Problem reading picture from the file %s. Error.\n", filename);
        return -1;
    }

    size = width * height * 4;
    bw_size = width * height;


    unsigned char* bw_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char));
    unsigned char* blr_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char));
    unsigned char* finish = (unsigned char*)malloc(size*sizeof(unsigned char));

    //€ркости в чб
    for (int i=0;i<bw_size;i++)
    {
        unsigned char r=picture[4*i];
        unsigned char g=picture[4*i+1];
        unsigned char b=picture[4*i+2];
        bw_pic[i]=(0.299*r+0.587*g+0.114*b);//гугл дал мне такую формулу
    }

    //зачерн€ем все области, кроме рабочих
    for (int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            if (!in_any_area(x,y))
                bw_pic[y*width+x]=0;
        }
    }

    //контраст
    contrast(bw_pic,bw_size);
    for(int i=0;i<bw_size;i++)
    {
        finish[4*i]=bw_pic[i];
        finish[4*i+1]=bw_pic[i];
        finish[4*i+2]=bw_pic[i];
        finish[4*i+3]=255;
    }
    write_png("contrast.png", finish, width, height);

    //гауссово размытие
    Gauss_blur(bw_pic, blr_pic, width, height);
    for(int i=0;i<bw_size;i++)
    {
        finish[4*i]=blr_pic[i];
        finish[4*i+1]=blr_pic[i];
        finish[4*i+2]=blr_pic[i];
        finish[4*i+3]=255;
    }
    write_png("gauss.png", finish, width, height);

    //подсчет кораблей в област€х
    int* visited=(int*)malloc(bw_size*sizeof(int));
    int ship_count=0;
    for (int y=0;y<height;y++)
    {
        for (int x=0;x<width;x++)
        {
            int idx=y*width+x;
            if (finish[idx*4]>187 && !visited[idx]) //не считаем шум
                {
                    ship_count++;
                    dfs(x,y,width,height,finish,visited);
                }
        }
    }
    printf("Numbers of ships: %d\n", ship_count);

    // не забыли почистить пам€ть!
    free(visited);
    free(bw_pic);
    free(blr_pic);
    free(finish);
    free(picture);

    return 0;
}

