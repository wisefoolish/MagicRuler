#include <graphics.h>
#include <math.h>
#include <vector>
const double PI = 3.1415926536; 		// π
const int WIDTH = 640; 					// 屏幕宽度
const int HEIGHT = 480; 				// 屏幕高度
const int GAMEPAD = 60; 				// 游戏手柄，移动多少距离旋转 60
const double ProjectSurfaceDis = 1000;	// 摄像机距离
const int up = 0;				// 魔尺块上面
const int down = 1;				// 魔尺块下面
const int front_1 = 2;			// 魔尺块前面某三角形 1
const int front_2 = 3;			// 魔尺块前面某三角形 2
const int left_1 = 4;			// 魔尺块左面某三角形 1
const int left_2 = 5;			// 魔尺块左面某三角形 2
const int right_1 = 6;			// 魔尺块右面某三角形 1
const int right_2 = 7;			// 魔尺块右面某三角形 2
using std::vector;

// 二维向量
class Vec2
{
public:
	double xx, yy;
	// 构造函数
	Vec2(double xx = 0, double yy = 0) : xx(xx), yy(yy) {}
	// 向量相加
	Vec2	operator+(Vec2 num) { return Vec2(this->xx + num.xx, this->yy + num.yy); }
	// 向量乘法
	Vec2	operator*(double num) { return Vec2(this->xx * num, this->yy * num); }
	// 向量点乘
	double	operator*(Vec2 num) { return this->xx * num.xx + this->yy * num.yy; }
	// 向量除法
	Vec2	operator/(double num) { return Vec2(this->xx / num, this->yy / num); }
	// 向量相减
	Vec2	operator-(Vec2 num) { return Vec2(this->xx - num.xx, this->yy - num.yy); }
	// 得到此向量模长
	double	GetLength() { return sqrt(this->xx * this->xx + this->yy * this->yy); }
	// 得到两向量之间的 cos 值
	double	GetCosBetween(Vec2 num) { return (*this) * num / this->GetLength() / num.GetLength(); }
	// 得到此向量的单位向量
	Vec2	GetUnitVector() { return (*this) / this->GetLength(); }
	// 得到此向量在另一个向量上的投影
	Vec2	GetProjectionTo(Vec2 num) { return num.GetUnitVector() * (this->GetCosBetween(num) * this->GetLength()); }
	// 得到此向量旋转 angle 后的向量
	Vec2	GetRotateVec(double angle) { return Vec2(this->xx * cos(angle) - this->yy * sin(angle), this->yy * cos(angle) + this->xx * sin(angle)); }
};

// 三维向量
class Vec3
{
public:
	double xx, yy, zz;
	// 构造函数
	Vec3(double xx = 0, double yy = 0, double zz = 0) : xx(xx), yy(yy), zz(zz) {}
	// 向量相加
	Vec3	operator+(Vec3 num) { return Vec3(this->xx + num.xx, this->yy + num.yy, this->zz + num.zz); }
	// 向量乘法
	Vec3	operator*(double num) { return Vec3(this->xx * num, this->yy * num, this->zz * num); }
	// 向量点乘
	double	operator*(Vec3 num) { return this->xx * num.xx + this->yy * num.yy + this->zz * num.zz; }
	// 向量除法
	Vec3	operator/(double num) { return Vec3(this->xx / num, this->yy / num, this->zz / num); }
	// 向量相减
	Vec3	operator-(Vec3 num) { return Vec3(this->xx - num.xx, this->yy - num.yy, this->zz - num.zz); }
	// 得到此向量模长
	double	GetLength() { return sqrt(this->xx * this->xx + this->yy * this->yy + this->zz * this->zz); }
	// 得到两向量之间的 cos 值
	double	GetCosBetween(Vec3 num) { return (*this) * num / this->GetLength() / num.GetLength(); }
	// 得到此向量的单位向量
	Vec3	GetUnitVector() { return (*this) / this->GetLength(); }
	// 得到此向量在另一个向量上的投影
	Vec3	GetProjectionTo(Vec3 num) { return num.GetUnitVector() * (this->GetCosBetween(num) * this->GetLength()); }
	// 向量叉乘
	Vec3	MultiplicationCross(Vec3 num) { return Vec3(this->yy * num.zz - this->zz * num.yy, -this->xx * num.zz + this->zz * num.xx, this->xx * num.yy - this->yy * num.xx); }
	// 求将此向量关于 X 轴，Y 轴，Z 轴旋转 a、b、c 度后的向量
	Vec3	GetRotateVec(double a, double b, double c)
	{
		Vec3 result = this->GetUnitVector();
		result = Vec3(result.xx, result.yy * cos(a) - result.zz * sin(a), result.zz * cos(a) + result.yy * sin(a)).GetUnitVector();
		result = Vec3(result.xx * cos(b) - result.zz * sin(b), result.yy, result.zz * cos(b) + result.xx * sin(b)).GetUnitVector();
		result = Vec3(result.xx * cos(c) - result.yy * sin(c), result.yy * cos(c) + result.xx * sin(c), result.zz).GetUnitVector();
		return (result * this->GetLength());
	}
};

