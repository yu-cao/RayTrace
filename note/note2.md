RayTracer第二部分

<hr>

动态模糊

当你应用光线追踪技术的时候，也就意味着你认为视觉质量比渲染时间更为重要。在第一部分的fuzzy反射和失焦模糊中，我们的做法是对每个像素点进行多次反复采样。这是一种暴力硬算的方法，但可以实现所有的效果，动态模糊也是其中之一。**在一个真实的相机中，快门打开在一段时间间隔中，而这段时间内相机和物体都可能出现移动的情况，它实际上是相机在我们想要的间隔上看到的平均值。**

我们能够通过在快门打开的时间段内的随机地把每个光线发射达到随机地消除的目的。只要object在那个时刻出现在它应该在的位置，通过光线在一个确定的时间，我们就能够得到一个正确的平均值答案，这就是为什么为什么随机光线追踪变得简单的基础

最简单的idea是在快门打开时随机生成光线，并在此时与模型相交；这个方法通常应用于相机移动或者object移动的时候，但每根光线都只存在一次；通过这种方式，光线跟踪器的“引擎”可以确保物体是光线所需的位置，并且交叉口内径不会发生太大变化

```cpp
class ray
{
public:
	ray(){}

	ray(const vec3 &a, const vec3 &b, float ti = 0.0)
	{
		A = a;
		B = b;
		_time = ti;
	}

	vec3 origin() const
	{ return A; }

	vec3 direction() const
	{ return B; }

	float time() const
	{ return _time; }

	vec3 point_at_parameter(float t) const
	{ return A + t * B; }

private:
	vec3 A;//光源
	vec3 B;//朝向
	float _time;//光线射出的时间（相比较快门按下之后）
};
```

修改了光线的属性后，**我们需要修改相机去产生在`time1`到`time2`之间随机时间发出的光线**。我们应该让相机对于`time1`到`time2`这段时间内保持追踪，而且应该在创建光线时由摄像机用户决定，这段代码并不难改，因为相机并没有移动，它只是在一段时间内发出光线而已

```cpp
	camera(vec3 lookfrom, vec3 lookat, const vec3 vup, const float vfov, const float aspect,
		   float aperture, float focus_dist, float t0, float t1)
	{
		time0 = t0;
		time1 = t1;
		//...
	}

	ray get_ray(float s, float t)
	{
	//...
		float time = time0 + drand48() * (time1 - time0);//使得随机在[time0,time1)的时间段内产生一条光线
		return ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset, time);
	}

private:
	//...
	float time0, time1;
};
```

接下来我们需要一个移动中的物体。这个物体会在`time0`到`time1`从`center0`移动到`center1`。在这个时间段外这个物体依然移动，所以不需要配合摄像机快门的时间。

我们可以给每种物体都写上移动的功能，给静止的物体的初始位置和终点位置设置为相同的值。也可以为移动的物体单独写一个类。后者可以提高一些效率。这都取决于你的个人习惯

与之前的普通Sphere的本质区别是这里：`rec.normal = (rec.p - center(r.time())) / radius;`，原来`center`就是这个object上某个点的坐标，现在引入`r.time`使得其从`center0`向`center1`出现了偏移，偏移的距离就是我们给出的时间与`time1-time0`时间之比

```cpp
//相比较sphere额外引入cen1(time1时刻的位置)；time0和time1这3个变量；原来的cen现在被cen0(time0时刻的位置)取代
class moving_sphere : public hitable
{
public:
	moving_sphere() = default;

	moving_sphere(vec3 cen0, vec3 cen1, float t0, float t1, float r, material *m) :
			center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r), mat_ptr(m){}

	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;

	vec3 center(float time) const
	{
		return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
	}

private:
	vec3 center0, center1;
	float time0, time1;
	float radius;
	material *mat_ptr;
};

bool moving_sphere::hit(const ray &r, float t_min, float t_max, hit_record &rec) const
{
	vec3 oc = r.origin() - center(r.time());
	float a = dot(r.direction(), r.direction());
	float b = dot(oc, r.direction());
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - a * c;
	if (discriminant > 0)
	{
		float temp = (-b - sqrt(discriminant)) / a;		if (temp < t_max && temp > t_min)//两个解中t小的那个光线击中了球面
		{
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center(r.time())) / radius;//核心区别：其实就是这里的center是通过r.time
			rec.mat_ptr = mat_ptr;
			return true;
		}
		temp = (-b + sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min)//判定两个解大的那个光线是否击中球面
		{
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center(r.time())) / radius;
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}
	return false;
}
```

之后我们需要更改涉及散射的材质代码（这里我们只修改漫反射材质），使之创建新的射线时赋值此射线的时间

```cpp
class lambertian : public material
{
public:
	lambertian(const vec3 &a) : albedo(a){}

	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
	{
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();		scattered = ray(rec.p, target - rec.p, r_in.time());//引入了时间的概念
		attenuation = albedo;
		return true;
	}

private:
	vec3 albedo;//反射率
};
```

