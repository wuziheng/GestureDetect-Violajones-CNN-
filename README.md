# GestureDetect based on OpenCV & Caffe
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;该项目是基于作者参加ARM在浙江大学举办的嵌入式算法大赛的赛题C手势识别整理而来。整套算法基于OpenCV和CaffeOnAcl实现,平台在ARM公司的Firefly3399（2×A72+4×A53）上定制的ubuntu16.04.比赛目标是在480p(640x480)的视频图像中最快检测出自定义的五种及以上的目标手势并准确分类。
 


### 环境需求
#
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;考虑到很多方面的原因，算法并没有做特别低层的优化，所以可移植性还是较高的。ubuntu系统下只要配置好以下的库，修改Makefile即可，并不限定平台。但如果你也是基于Firefly3399平台的用户，推荐参考如下链接&nbsp;[OAID/CaffeOnACL](https://github.com/OAID/CaffeOnACL/blob/master/acl_openailab/installation.md)&nbsp;安装opencv/caffe等其他依赖

- opencv-3.3.0
- CaffeOnACL/Caffe/Caffe2
- gflags

</br>


### 性能描述
#
- 题主选择的五项手势如下

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;hearta![hearta](example/heart_a.jpg )   &nbsp;&nbsp; heartb![heartb](example/heart_b.jpg)   &nbsp;&nbsp; greet![greet](example/greet.jpg)  &nbsp;&nbsp;six![six](example/six.jpg)   &nbsp;&nbsp;thumb![thumb](example/thumb.jpg)

- 检测速度
实际测试中：最快检测速度3ms/perframe，基本不出现误检测，但检出欠佳。
实际在帧率上，由于已经限定了采用opencv的采集和显示接口，而测试看来cv cap耗时大概在15ms到20ms，之间所以基本达到极限帧率了，如果想要在实际中优化，则需要提升采集的速度。

 

欢迎留言或者邮件交流694790961@qq.com


			
			
