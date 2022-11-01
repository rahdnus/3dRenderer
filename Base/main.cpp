#include "tgaimage.h"
#include "model.h"
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

const int width=800;
const int height=800;

void draw_line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
	bool steep = false;
	if (std::abs(x1 - x0) < std::abs(y1 - y0))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x1 < x0)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int dy = y1 - y0;
	int dx = x1 - x0;
	float derror = std::abs(dy) * 2;
	float error = 0;
	int y = y0;
	int yincr = (y1 > y0 ? 1 : -1);
	if (steep)
	{
		for (float x = x0; x < x1; x++)
		{
			image.set(y, x, color);
			error += derror;
			if (error > 0.5)
			{
				y += yincr;
				error -= dx * 2;
			}
		}
	}

	else
	{
		for (float x = x0; x < x1; x++)
		{
			image.set(x, y, color);
			error += derror;
			if (error > 0.5)
			{
				y += yincr;
				error -= dx * 2;
			}
		}
	}
}

int main(int argc, char **argv)
{
	Model *model=new Model("obj/head.obj");
	TGAImage image(width, height, TGAImage::RGB);
	for(int i=0;i<model->nfaces();i++)
	{
		std::vector<int> faces=model->face(i);
		int n=faces.size();
		for(int j=0;j<3;j++)
		{
			Vec3f v0=model->vert(faces[j]);
			Vec3f v1=model->vert(faces[(j+1)%3]);
			
 		int x0 = (v0.x+1.)*width/2.; 
        int y0 = (v0.y+1.)*height/2.; 
        int x1 = (v1.x+1.)*width/2.; 
        int y1 = (v1.y+1.)*height/2.; 
        draw_line(x0, y0, x1, y1, image, white); 
		}
	}
	// draw_line(13, 20, 80, 40, image, white);
	// draw_line(20, 13, 40, 80, image, green);
	// draw_line(80, 40, 13, 20, image, red);

	// i want to have the origin at the left bottom corner of the image
	image.flip_vertically();
	image.write_tga_file("output.tga");
	return 0;
}