然后我们修改主场景，使得所有漫反射材质的球都从`time0`时刻的原来位置移动到`time1`时刻的`center+vec3(0,0.5*drand48(),0)`位置，相机的快门时间就是`time0`到`time1`

```cpp
hitable *random_scene(){
	int n = 500;//500个球
	hitable **list = new hitable *[n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));//大地表面背景
	int i = 1;
	for (int a = -10; a < 10; a++)
	{
		for (int b = -10; b < 10; b++)
		{
			float choose_mat = drand48();
			vec3 center(a + 0.9 * drand48(), 0.2, b + 0.9 * drand48());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9)
			{
				if (choose_mat < 0.8)
				{  // diffuse
					list[i++] = new moving_sphere(center, center + vec3(0, 0.5 * drand48(), 0), 0.0, 1.0, 0.2,
												  new lambertian(vec3(drand48() * drand48(), drand48() * drand48(),
																	  drand48() * drand48())));
				}
				else if (choose_mat < 0.95)
				{ // metal
					list[i++] = new sphere(center, 0.2,
										   new metal(vec3(0.5 * (1 + drand48()), 0.5 * (1 + drand48()),
														  0.5 * (1 + drand48())), 0.5 * drand48()));
				}
				else
				{  // glass
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new hitable_list(list, i);
}

//修改相机参数
vec3 lookfrom(12,2,3);
vec3 lookat(0,0,0);
const vec3 vup(0,1,0);
const float fov = 20.0;
float dist_to_focus = 10;//焦距长度 为对焦到lookat位置的 长度
float aperture = 0.0;//光圈（透镜）大小
camera cam(lookfrom, lookat, vup, fov, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);//这里time0 = 0.0 time1 = 1.0
```

<hr>

层次包围盒(BVH)

这部分是目前渲染器中最困难的部分，但是可以是渲染器更加高效，所以我们需要花上一点时间去重构之前的`hitable`部分的代码。

寻找射线与物体的交点是渲染器主要耗时的地方之一，而且时间消耗随着场景内物体的数量线性递增，但是这是一种对于一样的model进行重复的搜索，我们要做的是使用二分法对场景内的物体进行搜索，而不是遍历所有物体。

因为我们使用数以亿计的光线在同一model上，我们可以对model进行分类，然后每个光线交叉点可以进行子线性搜索，但是，如何分类呢？

+ 按空间分
+ 按物体分：更好地能够被实现而且运行速度对绝大多数model来说都较快


对一组基元的边界体积的关键思想是找到一个完全包围所有对象的盒，比如说，当你计算包含10个物体的边界球体，任何没有击中的边界球体的光线显然也会miss掉所有10个物体。如果光线击中这个边界球体，那么光线可能击中10个物体的其中之一

```py
if(光线击中了bounding object)
    return 光线是否击中了bounded object
else
    return false;
```

但是最核心的一点是：我们怎么把分割这些objects变成子集呢？我们给出这样的要求：任何的物体只能在一个盒子中，但是这些包围盒之间是可以交叉的

为了能够让这个代码可以进行子递归，我们需要设计一个bound volumes hieratchical(BVH)，得到一个树形结构

Red Box和Blue Box都被Purple Box包围，Red和Blue之间有交集（盒子是可以重叠的），而且它们在盒子内并不是按照某种顺序排列的，他们只是在里面。所有这个树状结构也没有左子树右子树的说法。

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_BVH_1.png)

所以实现思路如下：

```py
if（射线碰到紫色盒子）
    hit0 = 碰撞 蓝色盒子内的物体
    hit1 = 碰撞 红色盒子内的物体
    if（hit0 或 hit1）
        return true和被撞击的物体的信息
    else 
        return false
```

但是这个算法work的前提是我们要有一个优秀的策略把空间进行划分为不同的盒子和一种快速，高效的光线和边界盒的交互，而且边界盒应该要足够紧凑。在实践的大多数model中，轴对齐包围盒（axis-aligned bounding boxes, AABB）一般比较好，但是这个只是一个经验规律，具体问题需要具体分析

任何方法去用来让光线与AABB进行交互都是可以的，而且所有我们需要知道的是：**我们是否hit了它**

AABB盒是一个简单的3D的六面体，每一边都平行于一个坐标平面，矩形边界框不一定是正方体，它的长、宽、高可以彼此不同。

一般使用“平板(slab)”方法。 这是基于n维观察的方法。**AABB只是n轴对齐的区间的交集**，通常称为“平板”。

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_AABB_1.png)

间隔就是两个端点之间的点，例如x，使得3 <x <5，或者更简洁地:` x in（3,5）`在2D空间中中，两个区间重叠产生2D AABB盒（矩形如上）