// 三维向量绕某个投影面旋转
Vec3 RotateVec3(Vec3 vertex, Vec3 XAcross, Vec3 YAcross, double angle)
{
	Vec3 ZAcross = XAcross.MultiplicationCross(YAcross).GetUnitVector();
	Vec3 Vert = vertex.GetProjectionTo(ZAcross);
	double length = vertex.GetLength();
	Vec2 proVec = Vec2(length * vertex.GetCosBetween(XAcross), length * vertex.GetCosBetween(YAcross));
	proVec = proVec.GetRotateVec(angle);
	return Vert + XAcross * proVec.xx + YAcross * proVec.yy;
}

// 2 维顶点
class Vertex2D
{
public:
	Vec2 pos;
	double R, G, B;
	double depth;
	Vertex2D(Vec2 pos = Vec2(), double R = 0, double G = 0, double B = 0, double depth = 0) :pos(pos), R(R), G(G), B(B), depth(depth) {}
};

// 3 维顶点
class Vertex3D
{
public:
	Vec3 pos;
	double R, G, B;
	Vertex3D(Vec3 pos = Vec3(), double R = 0, double G = 0, double B = 0) :pos(pos), R(R), G(G), B(B) {}
};

// 2 维三角形
class Triangle2D
{
public:
	Vertex2D vertex[3];
	// 判断是否在 2 维三角形内
	bool isInTriangle(Vec2 pos)
	{
		Vec2 vec_side_1 = vertex[1].pos - vertex[0].pos;
		Vec2 vec_side_2 = vertex[2].pos - vertex[0].pos;
		Vec2 vec_1 = pos - vertex[0].pos;
		// vec_side_1*m+vec_side_2*n=vec_1
		double n = (vec_1.xx * vec_side_1.yy - vec_1.yy * vec_side_1.xx) / (vec_side_2.xx * vec_side_1.yy - vec_side_2.yy * vec_side_1.xx);
		double m = (vec_1.yy * vec_side_2.xx - vec_1.xx * vec_side_2.yy) / (vec_side_2.xx * vec_side_1.yy - vec_side_2.yy * vec_side_1.xx);
		if (n >= 0 && m >= 0 && n + m <= 1)return true;
		return false;
	}
	// 获取某点的深度
	double GetDepth(Vec2 pos)
	{
		Vec2 vec_side_1 = vertex[1].pos - vertex[0].pos;
		Vec2 vec_side_2 = vertex[2].pos - vertex[0].pos;
		Vec2 vec_1 = pos - vertex[0].pos;
		// vec_side_1*m+vec_side_2*n=vec_1
		double n = (vec_1.xx * vec_side_1.yy - vec_1.yy * vec_side_1.xx) / (vec_side_2.xx * vec_side_1.yy - vec_side_2.yy * vec_side_1.xx);
		double m = (vec_1.yy * vec_side_2.xx - vec_1.xx * vec_side_2.yy) / (vec_side_2.xx * vec_side_1.yy - vec_side_2.yy * vec_side_1.xx);
		return vertex[0].depth + (vertex[1].depth - vertex[0].depth) * m + (vertex[1].depth - vertex[0].depth) * n;
	}
	// 绘画 2 维三角形
	void Draw(double* DepthMap, int w, int h)
	{
		int index[3] = { 0, 1, 2 };
		if (vertex[index[0]].pos.yy > vertex[index[1]].pos.yy)
			index[0] ^= index[1] ^= index[0] ^= index[1];	// 交换两个值
		if (vertex[index[0]].pos.yy > vertex[index[2]].pos.yy)
			index[0] ^= index[2] ^= index[0] ^= index[2];	// 交换两个值
		if (vertex[index[1]].pos.yy > vertex[index[2]].pos.yy)
			index[2] ^= index[1] ^= index[2] ^= index[1];	// 交换两个值
		Vec2 vec_side_1 = vertex[index[1]].pos - vertex[index[0]].pos;
		Vec2 vec_side_2 = vertex[index[2]].pos - vertex[index[0]].pos;
		// 0~1
		for (int i = (int)vertex[index[0]].pos.yy; i < (int)vertex[index[1]].pos.yy; i++)
		{
			if (i < 0 || i >= h) continue;
			double t_b = (i - (int)vertex[index[0]].pos.yy) / (double)((int)vertex[index[1]].pos.yy - (int)vertex[index[0]].pos.yy);
			double t_c = (i - (int)vertex[index[0]].pos.yy) / (double)((int)vertex[index[2]].pos.yy - (int)vertex[index[0]].pos.yy);
			int x_b = vertex[index[0]].pos.xx + (vertex[index[1]].pos.xx - vertex[index[0]].pos.xx) * t_b;
			int x_c = vertex[index[0]].pos.xx + (vertex[index[2]].pos.xx - vertex[index[0]].pos.xx) * t_c;
			if (x_b > x_c)
				x_b ^= x_c ^= x_b ^= x_c;
			for (int j = x_b; j < x_c; j++)
			{
				if (j < 0 || j >= w) continue;
				Vec2 vec_1 = Vec2(j, i) - vertex[index[0]].pos;
				// vec_side_1*m+vec_side_2*n=vec_1
				double n = (vec_1.xx * vec_side_1.yy - vec_1.yy * vec_side_1.xx) / (vec_side_2.xx * vec_side_1.yy - vec_side_2.yy * vec_side_1.xx);
				double m = (vec_1.yy * vec_side_2.xx - vec_1.xx * vec_side_2.yy) / (vec_side_2.xx * vec_side_1.yy - vec_side_2.yy * vec_side_1.xx);
				double depth = vertex[index[0]].depth + (vertex[index[1]].depth - vertex[index[0]].depth) * m + (vertex[index[2]].depth - vertex[index[0]].depth) * n;
				if (depth < DepthMap[i * w + j] || depth>0 && DepthMap[i * w + j] <= 0)
				{
					double R = max(min(vertex[index[0]].R + (vertex[index[1]].R - vertex[index[0]].R) * m + (vertex[index[2]].R - vertex[index[0]].R) * n, 255), 0);
					double G = max(min(vertex[index[0]].G + (vertex[index[1]].G - vertex[index[0]].G) * m + (vertex[index[2]].G - vertex[index[0]].G) * n, 255), 0);
					double B = max(min(vertex[index[0]].B + (vertex[index[1]].B - vertex[index[0]].B) * m + (vertex[index[2]].B - vertex[index[0]].B) * n, 255), 0);
					COLORREF col = RGB(R, G, B);
					putpixel(j, i, col);
					DepthMap[i * w + j] = depth;
				}
			}
		}
		// 1~2
		for (int i = (int)vertex[index[1]].pos.yy; i < (int)vertex[index[2]].pos.yy; i++)
		{
			if (i < 0 || i >= h) continue;
			double t_b = (i - (int)vertex[index[1]].pos.yy) / (double)((int)vertex[index[2]].pos.yy - (int)vertex[index[1]].pos.yy);
			double t_c = (i - (int)vertex[index[0]].pos.yy) / (double)((int)vertex[index[2]].pos.yy - (int)vertex[index[0]].pos.yy);
			int x_b = vertex[index[1]].pos.xx + (vertex[index[2]].pos.xx - vertex[index[1]].pos.xx) * t_b;
			int x_c = vertex[index[0]].pos.xx + (vertex[index[2]].pos.xx - vertex[index[0]].pos.xx) * t_c;
			if (x_b > x_c)
				x_b ^= x_c ^= x_b ^= x_c;
			for (int j = x_b; j < x_c; j++)
			{
				if (j < 0 || j >= w) continue;
				Vec2 vec_1 = Vec2(j, i) - vertex[index[0]].pos;
				// vec_side_1*m+vec_side_2*n=vec_1
				double n = (vec_1.xx * vec_side_1.yy - vec_1.yy * vec_side_1.xx) / (vec_side_2.xx * vec_side_1.yy - vec_side_2.yy * vec_side_1.xx);
				double m = (vec_1.yy * vec_side_2.xx - vec_1.xx * vec_side_2.yy) / (vec_side_2.xx * vec_side_1.yy - vec_side_2.yy * vec_side_1.xx);
				double depth = vertex[index[0]].depth + (vertex[index[1]].depth - vertex[index[0]].depth) * m + (vertex[index[2]].depth - vertex[index[0]].depth) * n;
				if (depth < DepthMap[i * w + j] || depth>0 && DepthMap[i * w + j] <= 0)
				{
					double R = max(min(vertex[index[0]].R + (vertex[index[1]].R - vertex[index[0]].R) * m + (vertex[index[2]].R - vertex[index[0]].R) * n, 255), 0);
					double G = max(min(vertex[index[0]].G + (vertex[index[1]].G - vertex[index[0]].G) * m + (vertex[index[2]].G - vertex[index[0]].G) * n, 255), 0);
					double B = max(min(vertex[index[0]].B + (vertex[index[1]].B - vertex[index[0]].B) * m + (vertex[index[2]].B - vertex[index[0]].B) * n, 255), 0);
					COLORREF col = RGB(R, G, B);
					putpixel(j, i, col);
					DepthMap[i * w + j] = depth;
				}
			}
		}
	}
};

