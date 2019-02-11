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

漫反射材质

不发光的漫射物体仅仅呈现周围环境的颜色，他们用自己的内在颜色调制它；所以我们如果使用多条光线在两个漫反射表面中间的弧线中时，它们很可能会有完全不同的随机的反射路线

它们有可能被表面完全吸收了（表面颜色越深代表吸收越强）；实际上任何随机化方向的算法都会产生看起来很粗糙的表面，最简单的方法之一就是理想的漫反射表面

简单来说，我们的做法是：从与命中点相切的单位半径球中选择一个随机点s，让光线方向变成那个方向，光源射出点是原来那根光线与表面的交点，再次进行追踪判断是否hit（递归调用）

我们应该忽视一些t很小的值，因为这些光线就等于没有射出，而保留会大大增加计算开销，同时会显著增加阴影的黑点，导致出现一些局部黑斑的问题

```cpp
//找到一个在单位球内的随机点
vec3 random_in_unit_sphere()
{
	vec3 p;
	do
	{
		p = 2.0 * vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);//得到一个值为[-1,1)的vec
	} while (p.squared_length() >= 1.0);
	return p;
}

vec3 color(const ray &r, hitable *world)
{
	hit_record rec;
	//确认是否在这个像素点上有光线跟任意球体碰撞
	//因为在两个球体相接触的位置，光线的一些t很小，我们应该忽视一些t几乎等于0的hit
	if (world->hit(r, 0.001, MAXFLOAT, rec))//tmin应该从0.0->0.0001，消除阴影暗斑影响
	{
		//反射的光线射向切点单位球内的某一方向（随机）
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();//找到切点单位球内一个随机点坐标
		return 0.5 * color(ray(rec.p, target - rec.p), world);//递归调用color，追踪反射的光线是否击中其他object，这里0.5代表反射后的光线强度只有原来的一半
	}
	else
	{
		vec3 unit_direction = unit_vector(r.direction());//归一化成单位坐标
		float t = 0.5 * (unit_direction.y() + 1.0f);//全部变成正数方便混色，t=1时变成blue，t=0时变成white
		return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);//就是混色操作，类似线性插值
	}
}
```

<hr>

金属质感

我们可能需要不同的object有不同的材质，我们应该建立一个抽象基类，漫反射与镜面反射都要继承这个抽象基类的接口

```cpp
class material
{
public:
	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const = 0;
};
```

漫反射上面已经完成，只需要简单的重构：

```cpp
vec3 random_in_unit_sphere() {
	vec3 p;
	do {
		p = 2.0*vec3(drand48(),drand48(),drand48()) - vec3(1,1,1);
	} while (p.squared_length() >= 1.0);
	return p;
}

//漫反射
class lambertian : public material
{
public:
	lambertian(const vec3 &a) : albedo(a){}

	//入射光，hit点的的记录，衰减，散射
	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
	{
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();//进行漫反射随机化反射方向
		scattered = ray(rec.p, target - rec.p);//散射光线
		attenuation = albedo;
		return true;
	}

private:
	vec3 albedo;//反射率
};
```

接下来考虑镜面反射，镜面反射遵循入射角=反射角的定律，所以，设入射光线为<b>v</b>，交点法线为<b>N</b>，反射光线为<b>v'</b>，可得<b>v‘</b> = <b>v</b> - 2 * dot(<b>v</b>, <b>N</b>) * <b>N</b>（减法是因为这两个矢量呈钝角，点积结果为负）

```cpp
//镜面反射的反射光线方向
vec3 reflect(const vec3 &v, const vec3 &n){
	return v - 2 * dot(v, n) * n;
}

class metal : public material
{
public:
	metal(const vec3 &a) : albedo(a) {}

	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
	{
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);//计算出反射光线方向
		scattered = ray(rec.p, reflected);
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);//反射光线与法线呈锐角，证明散射成功
	}

private:
	vec3 albedo;
};
```

此外，更改`sphere`和`hit_record`这两个类的参数，增加一个参数`mat_ptr`，指向`material`基类，用来给出球体的材质；此外，`sphere`在`hit`之后的渲染中在`rec`存储`t,p,normal`之外还要存储`mat_ptr`

我们还可以对镜面反射的光线进行一个随机的微小偏移，使得更加符合真实感，方法是在`metal`类中增加一个`fuzz`（模糊）的参数，现实中，越大的球体，对于反射光的`fuzz`也就会越大，但是有一个问题是对于大球体或者grazing rays，我们可能会把散射射到表面以下，我们应该在表面就吸收这些光，我们设定最大值为1的半径的球体进行模糊

```cpp
class metal : public material{
public:
	metal(const vec3 &a, float f) : albedo(a)
	{
		if (f < 1)
			fuzz = f;
		else
			fuzz = 1;
	}

	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
	{
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);//计算出反射光线方向
		scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);//反射光线与法线呈锐角，证明成功
	}

private:
	vec3 albedo;//反射率
	float fuzz;//反射光线模糊率[0,1]，传入为0即为不进行模糊操作
};
```

<hr>

折射介质

