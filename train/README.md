# Gesture Cascadeclassifier based on OpenCV

　　由于在ARM手势识别比赛中用到了基于OpenCV的级联分类器，为了方便在linux上训练和测试某一手势的级联分类器，并挑选最优的分类器模型。将一系列常用的指令和操作用python做如下封装，并总结相关经验。（样例图片涉及版权，如仅供测试代码使用）



### 环境需求
#
- linux下编译opencv3.0.0以上,可运行 opencv_traincascade
- python2.7 cv2
</br>


### 文件描述
#
 	
- ***six(自定义手势名称)***:  某一手势的主文件夹，其中主要包含该手势如下数据：
	- *pos*: 存放原始数据
	- *a_pos*：存放数据增强后的数据 
	- *xml*: 存放的训练好模型
	- *pos.vec*:（Opencv需要的正样本）
	- *neg.txt*:（Opencv需要的负样本列表）
 
* ***neg***: 通用的负样本数据存放

* ***val*** :  验证样本与标注存放,子文件夹名称与自定义手势相同(*six*)

-  ***util***: 通用工具文件夹，封装各类工具如下:
	- *train.py* :训练模型
	- *test.py*: 测试验证集图像，给出iou,precision,recall等性能
	- *xml.py*: 生成每一个stage对应的*cascade_i.xml*,便于挑选综合指标最好的分类器模型，评估最合适的stage数目.
	- *pos.py*: 指定生成正样本pos.vec
	- *neg.py*: 指定生成负样本neg.txt
</br>


### 使用说明
#
- **按照文件描述中的需求准备数据并放入相应文件夹，包括**：
	- 原始数据及标注（对于训练数据，我们将所有的检测目标都抠出来，resize到同样大小，数据增强的数据同理，所以他们是固定标注，例如20,20）
	- 数据增强后的数据及标注
	- 验证集数据及标注（对于验证数据，我们不抠出检测目标而采用大图+.txt标注用于测试分类器的recall性能）
	- 负样本数据（任意图片不需要标注）

- **生成*pos.vec*与*neg.txt*：**
	根据需求修改*util*中的*pos.py*,*neg.py*，配置是否使用aug数据，负样本数据量及数据集选择等等。
	 
		cd ./six
		python ../util/pos.py
    	python ../util/neg.py
	
- **训练：**
	修改train.py，相关参数，指定文件夹，训练。

		vim ../util/train.py
		python ../util/train.py
		
- **测试：**
	- 生成不同stages数目的cascade_i.xml: 参数为模型存放的文件夹
		
			python ../util/.xml ./xml
	
	- 测试指定stages数目的分类器指标:参数为主文件夹名与stage数目
	
			python ../util/test.py  six 12
	
	- 打印部分结果如下:165为测试图像序号，后面分别为iou/true label/detect label,最后的结果为iou_threshold=0.5下的pre,recall信息。
	
			165 (480, 640, 3) 0.5132 [1, 77, 39, 168, 168] [1, 103, 63, 128, 128]
			total true: 215; prediction: 1639; match :89
			precision: 0.0543; recall: 0.4140
</br>			


### 补充说明
# 
- 实际训练中，正样本需求量大概在300~1000这个量级，可做翻转增强，同时可以扣出手势并增加任意背景增强。
- 负样本数量越多越好,neg.txt中可以放置大量样本，这里默认10000，实际上训练如果stage过深，负样本数量会不足，同时，在实际视频中做手势检测，最容易出现false positive的检测就是人脸，所以可以在负样本中添加（大）部分人脸。
- 正样本需要准确，分布丰富且均衡。在笔者的大量测试中，通过变换背景对与cascadeclassifier的提升并不是很明显，如果是换做cnn方法提升会比较明显。相对而言在boost方法想要显著提高recall,则需要检测目标本身在正样本中目标的分布丰富而且均衡，而想要提高precision，则需要分布尽可能特征明显，并且准确，两者并不是完全的矛盾，但要视手势类别而定，推荐设计harr特征明显的手势，尽可能边缘宽阔，镂空明显，如双手heart。
- 补充上面提到如果想要训练多合一的viola-jones模型，就与上面提到的样本分布准确，特征明显矛盾，所以在实际试验中，训练结果是非常差的，一般会导致stage过深，同时每个stage的weak classifier超过最大限制，这样一来也就失去了使用viola-jones的价值，而且实际检测precision非常之低，基本不可能实用。（而且笔者在后续的算法中实际上在逻辑和框架设计上优化了，使得多个串联viola-jones分类器的效率可以达到单个分类器的性能所以也就放弃了训练多合一的viola-jones，关于这一点希望能多多讨论）
- 在上述描述的训练方法中，对于某一种单一手势，一般在11~14stages中会达到最高的recall和相对较高的precision,具体视手势本身特征和正样本分布而定。就笔者的测试中heart这种harr特征明显的手势在recall与precision上都表现良好，而six这种手势则差强人意。
</br>

欢迎留言或者邮件交流694790961@qq.com


			
			
