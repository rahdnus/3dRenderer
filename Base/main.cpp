#include "tgaimage.h"
#include "model.h"
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

const int width=800;
const int height=800;
Model *model;

Vec3f light_dir(0,0,-1);

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

void draw_wireframe(std::string address,TGAImage &image)
{
	model=new Model(address.c_str());
	for(int i=0;i<model->nfaces();i++)
	{
		std::vector<int> faces=model->face(i);
		for(int j=0;j<3;j++)
		{
			Vec3f v0=model->vert(faces[j]);
			Vec3f v1=model->vert(faces[(j+1)%3]);
			
 		int x0 = (v0.x+1.)*width/2.; 
        int y0 = (v0.y+1.)*height/2.; 
        int x1 = (v1.x+1.)*width/2.; 
        int y1 = (v1.y+1.)*height/2.; 
		draw_line(x0,y0,x1,y1,image,white);
		}
	}
}

Vec3f barycentric(Vec2i *pts, Vec2i P) { 
    Vec3f u = Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x)^Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y);
    /* `pts` and `P` has integer value as coordinates
       so `abs(u[2])` < 1 means `u[2]` is 0, that means
       triangle is degenerate, in this case return something with negative coordinates */
    if (std::abs(u.z)<1) return Vec3f(-1,1,1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z); 
} 
 /*
 public static bool PointInTriangle(Point A, Point B, Point C, Point P)
	{
			double s1 = C.y - A.y;
			double s2 = C.x - A.x;
			double s3 = B.y - A.y;
			double s4 = P.y - A.y;

			double w1 = (A.x * s1 + s4 * s2 - P.x * s1) / (s3 * s2 - (B.x-A.x) * s1);
			double w2 = (s4- w1 * s3) / s1;
			return w1 >= 0 && w2 >= 0 && (w1 + w2) <= 1;
	}
	
 */
void triangle(Vec2i *pts, TGAImage &image, TGAColor color) { 
    Vec2i bboxmin(image.get_width()-1,  image.get_height()-1); 
    Vec2i bboxmax(0, 0); 
    Vec2i clamp(image.get_width()-1, image.get_height()-1); 
    for (int i=0; i<3; i++) { 
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
	bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

	bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
	bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    } 
    Vec2i P; 
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) { 
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) { 
            Vec3f bc_screen  = barycentric(pts, P); 
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue; 
            image.set(P.x, P.y, color); 
        } 
    } 
} 

void draw_flat(std::string address,TGAImage &image)
{
	model=new Model(address.c_str());
	for(int i=0;i<model->nfaces();i++)
	{
		std::vector<int> face=model->face(i);
		Vec2i screencords[3];
		Vec3f objectcords[3];

		for(int j=0;j<3;j++)
		{
			Vec3f coords=model->vert(face[j]);
			screencords[j]=Vec2i((coords.x+1)*width/2,(coords.y+1)*height/2);
			objectcords[j]=coords;
		}

		Vec3f n=(objectcords[2]-objectcords[0])^(objectcords[1]-objectcords[0]);
		n.normalize();
		float intensity=n*light_dir;
		if(intensity>0)
			triangle(screencords,image,TGAColor(intensity*255, intensity*255, intensity*255, 255));
	}
}


#pragma region oldmethod
void draw_triangle(Vec2i v0,Vec2i v1,Vec2i v2,TGAImage &image,TGAColor color)
{
	if(v0.y>v1.y){std::swap(v0,v1);}
	if(v0.y>v2.y){std::swap(v2,v0);}
	if(v1.y>v2.y){std::swap(v1,v2);}

	int totalheight=v2.y-v0.y;
	int segmentheight=v1.y-v0.y+1;
	for(int y=v0.y;y<=v1.y;y++)
	{
		float alpha=(float)(y-v0.y)/totalheight;
		float beta=(float)(y-v0.y)/segmentheight;
		Vec2i A=v0+(v2-v0)*alpha;
		Vec2i B=v0+(v1-v0)*beta;
		if(A.x>B.x){std::swap(A,B);}
		for(int x=A.x;x<=B.x;x++)
		{
			image.set(x,y,color);
		}
	}
	int segmentBheight=v2.y-v1.y+1;

	for(int y=v1.y;y<=v2.y;y++)
	{
		float alpha=(float)(y-v0.y)/totalheight;
		float beta=(float)(y-v1.y)/segmentBheight;
		Vec2i A=v0+(v2-v0)*alpha;
		Vec2i B=v1+(v2-v1)*beta;
		if(A.x>B.x){std::swap(A,B);}
		for(int x=A.x;x<=B.x;x++)
		{
			image.set(x,y,color);
		}
	}
}
#pragma endregion oldmethod

int main(int argc, char **argv)
{
	TGAImage image(width, height, TGAImage::RGB);
	// draw_wireframe("obj/head.obj",image);
    // Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    // Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    // Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
	// Vec2i pts[3] = {Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160)}; 
	// draw_triangle(t0[0], t0[1], t0[2], image, red);
	// draw_triangle(t1[0], t1[1], t1[2], image, white);
	// draw_triangle(t2[0], t2[1], t2[2], image, green);
	// triangle(pts,image,red);
	draw_flat("obj/head.obj",image);

	// i want to have the origin at the left bottom corner of the image
	image.flip_vertically();
	image.write_tga_file("output.tga");
	return 0;
}