当光线击中一个间隔的时候，我们首先要判断是否光线击中了边界。比如还是以上面的2D AABB为例，光线的参数为t0与t1（假设光线是与纸面平行的）

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_AABB_2.png)

而在3D中，这些边界从直线变成了平面，平面的方程成为了`x = x0`和`x = x1`，那么如何检测光线射中了平面呢？对于光线我们之前有定义为：`p(t) = A + tB`，而这个等式是由`x/y/z`这三个坐标分量组成的，对于`x`来说，可以由上面等式退化得到`x0 = A.x + t0 * B.x`那么移项即可得到`t0 = (x0 - A.x) / B.x`同理我们也能得到`t1 = (x1 - A.x) / B.x`

但是，如何把1D的数学公式去表达是否hit这个概念呢？仔细观察下面这个图：

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_AABB_3.png)

**什么时候hit了？——当且仅当绿色和蓝色出现overlap的时候**，所以我们能写出以下代码

```py
compute(tx0,tx1)
compute(ty0,ty1)
return overlap?((tx0,tx1),(ty0,ty1))
```

这个概念可以优雅地推广到3D空间：

```py
compute(tx0,tx1)
compute(ty0,ty1)
compute(tz0,tz1)
return overlap?((tx0,tx1),(ty0,ty1),(tz0,tz1))
```

但是还是有一些缺陷的，比如

+ 从x轴正方向向负方向射的光线，则会有`(tx0,tx1)`大小颠倒的情况，比如会出现`(7,3)`的范围；
+ 除法可能会因为除数`B.x`是0导致NaN，但是这样的光线是合理的（也就是在x平面上没有移动的情况）这样的射线有的是在平板内的，有些不在，而且这个0会带正负号。幸运的是，在IEEE浮点标准下，在`B.x=0`时，t0和t1会都是`正无穷或者负无穷`，反正不是x0和x1之间的数字；第一个问题我们可以通过设置最大最小值来解决：

```py
tx0 = min((x0 - A.x) / B.x, (x1 - A.x) / B.x);
tx1 = max((x0 - A.x) / B.x, (x1 - A.x) / B.x);
```

+ 如果`B.x = 0`而且`x0 - A.x`或`x1 - A.x`也等于零（分子分母同为0），那么我们依然会得到NaN。在这种情况下，我们返回命中/不命中作为结果都行，但是需要等会儿重新进行访问

现在我们可以写出检测重叠的函数了，假定我们的射线不会颠倒（也就是说区间`(x0,x1)`中恒有`x0 < x1`），当我们击中时，我们想要返回true。这个二元重叠检测可以计算出区间(d,D)和(e,E)的重叠部分(f,F)

```py
bool overlap(d, D, e, E, f, F)
    f = max(d,e)
    F = min(D,E)
    return (f<F)
```

如果这部分出现NaN，我们就返回false

接下来我们可以编写自己的代码，一下代码可以进行一些优化，但是为了展示原理，暂时不进行优化

```cpp
inline float ffmin(float a, float b) { return a < b ? a : b; }
inline float ffmax(float a, float b) { return a > b ? a : b; }

class aabb {
public:
	aabb() {}
	aabb(const vec3& a, const vec3& b) { _min = a; _max = b;}

	vec3 min() const {return _min; }
	vec3 max() const {return _max; }

	bool hit(const ray& r, float tmin, float tmax) const {
		for (int a = 0; a < 3; a++) //分别针对xyz三个方向进行判定
		{
			float t0 = ffmin((_min[a] - r.origin()[a]) / r.direction()[a],
							 (_max[a] - r.origin()[a]) / r.direction()[a]);
			float t1 = ffmax((_min[a] - r.origin()[a]) / r.direction()[a],
							 (_max[a] - r.origin()[a]) / r.direction()[a]);
			tmin = ffmax(t0, tmin);
			tmax = ffmin(t1, tmax);
			if (tmax <= tmin)
				return false;
		}
		return true;
	}

private:
	vec3 _min;
	vec3 _max;
};
```

接下来需要增加一个虚函数作为接口，对于所有的hitable的情况去计算边界盒，接下来我们需要建立一个层级的盒子覆盖所有的基元，比如sphere将会作为叶子节点出现在其中。这个函数将会返回一个bool值因为不是所有基元都有边界盒（比如无限平面）；此外，物体需要有移动的属性，需要需要time1和time2作为帧间隔而且边界盒会绑定在该间隔内移动的对象

```cpp
class hitable
{
public:
	virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const = 0;
	virtual bool bounding_box(float t0, float t1, aabb& box) const = 0;
};
```

对于球体，边界盒函数是非常容易编写的：

```cpp
//绑定了球体外接正方体的左下角和右上角作为min和max
bool sphere::bounding_box(float t0, float t1, aabb &box) const
{
	box = aabb(center - vec3(radius,radius,radius),center + vec3(radius,radius,radius));
	return true;
}
```