// 3 维三角形
class Triangle3D
{
public:
	Vertex3D vertex[3];
	void Rotate(Vec3 XAcross, Vec3 YAcross, double angle)
	{
		vertex[0].pos = RotateVec3(vertex[0].pos, XAcross, YAcross, angle);
		vertex[1].pos = RotateVec3(vertex[1].pos, XAcross, YAcross, angle);
		vertex[2].pos = RotateVec3(vertex[2].pos, XAcross, YAcross, angle);
	}
};

// 魔尺块
class BlockMagicRuler
{
public:
	Triangle3D surface[8];
	Vec3 pos;
	Vec3 anchor;
	BlockMagicRuler() = default;
	BlockMagicRuler(double side, COLORREF col, bool isReverse, Vec3 pos, Vec3 anchor) :pos(pos), anchor(anchor)
	{
		double R = col % (1 << 8);
		double G = (col >> 8) % (1 << 8);
		double B = (col >> 16) % (1 << 8);
		// 以长方形侧面的中心为始
		if (!isReverse)
		{
			surface[up].vertex[0] = Vertex3D(Vec3(-side / sqrt(2), 0, side / 2), R, G, B);
			surface[up].vertex[1] = Vertex3D(Vec3(side / sqrt(2), 0, side / 2), R, G, B);
			surface[up].vertex[2] = Vertex3D(Vec3(0, side / sqrt(2), side / 2), R, G, B);

			surface[down].vertex[2] = Vertex3D(Vec3(-side / sqrt(2), 0, -side / 2), R, G, B);
			surface[down].vertex[1] = Vertex3D(Vec3(side / sqrt(2), 0, -side / 2), R, G, B);
			surface[down].vertex[0] = Vertex3D(Vec3(0, side / sqrt(2), -side / 2), R, G, B);

			surface[left_1].vertex[0] = Vertex3D(Vec3(0, side / sqrt(2), -side / 2), 64, 64, 64);
			surface[left_1].vertex[1] = Vertex3D(Vec3(-side / sqrt(2), 0, -side / 2), 64, 64, 64);
			surface[left_1].vertex[2] = Vertex3D(Vec3(0, side / sqrt(2), side / 2), 64, 64, 64);

			surface[left_2].vertex[2] = Vertex3D(Vec3(-side / sqrt(2), 0, side / 2), 64, 64, 64);
			surface[left_2].vertex[1] = Vertex3D(Vec3(-side / sqrt(2), 0, -side / 2), 64, 64, 64);
			surface[left_2].vertex[0] = Vertex3D(Vec3(0, side / sqrt(2), side / 2), 64, 64, 64);

			surface[right_1].vertex[0] = Vertex3D(Vec3(0, side / sqrt(2), side / 2), 64, 64, 64);
			surface[right_1].vertex[1] = Vertex3D(Vec3(side / sqrt(2), 0, side / 2), 64, 64, 64);
			surface[right_1].vertex[2] = Vertex3D(Vec3(0, side / sqrt(2), -side / 2), 64, 64, 64);

			surface[right_2].vertex[0] = Vertex3D(Vec3(side / sqrt(2), 0, -side / 2), 64, 64, 64);
			surface[right_2].vertex[1] = Vertex3D(Vec3(0, side / sqrt(2), -side / 2), 64, 64, 64);
			surface[right_2].vertex[2] = Vertex3D(Vec3(side / sqrt(2), 0, side / 2), 64, 64, 64);

			surface[front_1].vertex[0] = Vertex3D(Vec3(-side / sqrt(2), 0, -side / 2), 255, 255, 255);
			surface[front_1].vertex[1] = Vertex3D(Vec3(side / sqrt(2), 0, -side / 2), 255, 255, 255);
			surface[front_1].vertex[2] = Vertex3D(Vec3(-side / sqrt(2), 0, side / 2), 255, 255, 255);

			surface[front_2].vertex[0] = Vertex3D(Vec3(side / sqrt(2), 0, side / 2), 255, 255, 255);
			surface[front_2].vertex[1] = Vertex3D(Vec3(-side / sqrt(2), 0, side / 2), 255, 255, 255);
			surface[front_2].vertex[2] = Vertex3D(Vec3(side / sqrt(2), 0, -side / 2), 255, 255, 255);
		}
		else
		{
			surface[up].vertex[2] = Vertex3D(Vec3(-side / sqrt(2), 0, side / 2), 255, 255, 255);
			surface[up].vertex[1] = Vertex3D(Vec3(side / sqrt(2), 0, side / 2), 255, 255, 255);
			surface[up].vertex[0] = Vertex3D(Vec3(0, -side / sqrt(2), side / 2), 255, 255, 255);

			surface[down].vertex[0] = Vertex3D(Vec3(-side / sqrt(2), 0, -side / 2), 255, 255, 255);
			surface[down].vertex[1] = Vertex3D(Vec3(side / sqrt(2), 0, -side / 2), 255, 255, 255);
			surface[down].vertex[2] = Vertex3D(Vec3(0, -side / sqrt(2), -side / 2), 255, 255, 255);

			surface[left_1].vertex[0] = Vertex3D(Vec3(0, -side / sqrt(2), side / 2), 64, 64, 64);
			surface[left_1].vertex[1] = Vertex3D(Vec3(-side / sqrt(2), 0, side / 2), 64, 64, 64);
			surface[left_1].vertex[2] = Vertex3D(Vec3(0, -side / sqrt(2), -side / 2), 64, 64, 64);

			surface[left_2].vertex[0] = Vertex3D(Vec3(-side / sqrt(2), 0, -side / 2), 64, 64, 64);
			surface[left_2].vertex[1] = Vertex3D(Vec3(0, -side / sqrt(2), -side / 2), 64, 64, 64);
			surface[left_2].vertex[2] = Vertex3D(Vec3(-side / sqrt(2), 0, side / 2), 64, 64, 64);

			surface[right_1].vertex[0] = Vertex3D(Vec3(0, -side / sqrt(2), -side / 2), 64, 64, 64);
			surface[right_1].vertex[1] = Vertex3D(Vec3(side / sqrt(2), 0, -side / 2), 64, 64, 64);
			surface[right_1].vertex[2] = Vertex3D(Vec3(0, -side / sqrt(2), side / 2), 64, 64, 64);

			surface[right_2].vertex[0] = Vertex3D(Vec3(0, -side / sqrt(2), side / 2), 64, 64, 64);
			surface[right_2].vertex[1] = Vertex3D(Vec3(side / sqrt(2), 0, -side / 2), 64, 64, 64);
			surface[right_2].vertex[2] = Vertex3D(Vec3(side / sqrt(2), 0, side / 2), 64, 64, 64);

			surface[front_1].vertex[2] = Vertex3D(Vec3(-side / sqrt(2), 0, -side / 2), R, G, B);
			surface[front_1].vertex[1] = Vertex3D(Vec3(side / sqrt(2), 0, side / 2), R, G, B);
			surface[front_1].vertex[0] = Vertex3D(Vec3(-side / sqrt(2), 0, side / 2), R, G, B);

			surface[front_2].vertex[0] = Vertex3D(Vec3(side / sqrt(2), 0, -side / 2), R, G, B);
			surface[front_2].vertex[1] = Vertex3D(Vec3(-side / sqrt(2), 0, -side / 2), R, G, B);
			surface[front_2].vertex[2] = Vertex3D(Vec3(side / sqrt(2), 0, side / 2), R, G, B);
		}
	}
	// 旋转魔尺块
	void Rotate(Vec3 XAcross, Vec3 YAcross, double angle)
	{
		for (int i = 0; i < 8; i++) surface[i].Rotate(XAcross, YAcross, angle);
	}
};