不同的介质有不同的折射率，当光线击中这些object的时候，会分离成反射光线和折射光线。我们将会从折射光线与反射光线中随机选择，而且每次交互将只产生一条散射光线（散射=反射+折射 的统称）

最艰难的部分是去debug折射光线，技巧是：通常情况下，如果在有折射光线存在的情况下，首先让所有的光线都折射

`Snell`折射定律：`nsin(θ) = n'sin(θ')`(n是折射率，空气是1，glass是1.3~1.7，diamond是2.4）

需要注意一点：光学性质中，当光从高折射率的介质射向低折射率的介质时，可能会出现全反射的情况（当入射角大于某一个特殊值时）

```cpp
//折射，如果出现折射，折射的光线通过refracted引用返回
bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) {
	vec3 uv = unit_vector(v);//入射光线方向归一化
	float dt = dot(uv, n);//入射光线与法线作点积，得到结果即为-cosC，C为入射角（点积结果恒为负值）
	float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);//判定是否发生全反射
	if (discriminant > 0)//未发生全反射
	{
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);//得到折射光线方向
		return true;
	}
	else//全反射
		return false;
}

//折射介质
class dielectric : public material {
public:
	dielectric(float ri) : ref_idx(ri) {}

	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
	{
		vec3 outward_normal;//建立一个与入射光线恒为钝角的法线
		vec3 reflected = reflect(r_in.direction(), rec.normal);//计算反射光线方向
		float ni_over_nt;//即为ni/nt（ni是入射光原来的介质，nt是要进入的介质）
		attenuation = vec3(1.0, 1.0, 1.0);//这里表面是glass，即设定折射界面不会吸收光线
		vec3 refracted;//保存计算得到的折射光线的方向

		//让outward_normal始终是介质法线，与入射光线呈钝角；确定折光率n之比（ni是入射光原来的介质，nt是要进入的介质）
		if (dot(r_in.direction(), rec.normal) > 0)//从球体内射出（光密射光疏，可能全反射）
		{
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;
		}
		else//从球体外射入（光密射光密，不可能有全反射）
		{
			outward_normal = rec.normal;
			ni_over_nt = 1.0 / ref_idx;//恒<1，代入下面计算可知一定不可能会全反射
		}

		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
			scattered = ray(rec.p, refracted);
		else
		{
			scattered = ray(rec.p, refracted);
			return false;
		}
		return true;
	}

private:
	float ref_idx;//球体材质折光率，一般恒>1
};
```

现在出现了一个问题，折射材质的那个球只有一部分被渲染出来了（如果你正确执行了上面的代码），这是因为缺少了反射光线在这其中的定义，导致其只有折射，没有反射的效果

真正的玻璃还有一个特别的性质：随角度变化的反射率。这部分的方程非常复杂，但是这里使用Christophe Schick提出的一个简单的多项式进行近似：

```cpp
//使得折射介质能够随着角度的不同有不同的折射率
float schlick(float cosine, float ref_idx){
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

//折射介质
class dielectric : public material {
public:
	dielectric(float ri) : ref_idx(ri) {}

	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
	{
		vec3 outward_normal;//建立一个与入射光线恒为钝角的法线
		vec3 reflected = reflect(r_in.direction(), rec.normal);//计算反射光线方向
		float ni_over_nt;//即为ni/nt（ni是入射光原来的介质，nt是要进入的介质）
		attenuation = vec3(1.0, 1.0, 1.0);//这里表面是glass，即设定折射界面不会吸收光线
		vec3 refracted;//保存计算得到的折射光线的方向

		float reflect_prob;
		float cosine;

		//让outward_normal始终是介质法线，与入射光线呈钝角；确定折光率n之比（ni是入射光原来的介质，nt是要进入的介质）
		if (dot(r_in.direction(), rec.normal) > 0)//从球体内射出（光密射光疏，可能全反射）
		{
			outward_normal = -rec.normal;
			ni_over_nt = ref_idx;

			cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();
			cosine = sqrt(1 - ref_idx * ref_idx * (1 - cosine * cosine));
		}
		else//从球体外射入（光密射光密，不可能有全反射）
		{
			outward_normal = rec.normal;
			ni_over_nt = 1.0 / ref_idx;//恒<1，代入下面计算可知一定不可能会全反射

			cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
		}
		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
			reflect_prob = schlick(cosine, ref_idx);
		else
			reflect_prob = 1.0;
		if (drand48() < reflect_prob)
			scattered = ray(rec.p, reflected);
		else
			scattered = ray(rec.p, refracted);
		return true;
	}

private:
	float ref_idx;//球体材质折光率，一般恒>1
};
```

其中有一点非常有意思，就是如果你给一个折射介质以负数的半径，那么它的几何形状将不受影响，但表面法线会指向内部，因此它可以用作气泡来制作中空玻璃球

```cpp
	//创建两个球体，list[i]这个指针从基类hitable指向派生类sphere
	hitable *list[5];
	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));//原点(0,0,-1)，半径为0.5，漫反射材质
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2),0.0));//原点(1,0,-1)，半径为0.5，镜面反射材质，模糊度0
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
	list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
	
	//创建一个list，可以对整个球体的list进行逐一检查
	hitable *world = new hitable_list(list, 5);
```