对于移动的球体，我们可以在t0和t1时刻使用球体的aabb，然后计算这两个box的aabb情况：

```cpp
//moving_sphere.cpp
bool moving_sphere::bounding_box(float t0, float t1, aabb &box) const
{
	aabb box0(center(t0) - vec3(radius, radius, radius), center(t0) + vec3(radius, radius, radius));
	aabb box1(center(t1) - vec3(radius, radius, radius), center(t1) + vec3(radius, radius, radius));
	box = surrounding_box(box0, box1);
	return true;
}

//aabb.h
//把两个box拼起来组成一个大的box
aabb surrounding_box(aabb box0, aabb box1) {
	vec3 small( fmin(box0.min().x(), box1.min().x()),
				fmin(box0.min().y(), box1.min().y()),
				fmin(box0.min().z(), box1.min().z()));
	vec3 big  ( fmax(box0.max().x(), box1.max().x()),
				fmax(box0.max().y(), box1.max().y()),
				fmax(box0.max().z(), box1.max().z()));
	return aabb(small,big);
}
```

对于链表，可以在构造函数中直接存储边界盒，也可以在运行时计算，我们这里使用运行时计算，因为这一般只在BVH构造函数的时候调用

```cpp
bool hitable_list::bounding_box(float t0, float t1, aabb &box) const
{
	//尝试绑定第0个
	if(list_size < 1) return false;
	aabb temp_box;
	bool first_true = list[0]->bounding_box(t0,t1,temp_box);
	if(!first_true)
		return false;
	else
		box = temp_box;

	for (int i = 1; i < list_size; i++)
	{
		if (list[0]->bounding_box(t0, t1, temp_box))
			box = surrounding_box(box, temp_box);//尝试扩大绑定到box成为可以容纳整个list上所有物体
		else
			return false;
	}
	return true;
}
```

这要求支持aabb的`surrounding_box`函数计算两个边界盒，这个函数与上面的函数相同

一个BVH也是继承自hitable，就像hitable链表类型。它是一个容器，能够反馈这样一个查询：“光线击中了你吗？” 我们建立了一个`bvh_node`类进行这些操作，BVH的架构在前面提及的是一个树形结构方便子查询

注意到子节点的指针是一个通用的hitable，这也就保证这个指针同时能够指向所有hitable的派生类，比如`bvh_node`、`sphere`等其他类型

```cpp
class bvh_node : public hitable {
public:
	bvh_node() {}
	bvh_node(hitable **l, int n, float time0, float time1);
	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;

private:
	hitable *left;//指向左节点
	hitable *right;//指向右节点
	aabb box;//本节点
};

bool bvh_node::bounding_box(float t0, float t1, aabb& b) const {
	b = box;
	return true;
}
```

对于hit的判定非常简单：检测是否存在node上的边界盒被hit，如果被hit，就检测它的相关子节点，列出相关detail

```cpp
bool bvh_node::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	if (box.hit(r, t_min, t_max)) {
		hit_record left_rec, right_rec;
		bool hit_left = left->hit(r, t_min, t_max, left_rec);
		bool hit_right = right->hit(r, t_min, t_max, right_rec);
		if (hit_left && hit_right)//两个子节点都hit了
		{
			if (left_rec.t < right_rec.t)//保存较小的t的数据（更靠近屏幕前面）
				rec = left_rec;
			else
				rec = right_rec;
			return true;
		}
		else if (hit_left) {
			rec = left_rec;
			return true;
		}
		else if (hit_right) {
			rec = right_rec;
			return true;
		}
		else
			return false;
	}
	else return false;
}
```

任何效率结构中最复杂的部分就是构建这个结构（也就是list的bound）。关于BVH的一个很酷的事情是，只要BVH节点中的对象列表被分成两个子列表，命中检测就可以工作。如果合适分隔，这将最高效，为了生成一个尽量小的，包含所有对象的盒子，我们让每个节点沿一个轴分割列表：

+ 随机选择一个轴
+ 将节点内的图元对象进行快速排序
+ 左右子树各放一半

```cpp
int box_z_compare (const void * a, const void * b)
{
	aabb box_left, box_right;
	hitable *ah = *(hitable**)a;
	hitable *bh = *(hitable**)b;
	if(!ah->bounding_box(0,0, box_left) || !bh->bounding_box(0,0, box_right))
		std::cerr << "no bounding box in bvh_node constructor\n";
	if ( box_left.min().z() - box_right.min().z() < 0.0  )
		return -1;
	else
		return 1;
}

bvh_node::bvh_node(hitable **l, int n, float time0, float time1) {
	int axis = int(3*drand48());
	if (axis == 0)
		qsort(l, n, sizeof(hitable *), box_x_compare);
	else if (axis == 1)
		qsort(l, n, sizeof(hitable *), box_y_compare);
	else
		qsort(l, n, sizeof(hitable *), box_z_compare);
	if (n == 1) {
		left = right = l[0];
	}
	else if (n == 2) {
		left = l[0];
		right = l[1];
	}
	else {
		left = new bvh_node(l, n/2, time0, time1);
		right = new bvh_node(l + n/2, n - n/2, time0, time1);
	}
	aabb box_left, box_right;
	if(!left->bounding_box(time0,time1, box_left) || !right->bounding_box(time0,time1, box_right))
		std::cerr << "no bounding box in bvh_node constructor\n";
	box = surrounding_box(box_left, box_right);
}
```