// 魔尺
class MagicRuler
{
public:
	vector<BlockMagicRuler> blocks;
	int centerBlock;
	Vec3 pos;
	MagicRuler() = default;
	MagicRuler(int nums, double side, COLORREF col, Vec3 pos) :pos(pos)
	{
		centerBlock = nums / 2;
		for (int i = 0; i < nums; i++)
		{
			if (i % 2)
			{
				blocks.push_back(BlockMagicRuler(side, col, true, Vec3(0, 0, 0), Vec3(-side * sqrt(2) / 4, -side * sqrt(2) / 4, 0)));
			}
			else
			{
				blocks.push_back(BlockMagicRuler(side, col, false, Vec3(0, 0, 0), Vec3(-side * sqrt(2) / 4, side * sqrt(2) / 4, 0)));
			}
		}
	}
	void Rotate(int blockId, bool isLeft, double angle = PI / 2)
	{
		// blockId 固定住，对 blockId 的左或者右拧动
		// 旋转后锚点要改变
		// 如果关于左边旋转
		// 中心块假如被转就对全体逆转一次
		Vec3 XA, YA;
		if (isLeft)
		{
			XA = (blocks[blockId].surface[left_1].vertex[1].pos - blocks[blockId].surface[left_1].vertex[0].pos).GetUnitVector();
			YA = (blocks[blockId].surface[left_1].vertex[2].pos - blocks[blockId].surface[left_1].vertex[0].pos).GetUnitVector();
			for (int i = blockId - 1; i >= 0; i--)
			{
				blocks[i].Rotate(XA, YA, angle);
				blocks[i].anchor = RotateVec3(blocks[i].anchor, XA, YA, angle);
			}
		}
		// 如果关于右边旋转
		else
		{
			XA = (blocks[blockId].surface[right_1].vertex[1].pos - blocks[blockId].surface[right_1].vertex[0].pos).GetUnitVector();
			YA = (blocks[blockId].surface[right_1].vertex[2].pos - blocks[blockId].surface[right_1].vertex[0].pos).GetUnitVector();
			for (int i = blockId + 1; i < blocks.size(); i++)
			{
				blocks[i].Rotate(XA, YA, angle);
				blocks[i].anchor = RotateVec3(blocks[i].anchor, XA, YA, angle);
			}
		}
	}
	MagicRuler TempRotate(int blockId, bool isLeft, double angle)
	{
		MagicRuler result;
		result.blocks = this->blocks;
		result.centerBlock = blockId;
		Vec3 pos = this->blocks[this->centerBlock].pos;
		if (blockId < this->centerBlock)
		{
			for (int i = this->centerBlock - 1; i >= blockId; i--)
			{
				pos = pos - (this->blocks[i].surface[front_1].vertex[1].pos - this->blocks[i].surface[front_1].vertex[0].pos) / 2;
			}
		}
		else
		{
			for (int i = this->centerBlock; i < blockId; i++)
			{
				pos = pos + (this->blocks[i].surface[front_1].vertex[1].pos - this->blocks[i].surface[front_1].vertex[0].pos) / 2;
			}
		}
		result.pos = pos;
		Vec3 XA, YA;
		if (isLeft)
		{
			XA = (blocks[blockId].surface[left_1].vertex[1].pos - blocks[blockId].surface[left_1].vertex[0].pos).GetUnitVector();
			YA = (blocks[blockId].surface[left_1].vertex[2].pos - blocks[blockId].surface[left_1].vertex[0].pos).GetUnitVector();
			for (int i = blockId - 1; i >= 0; i--)
			{
				result.blocks[i].Rotate(XA, YA, angle);
				result.blocks[i].anchor = RotateVec3(blocks[i].anchor, XA, YA, angle);
			}
		}
		// 如果关于右边旋转
		else
		{
			XA = (blocks[blockId].surface[right_1].vertex[1].pos - blocks[blockId].surface[right_1].vertex[0].pos).GetUnitVector();
			YA = (blocks[blockId].surface[right_1].vertex[2].pos - blocks[blockId].surface[right_1].vertex[0].pos).GetUnitVector();
			for (int i = blockId + 1; i < blocks.size(); i++)
			{
				result.blocks[i].Rotate(XA, YA, angle);
				result.blocks[i].anchor = RotateVec3(blocks[i].anchor, XA, YA, angle);
			}
		}
		return result;
	}
};

