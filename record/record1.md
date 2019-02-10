光线追踪器的核心是把光线通过像素点，计算那些在那些光照射下的颜色；这是计算哪条光线从眼睛到像素的形式，计算光线相交，计算交点的颜色

把相机位置设定在(0,0,0)处，y轴向上，x轴向右，沿着屏幕向里的是z轴负半轴

<hr>

与球面相交光线

```cpp
/* 下面这个函数hit_sphere的由来：
 * 设球心为点C，球面上点为p，则球面公式为dot((p-C),(p-C)) = R^2
 * 设光源为A，朝向为B，则光线可以写成p(t) = A + tB的形式
 * 光线与球面相交就等于dot( (p(t) - C ),(p(t) - C) ) = R^2
 * 代入p(t)得：dot( (A + tB - C), (A + tB - C) ) = R^2
 * 展开方程得 t^2 * dot(B,B) + t * 2*dot(B,A-C) + (dot(A-C,A-C) - R^2) = 0
 * 其中，t是未知数，如果这个方程两个根就是光线跟球面有前后两个交点，1个根就是相切，0个根就是没有交点
 * */
bool hit_sphere(const vec3& center, float radius, const ray& r)
{
	vec3 oc = r.origin() - center;//光源与球心的连线，即上面的A-C

	float a = dot(r.direction(),r.direction());
	float b = 2.0 * dot(oc,r.direction());
	float c = dot(oc,oc) - radius * radius;
	float discriminant = b*b - 4*a*c;//求根公式
	return (discriminant > 0)
}

vec3 color(const ray &r)
{
	if (hit_sphere(vec3(0, 0, -1), 0.5, r))
		return vec3(1, 0, 0);//如果光线击中球体，该位置就是发红光，否则进行背景混合
	vec3 unit_direction = unit_vector(r.direction());//归一化成单位坐标
	float t = 0.5 * (unit_direction.y() + 1.0f);//全部变成正数方便混色，t=1时变成blue，t=0时变成white
	return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);//就是混色操作，类似线性插值
}
```

<hr>

表面法线

对于球体来说，表面法线就是球心指向球表面点的连线，改动上面的代码就可以：

```cpp
/* 下面这个函数hit_sphere的由来：
 * 设球心为点C，球面上点为p，则球面公式为dot((p-C),(p-C)) = R^2
 * 设光源为A，朝向为B，则光线可以写成p(t) = A + tB的形式
 * 光线与球面相交就等于dot( (p(t) - C ),(p(t) - C) ) = R^2
 * 代入p(t)得：dot( (A + tB - C), (A + tB - C) ) = R^2
 * 展开方程得 t^2 * dot(B,B) + t * 2*dot(B,A-C) + (dot(A-C,A-C) - R^2) = 0
 * 其中，t是未知数，如果这个方程两个根就是光线跟球面有前后两个交点，1个根就是相切，0个根就是没有交点
 * */
float hit_sphere(const vec3 &center, float radius, const ray &r)
{
	vec3 oc = r.origin() - center;//光源与球心的连线，即上面的A-C

	float a = dot(r.direction(), r.direction());
	float b = 2.0 * dot(oc, r.direction());
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4 * a * c;//求根公式中δ校验
	if (discriminant < 0)
		return -1.0;
	else
		return (-b - sqrt(discriminant)) / (2.0f * a);//求根公式计算出t值，取小的那个根（更加靠近光源，也就是第一次交点）
}

vec3 color(const ray &r)
{
	float t = hit_sphere(vec3(0, 0, -1), 0.5, r);//球心为(0,0,-1)，半径为0.5，光线是从(0,0,0)射向z=-1平面的每个像素点
	if (t > 0.0)
	{
		//与球面相交光线和光源指向球心的向量作差，得到球心指向球面的向量（这也就是球体表面法线N）
		vec3 N = unit_vector(r.point_at_parameter(t) - (vec3(0, 0, -1) - vec3(0, 0, 0)));
		return 0.5 * vec3(N.x() + 1, N.y() + 1, N.z() + 1);//N从[-1,1]->[0,1]范围
	}
	vec3 unit_direction = unit_vector(r.direction());//归一化成单位坐标
	t = 0.5 * (unit_direction.y() + 1.0f);//全部变成正数方便混色，t=1时变成blue，t=0时变成white
	return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);//就是混色操作，类似线性插值
}
```

<hr>

多物体

我们可以通过OOP的思想，对上述代码进行重构。

首先定义出抽象基类`hitable.h`，这个抽象基类定义了一个`hit`的接口，分别是光线r，两个交点的t值，还有一个`hit_record`，记录了了光线的击中球体时的各种数据

```cpp
struct hit_record
{
	float t;//击中时的t值
	vec3 p;//击中点的光线
	vec3 normal;//击中点的表面法线（归一化后）
};

class hitable
{
public:
	virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const = 0;
};
```

接下来是继承了`hitable.h`这个抽象基类的`sphere.h`，定义了球体的各个性质，定义了球体的球心和半径，此外就是是否被hit的判定

```cpp
class sphere: public hitable  {
public:
	sphere() {}
	sphere(vec3 cen, float r) : center(cen), radius(r)  {};
	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	vec3 center;
	float radius;
};

bool sphere::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
	vec3 oc = r.origin() - center;
	float a = dot(r.direction(), r.direction());
	float b = dot(oc, r.direction());//注意原来这里是b = 2 * dot(oc, r.direction()); 这个2提出来和4ac一起拿到根号外了
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - a * c;//这里不是b^2 - 4ac，原因见上
	if (discriminant > 0)
	{
		float temp = (-b - sqrt(discriminant)) / a;//依旧是交点t的求根公式；变形是因为分子提出了2，所以与分母的2a中的2约去
		if (temp < t_max && temp > t_min)//两个解中t小的那个光线击中了球面
		{
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center) / radius;
			return true;
		}
		temp = (-b + sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min)//判定两个解大的那个光线是否击中球面
		{
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center) / radius;
			return true;
		}
	}
	return false;
}
```