当进入的列表是两个元素时，我在每个子树中放置一个并结束递归。遍历算法应该是平滑的，不必检查空指针，所以如果只有一个元素，在每个子树中复制它

最后在main中的`random_scene`中返回bvh_node的构建即可

```cpp
	return new bvh_node(list, i, 0, 1);
```

<hr>

纹理

为了让整个图像成为我们真正想要的，我们要添加对于纹理的要求:

```cpp
class texture  {
public:
	//表示某个uv点的rgb值的接口
	virtual vec3 value(float u, float v, const vec3& p) const = 0;
};
```

我们现在能够通过纹理指针来添加纹理材质通过提换原来用vec3表达的颜色

```cpp
//漫反射
class lambertian : public material {
public:
	lambertian(texture *a) : albedo(a){}

	//入射光，hit点的的记录，衰减，散射
	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
	{
		vec3 target = rec.p + rec.normal + random_in_unit_sphere();//进行漫反射随机化反射方向
		scattered = ray(rec.p, target - rec.p, r_in.time());//散射光线
		attenuation = albedo->value(0, 0, rec.p);
		return true;
	}

private:
	texture *albedo;//反射率（根据绑定的纹理内容进行处理）
};
```

如果还是想要原来的效果的话，现在提供以下函数

```cpp
class constant_texture : public texture {
public:
	constant_texture() { }
	constant_texture(vec3 c) : color(c) { }

	virtual vec3 value(float u, float v, const vec3& p) const {
		return color;
	}

private:
	vec3 color;
};
```

原来是`new lambertain(vec3(0.5, 0.5, 0.5))`现在变成了`new lambertian(new constrtant_texture(vec3(0.5, 0.5, 0.5)))`，用来表示一个恒定的纹理

我们能够创造一个`checker texture`通过标注`sin`或者`cos`作为备用，如果我们在所有三个维度中乘以三角函数，则乘积的符号形成3D检查器模式，这些odd/even指针能够指向恒定纹理或者其他程序化纹理，这是由Pat Hanrahan在20世纪80年代引入的着色器网络的一部分

```cpp
//建立一个棋盘状的纹理
class checker_texture : public texture{
public:
	checker_texture(){}
	checker_texture(texture *t0, texture *t1) : even(t0), odd(t1){}

	virtual vec3 value(float u, float v, const vec3 &p) const
	{
		float sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
		if (sines < 0)
			return odd->value(u, v, p);
		else
			return even->value(u, v, p);
	}

private:
	texture *odd;
	texture *even;
};
```

<hr>

Perlin噪音

让纹理变得更cool，我们引入Perlin噪音

除了简单迅速以外，柏林噪声的另一个关键特性是它对于相同的输入永远返回相同的随机数字，输入点附近的点返回近似的数字。

我们可以用随机数的3D数组来平铺所有空间并在块中使用它们

现在如果我们创建一些正真使用这些在0和1之间的float值的纹理然后创造了灰色的颜色

```cpp
class perlin {
public:
	float noise(const vec3& p) const {
		float u = p.x() - floor(p.x());
		float v = p.y() - floor(p.y());
		float w = p.z() - floor(p.z());
		int i = int(4 * p.x()) & 255;
		int j = int(4 * p.y()) & 255;
		int k = int(4 * p.z()) & 255;
		return ranfloat[perm_x[i] ^ perm_y[j] ^ perm_z[k]];
	}
	static float *ranfloat;
	static int *perm_x;
	static int *perm_y;
	static int *perm_z;
};

static float* perlin_generate() {
	auto *p = new float[256];
	for (int i = 0; i < 256; ++i)
		p[i] = drand48();
	return p;
}

void permute(int *p, int n) {
	for (int i = n - 1; i > 0; i--)
	{
		int target = int(drand48() * (i + 1));
		int tmp = p[i];
		p[i] = p[target];
		p[target] = tmp;
	}
}

static int* perlin_generate_perm() {
	int * p = new int[256];
	for (int i = 0; i < 256; i++)
		p[i] = i;
	permute(p, 256);
	return p;
}

float *perlin::ranfloat = perlin_generate();
int *perlin::perm_x = perlin_generate_perm();
int *perlin::perm_y = perlin_generate_perm();
int *perlin::perm_z = perlin_generate_perm();
```

创造出Perlin噪声纹理：