// 得到三维空间中的点在投影面上的投影
Vec2 GetProjectInSurface(Vec3 project, Vec3 X_Across, Vec3 Y_Across, Vec2 pericenter)	// 这个是透视投影
{
	Vec3 Z_Acoss = X_Across.MultiplicationCross(Y_Across).GetUnitVector() * ProjectSurfaceDis;
	Vec3 tVec = Z_Acoss - project;
	double times = ProjectSurfaceDis / 2 / (tVec * Z_Acoss / ProjectSurfaceDis);
	return pericenter +
		Vec2(project.GetLength() * project.GetCosBetween(X_Across),
			project.GetLength() * project.GetCosBetween(Y_Across)) * abs(times);
}

// 获取 3 维点的深度
double GetDepth(Vec3 pos, Vec3 ZAcross, double cameraLength)
{
	double ori = pos * ZAcross / ZAcross.GetLength();
	return cameraLength - ori;
}

// 将 3 维顶点转化为 2 为顶点
Vertex2D TransformV3DToV2D(Vertex3D vertex, Vec3 XAcross, Vec3 YAcross, double dis, Vec2 pericenter)
{
	return Vertex2D(GetProjectInSurface(vertex.pos, XAcross, YAcross, pericenter), vertex.R, vertex.G, vertex.B,
		GetDepth(vertex.pos, XAcross.MultiplicationCross(YAcross), dis));
}

