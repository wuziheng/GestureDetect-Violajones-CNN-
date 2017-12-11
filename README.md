# GestureDetect Based On OpenCV & Caffe
[![GitHub license](http://dmlc.github.io/img/apache2.svg)](./LICENSE)
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;该项目是基于作者参加ARM在浙江大学举办的嵌入式算法大赛的赛题C手势识别整理而来。整套算法基于OpenCV和CaffeOnAcl实现,平台在ARM公司的[Rockchip RK3399](http://www.rock-chips.com/plus/3399.html)上定制的ubuntu16.04.比赛目标是在480p(640x480)的视频图像中最快检测出自定义的五种及以上的目标手势并准确分类。
 


### 环境需求
#
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;考虑到很多方面的原因，算法并没有做特别低层的优化，所以可移植性还是较高的。ubuntu系统下只要配置好以下的库，修改Makefile即可，并不限定平台。但如果你也是基于[Rockchip RK3399](http://www.rock-chips.com/plus/3399.html)平台的用户，推荐参考如下链接&nbsp;[OAID/CaffeOnACL](https://github.com/OAID/CaffeOnACL/blob/master/acl_openailab/installation.md)&nbsp;安装opencv/caffe等其他依赖

- opencv-3.3.0
- CaffeOnACL/Caffe/Caffe2
- gflags


### 性能描述
#
- **题主选择的五项手势**

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;hearta![hearta](example/heart_a.jpg )   &nbsp;&nbsp; heartb![heartb](example/heart_b.jpg)   &nbsp;&nbsp; greet![greet](example/greet.jpg)  &nbsp;&nbsp;six![six](example/six.jpg)   &nbsp;&nbsp;thumb![thumb](example/thumb.jpg)



- **inference 性能分析-**
详细的分析与介绍可以参考perfomance_report,这里我们就简单的列出对检测算法的性能测评（以下对应每个手势的时间代表的是这个算法检出为某个手势的时间，是包括定位和分类一起的时间。而不是单独测试某一个分类器的时间）：

</br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**（比赛版本 frame_control = 3 / use_cnn = 0）**
</br>
<div align=center> 

|distance=1~1.5m | no target (pre_process)| 1-hearta|1-heartb|1-greet|1-six|1-thumb|
| ------------- |-------------| --------|----------|----------|----------|----------|
| time     | 0.7ms| 2.5ms |3.5ms| 2.5ms| 4.9ms|2.2ms| 

</div>
</br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**（比赛版本 Muti-Gestures Test）**
</br>
<div align=center> 

|distance=1~1.5m | 2-hearta| 2-heartb|2-greet|2-six|2-thumb|muti-six`s|
| ------------- |-------------| --------|----------|----------|----------|----------|
| time     | 4.2ms| 8.1ms |3.8ms| 6.8ms| 4.0ms|10.3ms| 

</div>
</br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**（全局版本 frame_control = 1 / use_cnn = 0）**
</br>
<div align=center> 

|distance=1~1.5m | no target (pre_process) | hearta|heartb|greet|six|thumb|
| ------------- |-------------| --------|----------|----------|----------|----------|
| time     | 2.0ms| 3.3ms |4.8ms| 2.8ms| 6.0ms|3.5ms| 

</div>
</br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**（全局版本 Muti-Gestures Test）**
</br>
<div align=center> 

|distance=1~1.5m | 2-hearta| 2-heartb|2-greet|2-six|2-thumb|muti-six`s|
| ------------ |-------------| --------|----------|----------|----------|----------|
| time     | 8.3ms| 14.4ms |3.3ms| 6.0ms| 4.1ms|10.5ms| 

</div>
</br>

### CascadeClassifier训练
可以参考我放出来的指南[train](https://github.com/wuziheng/GestureDetect-Violajones-CNN-/tree/master/train)，复现我训练的分类器的结果。


欢迎留言或者邮件交流694790961@qq.com


			
			