```cpp
class noise_texture : public texture {
public:
	noise_texture() {}
	virtual vec3 value(float u, float v, const vec3& p) const {
		return vec3(1,1,1)*noise.noise(p);
	}

private:
	perlin noise;
};
```

使用两个球的简易demo进行测试：

```cpp
hitable *two_perlin_spheres()
{
	texture *pertext = new noise_texture();
	hitable **list = new hitable* [2];
	list[0] = new sphere(vec3(0,-1000,0),1000,new lambertian(pertext));
	list[1] = new sphere(vec3(0,2,0),2,new lambertian(pertext));
	return new hitable_list(list,2);
}
```

得到如下效果

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_Perlin_1.png)

我们可以让它更加线性化：

```cpp
inline float trilinear_interp(float c[2][2][2],float u, float v,float w)
{
	float accum = 0;
	for(int i = 0; i < 2; i++)
		for(int j = 0; j < 2; j++)
			for(int k = 0; k < 2; k++)
				accum += (i * u + (1 - i) * (1 - u)) * (j * v + (1 - j) * (1 - v)) * (k * w + (1 - k) * (1 - w)) *
						 c[i][j][k];
	return accum;
}

class perlin {
public:
	float noise(const vec3& p) const {
		float u = p.x() - floor(p.x());
		float v = p.y() - floor(p.y());
		float w = p.z() - floor(p.z());
//		int i = int(4 * p.x()) & 255;
//		int j = int(4 * p.y()) & 255;
//		int k = int(4 * p.z()) & 255;
//		return ranfloat[perm_x[i] ^ perm_y[j] ^ perm_z[k]];
		float c[2][2][2];
		int i = floor(p.x());
		int j = floor(p.y());
		int k = floor(p.z());
		for (int di = 0; di < 2; di++)
			for (int dj = 0; dj < 2; dj++)
				for (int dk = 0; dk < 2; dk++)
					c[di][dj][dk] = ranfloat[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];

		return trilinear_interp(c, u, v, w);
	}
```

得到以下效果：

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_Perlin_2.png)

但是这张图我们明显可以看到大地具有网格特征，优化它的标准技巧是使用hermite立方体来舍入插值

```cpp
	float noise(const vec3& p) const {
		float u = p.x() - floor(p.x());
		float v = p.y() - floor(p.y());
		float w = p.z() - floor(p.z());
		u = u * u * (3 - 2 * u);
		v = v * v * (3 - 2 * v);
		w = w * w * (3 - 2 * w);
```

得到以下效果，这次明显更像一个image了：

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_Perlin_3.png)

但是还是有一些像网格，那我们额外增加一个`scale`的控制参数进行控制

```cpp
class noise_texture : public texture {
public:
	noise_texture() {}
	noise_texture(float scale): scale(scale){}//需要在main函数中进行修改传入scale，否则渲染scale默认为0，渲染出来都是黑色
	
	virtual vec3 value(float u, float v, const vec3& p) const {
		return vec3(1,1,1)*noise.noise(scale * p);
	}

private:
	perlin noise;
	float scale;
};
```

调整后得到的图像如下：

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_Perlin_4.png)

但是还是有一点砖格的感觉，这也许是因为min和max这组总是在x/y/z轴的整数上，而不是浮点数；Ken Perlin的算法非常巧妙的地方是他没有直接将单位随机向量输出到点，而是使用点积来移动最大和最小值。

```
static vec3* perlin_generate() {
	auto *p = new vec3[256];
	for (int i = 0; i < 256; ++i)
		p[i] = unit_vector(vec3(-1 + 2*drand48(),-1 + 2*drand48(),-1 + 2*drand48())));
	return p;
}

inline float perlin_interp(vec3 c[2][2][2],float u, float v,float w)
{
	float uu = u * u * (3 - 2 * u);
	float vv = v * v * (3 - 2 * v);
	float ww = w * w * (3 - 2 * w);
	float accum = 0;
	for(int i = 0; i < 2; i++)
		for(int j = 0; j < 2; j++)
			for (int k = 0; k < 2; k++)
			{
				vec3 weight_v(u - i, v - j, w - k);
				accum += (i * uu + (1 - i) * (1 - uu)) * (j * vv + (1 - j) * (1 - vv)) * (k * ww + (1 - k) * (1 - ww)) *
						 dot(c[i][j][k], weight_v);
			}
	return accum;
}

class perlin {
public:
	float noise(const vec3& p) const {
		float u = p.x() - floor(p.x());
		float v = p.y() - floor(p.y());
		float w = p.z() - floor(p.z());

		vec3 c[2][2][2];
		int i = floor(p.x());
		int j = floor(p.y());
		int k = floor(p.z());
		for (int di = 0; di < 2; di++)
			for (int dj = 0; dj < 2; dj++)
				for (int dk = 0; dk < 2; dk++)
					c[di][dj][dk] = ranvec[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];

		return perlin_interp(c,u,v,w);
	}

private:
	static vec3 *ranvec;
	static int *perm_x;
	static int *perm_y;
	static int *perm_z;
};

//...

vec3 *perlin::ranvec = perlin_generate();
//...
```