接下来是另一个继承了`hitable.h`这个抽象基类的`hitable_list.h`

```cpp
class hitable_list: public hitable  {
public:
	hitable_list() {}
	hitable_list(hitable **l, int n) {list = l; list_size = n; }
	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	hitable **list;
	int list_size;
};

bool hitable_list::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
	hit_record temp_rec;
	bool hit_anything = false;
	double closest_so_far = t_max;
	for (int i = 0; i < list_size; i++)
	{
		//如果成功，temp_rec将会被刷新成包含击中的t值，击中点的光线矢量，击中点的表面法线组成的rec
		if (list[i]->hit(r, t_min, closest_so_far, temp_rec))
		{
			hit_anything = true;//确认这里是被光线击中，要被渲染的
			
			//刷新成返回点的t值，供下一个球体作为t_max值使用
			//不然后续的球体渲染得到的t会直接覆盖前面球体的t结果，导致前后渲染顺序出错
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}
	return hit_anything;
}
```

最后重写一下`main.cpp`的代码

```cpp
vec3 color(const ray &r, hitable *world)
{
	hit_record rec;
	if (world->hit(r, 0.0, MAXFLOAT, rec))//确认是否在这个像素点上有光线跟任意球体碰撞
		return 0.5 * vec3(rec.normal.x() + 1, rec.normal.y() + 1, rec.normal.z() + 1);//表面法线[-1,1]->[0,1]化
	else
	{
		vec3 unit_direction = unit_vector(r.direction());//归一化成单位坐标
		float t = 0.5 * (unit_direction.y() + 1.0f);//全部变成正数方便混色，t=1时变成blue，t=0时变成white
		return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);//就是混色操作，类似线性插值
	}
}

int main()
{
	//画面是200*100
	int nx = 200;
	int ny = 100;

	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	vec3 lower_left_corner(-2.0f, -1.0f, -1.0f);//画面左下角
	vec3 horizontal(4.0f, 0.0f, 0.0f);//画面水平长度
	vec3 vertical(0.0f, 2.0f, 0.0f);//画面垂直长度
	vec3 origin(0.0f, 0.0f, 0.0f);//光线原点

	//创建两个球体，list[i]这个指针从基类hitable指向派生类sphere
	hitable *list[2];
	list[0] = new sphere(vec3(0, 0, -1), 0.5);//原点(0,0,-1)，半径为0.5
	list[1] = new sphere(vec3(0, -100.5, -1), 100);//原点(0,-100,-1)，半径为100.5

	//创建一个list，可以对整个球体的list进行逐一检查
	hitable *world = new hitable_list(list, 2);

	for (int j = ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < nx; i++)
		{
			float u = float(i) / float(nx);
			float v = float(j) / float(ny);

			// 光线从原点射向z = -1的平面的每一个点
			ray r(origin, lower_left_corner + u * horizontal + v * vertical);

			//vec3 p = r.point_at_parameter(2.0);
			vec3 col = color(r, world);
			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}
}
```

<hr>

抗锯齿

边缘的像素点应该是前景色和背景色的混合；如果一定要严格分层将会大大增加代码的复杂度，所以暂时不进行分层的方法抗锯齿

此外，我们还需要一个随机数生成器生成出`[0,1)`范围的随机数

这里，我们重构了`camera`类，同时加入了反复采样和周围像素混合的方法进行抗锯齿

```cpp
class camera{
public:
	camera()
	{
		lower_left_corner = vec3(-2.0f, -1.0f, -1.0f);//画面左下角
		horizontal = vec3(4.0f, 0.0f, 0.0f);//画面水平长度
		vertical = vec3(0.0f, 2.0f, 0.0f);//画面垂直长度
		origin = vec3(0.0f, 0.0f, 0.0f);//光线原点
	}

	ray get_ray(float u, float v)// 光线从光源点射向z = -1的平面的每一个点
	{
		return ray(origin, lower_left_corner + u * horizontal + v * vertical - origin);
	}

	vec3 origin;
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
};
```

`main.cpp`

```cpp
int main()
{
	//画面是200*100
	int nx = 200;
	int ny = 100;
	int ns = 100;//对一个像素点重复采样进行抗锯齿

	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	//创建两个球体，list[i]这个指针从基类hitable指向派生类sphere
	hitable *list[2];
	list[0] = new sphere(vec3(0, 0, -1), 0.5);//原点(0,0,-1)，半径为0.5
	list[1] = new sphere(vec3(0, -100.5, -1), 100);//原点(0,-100,-1)，半径为100.5

	//创建一个list，可以对整个球体的list进行逐一检查
	hitable *world = new hitable_list(list, 2);

	//建立一个照相机
	camera cam;

	for (int j = ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < nx; i++)
		{
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++)//通过ns次的模糊化后，进行抗锯齿
			{
				//drand48生成一个[0,1)之间的double
				float u = float(i + drand48()) / float(nx);
				float v = float(j + drand48()) / float(ny);
				ray r = cam.get_ray(u, v);
				vec3 p = r.point_at_parameter(2.0);
				col += color(r, world);
			}
			col /= float(ns);

			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}
}
```

<hr>

反射材质