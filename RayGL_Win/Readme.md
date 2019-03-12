这部分主要是使用OpenGL来加速我原先用CPU做的比较naive的光线追踪

之前的基于CPU的光线追踪是通过遍历每个像素点，对每个像素点射出n条光线，然后平均化进行采样得到某一个点的RGB，逐一计算得到，而这个想法其实可以在GPU的着色器中得到优美的实现

比如，我们知道片元着色器就是逐像素遍历的，我们是否可以把计算放在片元着色器中进行呢？

我尝试了一下，暂时没有得到很好的成功，现在使用的方法是使用OpenGL 4.3以上版本中的计算着色器的方式进行，输出一张最后渲染结果的纹理，现在已经成功完成了这一部分的代码，最后的框架整合和bug调试将在未来几天完成

因为Apple将MacBook的OpenGL版本锁定在了4.1，而且在2018 WWDC中表示要抛弃OpenGL，全力推Metal，也就是说暂时我们是不能使用Mac平台进行基于OpenGL的GPU光线追踪了，为此我特地开了这个文件夹存储在Windows下的代码

有时间我会想想办法转移到片元着色器中进行处理

这份代码基于Visual Studio 2019 Preview 4.1 SVC1，已经基本完成目标要求

![image](https://github.com/yu-cao/RayTrace/blob/master/RayGL_Win/RayGL_Win/rayGL.gif)