// 将 3 维三角形转化为 2 维三角形
Triangle2D TransformT3DToT2D(Triangle3D tri, Vec3 XAcross, Vec3 YAcross, double dis, Vec2 pericenter)
{
	Triangle2D result;
	for (int i = 0; i < 3; i++)
		result.vertex[i] = TransformV3DToV2D(tri.vertex[i], XAcross, YAcross, dis, pericenter);
	return result;
}

// 鼠标横向拖动时 X 轴，Y 轴关于 Y 轴旋转
Vec3 HorizontalRotate(Vec3 X_Across, Vec3 Y_Across, double angle)
{
	Vec3 Z_Across = X_Across.MultiplicationCross(Y_Across);
	X_Across = X_Across * cos(angle) + Z_Across * sin(angle);
	return X_Across.GetUnitVector();
}

// 鼠标竖向拖动时 X 轴，Y 轴关于 X 轴旋转
Vec3 VerticalRotate(Vec3 X_Across, Vec3 Y_Across, double angle)
{
	Vec3 Z_Across = X_Across.MultiplicationCross(Y_Across);
	Y_Across = Y_Across * cos(angle) + Z_Across * sin(angle);
	return Y_Across.GetUnitVector();
}

// 绘画三维三角形
void DrawTriangle3D(Triangle3D tri, Vec3 XAcross, Vec3 YAcross, double dis, Vec2 pericenter, double* DepthMap, int w, int h)
{
	Vec3 Z_Across = XAcross.MultiplicationCross(YAcross);
	Vec3 normalVec = (tri.vertex[1].pos - tri.vertex[0].pos).MultiplicationCross(tri.vertex[2].pos - tri.vertex[0].pos);
	if (Z_Across * normalVec > 0)
		TransformT3DToT2D(tri, XAcross, YAcross, dis, pericenter).Draw(DepthMap, w, h);
}

// 画魔尺块
void DrawBlockMagicRuler(BlockMagicRuler& bmr, Vec3 XAcross, Vec3 YAcross, double dis, Vec2 pericenter, double* DepthMap, int w, int h)
{
	Triangle3D temp;
	for (int i = 0; i < 8; i++)
	{
		temp.vertex[0] = Vertex3D(bmr.surface[i].vertex[0].pos - bmr.anchor + bmr.pos, bmr.surface[i].vertex[0].R, bmr.surface[i].vertex[0].G,
			bmr.surface[i].vertex[0].B);
		temp.vertex[1] = Vertex3D(bmr.surface[i].vertex[1].pos - bmr.anchor + bmr.pos, bmr.surface[i].vertex[1].R, bmr.surface[i].vertex[1].G,
			bmr.surface[i].vertex[1].B);
		temp.vertex[2] = Vertex3D(bmr.surface[i].vertex[2].pos - bmr.anchor + bmr.pos, bmr.surface[i].vertex[2].R, bmr.surface[i].vertex[2].G,
			bmr.surface[i].vertex[2].B);
		DrawTriangle3D(temp, XAcross, YAcross, dis, pericenter, DepthMap, w, h);
	}
}

