# GestureDetect Based On OpenCV & Caffe
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
- **题主选择的五项手势**

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;hearta![hearta](example/heart_a.jpg )   &nbsp;&nbsp; heartb![heartb](example/heart_b.jpg)   &nbsp;&nbsp; greet![greet](example/greet.jpg)  &nbsp;&nbsp;six![six](example/six.jpg)   &nbsp;&nbsp;thumb![thumb](example/thumb.jpg)


- **流程分析**

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;关于检测速度，我们需要将整个问题的流程进行详细的分析。对于一帧测试图像整个过程大致分为如下图
![](example/ISP.png)
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;其中摄像头负责采集并做ISP等功能的时间由摄像头规格决定（包括曝光时间，处理速度等等）,DMA时间由带宽决定,而由cpu拷贝至算法指定的buffer的时间由Opencv算法的实现和cpu性能决定。上述这一部分在比赛中限定使用cv::VideoCapture() >> img 实现。
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;实际上笔者关闭了整个算法循环的内容，可以测出cv::VideoCapture>>img的速度约为25fps,即该摄像头规格为480p25.实际在使用中，如果算法while循环一次的时间小于40ms，则需要继续等待下一帧img通过memcpy传输过来。当然笔者也进行了测试，在该平台上cv::imshow()一帧480p的图像平均时间在10~13ms之间。在这个情况下，会显示cap>>img的时间约为（40-13）=27ms左右。当然笔者进一步的测试总结如下表格：
</br>

| | ISP+Capture  | memcpy |cv::imshow|inference|
| ------------- |-------------| --------|----------|-------------|
| time     | 40ms| 12ms(uncertain) |10~13ms| 3~5ms|

</br>

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;到这里我们也就理清了cv::VideoCapture的工作原理（应该是算法线程会去开辟一个新的现成去完成左边框内的工作）。而算法线程每一帧图像在副线程完成DMA之后，就会拷贝给算法开辟的buffer中去，这个间隔是40ms。也就是说如果memcpy+inference+cv::imshow()的时间小于40ms，这个时候在算法线程就会挂起等待DMA结束。
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**总结：**在Firefly-3399上，调用cv::VideoCapture作为采集，cv::imshow()作为显示，如果你的inference时间小于15ms（40-13-12）每帧，那么多余的速度将不会起到任何增益。例如笔者这里的inference在没有目标的时候大概是1ms,出现很大目标时是7~10ms,这样的波动根本不会影响实际帧率，因为算法永远会挂起等待DMA。考虑整个系统的流水线取决于最慢的一级，而摄像头一般都有最低的曝光和ISP处理时间，而这个时间将成为整个系统的瓶颈，所以再优化检测算法的速度（inference）也只有理论上的意义了。

**-inference 性能分析-**
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;有了上面的讨论，这里我们就可以进入对检测算法的性能测评（以下对应每个手势的时间代表的是这个算法检出为某个手势的时间，是包括定位和分类一起的时间。而不是单独测试某一个分类器的时间）：

**（比赛版本 frame_control = 3 / use_cnn = 0）**
</br>

|distance=1~1.5m | no target (pre_process)| 1-hearta|1-heartb|1-greet|1-six|1-thumb|
| ------------- |-------------| --------|----------|----------|----------|----------|
| time     | 0.7ms| 2.5ms |3.5ms| 2.5ms| 4.9ms|2.2ms| 

</br>
**（比赛版本 Muti-Gestures Test）**
</br>

|distance=1~1.5m | 2-hearta| 2-heartb|2-greet|2-six|2-thumb|muti-six`s|
| ------------- |-------------| --------|----------|----------|----------|----------|
| time     | 4.2ms| 8.1ms |3.8ms| 6.8ms| 4.0ms|10.3ms| 

</br>
**（全局版本 frame_control = 1 / use_cnn = 0）**
</br>

|distance=1~1.5m | no target (pre_process) | hearta|heartb|greet|six|thumb|
| ------------- |-------------| --------|----------|----------|----------|----------|
| time     | 2.0ms| 3.3ms |4.8ms| 2.8ms| 6.0ms|3.5ms| 

</br>
**（全局版本 Muti-Gestures Test）**
</br>

|distance=1~1.5m | 2-hearta| 2-heartb|2-greet|2-six|2-thumb|muti-six`s|
| ------------ |-------------| --------|----------|----------|----------|----------|
| time     | 8.3ms| 14.4ms |3.3ms| 6.0ms| 4.1ms|10.5ms| 

</br>

**ps:** 

- 1.测试距离为1~1.5m,这个距离决定了vj搜索的最小框与最大框的参数，会对速度产生一定影响,可以根据需求修改，但是不要太远。对于太小的目标而言，算法框架可能需要精细的调整。（其实也很简单，可以咨询我hhhh）
- 2.手势速度差距来源于手势训练处的xml的深度和每一层的弱分类器个数，总的来说，在训练分类器的过程里，还是要以recall和precision为关注点，训练出来可以用的再考虑在这一部分提升速度（详细可以参考 [GestureDetect based on OpenCV & Caffe/train](https://github.com/wuziheng/GestureDetect-Violajones-CNN-/tree/master/train)。
- 3.以上的度比赛版本是在每3帧才全局搜索一次而其余两帧只做近邻搜索分类的情况下做的。全局版则是在每一帧都做全局搜索的情况下的测试结果。
- 4.对比结果发现，在预处理的时间上，比赛版有优势，但是在Viola-jones的时间上，全局版有优势。随着每一帧画面中手势数量的增多，Viola-jones的时间几乎是线性增加的，而预处理时间几乎不会变化，所以说在Muti-Gesture的应用场景里，我们推荐 frame_control=1,2，而在确定只有Single-Gesture的场景中，选择  frame_control=3。比赛情况为后者。
- 5.全局版会出现少量的误检,而比赛版则不会,因为 frame_control 还控制了算法逻辑框架的其他部分可以消除大部分误检测。这也是选择比赛版的原因之一。
- 6.算法还保留了利用 CNN 对检出的手势框进行再确认的接口，本意是用来提高precision，但是后期笔者解决了 precision 的问题，并且 CNN 较慢而且 recall 很低，会导致检出的手势也会被误判为背景，所以在比赛版本里关闭了 CNN 测试的接口。CNN在板子上的性能大约为10ms一个候选框。

欢迎留言或者邮件交流694790961@qq.com


			
			
