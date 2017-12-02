import cv2
import numpy as np
from glob import glob
import matplotlib.pyplot as plt
import os
import sys

from cal import cal_iou

def load(pose):
    data_path = '../val/%s'%pose
       
    if os.path.exists('%s/%s.npy'%(data_path,pose)):
        data = np.load('%s/%s.npy'%(data_path,pose))
        print 'data len: ',len(data)
        return data
    else:
        test_img_list = glob('%s/*.jpg'%data_path)
        #anno_list = [f.replace('.jpg','_anno.txt') for f in test_img_list]
        data = []
        for f in test_img_list:
            img  = cv2.imread(f)
            lines = open(f.replace('.jpg','_anno.txt'),'rb').readlines()
            anno = []
            for line in lines:
                line = [int(code) for code in line.split(' ')]
                anno.append(line)
            data.append([img,anno])
        np.save('%s/%s.npy'%(data_path,pose),data)
        print 'data len: ',len(data)
        return  data

    

def varify(res,anno,thresh = 0.5):
    
    match = 0 
    true = len(anno)
    predict = len(res)
    try:
        for a in anno:
            a1 = [a[0],a[1],a[2],a[3]-a[1],a[4]-a[2]] 
            for b in res:
                b1 = [1,b[0],b[1],b[2],b[3]]
                iou = cal_iou(a1,b1)
                if iou> thresh:
                    anno.remove(a)
                #res.remove(b)
                    match+=1
                    print '%.04f'%iou,a1,b1
    except:
        pass
    return true,predict,match




if __name__=='__main__':
    #pose = 'thumb'
    pose = sys.argv[1]
    data = load(pose)
    cls = cv2.CascadeClassifier('../%s/xml/cascade_%s.xml'%(sys.argv[1],sys.argv[2]))

    true = 0
    pre = 0
    match = 0
    for i in range(len(data)): 
        print i,data[i][0].shape 
        gray = data[i][0]
        ans = cls.detectMultiScale(gray,1.1,4,minSize=(20,20),maxSize=(300,300))
        i,j,k = varify(ans,data[i][1])
        true+=i
        pre+=j
        match+=k
        print
    print 'total true: %d; prediction: %d; match :%d'%(true,pre,match)
    print 'precision: %.04f; recall: %.04f'%(match/float(pre),match/float(true))
         