// 画魔尺
void DrawMagicRuler(MagicRuler& mr, Vec3 XAcross, Vec3 YAcross, double dis, Vec2 pericenter, double* DepthMap, int w, int h)
{
	// 以中心块开始，计算其他所有块的位置
	Vec3 pos = mr.pos;
	for (int i = mr.centerBlock; i < mr.blocks.size(); i++)
	{
		BlockMagicRuler temp = mr.blocks[i];
		temp.pos = pos;
		pos = pos + (temp.surface[front_1].vertex[1].pos - temp.surface[front_1].vertex[0].pos) / 2;
		DrawBlockMagicRuler(temp, XAcross, YAcross, dis, pericenter, DepthMap, w, h);
	}
	pos = mr.pos;
	for (int i = mr.centerBlock - 1; i >= 0; i--)
	{
		BlockMagicRuler temp = mr.blocks[i];
		pos = pos - (temp.surface[front_1].vertex[1].pos - temp.surface[front_1].vertex[0].pos) / 2;
		temp.pos = pos;
		DrawBlockMagicRuler(temp, XAcross, YAcross, dis, pericenter, DepthMap, w, h);
	}
}

// 判断是否在魔尺内
double isInBlockMagicRuler(BlockMagicRuler& bmr, Vec3 XAcross, Vec3 YAcross, double dis, Vec2 pericenter, Vec2 pos)
{
	Vec3 ZAcross = XAcross.MultiplicationCross(YAcross);
	double minDepth = -1;
	for (int i = 0; i < 8; i++)
	{
		Vec3 normalVec = (bmr.surface[i].vertex[1].pos - bmr.surface[i].vertex[0].pos).
			MultiplicationCross(bmr.surface[i].vertex[2].pos - bmr.surface[i].vertex[0].pos);
		if (ZAcross * normalVec > 0)
		{
			Triangle2D tri = TransformT3DToT2D(bmr.surface[i], XAcross, YAcross, dis, pericenter);
			if (tri.isInTriangle(pos))
			{
				if (minDepth != -1)
					minDepth = min(minDepth, tri.GetDepth(pos));
				else
					minDepth = tri.GetDepth(pos);
			}
		}
	}
	return minDepth;
}

// 获得选中的魔尺块下标
int GetSelectBlock(MagicRuler& mr, Vec3 XAcross, Vec3 YAcross, double dis, Vec2 pericenter, Vec2 point)
{
	int result = -1;
	double minDepth = -1;
	Vec3 pos = mr.pos;
	for (int i = mr.centerBlock; i < mr.blocks.size(); i++)
	{
		BlockMagicRuler temp = mr.blocks[i];
		temp.pos = pos;
		for (int i = 0; i < 8; i++)
		{
			temp.surface[i].vertex[0].pos = temp.surface[i].vertex[0].pos - temp.anchor + temp.pos;
			temp.surface[i].vertex[1].pos = temp.surface[i].vertex[1].pos - temp.anchor + temp.pos;
			temp.surface[i].vertex[2].pos = temp.surface[i].vertex[2].pos - temp.anchor + temp.pos;
		}
		pos = pos + (temp.surface[front_1].vertex[1].pos - temp.surface[front_1].vertex[0].pos) / 2;
		double tempDepth = isInBlockMagicRuler(temp, XAcross, YAcross, dis, pericenter, point);
		if (tempDepth > 0 && (minDepth < 0 || tempDepth < minDepth))
		{
			minDepth = tempDepth;
			result = i;
		}
	}
	pos = mr.pos;
	for (int i = mr.centerBlock - 1; i >= 0; i--)
	{
		BlockMagicRuler temp = mr.blocks[i];
		pos = pos - (temp.surface[front_1].vertex[1].pos - temp.surface[front_1].vertex[0].pos) / 2;
		temp.pos = pos;
		for (int i = 0; i < 8; i++)
		{
			temp.surface[i].vertex[0].pos = temp.surface[i].vertex[0].pos - temp.anchor + temp.pos;
			temp.surface[i].vertex[1].pos = temp.surface[i].vertex[1].pos - temp.anchor + temp.pos;
			temp.surface[i].vertex[2].pos = temp.surface[i].vertex[2].pos - temp.anchor + temp.pos;
		}
		double tempDepth = isInBlockMagicRuler(temp, XAcross, YAcross, dis, pericenter, point);
		if (tempDepth > 0 && (minDepth < 0 || tempDepth < minDepth))
		{
			minDepth = tempDepth;
			result = i;
		}
	}
	return result;
}