最终结果如下：

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_Perlin_5.png)

我们通常使用具有多个相加频率的复合噪声。 这称为Turbulence，是对噪音重复调用的总统称：

```cpp
float turb(const vec3& p, int depth=7) const {
    float accum = 0;
    vec3 temp_p = p;
    float weight = 1.0;
    for (int i = 0; i < depth; i++) {
        accum += weight*noise(temp_p);
        weight *= 0.5;
        temp_p *= 2;
    }
    return fabs(accum);
}
```

得到如下图像：

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_Perlin_6.png)

这是我们直接使用scale对于其进行直接的缩放，基本的想法是去将颜色属性进行一些sin变换，然后进行一些调整以符合预期效果，如下：

```cpp
virtual vec3 value(float u, float v, const vec3& p) const {
	//return vec3(1,1,1)*noise.turb(scale * p);
	return vec3(1, 1, 1) * 0.5 * (1 + sin(scale * p.z() + 10 * noise.turb(p)));
}
```

得到以下图像：

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_Perlin_7.png)

<hr>

图像纹理映射

也就是给模型加上贴图，我们可以像上面一样通过程序化使用一个向量p为纯色纹理建立索引，也可以通过一个image使用其2D的u，v进行纹理绑定，而且需要一个值来调整贴图的缩放

对于一个像素为(i,j)的在整个image为nx和ny的时候，我们可以有如下公式

```py
u = i / (nx - 1)
v = j / (ny - 1)
```

对于球体，我们需要想三维矢量转换为极坐标再转换为u,v

从向量的

```
x = cos(φ)*cos(θ)
y = sin(φ)*cos(θ)
z = sin(θ)
```

可以反求得

```
φ = atan2(y,x)//计算结果为[-π, π]
θ = asin(z)//计算结果为[-π/2, π/2]
```

上面的θ指的是从极点下来的角度，φ指的是围着通过极点的轴的角度

```
u = φ / (2 * π)
v = θ / π
```

得到以下image_texture类，用来进行外部image对球体进行绑定

```cpp
class image_texture : public texture
{
public:
	image_texture(){}
	image_texture(unsigned char *pixels, int A, int B) : data(pixels), nx(A), ny(B)
	{}

	virtual vec3 value(float u, float v, const vec3 &p) const;
private:
	unsigned char *data;
	int nx, ny;
};

vec3 image_texture::value(float u, float v, const vec3 &p) const
{
	int i = u * nx;
	int j = (1 - v) * ny - 0.001;
	if (i < 0) i = 0;
	if (j < 0) j = 0;
	if (i > nx - 1)i = nx - 1;
	if (j > ny - 1)j = ny - 1;
	float r = int(data[3 * i + 3 * nx * j]) / 255.0;
	float g = int(data[3 * i + 3 * nx * j + 1]) / 255.0;
	float b = int(data[3 * i + 3 * nx * j + 2]) / 255.0;
	return vec3(r, g, b);
}
```

由此，uv的范围能够被一个统一的方程计算，也就是

```cpp
	if (temp < t_max && temp > t_min)	
	{
//...
			get_sphere_uv((rec.p - center) / radius, rec.u, rec.v);
//...
	}
	temp = (-b + sqrt(discriminant)) / a;
	if (temp < t_max && temp > t_min)	{
//...
		get_sphere_uv((rec.p - center) / radius, rec.u, rec.v);
//...
	}
```

由此我们可以编写场景代码：

```cpp
hitable *earth() {
    int nx, ny, nn;
    unsigned char *tex_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);
    material *mat = new lambertian(new image_texture(tex_data, nx, ny));
    return new sphere(vec3(0,0, 0), 2, mat);
}
```

不要忘记：在使用lambertian时务必要修改attenuation的使用值，使之调用`image_texture`的`value`函数（不然会出现纹理bind渲染出来无现象的情况）

```cpp
class lambertian : public material
{
public:
	lambertian(texture *a) : albedo(a){}

	//入射光，hit点的的记录，衰减，散射
	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
	{
		//...
		attenuation = albedo->value(rec.u, rec.v, rec.p);//需要在反射强度上通过u,v值进行控制
		return true;
	}

private:
	texture *albedo;//反射率（根据绑定的纹理内容进行处理）
};
```

<hr>

正方形与光照

这部分我们开始制作光源，首先我们需要一个发光的材质，我们把这个设为背景一样，不进行反射：

```cpp
//自发光材质
class diffuse_light:public material
{
public:
	diffuse_light(texture *a):emit(a){}

	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const override
	{ return false; }

	virtual vec3 emitted(float u, float v, const vec3 &p) const override
	{ return emit->value(u, v, p); }

private:
	texture *emit;
};
```

修改material这个抽象基类，提供emitted接口，对于没有emitted的材质，默认emitted为0

```cpp
class material
{
public:
	virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const = 0;

	virtual vec3 emitted(float u, float v, const vec3 &p) const
	{ return vec3(0, 0, 0); }//对于所有不发光的，一律设置发光是(0,0,0)，使之不产生叠加效应
};
```

接下来为了验证的自发光材质真的有效，我们将背景设置为黑色

```cpp
//depth：进行多少次光线追踪
vec3 color(const ray &r, hitable *world, int depth)
{
	hit_record rec;
	if (world->hit(r, 0.001, FLT_MAX, rec))
	{
		ray scattered;//散射光线
		vec3 attenuation;//反射率
		vec3 emitted = rec.mat_ptr->emitted(rec.u,rec.v,rec.p);//增加了自发光的效应
		if (depth < 50 && rec.mat_ptr->scatter(r,rec,attenuation,scattered))//调用两个派生类进行分别的渲染
			return emitted + attenuation * color(scattered, world, depth + 1);
		else
			return emitted;
	}
	else
	{
//		vec3 unit_direction = unit_vector(r.direction());//归一化成单位坐标
//		float t = 0.5 * (unit_direction.y() + 1.0f);//全部变成正数方便混色，t=1时变成blue，t=0时变成white
//		return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);//就是混色操作，类似线性插值
		return vec3(0,0,0);
	}
}
```

接下来我们要建立矩形，相比较球体只需要球心和半径，矩形就比较复杂了

![image](https://github.com/yu-cao/RayTrace/blob/master/note/graph/note2_Rectangle_1.png)

由光线方程`p(t) = A + tB`，可得在z分轴上有`z(t) = A.z + t*B.z`当我们对于一个已知平面`z = k`来说，t就是可解的`t = (k - A.z) / B.z`

一旦我们有t，就可以得到x与y的方程

```
x = A.x + t*B.x
y = A.y + t*B.y
```

如果hit，则x∈[x0,x1],y∈[y0,y1]，由此可以完成代码（同理可以完成xz平面，yz平面的代码）：

```cpp
class xy_rect : public hitable
{
public:
	xy_rect(){}
	xy_rect(float _x0, float _x1, float _y0, float _y1, float _k, material *mat) : x0(_x0), x1(_x1), y0(_y0), y1(_y1),
																				   k(_k), mp(mat){};

	virtual bool hit(const ray &r, float t0, float t1, hit_record &rec) const;

	virtual bool bounding_box(float t0, float t1, aabb &box) const
	{
		box = aabb(vec3(x0, y0, k - 0.0001), vec3(x1, y1, k + 0.0001));//aabb盒取矩阵的左下角和右上角，包含一个薄平面
		return true;
	}

	material *mp;
	float x0, x1, y0, y1, k;
};

bool xy_rect::hit(const ray &r, float t0, float t1, hit_record &rec) const
{
	float t = (k - r.origin().z()) / r.direction().z();//通过k计算得到t值，并且判断合理性
	if (t < t0 || t > t1)
		return false;
	float x = r.origin().x() + t * r.direction().x();//计算得到x和y并判断合理性
	float y = r.origin().y() + t * r.direction().y();
	if (x < x0 || x > x1 || y < y0 || y > y1)
		return false;
	rec.u = (x - x0) / (x1 - x0);//得到光线击中点在矩形表面的uv值
	rec.v = (y - y0) / (y1 - y0);
	rec.t = t;
	rec.mat_ptr = mp;//材质绑定
	rec.p = r.point_at_parameter(t);//击中点的光线常数：(A+tB)的值
	rec.normal = vec3(0, 0, 1);//因为是xy平面，必定与z轴垂直，所以z = 1即是法线方向
	return true;
}
```

我们提供一个上方的球体和右端的矩形

```cpp
hitable *simple_light()
{
	texture *pertext = new noise_texture(4);
	hitable **list = new hitable *[4];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(pertext));
	list[1] = new sphere(vec3(0, 2, 0), 2, new lambertian(pertext));
	//注意到我们设置的亮度大于(1,1,1)，允许其照亮其他东西
	list[2] = new sphere(vec3(0, 7, 0), 2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	list[3] = new xy_rect(3, 5, 1, 3, -2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	return new hitable_list(list, 4);
}

//照相机设定常数如下
	hitable *world = simple_light();

	vec3 lookfrom(13,2,3);
	vec3 lookat(0,2,0);
	const vec3 vup(0,1,0);
	const float fov = 30.0;
	float dist_to_focus = 10;//焦距长度 为对焦到lookat位置的 长度
	float aperture = 0.0;//光圈（透镜）大小
	camera cam(lookfrom, lookat, vup, fov, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
```