// 主函数
int main()
{
	Vec3 X_Across(1, 0, 0), Y_Across(0, 1, 0);
	initgraph(WIDTH, HEIGHT);
	BeginBatchDraw();
	MagicRuler mr(24, 50, BLUE, Vec3(0, 0, 0));
	MagicRuler tempMr;
	double* DepthMap = new double[WIDTH * HEIGHT];
	memset(DepthMap, 0, sizeof(double) * WIDTH * HEIGHT);
	bool isExit = false;
	bool isLPress = false;
	bool isRPress = false;
	Vec2 ori_L, ori_R;
	Vec2 rotateCenter;
	int selectBlock = -1;
	ExMessage msg;
	while (!isExit)
	{
		while (peekmessage(&msg, EM_KEY | EM_MOUSE))
		{
			if (msg.message == WM_KEYDOWN)
			{
				if (msg.vkcode == VK_ESCAPE) isExit = true;
			}
			else if (msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN || msg.message == WM_LBUTTONUP ||
				msg.message == WM_RBUTTONDOWN || msg.message == WM_RBUTTONUP)
			{
				if (!isLPress && msg.lbutton)
				{
					// 检测是否点到魔尺块
					ori_L = Vec2(msg.x, msg.y);
					selectBlock = GetSelectBlock(mr, X_Across, Y_Across, ProjectSurfaceDis, Vec2(WIDTH / 2, HEIGHT / 2), ori_L);
					if (selectBlock != -1)
					{
						tempMr = mr;
					}
					isLPress = true;
				}
				else if (isLPress && msg.lbutton)
				{
					Vec2 next = Vec2(msg.x, msg.y);
					if (selectBlock == -1)
					{
						ori_L = next - ori_L;
						double Th = 0;
						double Fi = 0;
						Th = ori_L.xx / GAMEPAD * PI / 3;
						Fi = ori_L.yy / GAMEPAD * PI / 3;
						X_Across = HorizontalRotate(X_Across, Y_Across, Th);
						Y_Across = VerticalRotate(X_Across, Y_Across, Fi);
						ori_L = next;
					}
					else
					{
						Vec2 extent = next - ori_L;
						double angle = extent.yy / GAMEPAD * PI / 3;
						tempMr = mr.TempRotate(selectBlock, true, angle);
					}
				}
				else if (isLPress && !msg.lbutton)
				{
					if (selectBlock != -1)
					{
						Vec2 next = Vec2(msg.x, msg.y);
						Vec2 extent = next - ori_L;
						double angle = extent.yy / GAMEPAD * PI / 3;
						int k = (int)(angle / (PI * 2) + 1) - 1;
						angle -= k * 2 * PI;
						if (angle > PI / 3 && angle < PI * 2 / 3)
						{
							mr.Rotate(selectBlock, true, PI / 2);
						}
						else if (angle > PI * 5 / 6 && angle < PI * 7 / 6)
						{
							mr.Rotate(selectBlock, true, PI);
						}
						else if (angle > PI * 4 / 3 && angle < PI * 5 / 3)
						{
							mr.Rotate(selectBlock, true, PI * 3 / 2);
						}
					}
					isLPress = false;
					selectBlock = -1;
				}
				else if (!isRPress && msg.rbutton)
				{
					ori_R = Vec2(msg.x, msg.y);
					selectBlock = GetSelectBlock(mr, X_Across, Y_Across, ProjectSurfaceDis, Vec2(WIDTH / 2, HEIGHT / 2), ori_R);
					if (selectBlock != -1)
					{
						tempMr = mr;
					}
					isRPress = true;
				}
				else if (isRPress && msg.rbutton)
				{
					if (selectBlock != -1)
					{
						Vec2 next(msg.x, msg.y);
						Vec2 extent = next - ori_L;
						double angle = extent.yy / GAMEPAD * PI / 3;
						tempMr = mr.TempRotate(selectBlock, false, angle);
					}
				}
				else if (isRPress && !msg.rbutton)
				{
					if (selectBlock != -1)
					{
						Vec2 next = Vec2(msg.x, msg.y);
						Vec2 extent = next - ori_L;
						double angle = extent.yy / GAMEPAD * PI / 3;
						int k = (int)(angle / (PI * 2) + 1) - 1;
						angle -= k * 2 * PI;
						if (angle > PI / 3 && angle < PI * 2 / 3)
						{
							mr.Rotate(selectBlock, false, PI / 2);
						}
						else if (angle > PI * 5 / 6 && angle < PI * 7 / 6)
						{
							mr.Rotate(selectBlock, false, PI);
						}
						else if (angle > PI * 4 / 3 && angle < PI * 5 / 3)
						{
							mr.Rotate(selectBlock, false, PI * 3 / 2);
						}
					}
					isRPress = false;
					selectBlock = -1;
				}
			}
		}
		cleardevice();
		memset(DepthMap, 0, sizeof(double) * WIDTH * HEIGHT);
		if (selectBlock == -1)
			DrawMagicRuler(mr, X_Across, Y_Across, ProjectSurfaceDis, Vec2(WIDTH / 2, HEIGHT / 2), DepthMap, WIDTH, HEIGHT);
		else
			DrawMagicRuler(tempMr, X_Across, Y_Across, ProjectSurfaceDis, Vec2(WIDTH / 2, HEIGHT / 2), DepthMap, WIDTH, HEIGHT);
		FlushBatchDraw();
	}
	delete[] DepthMap;
	return 0;
}
